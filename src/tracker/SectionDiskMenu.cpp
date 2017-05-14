/*
 *  tracker/SectionDiskMenu.cpp
 *
 *  Copyright 2009 Peter Barth
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *  SectionDiskMenu.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.07.05.
 *
 */

#include "SectionDiskMenu.h"
#include "Tracker.h"
#include "TrackerConfig.h"
#include "TrackerSettingsDatabase.h"
#include "ModuleEditor.h"
#include "GlobalColorConfig.h"

#include "Container.h"
#include "MessageBoxContainer.h"
#include "RadioGroup.h"
#include "Seperator.h"
#include "Slider.h"
#include "StaticText.h"
#include "ListBox.h"
#include "ListBoxFileBrowser.h"
#include "CheckBox.h"
#include "PPUIConfig.h"
#include "PatternEditorControl.h"

#include "SectionSwitcher.h"
#include "SectionHDRecorder.h"

#include "DialogBase.h"

#include "PatternTools.h"
#include "Tools.h"
#include "PPPath.h"
#include "XMFile.h"
#include "PPSavePanel.h"

#include "FileExtProvider.h"

#include "ControlIDs.h"

enum ControlIDs
{
	DISKMENU_BUTTON_FLIP = 7000,
	DISKMENU_BUTTON_EXIT,

	DISKMENU_NORMAL_STATICTEXT_HEADING,
	DISKMENU_NORMAL_BUTTON_LOAD_SONG,
	DISKMENU_NORMAL_BUTTON_SAVE_SONG,
	DISKMENU_NORMAL_BUTTON_LOAD_PATTERN,
	DISKMENU_NORMAL_BUTTON_SAVE_PATTERN,
	DISKMENU_NORMAL_BUTTON_LOAD_TRACK,
	DISKMENU_NORMAL_BUTTON_SAVE_TRACK,
	DISKMENU_NORMAL_BUTTON_LOAD_INSTRUMENT,
	DISKMENU_NORMAL_BUTTON_SAVE_INSTRUMENT,
	DISKMENU_NORMAL_BUTTON_LOAD_SAMPLE,
	DISKMENU_NORMAL_BUTTON_SAVE_SAMPLE,

	DISKMENU_NORMAL_RADIOGROUP_SONGTYPE,
	DISKMENU_NORMAL_RADIOGROUP_PATTERNTYPE,
	DISKMENU_NORMAL_RADIOGROUP_TRACKTYPE,
	DISKMENU_NORMAL_RADIOGROUP_INSTRUMENTTYPE,
	DISKMENU_NORMAL_RADIOGROUP_SAMPLETYPE,

	DISKMENU_CLASSIC_STATICTEXT_HEADING,
	DISKMENU_CLASSIC_BUTTON_TYPE_TYPE,
	DISKMENU_CLASSIC_BUTTON_TYPE_MODULE,
	DISKMENU_CLASSIC_BUTTON_TYPE_INSTRUMENT,
	DISKMENU_CLASSIC_BUTTON_TYPE_SAMPLE,
	DISKMENU_CLASSIC_BUTTON_TYPE_PATTERN,
	DISKMENU_CLASSIC_BUTTON_TYPE_TRACK,
	DISKMENU_CLASSIC_BUTTON_SAVE,
	DISKMENU_CLASSIC_LISTBOX_NAME,
	DISKMENU_CLASSIC_LISTBOX_BROWSER,
	
	DISKMENU_CLASSIC_BUTTON_PREV,
	DISKMENU_CLASSIC_BUTTON_NEXT,
	DISKMENU_CLASSIC_BUTTON_PARENT,
	DISKMENU_CLASSIC_BUTTON_ROOT,
	DISKMENU_CLASSIC_BUTTON_HOME,
	DISKMENU_CLASSIC_BUTTON_REFRESH,
	DISKMENU_CLASSIC_BUTTON_LOAD,
	DISKMENU_CLASSIC_BUTTON_DELETE,
	DISKMENU_CLASSIC_BUTTON_MAKEDIR,

	DISKMENU_CLASSIC_STATICTEXT_FILTEREXTENSIONS,
	DISKMENU_CLASSIC_CHECKBOX_FILTEREXTENSIONS,
	
	DISKMENU_CLASSIC_STATICTEXT_SORTBY,
	DISKMENU_CLASSIC_BUTTON_SORTBY,
	DISKMENU_CLASSIC_BUTTON_SORTORDER,

	DISKMENU_CLASSIC_BUTTON_VEXTEND,
	DISKMENU_CLASSIC_BUTTON_HEXTEND,

	DISKMENU_CLASSIC_BUTTON_DIR0,
	DISKMENU_CLASSIC_BUTTON_DIR1,
	DISKMENU_CLASSIC_BUTTON_DIR2,
	DISKMENU_CLASSIC_BUTTON_DIR3,
	DISKMENU_CLASSIC_BUTTON_DIR4,
	DISKMENU_CLASSIC_BUTTON_STOREDIR, // this needs to be DISKMENU_CLASSIC_BUTTON_DIR5+1
	
	DISKMENU_CLASSIC_BUTTON_TYPE,
	
	RESPONDMESSAGEBOX_OVERWRITE,
	RESPONDMESSAGEBOX_DELETE
};

#define RADIOGROUPTOINDEX(ID) ((ID) - DISKMENU_NORMAL_RADIOGROUP_SONGTYPE)
#define INDEXTORADIOGROUP(INDEX) ((INDEX) + DISKMENU_NORMAL_RADIOGROUP_SONGTYPE)

// Class which responds to the above message box clicks
class DialogResponderDisk : public DialogResponder
{
private:
	SectionDiskMenu& section;
	
public:
	DialogResponderDisk(SectionDiskMenu& section) :
		section(section)
	{
	}
	
	virtual pp_int32 ActionOkay(PPObject* sender)
	{
		if (reinterpret_cast<PPDialogBase*>(sender)->getID() == RESPONDMESSAGEBOX_OVERWRITE)
		{
			section.saveCurrent();
			return 1;
		}
		else if (reinterpret_cast<PPDialogBase*>(sender)->getID() == RESPONDMESSAGEBOX_DELETE)
		{
			section.deleteCurrent();
		}
		return 0;
	}
	
	virtual pp_int32 ActionCancel(PPObject* sender)
	{
		return 0;
	}
};

class PPDummySavePanel : public PPSavePanel
{
public:
	PPDummySavePanel(const PPSystemString& defaultFileName) : 
		PPSavePanel(NULL, "", defaultFileName)
	{
		this->defaultFileName = defaultFileName;
	}

	virtual ReturnCodes runModal()
	{
		return ReturnCodeOK;
	}
	
	virtual const PPSystemString& getFileName() 
	{ 
		return defaultFileName; 
	}	
};

class ColorQueryListener : public PPListBox::ColorQueryListener
{
private:
	SectionDiskMenu& sectionDiskMenu;
	
public:
	ColorQueryListener(SectionDiskMenu& theSectionDiskMenu) :
		sectionDiskMenu(theSectionDiskMenu)
	{
	}
	
	virtual PPColor getColor(pp_uint32 index, PPListBox& sender) 
	{
		PPListBoxFileBrowser& listBoxFiles = static_cast<PPListBoxFileBrowser&>(sender);
	
		const PPColor& fileColor = GlobalColorConfig::getInstance()->getColor(GlobalColorConfig::ColorTextHighlited);
		const PPColor& dirColor = GlobalColorConfig::getInstance()->getColor(GlobalColorConfig::ColorForegroundText);
	
		return listBoxFiles.getPathEntry(index)->isFile() ? fileColor : dirColor; 
	}
};

