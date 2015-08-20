/*
 *  tracker/SectionAdvancedEdit.cpp
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
 *  SectionAdvancedEdit.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.05.05.
 *
 */

#include "SectionAdvancedEdit.h"
#include "Tracker.h"
#include "ToolInvokeHelper.h"
#include "TrackerConfig.h"
#include "ModuleEditor.h"

#include "CheckBox.h"
#include "Container.h"
#include "RadioGroup.h"
#include "Seperator.h"
#include "StaticText.h"
#include "PatternEditorControl.h"

#include "PatternTools.h"
#include "Tools.h"

#include "ControlIDs.h"

enum ControlIDs
{
	ADVEDIT_BUTTON_EXIT				= 32000,
	ADVEDIT_BUTTON_INSREMAP,
	ADVEDIT_BUTTON_INTERPOLATE,

	ADVEDIT_BUTTON_VOLSCALETRACK,
	ADVEDIT_BUTTON_VOLSCALEPATTERN,
	ADVEDIT_BUTTON_VOLSCALEBLOCK,

	ADVEDIT_BUTTON_SPLITBLOCK,
	ADVEDIT_BUTTON_SPLITTRACK,
	ADVEDIT_BUTTON_SPLITTRACKPLUS,
	ADVEDIT_BUTTON_SPLITTRACKMINUS,
	ADVEDIT_CHECKBOX_SPLITTRACK,
	ADVEDIT_CHECKBOX_SPLITTRACKNOTEOFF,
	ADVEDIT_TEXT_SPLITTRACK,

	ADVEDIT_RADIO_CONVERT,
	ADVEDIT_BUTTON_CONVERT,
	ADVEDIT_RADIO_REMOVE,
	ADVEDIT_BUTTON_REMOVE
};

SectionAdvancedEdit::SectionAdvancedEdit(Tracker& theTracker) :
	SectionUpperLeft(theTracker),
	splitTrackNumSubsequentChannels(1),
	checkBoxSplitTrack(NULL),
	checkBoxSplitTrackNoteOff(NULL)	
{
}

SectionAdvancedEdit::~SectionAdvancedEdit()
{
}

