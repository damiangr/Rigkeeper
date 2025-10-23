/*
  ==============================================================================

    Rigkeeper.h
    Created: 20 Feb 2021 9:58:20am
    Author:  Damian Greda

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#ifdef JUCE_WINDOWS
    #include "ClipboardUtils_win32.h"
#endif
#define MENU_OFFSET 10
#define FILE_SIZE_LIMIT 1048576  //1MB size limit

//==============================================================================
/*
*/
class Rigkeeper : public Label,
                     public DragAndDropContainer,
                     public FileDragAndDropTarget,
                     public PopupMenu,
                     public SystemClipboard


{
public:
	Rigkeeper()
	{
		svg = (XmlDocument::parse(BinaryData::background2_svg));
		background_image = Drawable::createFromSVG(*svg);
		addAndMakeVisible(*background_image);
		background_image->setTransformToFit(getLocalBounds().toFloat(),RectanglePlacement::centred);
		background_image->setAlpha(0.2f);
		if (this->isNotEmpty())
		{
		this->setText(String(File::createLegalFileName(rigFileName) + "\n"), dontSendNotification);
		}
		else this->setText(message, dontSendNotification);

	}

	~Rigkeeper() override
	{
		// Clean up temp file when component is destroyed
		cleanupTempFile();
	}

	// Helper function to log to file
	void logToFile(const String& message)
	{
		File logFile = File::getSpecialLocation(File::userDocumentsDirectory)
			.getChildFile("Rigkeeper")
			.getChildFile("rigkeeper_debug.log");
		
		if (!logFile.getParentDirectory().exists())
			logFile.getParentDirectory().createDirectory();
		
		FileOutputStream stream(logFile, 1024 * 1024); // 1MB buffer
		if (stream.openedOk())
		{
			Time now = Time::getCurrentTime();
			String timeStr = now.formatted("%Y-%m-%d %H:%M:%S");
			stream.writeText("[" + timeStr + "] " + message + "\n", false, false, nullptr);
			stream.flush();
		}
		
		// Also use DBG for debugger output
		DBG(message);
	}

	void resized() override
	{
		if (background_image != nullptr)
			background_image->setTransformToFit(getLocalBounds().toFloat(), RectanglePlacement::centred);
	}
	bool isInterestedInFileDrag(const StringArray& /*files*/) override
	{
		return true;
	}

	void fileDragEnter(const StringArray& /*files*/, int /*x*/, int /*y*/) override
	{
		somethingIsBeingDraggedOver = true;
		repaint();
	}

	void fileDragMove(const StringArray& /*files*/, int /*x*/, int /*y*/) override
	{
	}

	void fileDragExit(const StringArray& /*files*/) override
	{
		somethingIsBeingDraggedOver = false;
		this->repaint();
	}

	void filesDropped(const StringArray& files, int /*x*/, int /*y*/) override
	{
		if (files.size() == 1)
		{
			String rigFileFullPath = files[0];
			if (checkAllowedFileSize(rigFileFullPath, fileSizeLimit))
			{
				setFromFile(rigFileFullPath);
			}
			else
			{
				juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Error", ("File does not exist or exceeds " + String(fileSizeLimit) + " bytes limit !"));
			}
			
			
		}
		else
		{
			juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Error", "Too many files dragged. Only one file is allowed!");
		}
		somethingIsBeingDraggedOver = false;
		this->repaint();
	}

	void mouseDown(const MouseEvent& inEvent) override
	{
		if (inEvent.mods.isLeftButtonDown())
		{
			if (!isPopupActive)
			{
				if (!rigContentBase64.isEmpty())
				{
					DBG("Left button down - preparing drag data");
					// Create temp file for external drag operation
					String tempFileName = rigFileName;
					
					// Ensure proper file extension if missing
					if (!tempFileName.contains("."))
					{
						tempFileName += ".kipr"; // Default Kemper rig extension
					}
					
					// Use Documents folder for better app compatibility
					File documentsDir = File::getSpecialLocation(File::userDocumentsDirectory);
					File rigkeeperDir = documentsDir.getChildFile("Rigkeeper");
					if (!rigkeeperDir.exists())
						rigkeeperDir.createDirectory();
					
					File tempRigFile = rigkeeperDir.getChildFile(tempFileName);
					if (tempRigFile.existsAsFile()) { tempRigFile.deleteFile(); }
					tempRigFileName = tempRigFile.getFullPathName();
					if (tempRigFile.create().ok())
					{
						MemoryBlock mb;
						mb.fromBase64Encoding(rigContentBase64);
						tempRigFile.replaceWithData(mb.getData(), mb.getSize());
						
						// Ensure file is fully written and has proper attributes
						tempRigFile.setLastModificationTime(Time::getCurrentTime());
						tempRigFile.setReadOnly(false, false);
						
						// Try to set Windows file attributes to make it look more "real"
#ifdef JUCE_WINDOWS
						String winPath = tempRigFile.getFullPathName();
						DWORD attrs = GetFileAttributesW(winPath.toWideCharPointer());
						if (attrs != INVALID_FILE_ATTRIBUTES)
						{
							// Remove any temporary attributes
							DWORD newAttrs = attrs & ~FILE_ATTRIBUTE_TEMPORARY;
							SetFileAttributesW(winPath.toWideCharPointer(), newAttrs);
							logToFile("File created: " + tempRigFileName);
							logToFile("File size: " + String(tempRigFile.getSize()));
							logToFile("Attributes before: 0x" + String::toHexString((int)attrs));
							logToFile("Attributes after: 0x" + String::toHexString((int)newAttrs));
						}
#endif
						
						logToFile("Temp file created successfully: " + tempRigFileName);
						
						// Small delay to ensure file is fully written and attributes set
						Thread::sleep(50);
					}
					else
					{
						DBG("Failed to create temp file");
					}
				}
			}
			else isPopupActive = false;
		}
		else if (inEvent.mods.isPopupMenu())
		{
			DBG("Right button Down- popup menu");
			showPopUpMenu();
		}
		else { DBG("No mouse button clicked"); };
	}

	void mouseDrag(const MouseEvent& inEvent) override
	{
		// Only start drag if we have content and temp file exists
		if (!rigContentBase64.isEmpty() && !tempRigFileName.isEmpty() && File(tempRigFileName).existsAsFile())
		{
		File dragFile(tempRigFileName);
		
		logToFile("========== DRAG OPERATION START ==========");
		logToFile("Temp file: " + tempRigFileName);
		
#if JUCE_WINDOWS
		// Windows: Use Shell IDataObject for drag-and-drop (same approach as clipboard)
		// This makes Windows think the file comes from Explorer!
		COleInitialize init;
		
		if (SUCCEEDED(init.m_hr))
		{
			// Get IDataObject from Shell (same as clipboard copy)
			IDataObject* pdto = nullptr;
			
			logToFile("Getting IDataObject from Shell for file...");
			HRESULT hr = GetUIObjectOfFile(nullptr, dragFile.getFullPathName().toWideCharPointer(), 
			                                IID_PPV_ARGS(&pdto));
			
			if (SUCCEEDED(hr) && pdto != nullptr)
			{
				logToFile("IDataObject obtained, starting SHDoDragDrop...");
				
				// Use SHDoDragDrop which handles IDropSource internally
				DWORD dwEffect = 0;
				hr = SHDoDragDrop(nullptr, pdto, nullptr, DROPEFFECT_COPY | DROPEFFECT_LINK, &dwEffect);
				
				logToFile("SHDoDragDrop returned: 0x" + String::toHexString((int)hr));
				logToFile("Drop effect: " + String(dwEffect));
				
				pdto->Release();
			}
			else
			{
				logToFile("Failed to get IDataObject, HR: 0x" + String::toHexString((int)hr));
			}
		}
		else
		{
			logToFile("OLE initialization failed");
		}
#elif JUCE_MAC
		// macOS: Use NSPasteboard with file promise provider for Finder-like drag
		// This will be implemented in ClipboardUtils_mac.mm as native Objective-C++
		logToFile("macOS: Using Finder-style drag with NSPasteboard");
		
		// For now, fall back to JUCE's drag which should work on macOS
		// (The issue was Windows-specific with Shell IDataObject requirement)
		StringArray files;
		files.add(tempRigFileName);
		performExternalDragDropOfFiles(files, false);
		logToFile("macOS drag operation completed");
#else
		// Other platforms: use JUCE's standard drag
		StringArray files;
		files.add(tempRigFileName);
		performExternalDragDropOfFiles(files, false);
#endif
		
		logToFile("========== DRAG OPERATION END ==========");
		}
		else
		{
			logToFile("Drag NOT initiated - missing data or file");
		}
	}
	
	void mouseDoubleClick(const MouseEvent& /*inEvent*/) override
	{
		// On double-click, open the folder containing the file
		// This allows users to drag from the folder if direct drag doesn't work
		if (!tempRigFileName.isEmpty() && File(tempRigFileName).existsAsFile())
		{
			File rigFile(tempRigFileName);
			rigFile.revealToUser();
		}
	}

	bool shouldDropFilesWhenDraggedExternally(const DragAndDropTarget::SourceDetails& sourceDetails,
	                                          juce::StringArray& files, bool& canMoveFiles) override
	{
		DBG("shouldDropFilesWhenDraggedExternally called");
		DBG("Source description: " + sourceDetails.description.toString());
		
		// Ensure temp file exists before offering it for external drop
		if (!tempRigFileName.isEmpty() && File(tempRigFileName).existsAsFile())
		{
			files.add(tempRigFileName);
			canMoveFiles = false; // Don't move, just copy
			DBG("Offering file for external drop: " + tempRigFileName);
			return true;
		}
		
		DBG("No valid file to offer for external drop");
		return false;
	}

	String getRigContent()
	{
		return rigContentBase64;
	}

	String getRigFileName()
	{
		return rigFileName;
	}

	void setRigContent(String content)
	{
		rigContentBase64 = content;
	}

	void setRigFileName(String fileName)
	{
		rigFileName = fileName;
	}

	void displayInitialText()
	{
		this->setText(message, dontSendNotification);
		this->repaint();
	}

	void showPopUpMenu()
	{
		PopupMenu cpyPstMenu;
		PopupMenu pstSubMenu;
		isPopupActive = true;
		
		// Get clipboard files and store in member variable
		clipboardFiles.clear();
#ifdef JUCE_WINDOWS
		auto numfiles = getClipboardFileArray(clipboardFiles);
#else
		auto numfiles = 0;
#endif			
		cpyPstMenu.addItem(1, "Copy", this->isNotEmpty(),false);
		cpyPstMenu.addItem(4, "Open Folder", this->isNotEmpty(), false);
		
		if (numfiles >= 2)
		{
			for (int i = 0; i < numfiles; i++) {
				File fname = File(clipboardFiles[i]);
				pstSubMenu.addItem(i + MENU_OFFSET, fname.getFileName());
			}
			cpyPstMenu.addSubMenu("Paste", pstSubMenu);
		}
		else 
		{
			cpyPstMenu.addItem(2, "Paste", numfiles, false);
		}
		cpyPstMenu.addSeparator();
		cpyPstMenu.addItem(3, "About", true, false);
		cpyPstMenu.showMenuAsync(PopupMenu::Options().withTargetComponent(this), [this](int result) {
		if (result == 0)
		{
			// user dismissed the menu without picking anything
		}
		else if (result == 1)
		{
			DBG("Copy operation started");
			
			if (!rigContentBase64.isEmpty() && !rigFileName.isEmpty())
			{
				// Ensure filename has .kipr extension
				String tempFileName = rigFileName;
				if (!tempFileName.endsWithIgnoreCase(".kipr"))
				{
					tempFileName += ".kipr";
					DBG("Added .kipr extension to temp file: " + tempFileName);
				}
				
				// Try Documents folder instead of temp directory for better app compatibility
				File documentsDir = File::getSpecialLocation(File::userDocumentsDirectory);
				File rigkeeperDir = documentsDir.getChildFile("Rigkeeper");
				if (!rigkeeperDir.exists())
					rigkeeperDir.createDirectory();
				
				File tempRigFile = rigkeeperDir.getChildFile(tempFileName);
				if (tempRigFile.existsAsFile()) { tempRigFile.deleteFile(); }
				tempRigFileName = tempRigFile.getFullPathName();
				
				if (tempRigFile.create().ok())
				{
					MemoryBlock mb;
					if (mb.fromBase64Encoding(rigContentBase64))
					{
						tempRigFile.replaceWithData(mb.getData(), mb.getSize());
						
						// Ensure file is fully written and has proper attributes
						tempRigFile.setLastModificationTime(Time::getCurrentTime());
						tempRigFile.setReadOnly(false, false);
						
						// Try to set Windows file attributes to make it look more "real"
#ifdef JUCE_WINDOWS
						String winPath = tempRigFile.getFullPathName();
						DWORD attrs = GetFileAttributesW(winPath.toWideCharPointer());
						if (attrs != INVALID_FILE_ATTRIBUTES)
						{
							// Remove any temporary attributes
							SetFileAttributesW(winPath.toWideCharPointer(), 
											 attrs & ~FILE_ATTRIBUTE_TEMPORARY);
						}
#endif
						
						// Small delay to ensure file is fully written and attributes set
						Thread::sleep(50);
						
#ifdef JUCE_WINDOWS
						if (copyToClipboard(tempRigFile.getFullPathName()))
						{
							DBG("File copied to clipboard: " + tempRigFile.getFullPathName());
						}
						else
						{
							DBG("Failed to copy file to clipboard");
						}
#endif
					}
					else
					{
						DBG("Failed to decode base64 content");
					}
				}
				else
				{
					DBG("Failed to create temp file");
				}
			}
			else
			{
				DBG("No content to copy");
			}
		}
		else if (result == 2)
		{
			StringArray fileNames;
#ifdef JUCE_WINDOWS
            auto numfiles2 = getClipboardFileArray(fileNames);
#else
            auto numfiles2 = 0;
#endif
			if (numfiles2 > 0 && fileNames.size() > 0) 
			{ 
				DBG("Files found in clipboard: " + String(numfiles2));
				if (checkAllowedFileSize(fileNames[0], fileSizeLimit))
				{
					setFromFile(fileNames[0]);
				}
				else
				{
					juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Error", ("File does not exist or exceeds " + String(fileSizeLimit) + " bytes limit !"));
				}
			}
			else
			{
				juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Info", "No files found in clipboard!");
			}
		}
		else if (result == 3)
		{			
			String infoText(String(ProjectInfo::projectName) + " v" + ProjectInfo::versionString + newLine +"author: " + ProjectInfo::companyName + newLine + "e-mail: toast.midi.editor@gmail.com");			
			infoText += newLine + newLine + "This is free software. You can also buy me a coffee for working on this plugin by visiting the PayPal link.";
			juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "About", infoText);
		}
		else if (result == 4)
		{
			// Open folder - workaround for drag-drop issues
			DBG("Open folder operation");
			if (!tempRigFileName.isEmpty() && File(tempRigFileName).existsAsFile())
			{
				File rigFile(tempRigFileName);
				rigFile.revealToUser();
			}
			else
			{
				juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Info", "No file available. Load a rig first.");
			}
		}
		
		else if (result >= MENU_OFFSET)
		{
			int fileIndex = result - MENU_OFFSET;
			if (fileIndex >= 0 && fileIndex < clipboardFiles.size())
			{
				String selectedFile = clipboardFiles[fileIndex];
				if (checkAllowedFileSize(selectedFile, fileSizeLimit))
				{
					setFromFile(selectedFile);
				}
				else
				{
					juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Error", ("File does not exist or exceeds " + String(fileSizeLimit) + " bytes limit !"));
				}
				DBG("Selected file: " + selectedFile);
			}
		}
		}); // End of lambda callback
	}

	bool isNotEmpty()
	{
		return  rigFileName.isNotEmpty() && rigContentBase64.isNotEmpty();
	}

	void setFromFile(String file)
	{
		File rigFile(file);
		
		if (rigFile.existsAsFile())
		{
			rigFileName = rigFile.getFileName();
			MemoryBlock rigMemoryBlock;
			rigFile.loadFileAsData(rigMemoryBlock);
			rigContentBase64 = rigMemoryBlock.toBase64Encoding();
			Rigkeeper::setText(String(File::createLegalFileName(rigFileName) + "\n"), sendNotification);
			
			// Clear old temp file path since we have new content
			cleanupTempFile();
			
			DBG("File loaded: " + rigFileName);
		}
	}
	
	void cleanupTempFile()
	{
		if (!tempRigFileName.isEmpty())
		{
			File tempFile(tempRigFileName);
			if (tempFile.existsAsFile())
			{
				tempFile.deleteFile();
				DBG("Cleaned up temp file: " + tempRigFileName);
			}
			tempRigFileName = "";
		}
	}
	bool checkAllowedFileSize(String file, uint32_t filesize)
	{
		File rigFile(file);

		if (rigFile.existsAsFile())
		{
			auto size = rigFile.getSize();
			return (size <= filesize);
		}
		else return false;
	}

private:
	uint32_t fileSizeLimit = FILE_SIZE_LIMIT;
	bool isPopupActive = false;
	std::unique_ptr<XmlElement> svg;
	std::unique_ptr<Drawable> background_image;
	String tempRigFileName;
	DrawableRectangle rect;
	bool somethingIsBeingDraggedOver = false;
	String message{
		"Right click copy/paste Rig file onto/from this component!\n\n"
		"You can also drag-and-drop Rig from RigManager or folder"
	};
	String rigFileName{""};
	String rigContentBase64{""};
	StringArray clipboardFiles; // Store clipboard files for menu operations
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rigkeeper)
};