SectionDiskMenu::SectionDiskMenu(Tracker& theTracker) :
	SectionUpperLeft(theTracker, NULL, new DialogResponderDisk(*this)),
	diskMenuVisible(false),
	classicViewState(BrowseModules),
	forceClassicBrowser(false),
	moduleTypeAdjust(true),
	sortAscending(true),
	storePath(false),
	listBoxFiles(NULL),
	editFieldCurrentFile(NULL),
	currentActiveRadioGroup(NULL)
{
	normalViewControls = new PPSimpleVector<PPControl>(0, false);
	classicViewControls = new PPSimpleVector<PPControl>(0, false);

	fileFullPath = new PPSystemString();
	file = new PPSystemString();

	lastFocusedControl = NULL;

#ifdef __LOWRES__
	lastSIPOffsetMove = 0;
#endif

	colorQueryListener = new ColorQueryListener(*this);
}

SectionDiskMenu::~SectionDiskMenu()
{
	delete colorQueryListener;
	
	delete fileFullPath;
	delete file;
	
	delete normalViewControls;
	delete classicViewControls;
}

pp_int32 SectionDiskMenu::getCurrentSelectedSampleSaveType()
{
	if (sectionContainer == NULL)
		return -1;
		
	switch (static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SAMPLETYPE))->getChoice())
	{
		case 0:
			return FileTypes::FileTypeSampleWAV;
		case 1:
			return FileTypes::FileTypeSampleIFF;
	}
	
	return -1;
}

pp_int32 SectionDiskMenu::handleEvent(PPObject* sender, PPEvent* event)
{
	PPScreen* screen = tracker.screen;

	if (event->getID() == eCommand)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case DISKMENU_NORMAL_BUTTON_LOAD_SONG:
			{
				if (screen->getModalControl())
					break;
				
				tracker.loadTypeWithDialog(FileTypes::FileTypeSongAllModules);
				break;
			}

		
			case DISKMENU_NORMAL_BUTTON_SAVE_SONG:
			{
				switch (static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SONGTYPE))->getChoice())
				{
					case 0:
						tracker.saveTypeWithDialog(FileTypes::FileTypeSongXM);
						break;
						
					case 1:
						tracker.saveTypeWithDialog(FileTypes::FileTypeSongMOD);
						break;
						
					case 2:
						tracker.sectionSwitcher->showUpperSection(tracker.sectionHDRecorder);
						break;
				}

				break;
			}

			case DISKMENU_NORMAL_BUTTON_LOAD_PATTERN:
			{
				tracker.loadTypeWithDialog(FileTypes::FileTypePatternXP);
				break;
			}

			case DISKMENU_NORMAL_BUTTON_SAVE_PATTERN:
			{
				tracker.saveTypeWithDialog(FileTypes::FileTypePatternXP);
				break;
			}

			case DISKMENU_NORMAL_BUTTON_LOAD_TRACK:
			{
				tracker.loadTypeWithDialog(FileTypes::FileTypeTrackXT);
				break;
			}

			case DISKMENU_NORMAL_BUTTON_SAVE_TRACK:
			{
				tracker.saveTypeWithDialog(FileTypes::FileTypeTrackXT);
				break;
			}

			case DISKMENU_NORMAL_BUTTON_LOAD_INSTRUMENT:
			{
				tracker.loadTypeWithDialog(FileTypes::FileTypeSongAllInstruments);
				break;
			}

			case DISKMENU_NORMAL_BUTTON_SAVE_INSTRUMENT:
			{
				tracker.saveTypeWithDialog(FileTypes::FileTypeInstrumentXI);
				break;
			}

			case DISKMENU_NORMAL_BUTTON_LOAD_SAMPLE:
			{
				tracker.loadTypeWithDialog(FileTypes::FileTypeSongAllSamples);
				break;
			}

			case DISKMENU_NORMAL_BUTTON_SAVE_SAMPLE:
			{
				tracker.saveTypeWithDialog(getCurrentSelectedSampleSaveType());	
				break;
			}

			case DISKMENU_BUTTON_FLIP:
			{
				flip();
				break;
			}
			
			case DISKMENU_CLASSIC_BUTTON_TYPE_TYPE:
			{
				switchState(BrowseAll);
				break;
			}

			case DISKMENU_CLASSIC_BUTTON_TYPE_MODULE:
			{
				switchState(BrowseModules);
				break;
			}

			case DISKMENU_CLASSIC_BUTTON_TYPE_INSTRUMENT:
			{
				switchState(BrowseInstruments);
				break;
			}

			case DISKMENU_CLASSIC_BUTTON_TYPE_SAMPLE:
			{
				switchState(BrowseSamples);
				break;
			}

			case DISKMENU_CLASSIC_BUTTON_TYPE_PATTERN:
			{
				switchState(BrowsePatterns);
				break;
			}

			case DISKMENU_CLASSIC_BUTTON_TYPE_TRACK:
			{
				switchState(BrowseTracks);
				break;
			}

			case DISKMENU_CLASSIC_BUTTON_SAVE:
			{
				prepareSave();
				break;
			}

			case DISKMENU_CLASSIC_BUTTON_PREV:
				prev();
				break;
				
			case DISKMENU_CLASSIC_BUTTON_NEXT:
				next();
				break;

			case DISKMENU_CLASSIC_BUTTON_PARENT:
				parent();
				break;

			case DISKMENU_CLASSIC_BUTTON_ROOT:
				root();
				break;

			case DISKMENU_CLASSIC_BUTTON_HOME:
				home();
				break;

			case DISKMENU_CLASSIC_BUTTON_REFRESH:
				reload();
				break;

			case DISKMENU_CLASSIC_BUTTON_DELETE:
				showDeleteMessageBox();
				break;

			case DISKMENU_CLASSIC_BUTTON_LOAD:
				handleLoadOrStep();
				break;

			case DISKMENU_CLASSIC_BUTTON_SORTBY:
				listBoxFiles->cycleSorting();
				reload();
				updateButtonStates();
				break;
				
			case DISKMENU_CLASSIC_BUTTON_SORTORDER:
				sortAscending = !sortAscending;
				listBoxFiles->setSortAscending(sortAscending);
				reload();
				updateButtonStates();
				break;
				
			case DISKMENU_CLASSIC_CHECKBOX_FILTEREXTENSIONS:
				updateFilter();
				break;

			case DISKMENU_CLASSIC_BUTTON_VEXTEND:
				resizeBrowserVertically();
				break;

			case DISKMENU_CLASSIC_BUTTON_HEXTEND:
				resizeBrowserHorizontally();
				break;
				
			case DISKMENU_CLASSIC_BUTTON_STOREDIR:
			{
				storePath = !storePath;
				PPButton* button = reinterpret_cast<PPButton*>(sender);
				button->setPressed(storePath);
				screen->paintControl(button);
				break;
			}
			
			case DISKMENU_CLASSIC_BUTTON_DIR0:
			case DISKMENU_CLASSIC_BUTTON_DIR1:
			case DISKMENU_CLASSIC_BUTTON_DIR2:
			case DISKMENU_CLASSIC_BUTTON_DIR3:
			case DISKMENU_CLASSIC_BUTTON_DIR4:
			{
				PPButton* button = reinterpret_cast<PPButton*>(sender);
				
				PPString strKey = getKeyFromPredefPathButton(button);
				
				if (storePath)
				{
					PPString path = listBoxFiles->getCurrentPathAsASCIIString();
					tracker.settingsDatabase->store(strKey, path);
					
					storePath = !storePath;
					PPButton* button = static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_STOREDIR));
					button->setPressed(storePath);
					screen->paintControl(button);				
				}
				else
				{
					PPDictionaryKey* key = tracker.settingsDatabase->restore(strKey);
					if (key)
					{
						PPSystemString str(key->getStringValue());
						bool res = listBoxFiles->gotoPath(str);
						
						if (!res)
							tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Couldn't change directory.", Tracker::MessageBox_OK);		
						
						screen->paintControl(listBoxFiles);
					}
					else
						tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "No directory defined.", Tracker::MessageBox_OK);							
				}
				
				break;
			}
			
			case DISKMENU_BUTTON_EXIT:
				show(false);
				break;

		}
		
	}
	else if (reinterpret_cast<PPControl*>(sender) == listBoxFiles && event->getID() == eConfirmed)
	{
		handleLoadOrStep();
	}
	else if (event->getID() == eSelection)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{	
			case DISKMENU_CLASSIC_LISTBOX_BROWSER:
			{
				updateFilenameEditFieldFromBrowser();
				break;
			}
			
			case DISKMENU_NORMAL_RADIOGROUP_SONGTYPE:
			case DISKMENU_NORMAL_RADIOGROUP_PATTERNTYPE:
			case DISKMENU_NORMAL_RADIOGROUP_TRACKTYPE:
			case DISKMENU_NORMAL_RADIOGROUP_INSTRUMENTTYPE:
			case DISKMENU_NORMAL_RADIOGROUP_SAMPLETYPE:
				currentActiveRadioGroup = reinterpret_cast<PPRadioGroup*>(sender);
				updateFilenameEditFieldExtension(classicViewState);
				break;
		}
		
	}
	else if (event->getID() == eFileSystemChanged)
	{
		reload();
	}
	else if (event->getID() == eValueChanged)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case DISKMENU_CLASSIC_LISTBOX_NAME:
			{
				const PPString* str = *(reinterpret_cast<const PPString* const*>(event->getDataPtr()));
				*fileFullPath = listBoxFiles->getCurrentPathAsString();
				PPSystemString temp(*str);
				*file = temp;
				fileFullPath->append(*file);	
				assureExtension();		
				break;
			}
		}
	}

	return 0;
}