pp_int32 SectionAdvancedEdit::handleEvent(PPObject* sender, PPEvent* event)
{
	PPScreen* screen = tracker.screen;
	char buffer[100];

	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case ADVEDIT_BUTTON_INSREMAP:
				if (event->getID() != eCommand)
					break;

				tracker.initAdvEdit();									
				break;

			case ADVEDIT_BUTTON_INTERPOLATE:
			{
				if (event->getID() != eCommand)
					break;

				pp_int32 res = tracker.getPatternEditor()->interpolateValuesInSelection();
				if (res)
					screen->paint();
				else
					tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Please select at least 2 values.", Tracker::MessageBox_OK);
				
				break;
			}
			
			case ADVEDIT_BUTTON_VOLSCALETRACK:
			{
				if (event->getID() != eCommand)
					break;

				tracker.toolInvokeHelper->invokeTool(ToolInvokeHelper::ToolTypeTrackVolumeScale);
				break;
			}

			case ADVEDIT_BUTTON_VOLSCALEPATTERN:
			{
				if (event->getID() != eCommand)
					break;

				tracker.toolInvokeHelper->invokeTool(ToolInvokeHelper::ToolTypePatternVolumeScale);
				break;
			}

			case ADVEDIT_BUTTON_VOLSCALEBLOCK:
			{
				if (event->getID() != eCommand)
					break;

				tracker.toolInvokeHelper->invokeTool(ToolInvokeHelper::ToolTypeSelectionVolumeScale);
				break;
			}
			
			case ADVEDIT_BUTTON_SPLITTRACKPLUS:
			{
				splitTrackNumSubsequentChannels++;

				if (splitTrackNumSubsequentChannels >= (signed)tracker.moduleEditor->getNumChannels())
					splitTrackNumSubsequentChannels = (signed)tracker.moduleEditor->getNumChannels()-1;
				if (splitTrackNumSubsequentChannels < 1)
					splitTrackNumSubsequentChannels = 1;

				update();				
				break;
			}

			case ADVEDIT_BUTTON_SPLITTRACKMINUS:
			{
				splitTrackNumSubsequentChannels--;
				
				if (splitTrackNumSubsequentChannels < 1)
					splitTrackNumSubsequentChannels = 1;
				
				update();
				break;
			}
			
			case ADVEDIT_BUTTON_SPLITTRACK:
			{
				if (event->getID() != eCommand)
					break;

				pp_int32 res = tracker.getPatternEditor()->splitTrack(splitTrackNumSubsequentChannels, 
																	  checkBoxSplitTrack->isChecked(),
																	  checkBoxSplitTrackNoteOff->isChecked());
				if (res>=0)
					screen->paint();
				else
					tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, "Please select only within one track.", Tracker::MessageBox_OK);

				break;
			}

			case ADVEDIT_BUTTON_CONVERT:
			{
				if (event->getID() != eCommand)
					break;

				pp_int32 res = 0;
				switch (static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(ADVEDIT_RADIO_CONVERT))->getChoice())
				{
					case 0:
						res = tracker.moduleEditor->panConvertSong(ModuleEditor::PanConversionTypeConvert_E8x);
						break;
					case 1:
						res = tracker.moduleEditor->panConvertSong(ModuleEditor::PanConversionTypeConvert_80x);
						break;
				}	

				if (res)
					sprintf(buffer, "%i commands have been converted", res);
				else
					sprintf(buffer, "Nothing to do");
				
				tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, buffer, Tracker::MessageBox_OK);

				break;
			}
				
			case ADVEDIT_BUTTON_REMOVE:
			{
				pp_int32 res = 0;
				switch (static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(ADVEDIT_RADIO_REMOVE))->getChoice())
				{
					case 0:
						res = tracker.moduleEditor->panConvertSong(ModuleEditor::PanConversionTypeRemove_E8x);
						break;
					case 1:
						res = tracker.moduleEditor->panConvertSong(ModuleEditor::PanConversionTypeRemove_8xx);
						break;
				}	

				if (res)
					sprintf(buffer, "%i commands have been erased", res);
				else
					sprintf(buffer, "Nothing to do");
				
				tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, buffer, Tracker::MessageBox_OK);
	
				break;
			}
			
			case ADVEDIT_BUTTON_EXIT:
				if (event->getID() != eCommand)
					break;

				show(false);
				break;
		}
		
	}

	return 0;
}

