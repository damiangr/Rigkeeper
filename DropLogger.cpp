// Simple drop target logger to see what data formats are being offered
// Compile with: cl /EHsc DropLogger.cpp ole32.lib shell32.lib user32.lib
// Run and drag files to the window to see what's offered

#define UNICODE
#include <windows.h>
#include <ole2.h>
#include <shlobj.h>
#include <stdio.h>

class DropTargetLogger : public IDropTarget
{
private:
    LONG m_refCount;
    HWND m_hwnd;

public:
    DropTargetLogger(HWND hwnd) : m_refCount(1), m_hwnd(hwnd) {}

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv)
    {
        if (riid == IID_IUnknown || riid == IID_IDropTarget)
        {
            *ppv = this;
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() { return InterlockedIncrement(&m_refCount); }
    ULONG STDMETHODCALLTYPE Release()
    {
        LONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }

    // IDropTarget
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        wprintf(L"\n========== DragEnter ==========\n");
        wprintf(L"Key state: 0x%08X\n", grfKeyState);
        wprintf(L"Position: (%d, %d)\n", pt.x, pt.y);
        wprintf(L"Effect in: 0x%08X\n", *pdwEffect);

        // Enumerate available formats
        IEnumFORMATETC* pEnum = NULL;
        if (SUCCEEDED(pDataObj->EnumFormatEtc(DATADIR_GET, &pEnum)))
        {
            wprintf(L"\nAvailable formats:\n");
            FORMATETC fmt;
            while (pEnum->Next(1, &fmt, NULL) == S_OK)
            {
                wchar_t formatName[256] = L"";
                if (fmt.cfFormat < 0xC000)
                {
                    // Predefined format
                    switch (fmt.cfFormat)
                    {
                    case CF_TEXT: wcscpy_s(formatName, L"CF_TEXT"); break;
                    case CF_UNICODETEXT: wcscpy_s(formatName, L"CF_UNICODETEXT"); break;
                    case CF_HDROP: wcscpy_s(formatName, L"CF_HDROP"); break;
                    default:
                        swprintf_s(formatName, L"Predefined:%d", fmt.cfFormat);
                    }
                }
                else
                {
                    // Registered format
                    GetClipboardFormatNameW(fmt.cfFormat, formatName, 256);
                }

                wprintf(L"  - Format: %s (0x%04X)\n", formatName, fmt.cfFormat);
                wprintf(L"    Type: %d, Aspect: %d, Index: %d\n",
                    fmt.tymed, fmt.dwAspect, fmt.lindex);
            }
            pEnum->Release();
        }

        // Try to get HDROP data
        FORMATETC fmtHDROP = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium;
        if (SUCCEEDED(pDataObj->GetData(&fmtHDROP, &medium)))
        {
            HDROP hDrop = (HDROP)GlobalLock(medium.hGlobal);
            if (hDrop)
            {
                UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
                wprintf(L"\nHDROP files (%u):\n", fileCount);

                for (UINT i = 0; i < fileCount; i++)
                {
                    wchar_t filePath[MAX_PATH];
                    if (DragQueryFileW(hDrop, i, filePath, MAX_PATH))
                    {
                        wprintf(L"  %u: %s\n", i + 1, filePath);

                        // Get file attributes
                        DWORD attrs = GetFileAttributesW(filePath);
                        wprintf(L"     Attributes: 0x%08X", attrs);
                        if (attrs & FILE_ATTRIBUTE_ARCHIVE) wprintf(L" ARCHIVE");
                        if (attrs & FILE_ATTRIBUTE_TEMPORARY) wprintf(L" TEMPORARY");
                        if (attrs & FILE_ATTRIBUTE_HIDDEN) wprintf(L" HIDDEN");
                        wprintf(L"\n");
                    }
                }
                GlobalUnlock(medium.hGlobal);
            }
            ReleaseStgMedium(&medium);
        }

        *pdwEffect = DROPEFFECT_COPY;
        wprintf(L"Effect out: 0x%08X\n", *pdwEffect);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragLeave()
    {
        wprintf(L"========== DragLeave ==========\n");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        wprintf(L"========== Drop ==========\n");
        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        DrawTextW(hdc, L"Drag files here to see debug info in console", -1, &rect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);

    OleInitialize(NULL);

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DropLogger";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"DropLogger", L"Drop Target Logger",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL, NULL, hInstance, NULL);

    DropTargetLogger* dropTarget = new DropTargetLogger(hwnd);
    RegisterDragDrop(hwnd, dropTarget);

    wprintf(L"Drop Target Logger Ready\n");
    wprintf(L"Drag files to the window to see debug information\n\n");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    RevokeDragDrop(hwnd);
    dropTarget->Release();
    OleUninitialize();

    return 0;
}