void SectionDiskMenu::init(pp_int32 px, pp_int32 py)
{
	pp_int32 i;

	PPScreen* screen = tracker.screen;

	PPContainer* container = new PPContainer(CONTAINER_ADVEDIT, tracker.screen, this, PPPoint(px, py), PPSize(320,UPPERLEFTSECTIONHEIGHT), false);
	container->setColor(TrackerConfig::colorThemeMain);	

	container->addControl(new PPStaticText(DISKMENU_NORMAL_STATICTEXT_HEADING, NULL, NULL, PPPoint(px + 2, py + 2), "Disk operations", true, true));

	pp_int32 buttonWidth = 8*4+4;
	pp_int32 buttonHeight = 11;
	
	pp_int32 x = px+container->getSize().width-(buttonWidth+4);
	pp_int32 y = py+container->getSize().height-(buttonHeight+4);

	container->addControl(new PPSeperator(0, screen, PPPoint(px + 2, y - 4), container->getSize().width - 4, TrackerConfig::colorThemeMain, true));
	
	PPButton* button = new PPButton(DISKMENU_BUTTON_EXIT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight+1));
	button->setText("Exit");
	container->addControl(button);
	
	pp_int32 dx = 6;
	pp_int32 dy = 16;
	// ---- Song ----------
	x = px + 2;
	y = py + dy;
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x,y), "Song:", true));
	
	y+=12;
	buttonWidth = 8*7+2;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_LOAD_SONG, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Load");
	container->addControl(button);
	y+=buttonHeight+1;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_SAVE_SONG, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Save As");
	container->addControl(button);
	y+=buttonHeight;
	
	PPRadioGroup* radioGroup = new PPRadioGroup(DISKMENU_NORMAL_RADIOGROUP_SONGTYPE, screen, this, PPPoint(x, y), PPSize(buttonWidth, 3*14));
	radioGroupLocations[RADIOGROUPTOINDEX(DISKMENU_NORMAL_RADIOGROUP_SONGTYPE)] = radioGroup->getLocation(); 
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->addItem(".xm");
	radioGroup->addItem(".mod");
	radioGroup->addItem(".wav");
	container->addControl(radioGroup);

	// ---- Pattern ----------
	x += buttonWidth+dx;
	y = py + dy;
	container->addControl(new PPSeperator(0, screen, PPPoint(x - 4, y - 2), container->getLocation().y + container->getSize().height - y - 17, TrackerConfig::colorThemeMain, false));
	
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x,y), "Patt:", true));
	
	y+=12;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_LOAD_PATTERN, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Load");
	container->addControl(button);
	y+=buttonHeight+1;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_SAVE_PATTERN, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Save As");
	container->addControl(button);
	y+=buttonHeight;
	
	radioGroup = new PPRadioGroup(DISKMENU_NORMAL_RADIOGROUP_PATTERNTYPE, screen, this, PPPoint(x, y), PPSize(buttonWidth, 30));
	radioGroupLocations[RADIOGROUPTOINDEX(DISKMENU_NORMAL_RADIOGROUP_PATTERNTYPE)] = radioGroup->getLocation(); 
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->addItem(".xp");
	container->addControl(radioGroup);

	// ---- Track ----------
	x += buttonWidth+dx;
	y = py + dy;
	container->addControl(new PPSeperator(0, screen, PPPoint(x - 4, y - 2), container->getLocation().y + container->getSize().height - y - 17, TrackerConfig::colorThemeMain, false));

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x,y), "Track:", true));
	
	y+=12;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_LOAD_TRACK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Load");
	container->addControl(button);
	y+=buttonHeight+1;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_SAVE_TRACK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Save As");
	container->addControl(button);
	y+=buttonHeight;
	
	radioGroup = new PPRadioGroup(DISKMENU_NORMAL_RADIOGROUP_TRACKTYPE, screen, this, PPPoint(x, y), PPSize(buttonWidth, 30));
	radioGroupLocations[RADIOGROUPTOINDEX(DISKMENU_NORMAL_RADIOGROUP_TRACKTYPE)] = radioGroup->getLocation(); 
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->addItem(".xt");
	container->addControl(radioGroup);

	// ---- Instrument ----------
	x += buttonWidth+dx;
	y = py + dy;
	container->addControl(new PPSeperator(0, screen, PPPoint(x - 4, y - 2), container->getLocation().y + container->getSize().height - y - 17, TrackerConfig::colorThemeMain, false));

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x,y), "Instr:", true));
	
	y+=12;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_LOAD_INSTRUMENT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Load");
	container->addControl(button);
	y+=buttonHeight+1;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_SAVE_INSTRUMENT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Save As");
	container->addControl(button);
	y+=buttonHeight;
	
	radioGroup = new PPRadioGroup(DISKMENU_NORMAL_RADIOGROUP_INSTRUMENTTYPE, screen, this, PPPoint(x, y), PPSize(buttonWidth, 30));
	radioGroupLocations[RADIOGROUPTOINDEX(DISKMENU_NORMAL_RADIOGROUP_INSTRUMENTTYPE)] = radioGroup->getLocation(); 
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->addItem(".xi");
	container->addControl(radioGroup);

	// ---- Sample ----------
	x += buttonWidth+dx;
	y = py + dy;
	container->addControl(new PPSeperator(0, screen, PPPoint(x - 4, y - 2), container->getLocation().y + container->getSize().height - y - 17, TrackerConfig::colorThemeMain, false));

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x,y), "Sample:", true));
	
	y+=12;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_LOAD_SAMPLE, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Load");
	container->addControl(button);
	y+=buttonHeight+1;
	button = new PPButton(DISKMENU_NORMAL_BUTTON_SAVE_SAMPLE, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Save As");
	container->addControl(button);
	y+=buttonHeight;
	
	radioGroup = new PPRadioGroup(DISKMENU_NORMAL_RADIOGROUP_SAMPLETYPE, screen, this, PPPoint(x, y), PPSize(buttonWidth, 2*14));
	radioGroupLocations[RADIOGROUPTOINDEX(DISKMENU_NORMAL_RADIOGROUP_SAMPLETYPE)] = radioGroup->getLocation(); 
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->addItem(".wav");
	radioGroup->addItem(".iff");
	container->addControl(radioGroup);
	
	// Now get all controls built for the "normal" view (not the FT2 retro view) and save them, 
	// so we can hide them later easily, without referring to the IDs or whatever 
	// (except for the section heading and the exit button)
	PPSimpleVector<PPControl>& controls = container->getControls();
	for (i = 0; i < controls.size(); i++)
	{
		if (controls.get(i)->getID() != DISKMENU_BUTTON_EXIT)
		{
			normalViewControls->add(controls.get(i));
		}
	}
	
	// remember that, this is where we start gathering the controls for the "classic" view
	pp_int32 firstClassicControlIndex = i + 1;
	
	// go on with some more stuff
	x = container->getLocation().x + container->getSize().width - 28;
	pp_int32 x4 = x;
	y = py + 2;
	button = new PPButton(DISKMENU_BUTTON_FLIP, screen, this, PPPoint(x, y), PPSize(24, 11), false);
	button->setText("Flip");
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	container->addControl(button);
	
	x = px + 2;
	y = py + 2;
	container->addControl(new PPStaticText(DISKMENU_CLASSIC_STATICTEXT_HEADING, NULL, NULL, PPPoint(x, y), "Disk op", true, true));	
	y+=13;

	// add type buttons
	static const char* buttonTexts[6] = {"Type", "Module", "Instr.", "Sample", "Patt.", "Track"};
	pp_int32 numButtons = (DISKMENU_CLASSIC_BUTTON_TYPE_TRACK-DISKMENU_CLASSIC_BUTTON_TYPE_TYPE+1);
	ASSERT((sizeof(buttonTexts)/sizeof(const char*)) == numButtons);

	pp_int32 bWidth = 7*8;
	pp_int32 bHeight = 14;

	y--;
	pp_int32 y3 = y;

	for (i = 0; i < numButtons; i++)
	{
		/*if (i == 0)
			button = new PPButton(i + DISKMENU_CLASSIC_BUTTON_TYPE_TYPE, screen, this, PPPoint(x, y), PPSize(bWidth, bHeight), false, true, false);
		else*/
		
		button = new PPButton(i + DISKMENU_CLASSIC_BUTTON_TYPE_TYPE, screen, this, PPPoint(x, y), PPSize(bWidth, bHeight), false);
		
		button->setColor(TrackerConfig::colorThemeMain);
		button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
		button->setText(buttonTexts[i]);
		container->addControl(button);
		y+=bHeight;
	}
	
	pp_int32 x3 = x + bWidth + 3;

	y+=5;
	
	// add save button
	button = new PPButton(DISKMENU_CLASSIC_BUTTON_SAVE, screen, this, PPPoint(x, y), PPSize(bWidth, buttonHeight+1));
	button->setText("Save");
	container->addControl(button);	

	// file browser
	pp_int32 lbWidth = (px + container->getSize().width) - x3 - 4;
	pp_int32 lbHeight = (py+container->getSize().height-(buttonHeight+4)) - y3 - 5;
	listBoxFiles = new PPListBoxFileBrowser(DISKMENU_CLASSIC_LISTBOX_BROWSER, screen, this, PPPoint(x3, y3), PPSize(lbWidth, lbHeight));
	listBoxFiles->setBorderColor(TrackerConfig::colorThemeMain);
	listBoxFiles->setFilePrefix("");
	listBoxFiles->setDirectoryPrefix("");
	listBoxFiles->setDirectorySuffixPathSeperator();
	listBoxFiles->setSortAscending(sortAscending);
	listBoxFiles->setColorQueryListener(colorQueryListener);
	container->addControl(listBoxFiles);
	fileBrowserExtent = listBoxFiles->getSize();

	y3+=2;
	x3+=2;
	
	PPControl* ctrl;
	
	ctrl = new PPStaticText(DISKMENU_CLASSIC_STATICTEXT_SORTBY, NULL, NULL, PPPoint(x3, y3), "Sort by:", true);
	container->addControl(ctrl);	
	x3+=8*8;

	button = new PPButton(DISKMENU_CLASSIC_BUTTON_SORTBY, screen, this, PPPoint(x3, y3-1), PPSize(51, 11), false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Extension");
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	container->addControl(button);

	button = new PPButton(DISKMENU_CLASSIC_BUTTON_SORTORDER, screen, this, PPPoint(x3+button->getSize().width, y3-1), PPSize(13, 11), false);
	button->setText(sortAscending ? "\xfd" : "\xfe");
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	container->addControl(button);

	x3+=80;

	ctrl = new PPStaticText(DISKMENU_CLASSIC_STATICTEXT_FILTEREXTENSIONS, NULL, NULL, PPPoint(x3, y3), "Type filter:", true);
	container->addControl(ctrl);

	ctrl = new PPCheckBox(DISKMENU_CLASSIC_CHECKBOX_FILTEREXTENSIONS, screen, this, PPPoint(x3 + 12*8+2, y3-1));
	container->addControl(ctrl);

	buttonWidth = 27;
	pp_int32 y5 = y3 + 12;
	pp_int32 x5 = container->getLocation().x + container->getSize().width - buttonWidth - 5;
	numButtons = 5;
	for (i = 0; i < numButtons; i++)
	{
		pp_int32 buttonHeight = 11;
		char temp[80];
		sprintf(temp, "DIR%d", i+1);
		PPFont* font = PPFont::getFont(PPFont::FONT_TINY);
		button = new PPButton(DISKMENU_CLASSIC_BUTTON_DIR0+i, screen, this, PPPoint(x5, y5), PPSize(buttonWidth, buttonHeight));
		button->setFont(font);
		button->setText(temp);
		container->addControl(button);	
		y5+=buttonHeight+1;
	}
	button = new PPButton(DISKMENU_CLASSIC_BUTTON_STOREDIR, screen, this, PPPoint(x5, y5), PPSize(buttonWidth, 9), true, true, false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Store");
	container->addControl(button);	
	
	// save edit list box
	buttonWidth = container->getControlByID(DISKMENU_BUTTON_EXIT)->getLocation().x - (x + bWidth + 4) - 4;

	editFieldCurrentFile = new PPListBox(DISKMENU_CLASSIC_LISTBOX_NAME, screen, this, PPPoint(x + bWidth + 4, y), PPSize(buttonWidth, buttonHeight+1), true, true, false);
	editFieldCurrentFile->showSelection(false);
	editFieldCurrentFile->setSingleButtonClickEdit(true);
	editFieldCurrentFile->setBorderColor(TrackerConfig::colorThemeMain);

	char str[MP_MAXTEXT+1];
	memset(str, 0, sizeof(str));

	editFieldCurrentFile->addItem(str);
	editFieldCurrentFile->setMaxEditSize(255);

	container->addControl(editFieldCurrentFile);

	y = py + 2;
	x = px + 2 + 8*7 + 5;

	static const char* buttonTexts2[] = {"<-", "->", "Up", "Root", "Home", "Reload", "Load", "Del", "MkDir"};
	static const pp_int32 buttonSpacing[] = {0,0,0,0,0,3,0,0,0};
	numButtons = sizeof(buttonTexts2)/sizeof(const char*);

	bHeight = 10;
	for (i = 0; i < numButtons; i++)
	{
		PPFont* font = PPFont::getFont(PPFont::FONT_TINY);
		bWidth = font->getStrWidth(buttonTexts2[i]) + 3;
		button = new PPButton(DISKMENU_CLASSIC_BUTTON_PREV+i, screen, this, PPPoint(x, y), PPSize(bWidth, bHeight));
		button->setFont(font);
		button->setText(buttonTexts2[i]);
		container->addControl(button);	
		x+=bWidth+1+buttonSpacing[i];
	}
	
	x=x4-13*2;
	button = new PPButton(DISKMENU_CLASSIC_BUTTON_VEXTEND, screen, this, PPPoint(x, y), PPSize(13, bHeight+1), false);
	button->setText(TrackerConfig::stringButtonCollapsed);
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	container->addControl(button);	
	x+=13;
	button = new PPButton(DISKMENU_CLASSIC_BUTTON_HEXTEND, screen, this, PPPoint(x, y), PPSize(13, bHeight+1), false);
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText(">");
	button->setColor(TrackerConfig::colorThemeMain);
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));
	container->addControl(button);	
	
	y = py+container->getSize().height-(buttonHeight+4);
	container->addControl(new PPSeperator(0, screen, PPPoint(px + 2, y - 4), container->getSize().width - 4, TrackerConfig::colorThemeMain, true));

	// gather controls for the classic view, we simply start counting from the last control we found
	// the last time
	for (i = firstClassicControlIndex; i < controls.size(); i++)
	{
		classicViewControls->add(controls.get(i));
		controls.get(i)->hide(true);
	}
	
	tracker.screen->addControl(container);

	sectionContainer = container;

	// fit dir buttons to browser listbox size
	fitDirButtons();
	
	initialised = true;

	showSection(false);
}

