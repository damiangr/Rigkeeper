/*
  ==============================================================================

    ClipboardOleInit.h
    Created: 2 Mar 2021 10:10:55am
    Author:  MusicCreator

  ==============================================================================
*/

#pragma once

#include <ole2.h>
#include <shlobj.h>
#include <atlbase.h>


class COleInitialize
{
public:
	COleInitialize() : m_hr(OleInitialize(NULL))
	{
	}

	~COleInitialize() { if (SUCCEEDED(m_hr)) OleUninitialize(); }
	operator HRESULT() const { return m_hr; }
	HRESULT m_hr;
};


inline HRESULT GetUIObjectOfFile(HWND hwnd, LPCWSTR pszPath, REFIID riid, void** ppv)
{
	*ppv = NULL;
	HRESULT hr;
	LPITEMIDLIST pidl;
	SFGAOF sfgao;
	if (SUCCEEDED(hr = SHParseDisplayName(pszPath, NULL, &pidl, 0, &sfgao)))
	{
		IShellFolder* psf;
		LPCITEMIDLIST pidlChild;
		if (SUCCEEDED(hr = SHBindToParent(pidl, IID_IShellFolder,
			(void**)&psf, &pidlChild)))
		{
			hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);
			psf->Release();
		}
		CoTaskMemFree(pidl);
	}
	return hr;
}

inline bool copyToClipboard(String file)
{
	COleInitialize init;
	CComPtr<IDataObject> spdto;

	if (SUCCEEDED(init) &&
		SUCCEEDED(GetUIObjectOfFile(nullptr, file.toWideCharPointer(), IID_PPV_ARGS(&spdto))) &&
		SUCCEEDED(OleSetClipboard(spdto)) &&
		SUCCEEDED(OleFlushClipboard()))
	{
		return true;
	}
	else return false;
}


//Return number of clipboard file names and adds file names to passed StringArray reference
inline UINT getClipboardFileArray(StringArray& fileArray)
{
	UINT fileCount = 0;
	TCHAR lpszFileName[MAX_PATH];
	if (IsClipboardFormatAvailable(CF_HDROP) && OpenClipboard(NULL))
	{
		HGLOBAL h = GetClipboardData(CF_HDROP);
		HDROP ptr = (HDROP)GlobalLock(h);
		if (h != NULL && ptr != NULL)
		{
			fileCount = DragQueryFile(ptr, 0xFFFFFFFF, nullptr, 0);
			for (UINT i = 0; i < fileCount; ++i)
			{
				UINT filenameLength = DragQueryFile(ptr, i, nullptr, 0);
				DragQueryFile(ptr, i, lpszFileName, filenameLength+1);
				fileArray.add(lpszFileName);
			}
			GlobalLock(h);
		}
		CloseClipboard();
	}
	return fileCount;
}
