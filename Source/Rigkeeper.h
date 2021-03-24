/*
  ==============================================================================

    Rigkeeper.h
    Created: 20 Feb 2021 9:58:20am
    Author:  Damian Greda

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#define MENU_OFFSET 10


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
	}

	void resized() override
	{
		if (background_image != nullptr)
			background_image->setTransformToFit(getLocalBounds().toFloat(), RectanglePlacement::centred);
	}
	bool isInterestedInFileDrag(const StringArray& files) override
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
			setFromFile(rigFileFullPath);
			
		}
		else
		{
			this->setText("Too many files dragged. Only one file is allowed.", dontSendNotification);
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
					DBG("Left button down");
					File tempRigFile = File::getSpecialLocation(File::tempDirectory).getChildFile(rigFileName);
					if (tempRigFile.existsAsFile()) { tempRigFile.deleteFile(); }
					tempRigFileName = tempRigFile.getFullPathName();
					if (tempRigFile.create().ok())
					{
						MemoryBlock mb;
						mb.fromBase64Encoding(rigContentBase64);
						tempRigFile.replaceWithData(mb.getData(), mb.getSize());
					}
					
				StringArray files = StringArray(tempRigFile.getFullPathName());
				performExternalDragDropOfFiles(files, true);
					
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
		int x = 0;
		int y = 0;
		Image dragImage = createComponentSnapshot(getBounds());
		MouseEvent e2(inEvent.getEventRelativeTo(this));
		const Point<int> p(x - e2.x, y - e2.y);
		startDragging("Ownr", nullptr, dragImage, true, &p);
		performExternalDragDropOfFiles(tempRigFileName, true, nullptr, [this](void){});
		StringArray files = StringArray(tempRigFileName);
		performExternalDragDropOfFiles(files, true);
	}

	bool shouldDropFilesWhenDraggedExternally(const DragAndDropTarget::SourceDetails& sourceDetails,
	                                          juce::StringArray& files, bool& canMoveFiles) override
	{
		DBG(sourceDetails.description.toString());
		DBG("shouldDropFilesWhenDraggedExternally");
		files.add(File::createLegalPathName(
			File::getSpecialLocation(File::tempDirectory).getChildFile(rigFileName).getFullPathName()));
		canMoveFiles = true;
		return true;
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
		StringArray tmp;
		auto numfiles = SystemClipboard::getClipboardFileArray(tmp);			
		cpyPstMenu.addItem(1, "Copy", this->isNotEmpty(),false);
		
		if (numfiles >= 2)
		{
			for (int i = 0; i < numfiles; i++) {
				File fname = File(tmp[i]);
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
		const int result = cpyPstMenu.show();
		if (result == 0)
		{
			// user dismissed the menu without picking anything
			cpyPstMenu.dismissAllActiveMenus();
			this->getTopLevelComponent();
		}
		else if (result == 1)
		{

			DBG("Left button down");
			File tempRigFile = File::getSpecialLocation(File::tempDirectory).getChildFile(rigFileName);
			if (tempRigFile.existsAsFile()) { tempRigFile.deleteFile(); }
			tempRigFileName = tempRigFile.getFullPathName();
			if (tempRigFile.create().ok())
			{
				MemoryBlock mb;
				if (mb.fromBase64Encoding(rigContentBase64))
				{
					tempRigFile.replaceWithData(mb.getData(), mb.getSize());
					StringArray files = StringArray(tempRigFile.getFullPathName());
					SystemClipboard::copyFileToClipboard(tempRigFile.getFullPathName().toWideCharPointer());
				}
				else return;
			}
		}
		else if (result == 2)
		{
			StringArray fileNames;
            auto numfiles = SystemClipboard::getClipboardFileArray(fileNames);
			if (numfiles >= 0) 
			{
				DBG(String(numfiles));
				setFromFile(tmp[0]);
			}
		}
		else if (result == 3)
		{			
			ScopedPointer<AlertWindow> alert = new AlertWindow("About", "", AlertWindow::AlertIconType::NoIcon);			
			alert->addButton("Close", 1);
			alert->addButton("PayPal Coffee", 2);
			String infoText(String(ProjectInfo::projectName) + " v" + ProjectInfo::versionString + newLine +"author: " + ProjectInfo::companyName + newLine + "e-mail: toast.midi.editor@gmail.com");			
			alert->addTextBlock(infoText);		
			alert->addTextBlock("This is free software. You can also buy me a coffee for working on this plugin by clicking the PayPal Coffee button. ");			
			int i = alert->runModalLoop();
			if (i == 1)
			{
				userTriedToCloseWindow();	
			}
			else
			{
				URL("https://www.paypal.me/pools/c/8xDMCbwiOm").launchInDefaultBrowser();				
			}
			
		}
		
		else if (result >= MENU_OFFSET)
		{
			setFromFile(tmp[result - MENU_OFFSET]);
			DBG(tmp[result - MENU_OFFSET]);
		}
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
		}
	}

private:
	
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
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rigkeeper)
};