void SectionDiskMenu::show(bool bShow)
{
	// sanity check
	if (bShow == diskMenuVisible)
		return;

#ifdef __LOWRES__
	tracker.screen->pauseUpdate(true);
#endif

	PPRadioGroup* radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SONGTYPE));

	if (bShow)
	{
		if (moduleTypeAdjust)
		{
			switch (tracker.moduleEditor->getSaveType())
			{
				case ModuleEditor::ModSaveTypeXM:
					radioGroup->setChoice(0);
					break;
					
				case ModuleEditor::ModSaveTypeMOD:
					radioGroup->setChoice(1);
					break;
					
				default:
					ASSERT(false);
			}
		}		

		// restore CWD in case it has been changed
		// through the native file requester (e.g. on Windows)
		if (currentPath.length() != 0)
		{
			setCurrentPath(currentPath, false);
		}
	}
	else
	{
		// store CWD
		currentPath = getCurrentPath();
	}
	
	diskMenuVisible = bShow;
	
	SectionUpperLeft::show(bShow);
	
	if (bShow)
	{
#ifdef __LOWRES__
		pp_int32 y = tracker.screen->getControlByID(CONTAINER_INPUTDEFAULT)->getLocation().y;
		replaceInstrumentListBoxes(true, y);
		tracker.getPatternEditorControl()->show(false);
#endif		
		prepareSection();
		lastFocusedControl = tracker.screen->getFocusedControl();		
		tracker.screen->setFocus(listBoxFiles);
	
		if (forceClassicBrowser)
		{
			showNormalView(false);
			showClassicView(true);
			updateClassicView();
			updateFilenameEditField(classicViewState);
			forceClassicBrowser = false;
		}
	}
	else
	{
#ifdef __LOWRES__
		replaceInstrumentListBoxes(false);
		tracker.getPatternEditorControl()->show(true);
#endif		
		tracker.screen->setFocus(lastFocusedControl);
	}