void SectionAdvancedEdit::init(pp_int32 px, pp_int32 py)
{
	PPScreen* screen = tracker.screen;

	PPContainer* container = new PPContainer(CONTAINER_ADVEDIT, tracker.screen, this, PPPoint(px, py), PPSize(320,UPPERLEFTSECTIONHEIGHT), false);
	container->setColor(TrackerConfig::colorThemeMain);	
	tracker.screen->addControl(container);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(px + 2, py + 2), "Advanced editing", true, true));

	PPSize size = container->getSize();

	pp_int32 buttonWidth = 8*4+4;
	pp_int32 buttonHeight = 11;
	
	pp_int32 x = px+container->getSize().width-(buttonWidth+4);
	pp_int32 y = py+container->getSize().height-(buttonHeight+4);
	
	container->addControl(new PPSeperator(0, screen, PPPoint(x - 6, y - 5), 5 + buttonHeight + 3, TrackerConfig::colorThemeMain, false));
	container->addControl(new PPSeperator(0, screen, PPPoint(px+2, y - 5), container->getSize().width-5, container->getColor(), true));	

	PPButton* button = new PPButton(ADVEDIT_BUTTON_EXIT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Exit");
	container->addControl(button);

	x = px+4;
	y = py+4+12;
	
	buttonWidth = 76;
	button = new PPButton(ADVEDIT_BUTTON_INSREMAP, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Remap Ins");
	container->addControl(button);

	x+=button->getSize().width+1;

	buttonWidth = 68;
	button = new PPButton(ADVEDIT_BUTTON_INTERPOLATE, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Interpol");
	container->addControl(button);

	x+=button->getSize().width+2;

	container->addControl(new PPSeperator(0, screen, PPPoint(x, py), 30, container->getColor(), false));	
	x+=4;
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x, py + 2), "Volume scale", true, true));

	x+=1;
	
	buttonWidth = (container->getSize().width - 5 - (x-px)) / 3;
	
	button = new PPButton(ADVEDIT_BUTTON_VOLSCALETRACK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Track");
	container->addControl(button);
	x+=button->getSize().width+1;

	button = new PPButton(ADVEDIT_BUTTON_VOLSCALEPATTERN, screen, this, PPPoint(x, y), PPSize(buttonWidth-1,buttonHeight));
	button->setText("Patt");
	container->addControl(button);
	x+=button->getSize().width+1;

	button = new PPButton(ADVEDIT_BUTTON_VOLSCALEBLOCK, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Block");
	container->addControl(button);

	y+=button->getSize().height+3;

	x = px;

	container->addControl(new PPSeperator(0, screen, PPPoint(x+2, y), size.width-5, container->getColor(), true));	

	pp_int32 y3 = y+1;
	
	y+=3;
	
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y + 2), "Split track options:", true));	

	y+=12;
	
	container->addControl(new PPStaticText(ADVEDIT_TEXT_SPLITTRACK, NULL, NULL, PPPoint(x + 2, y + 2), "Use subsequent 00 channels", true));		

	button = new PPButton(ADVEDIT_BUTTON_SPLITTRACKPLUS, screen, this, PPPoint(x+214, y), PPSize(11, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);

	button = new PPButton(ADVEDIT_BUTTON_SPLITTRACKMINUS, screen, this, PPPoint(x+214+12, y), PPSize(11, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

	y+=12;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y + 2), "Block only", true));		

	checkBoxSplitTrack = new PPCheckBox(ADVEDIT_CHECKBOX_SPLITTRACK, screen, this, PPPoint(x + 11*8, y));
	checkBoxSplitTrack->checkIt(false);
	container->addControl(checkBoxSplitTrack);
	
	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 2 + 12*8 + 4, y + 2), "Insert note off", true));		

	checkBoxSplitTrackNoteOff = new PPCheckBox(ADVEDIT_CHECKBOX_SPLITTRACKNOTEOFF, screen, this, PPPoint(x + 28*8 + 3, y));
	checkBoxSplitTrackNoteOff->checkIt(false);
	container->addControl(checkBoxSplitTrackNoteOff);
	

	y+=12;

	container->addControl(new PPSeperator(0, screen, PPPoint(x+2, y), size.width-5, container->getColor(), true));	

	container->addControl(new PPSeperator(0, screen, PPPoint(x + 240, y3), y-y3, TrackerConfig::colorThemeMain, false));

	button = new PPButton(ADVEDIT_BUTTON_SPLITTRACK, screen, this, PPPoint(x + 253, y3 + 10), PPSize(54,20));
	button->setText("Split");
	container->addControl(button);

	y+=3;

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 2, y + 2), "Panning conversion (entire song only)", true));	

	y+=13;

	PPRadioGroup* radioGroup = new PPRadioGroup(ADVEDIT_RADIO_CONVERT, screen, this, PPPoint(x, y-3), PPSize(80, 14));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->setHorizontal(true);
	radioGroup->addItem("E8x");
	radioGroup->addItem("80x");

	container->addControl(radioGroup);
	
	x+=radioGroup->getSize().width+1+3;
	
	buttonWidth = 8*8+4;
	button = new PPButton(ADVEDIT_BUTTON_CONVERT, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Convert");
	container->addControl(button);
	x+=button->getSize().width;

	x+=12;
	radioGroup = new PPRadioGroup(ADVEDIT_RADIO_REMOVE, screen, this, PPPoint(x, y-3), PPSize(80, 14));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->setHorizontal(true);
	radioGroup->addItem("E8x");
	radioGroup->addItem("8xx");

	container->addControl(radioGroup);
	
	x+=radioGroup->getSize().width+1+3;
	
	buttonWidth = 8*8+4;
	button = new PPButton(ADVEDIT_BUTTON_REMOVE, screen, this, PPPoint(x, y), PPSize(buttonWidth,buttonHeight));
	button->setText("Remove");
	container->addControl(button);
	x+=button->getSize().width+1;

	sectionContainer = container;

	initialised = true;

	showSection(false);
}

void SectionAdvancedEdit::update(bool repaint/* = true*/)
{
	char buffer[80];

	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	
	ASSERT(sectionContainer);
	
	PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(ADVEDIT_TEXT_SPLITTRACK));

	sprintf(buffer, "Use subsequent %02i channels", splitTrackNumSubsequentChannels);
	
	text->setText(buffer);

	if (repaint)
		tracker.screen->paintControl(container);
}