#ifdef __LOWRES__
	pp_int32 deltay = sectionContainer->getSize().height - tracker.UPPERSECTIONDEFAULTHEIGHT();
	
	pp_int32 newSIPOffsetMove = bShow ? -deltay : deltay;

	// Only move SIP panel up/down on show/hide when it's exactly the complementary
	// to the previous move, otherwise we'll be moving it out of range
	if ((lastSIPOffsetMove != 0 && lastSIPOffsetMove == -newSIPOffsetMove) ||
		lastSIPOffsetMove == 0)
	{
		tracker.moveInputControls(newSIPOffsetMove);
		lastSIPOffsetMove = newSIPOffsetMove;
	}

	tracker.screen->paint();
	tracker.screen->pauseUpdate(false);
	if (!bShow)
	{
		tracker.screen->update();
	}
#endif
}

void SectionDiskMenu::update(bool repaint/* = true*/)
{
}

bool SectionDiskMenu::isActiveEditing()
{
	PPListBox* listBox = static_cast<PPListBox*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_LISTBOX_NAME));
	
	ASSERT(listBox);
	
	if (tracker.screen->hasFocus(sectionContainer) && listBox->isEditing())							
		return true;
		
	return false;
}

bool SectionDiskMenu::isFileBrowserVisible()
{
	return listBoxFiles->isVisible();
}

bool SectionDiskMenu::fileBrowserHasFocus()
{
	return tracker.screen->hasFocus(sectionContainer) && listBoxFiles->gotFocus();
}

void SectionDiskMenu::setFileBrowserShowFocus(bool showFocus)
{
	listBoxFiles->setShowFocus(showFocus);
}

void SectionDiskMenu::selectSaveType(pp_uint32 type)
{
	switch (type)
	{
		case FileTypes::FileTypeSongMOD:
			static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SONGTYPE))->setChoice(1);
			classicViewState = BrowseModules;
			break;
		case FileTypes::FileTypeSongAllModules:
		case FileTypes::FileTypeSongXM:
			static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SONGTYPE))->setChoice(0);
			classicViewState = BrowseModules;
			break;
		case FileTypes::FileTypeSongWAV:
			static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SONGTYPE))->setChoice(2);
			classicViewState = BrowseModules;
			break;

		case FileTypes::FileTypePatternXP:
			static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_PATTERNTYPE))->setChoice(0);
			classicViewState = BrowsePatterns;
			break;

		case FileTypes::FileTypeTrackXT:
			static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_TRACKTYPE))->setChoice(0);
			classicViewState = BrowseTracks;
			break;

		case FileTypes::FileTypeInstrumentXI:
		case FileTypes::FileTypeSongAllInstruments:
			static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_INSTRUMENTTYPE))->setChoice(0);
			classicViewState = BrowseInstruments;
			break;

		case FileTypes::FileTypeSampleWAV:
		case FileTypes::FileTypeSongAllSamples:
			static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SAMPLETYPE))->setChoice(0);
			classicViewState = BrowseSamples;
			break;
		case FileTypes::FileTypeSampleIFF:
			static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SAMPLETYPE))->setChoice(1);
			classicViewState = BrowseSamples;
			break;
			
		default:
			ASSERT(false);
	}

	forceClassicBrowser = true;
}

pp_uint32 SectionDiskMenu::getDefaultConfigUInt32()
{
	// default is:
	// classic view is NOT visible = 0
	// list box is v-extended = 2
	// list box is h-extended = 4
	// sort order is ascending = 8
	// filtering by file extension is ON = 16
	return 0 + 2 + 4 + 8 + 16;
}

pp_uint32 SectionDiskMenu::getConfigUInt32()
{
	pp_uint32 result = 0;
	
	// Classic view visible
	result |= classicViewVisible ? 1 : 0;	
	// Extended list box = Bit 1
	result |= (listBoxFiles->getSize().height == fileBrowserExtent.height) ? 2 : 0;
	// Extended list box = Bit 2
	result |= (listBoxFiles->getSize().width == fileBrowserExtent.width) ? 4 : 0;
	// Sort order = Bit 3
	result |= sortAscending ? 8 : 0;
	// filter types = Bit 4
	result |= static_cast<PPCheckBox*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_CHECKBOX_FILTEREXTENSIONS))->isChecked() ? 16 : 0;
	
	// filter types = Bit 24 and above
	result |= (pp_uint32)listBoxFiles->getSortType() << 24;				
	
	return result;
}

void SectionDiskMenu::setConfigUInt32(pp_uint32 config)
{
	if (config & 1)
	{
		showNormalView(false);
		showClassicView(true);
	}
	else if (!(config & 1))
	{
		showNormalView(true);
		showClassicView(false);
	}
	
	if ((config & 2) && listBoxFiles->getSize().height != fileBrowserExtent.height)
	{
		resizeBrowserVertically();
	}
	else if (!(config & 2) && listBoxFiles->getSize().height == fileBrowserExtent.height)
	{
		resizeBrowserVertically();
	}
	if ((config & 4) && listBoxFiles->getSize().width != fileBrowserExtent.width)
	{
		resizeBrowserHorizontally();
	}
	else if (!(config & 4) && listBoxFiles->getSize().width == fileBrowserExtent.width)
	{
		resizeBrowserHorizontally();
	}
	sortAscending = (config & 8) != 0;
	listBoxFiles->setSortAscending(sortAscending);

	PPCheckBox* checkBox = static_cast<PPCheckBox*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_CHECKBOX_FILTEREXTENSIONS));
	ASSERT(checkBox);
	checkBox->checkIt((config & 16) != 0);

	listBoxFiles->setSortType((PPListBoxFileBrowser::SortTypes)(config >> 24));
	updateButtonStates(false);
}

PPSystemString SectionDiskMenu::getCurrentPath()
{
	return listBoxFiles->getCurrentPathAsString();
}

void SectionDiskMenu::setCurrentPath(const PPSystemString& path, bool reload/* = true*/)
{
	listBoxFiles->gotoPath(path, reload);
}

PPString SectionDiskMenu::getCurrentPathASCII()
{
	char* nameASCIIZ = listBoxFiles->getCurrentPathAsString().toASCIIZ();	
	PPString result(nameASCIIZ);
	delete[] nameASCIIZ;
	return result;
}

void SectionDiskMenu::resizeInstrumentContainer()
{
#ifdef __LOWRES__
	PPControl* ctrl1 = tracker.screen->getControlByID(CONTAINER_INPUTDEFAULT);
	PPControl* ctrl2 = tracker.screen->getControlByID(CONTAINER_INPUTEXTENDED);

	pp_int32 y = ctrl1->isVisible() ? ctrl1->getLocation().y : ctrl2->getLocation().y;

	replaceAndResizeInstrumentListContainer(y);
	
	tracker.screen->paint(false);
#endif	
}

void SectionDiskMenu::setCycleFilenames(bool cycleFilenames)
{
	listBoxFiles->setCycleFilenames(cycleFilenames);
}

void SectionDiskMenu::prepareSection()
{
	updateFilter();

	updateFilenameEditField(classicViewState);
}

void SectionDiskMenu::showNormalView(bool bShow)
{
	pp_uint32 i;

	for (i = 0; i < (unsigned)normalViewControls->size(); i++)
		normalViewControls->get(i)->hide(!bShow);
	
	for (i = 0; i < sizeof(radioGroupLocations) / sizeof(PPPoint); i++)
		static_cast<PPContainer*>(sectionContainer)->getControlByID(INDEXTORADIOGROUP(i))->setLocation(radioGroupLocations[i]);
				
	if (bShow && isActiveEditing())
		tracker.screen->setFocus(lastFocusedControl);
}

void SectionDiskMenu::updateClassicView(bool repaint/* = true*/)
{
	pp_uint32 i;
	
	if (!classicViewVisible)
		return;
	
	updateButtonStates(false);
	
	bool cond = classicViewState != BrowseAll;

	static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_TYPE_TYPE))->setClickable(cond);
	static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_SAVE))->setClickable(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_TYPE_MODULE)->hide(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_TYPE_INSTRUMENT)->hide(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_TYPE_SAMPLE)->hide(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_TYPE_PATTERN)->hide(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_TYPE_TRACK)->hide(cond);		
	
	if (classicViewState == BrowseAll)
	{
		currentActiveRadioGroup = NULL;
	}

	PPPoint pos = static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_TYPE_MODULE)->getLocation();
	
	for (i = 0; i < sizeof(radioGroupLocations) / sizeof(PPPoint); i++)
	{
		static_cast<PPContainer*>(sectionContainer)->getControlByID(INDEXTORADIOGROUP(i))->setLocation(pos);
		static_cast<PPContainer*>(sectionContainer)->getControlByID(INDEXTORADIOGROUP(i))->hide(true);
	}
	
	switch (classicViewState)
	{
		case BrowseModules:
			currentActiveRadioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SONGTYPE));
			break;
		case BrowseInstruments:
			currentActiveRadioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_INSTRUMENTTYPE));
			break;
		case BrowseSamples:
			currentActiveRadioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_SAMPLETYPE));
			break;
		case BrowsePatterns:
			currentActiveRadioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_PATTERNTYPE));
			break;
		case BrowseTracks:
			currentActiveRadioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_NORMAL_RADIOGROUP_TRACKTYPE));
			break;
		case BrowseAll:
		case BrowseLAST:
			break;
	}

	cond = listBoxFiles->getSize().height == fileBrowserExtent.height;
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_STATICTEXT_FILTEREXTENSIONS)->hide(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_CHECKBOX_FILTEREXTENSIONS)->hide(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_STATICTEXT_SORTBY)->hide(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_SORTBY)->hide(cond);
	static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_SORTORDER)->hide(cond);
	
	for (i = DISKMENU_CLASSIC_BUTTON_DIR0; i <= DISKMENU_CLASSIC_BUTTON_STOREDIR; i++)
		static_cast<PPContainer*>(sectionContainer)->getControlByID(i)->hide(listBoxFiles->getSize().width == fileBrowserExtent.width);

	if (currentActiveRadioGroup)
		currentActiveRadioGroup->hide(false);
	
	if (repaint)
		tracker.screen->paintControl(sectionContainer);
}

void SectionDiskMenu::showClassicView(bool bShow)
{
	for (pp_int32 i = 0; i < classicViewControls->size(); i++)
		classicViewControls->get(i)->hide(!bShow);
		
	classicViewVisible = bShow;

	if (bShow)
	{
		updateClassicView();
	}
}

bool SectionDiskMenu::isNormalViewVisible()
{
	bool res = true;

	for (pp_int32 i = 0; i < normalViewControls->size(); i++)
		if (!normalViewControls->get(i)->isVisible())
			res = false;
			
	return res;
}

bool SectionDiskMenu::isClassicViewVisible()
{
	bool res = true;

	for (pp_int32 i = 0; i < classicViewControls->size(); i++)
		if (!classicViewControls->get(i)->isVisible())
			res = false;
			
	return res;
}

void SectionDiskMenu::flip()
{
	if (isNormalViewVisible())
	{
		showNormalView(false);
		showClassicView(true);
		updateFilenameEditField(classicViewState);
	}
	else
	{
		showNormalView(true);
		showClassicView(false);
	}
	
	PPScreen* screen = tracker.screen;

	screen->paintControl(sectionContainer);
}

void SectionDiskMenu::updateFilenameEditFieldExtension(ClassicViewStates viewState)
{
	if (currentActiveRadioGroup)
	{
		PPSystemString file = this->file->stripExtension();		
		if (file.length())
		{			
			PPSystemString ext(currentActiveRadioGroup->getItem(currentActiveRadioGroup->getChoice()));			
			file.append(ext);			
			updateFilenameEditField(file);
		}
	}
}

void SectionDiskMenu::updateFilenameEditField(ClassicViewStates viewState)
{
	PPSystemString ext;
	
	if (currentActiveRadioGroup)
		ext = PPSystemString(currentActiveRadioGroup->getItem(currentActiveRadioGroup->getChoice()));

	switch (viewState)
	{
		case BrowseAll:
			*file = tracker.moduleEditor->getModuleFileName();
			break;
			
		case BrowseModules:
			if (moduleTypeAdjust)
				*file = tracker.moduleEditor->getModuleFileName();
			else
			{
				*file = tracker.moduleEditor->getModuleFileName().stripExtension();
				file->append(ext);
			}
			break;
		case BrowseInstruments:
		{
			*file = tracker.moduleEditor->getInstrumentFileName(tracker.listBoxInstruments->getSelectedIndex());			
			file->append(ext);
			break;
		}
		case BrowseSamples:
		{
			*file = tracker.moduleEditor->getSampleFileName(tracker.listBoxInstruments->getSelectedIndex(), tracker.listBoxSamples->getSelectedIndex());
			file->append(ext);
			break;
		}
		case BrowsePatterns:
		{
			*file = tracker.moduleEditor->getModuleFileName().stripExtension();
			file->append(ext);
			break;
		}
		case BrowseTracks:
		{
			*file = tracker.moduleEditor->getModuleFileName().stripExtension();
			file->append(ext);
			break;	
		}
		case BrowseLAST:
			break;

	}
	
	updateFilenameEditField(*file);
}

void SectionDiskMenu::updateFilenameEditField(const PPSystemString& fileName)
{
	editFieldCurrentFile->clear();
	
	char* nameASCIIZ = fileName.toASCIIZ();
	PPString str(nameASCIIZ);
	editFieldCurrentFile->addItem(str);
	delete[] nameASCIIZ;
	
	*file = fileName;
	
	tracker.screen->paintControl(editFieldCurrentFile);		
}

void SectionDiskMenu::updateFilenameEditFieldFromBrowser()
{
	updateButtonStates();

	if (listBoxFiles->currentSelectionIsFile())
	{
		updateFilenameEditField(listBoxFiles->getCurrentSelectedPathEntry()->getName());
	}
}

void SectionDiskMenu::handleLoadOrStep()
{
	if (listBoxFiles->stepIntoCurrentSelection())
	{
		listBoxFiles->refreshFiles();
		updateButtonStates();
		tracker.screen->paintControl(listBoxFiles);
	}
	else
	{
		loadCurrentSelectedFile();
	}
}

void SectionDiskMenu::loadCurrentSelectedFile()
{
	PPSystemString fileFullPath = listBoxFiles->getCurrentPathAsString();
	fileFullPath.append(listBoxFiles->getCurrentSelectedPathEntry()->getName());	

	switch (classicViewState)
	{
		case BrowseAll:
			tracker.loadGenericFileType(fileFullPath);
			break;
		case BrowseModules:
			tracker.loadTypeFromFile(FileTypes::FileTypeSongAllModules, fileFullPath);
			break;
		case BrowseInstruments:
			tracker.loadTypeFromFile(FileTypes::FileTypeSongAllInstruments, fileFullPath);
			break;
		case BrowseSamples:
			tracker.loadTypeFromFile(FileTypes::FileTypeSongAllSamples, fileFullPath);
			break;
		case BrowsePatterns:
			tracker.loadTypeFromFile(FileTypes::FileTypePatternXP, fileFullPath);
			break;
		case BrowseTracks:
			tracker.loadTypeFromFile(FileTypes::FileTypeTrackXT, fileFullPath);
			break;
		case BrowseLAST:
			break;

	}
	
	updateFilenameEditFieldExtension(classicViewState);
}

void SectionDiskMenu::showOverwriteMessageBox()
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}
	
	dialog = new PPDialogBase(tracker.screen, 
							  responder, 
							  RESPONDMESSAGEBOX_OVERWRITE, 
							  "Overwrite existing file?");
	dialog->show();	
}
	
void SectionDiskMenu::prepareSave()
{
	if (editFieldCurrentFile->isEditing())
		editFieldCurrentFile->commitChanges();

	assureExtension();

	PPSystemString fileFullPath = listBoxFiles->getCurrentPathAsString();
	
	PPSystemString file = this->file->stripExtension();
	
	if (file.length())
	{
		fileFullPath.append(*this->file);
		
		if (XMFile::exists(fileFullPath))
		{
			showOverwriteMessageBox();
		}
		else
		{
			saveCurrent();
		}
	}
}

void SectionDiskMenu::saveCurrent()
{
	PPSystemString fileFullPath = listBoxFiles->getCurrentPathAsString();
	
	fileFullPath.append(*this->file);
	
	bool res = true;
	
	FileTypes saveType;
	
	switch (classicViewState)
	{
		case BrowseAll:
			ASSERT(false);
			break;
		case BrowseModules:
		{
			switch (currentActiveRadioGroup->getChoice())
			{
				case 0:
					saveType = FileTypes::FileTypeSongXM;
					break;
					
				case 1:
					saveType = FileTypes::FileTypeSongMOD;
					break;
					
				case 2:
				{
					tracker.sectionHDRecorder->setCurrentFileName(fileFullPath);
					tracker.sectionSwitcher->showUpperSection(tracker.sectionHDRecorder);
					if (dialog &&
						tracker.screen->getModalControl() == dialog->getMessageBoxContainer())
					{
						tracker.screen->setModalControl(NULL);
					}
					return;
				}
			}
			
			break;
		}
		
		case BrowseInstruments:
			saveType = FileTypes::FileTypeInstrumentXI;
			break;
		case BrowseSamples:
		{
			switch (currentActiveRadioGroup->getChoice())
			{
				case 0:
					saveType = FileTypes::FileTypeSampleWAV;
					break;
					
				case 1:
					saveType = FileTypes::FileTypeSampleIFF;
					break;
			}
			
			break;
		}
		case BrowsePatterns:
			saveType = FileTypes::FileTypePatternXP;
			break;
		case BrowseTracks:
			saveType = FileTypes::FileTypeTrackXT;
			break;
		case BrowseLAST:
			break;

	}
	
	res = tracker.prepareSavingWithDialog(saveType);

	if (tracker.savePanel)
	{
		delete tracker.savePanel;
		tracker.savePanel = new PPDummySavePanel(fileFullPath);
	}

	if (res)
	{		
		tracker.saveTypeWithDialog(saveType, this);	
	}
	else
	{
		tracker.fileSystemChangedListener = this;
	}
	
	if (dialog &&
		tracker.screen->getModalControl() == dialog->getMessageBoxContainer())
	{
		tracker.screen->setModalControl(NULL);
	}
		
}

void SectionDiskMenu::showDeleteMessageBox()
{
	if (!listBoxFiles->currentSelectionIsFile())
		return;

	showMessageBox(RESPONDMESSAGEBOX_DELETE, "Delete selected file?");
}

void SectionDiskMenu::deleteCurrent()
{
	PPSystemString fileFullPath = listBoxFiles->getCurrentPathAsString();
	
	fileFullPath.append(listBoxFiles->getCurrentSelectedPathEntry()->getName());
	
	XMFile::remove(fileFullPath);
	
	reload();
}

void SectionDiskMenu::updateButtonStates(bool repaint/* = true*/)
{
	static const pp_uint32 IDs[] = 
	{
		DISKMENU_CLASSIC_BUTTON_PREV, 
		DISKMENU_CLASSIC_BUTTON_NEXT, 
		DISKMENU_CLASSIC_BUTTON_PARENT, 
		DISKMENU_CLASSIC_BUTTON_ROOT, 
		DISKMENU_CLASSIC_BUTTON_HOME, 
		DISKMENU_CLASSIC_BUTTON_LOAD, 
		DISKMENU_CLASSIC_BUTTON_DELETE, 
		DISKMENU_CLASSIC_BUTTON_MAKEDIR
	};
	
	const bool states[] = 
	{
		listBoxFiles->canPrev(),
		listBoxFiles->canNext(),
		listBoxFiles->canGotoParent(),
		listBoxFiles->canGotoRoot(),
		listBoxFiles->canGotoHome(),
		true,
		listBoxFiles->currentSelectionIsFile(),
		false
	};
	
	ASSERT(sizeof(states)/sizeof(bool) == sizeof(IDs)/sizeof(pp_uint32));
	
	pp_uint32 i;
	for (i = 0; i < sizeof(IDs)/sizeof(pp_uint32); i++)
	{
		bool b = states[i];
		PPButton* button = static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(IDs[i]));
		if (button->isClickable() != b)
		{
			button->setClickable(b);
			if (repaint)
				tracker.screen->paintControl(button);
		}
	}
	
	const char* stateText = listBoxFiles->currentSelectionIsFile() ? "Load" : "Step";
	PPButton* button = static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_LOAD));
	if (button->getText().compareTo(stateText) != 0)
	{
		button->setText(stateText);
		if (repaint)
			tracker.screen->paintControl(button);		
	}

	stateText = sortAscending ? "\xfd" : "\xfe";
	button = static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_SORTORDER));
	if (button->getText().compareTo(stateText) != 0)
	{
		button->setText(stateText);
		if (repaint)
			tracker.screen->paintControl(button);		
	}

	static const char* sortTypes[PPListBoxFileBrowser::NumSortRules] = {"Name", "Size", "Extension"};
	stateText = sortTypes[listBoxFiles->getSortType()];
	button = static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_SORTBY));
	if (button->getText().compareTo(stateText) != 0)
	{
		button->setText(stateText);
		if (repaint)
			tracker.screen->paintControl(button);		
	}

	PPString stateText2 = (listBoxFiles->getSize().height == fileBrowserExtent.height) ? TrackerConfig::stringButtonExtended : TrackerConfig::stringButtonCollapsed;
	button = static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_VEXTEND));
	if (button->getText().compareTo(stateText2) != 0)
	{
		button->setText(stateText2);
		if (repaint)
			tracker.screen->paintControl(button);		
	}

	stateText2 = (listBoxFiles->getSize().width == fileBrowserExtent.width) ? ">" : "<";
	button = static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_BUTTON_HEXTEND));
	if (button->getText().compareTo(stateText2) != 0)
	{
		button->setText(stateText2);
		if (repaint)
			tracker.screen->paintControl(button);		
	}
	
	// update directory buttons
#if 0
	for (i = DISKMENU_CLASSIC_BUTTON_DIR0; i <= DISKMENU_CLASSIC_BUTTON_DIR4; i++)
	{
		PPButton* button = static_cast<PPButton*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(i));		
		PPString strKey = getKeyFromPredefPathButton(button);		
		PPDictionaryKey* key = tracker.settingsDatabase->restore(strKey);		
		button->enable(key != NULL);
	}
#endif
}

void SectionDiskMenu::next(bool repaint/* = true*/)
{
	listBoxFiles->next();
	tracker.screen->paintControl(listBoxFiles);
	updateButtonStates();
}

void SectionDiskMenu::prev(bool repaint/* = true*/)
{
	listBoxFiles->prev();
	tracker.screen->paintControl(listBoxFiles);
	updateButtonStates();
}

void SectionDiskMenu::parent(bool repaint/* = true*/)
{
	listBoxFiles->gotoParent();
	tracker.screen->paintControl(listBoxFiles);
	updateButtonStates();
}

void SectionDiskMenu::root(bool repaint/* = true*/)
{
	listBoxFiles->gotoRoot();
	tracker.screen->paintControl(listBoxFiles);
	updateButtonStates();
}

void SectionDiskMenu::home(bool repaint/* = true*/)
{
	listBoxFiles->gotoHome();
	tracker.screen->paintControl(listBoxFiles);
	updateButtonStates();
}

void SectionDiskMenu::reload(bool repaint/* = true*/)
{
	PPPathEntry* pathEntry = NULL;
	const PPPathEntry* src = listBoxFiles->getCurrentSelectedPathEntry();

	if (src)
		pathEntry = src->clone();

	listBoxFiles->saveState();
	listBoxFiles->refreshFiles();
	listBoxFiles->restoreState(false);
	
	if (pathEntry)
	{
		for (pp_int32 i = 0; i < listBoxFiles->getNumItems(); i++)
		{
			if (pathEntry->compareTo(*listBoxFiles->getPathEntry(i)))
			{
				listBoxFiles->setSelectedIndex(i, false);
				break;
			}
		}
		delete pathEntry;
	}
	
	if (repaint)
		tracker.screen->paintControl(listBoxFiles);

}

void SectionDiskMenu::updateFilter(bool repaint/* = true*/)
{
	FileExtProvider fileExtProvider;

	listBoxFiles->clearExtensions();
	
	const char* const* extensions = NULL;
	
	if (static_cast<PPCheckBox*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(DISKMENU_CLASSIC_CHECKBOX_FILTEREXTENSIONS))->isChecked())
	{
		switch (classicViewState)
		{
			case BrowseModules:
				extensions = fileExtProvider.getModuleExtensions();
				break;
			case BrowseInstruments:
				extensions = fileExtProvider.getInstrumentExtensions();
				break;
			case BrowseSamples:
				extensions = fileExtProvider.getSampleExtensions();
				break;
			case BrowsePatterns:
				extensions = fileExtProvider.getPatternExtensions();
				break;
			case BrowseTracks:
				extensions = fileExtProvider.getTrackExtensions();
				break;
			case BrowseAll: 
			case BrowseLAST:
				break;
		}
	}
	
	if (extensions)
		listBoxFiles->addExtensions(extensions);
	
	reload(repaint);
}

void SectionDiskMenu::switchState(ClassicViewStates viewState)
{
	classicViewState = viewState;
	updateFilter(false);
	updateClassicView();
	updateFilenameEditField(classicViewState);	
}

void SectionDiskMenu::resizeBrowserVertically()
{
	PPSize size = listBoxFiles->getSize();
	PPPoint location = listBoxFiles->getLocation();
	if (size.height == fileBrowserExtent.height)
	{
		size.height-=13;
		listBoxFiles->setSize(size);
		location.y+=13;
		listBoxFiles->setLocation(location);		
	}
	else
	{
		size.height+=13;
		listBoxFiles->setSize(size);
		location.y-=13;
		listBoxFiles->setLocation(location);
	}
	
	fitDirButtons();
	
	updateClassicView();
}

void SectionDiskMenu::resizeBrowserHorizontally()
{
	PPSize size = listBoxFiles->getSize();
	if (size.width == fileBrowserExtent.width)
	{
		size.width-=32;
		listBoxFiles->setSize(size);
	}
	else
	{
		size.width+=32;
		listBoxFiles->setSize(size);
	}
	
	updateClassicView();
}

void SectionDiskMenu::fitDirButtons()
{
	pp_int32 height = (listBoxFiles->getSize().height-2) / 6;

	pp_int32 cy = ((listBoxFiles->getSize().height-2) - (height * 6));

	pp_int32 y = listBoxFiles->getLocation().y + cy;
		
	for (pp_int32 i = DISKMENU_CLASSIC_BUTTON_DIR0; i <= DISKMENU_CLASSIC_BUTTON_STOREDIR; i++)
	{
		PPControl* ctrl = static_cast<PPContainer*>(sectionContainer)->getControlByID(i);
		PPPoint p = ctrl->getLocation();
		p.y = y;
		ctrl->setLocation(p);
		PPSize s = ctrl->getSize();
		s.height = height-1;
		ctrl->setSize(s);
		y+=height;
	}
}

PPString SectionDiskMenu::getKeyFromPredefPathButton(PPControl* button)
{
	pp_int32 id = button->getID();
	
	id -= DISKMENU_CLASSIC_BUTTON_DIR0;
	
	if (id >= 0 && id < 5)
	{
		static const char* keys[BrowseLAST] = 
		{
			"PREDEF_PATH_ALL",
			"PREDEF_PATH_MODULES",
			"PREDEF_PATH_INSTRUMENTS",
			"PREDEF_PATH_SAMPLES",
			"PREDEF_PATH_PATTERNS",
			"PREDEF_PATH_TRACKS",
		};
	
		char result[1024];
		
		sprintf(result, "%s_%d", keys[classicViewState], id);
		
		return result;
	}
	else return "";
}

void SectionDiskMenu::assureExtension()
{
	updateFilenameEditFieldExtension(classicViewState);
}




