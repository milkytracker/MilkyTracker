/*
 *  tracker/SectionTranspose.cpp
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
 *  SectionTranspose.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.04.05.
 *
 */

#include "SectionTranspose.h"
#include "Tracker.h"
#include "TrackerConfig.h"
#include "ModuleEditor.h"

#include "RadioGroup.h"
#include "Seperator.h"
#include "StaticText.h"
#include "Container.h"
#include "PatternEditorControl.h"

#include "DialogBase.h"

#include "PatternTools.h"
#include "Tools.h"
#include "ControlIDs.h"

enum ControlIDs 
{
	TRANSPOSE_BUTTON_INS_PLUS	= 31100,
	TRANSPOSE_BUTTON_INS_MINUS,
	TRANSPOSE_BUTTON_INS_RANGESTART_PLUS,
	TRANSPOSE_BUTTON_INS_RANGESTART_MINUS,
	TRANSPOSE_BUTTON_INS_RANGEEND_PLUS,
	TRANSPOSE_BUTTON_INS_RANGEEND_MINUS,
	TRANSPOSE_BUTTON_NOTE_PLUS,
	TRANSPOSE_BUTTON_NOTE_MINUS,
	TRANSPOSE_BUTTON_NOTE_RANGESTART_PLUS,
	TRANSPOSE_BUTTON_NOTE_RANGESTART_MINUS,
	TRANSPOSE_BUTTON_NOTE_RANGEEND_PLUS,
	TRANSPOSE_BUTTON_NOTE_RANGEEND_MINUS,
	TRANSPOSE_BUTTON_AMOUNT_PLUS,
	TRANSPOSE_BUTTON_AMOUNT_MINUS,
	TRANSPOSE_BUTTON_AMOUNT_NOTEUP,
	TRANSPOSE_BUTTON_AMOUNT_NOTEDOWN,
	TRANSPOSE_BUTTON_AMOUNT_OCTAVEUP,
	TRANSPOSE_BUTTON_AMOUNT_OCTAVEDOWN,
	TRANSPOSE_BUTTON_USER1,
	TRANSPOSE_BUTTON_USER2,
	TRANSPOSE_BUTTON_USER3,
	TRANSPOSE_BUTTON_USER4,
	TRANSPOSE_BUTTON_EXIT,

	TRANSPOSE_TEXT_INS,
	TRANSPOSE_TEXT_INS_RANGESTART,
	TRANSPOSE_TEXT_INS_RANGEEND,
	TRANSPOSE_TEXT_NOTE,
	TRANSPOSE_TEXT_NOTE_RANGESTART,
	TRANSPOSE_TEXT_NOTE_RANGEEND,
	TRANSPOSE_TEXT_AMOUNT,
	TRANSPOSE_TEXT_AMOUNT2
};

// Class which responds to message box clicks
class DialogResponderTranspose : public DialogResponder
{
private:
	SectionTranspose& section;
	
public:
	DialogResponderTranspose(SectionTranspose& section) :
		section(section)
	{
	}
	
	virtual pp_int32 ActionOkay(PPObject* sender)
	{
		switch (reinterpret_cast<PPDialogBase*>(sender)->getID())
		{
			case MESSAGEBOX_TRANSPOSEPROCEED:
				section.transposeSong();
				break;
		}
		return 0;
	}
};

SectionTranspose::SectionTranspose(Tracker& theTracker) :
	SectionUpperLeft(theTracker, NULL, new DialogResponderTranspose(*this))
{
	currentInstrumentRangeStart = 0;
	currentInstrumentRangeEnd = 0;
	currentTransposeAmount = 1;
	
	currentNote = currentNoteRangeStart = currentNoteRangeEnd = 48;
}

SectionTranspose::~SectionTranspose()
{
}

void SectionTranspose::setCurrentInstrument(pp_int32 instrument, bool redraw/* = true*/) 
{ 
	instrument--;

	currentInstrument = currentInstrumentRangeStart = currentInstrumentRangeEnd = instrument; 
	
	if (sectionContainer->isVisible())
	{
		update(redraw);
	}
}

pp_int32 SectionTranspose::handleEvent(PPObject* sender, PPEvent* event)
{	
	PPRadioGroup* radioGroup;

	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case TRANSPOSE_BUTTON_INS_PLUS:
			{
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(2));
				radioGroup->setChoice(0);

				if (currentInstrument < 254)
					currentInstrument++;
				update();
				break;
			}

			case TRANSPOSE_BUTTON_INS_MINUS:
			{
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(2));
				radioGroup->setChoice(0);

				if (currentInstrument > 0)
					currentInstrument--;
				update();
				break;
			}
			
			case TRANSPOSE_BUTTON_INS_RANGESTART_PLUS:
			{				
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(2));
				radioGroup->setChoice(2);

				if (currentInstrumentRangeStart < 254)
					currentInstrumentRangeStart++;
				if (currentInstrumentRangeEnd < currentInstrumentRangeStart)
					currentInstrumentRangeStart = currentInstrumentRangeEnd;
				update();
				break;
			}

			case TRANSPOSE_BUTTON_INS_RANGESTART_MINUS:
			{				
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(2));
				radioGroup->setChoice(2);

				if (currentInstrumentRangeStart > 0)
					currentInstrumentRangeStart--;
				if (currentInstrumentRangeStart > currentInstrumentRangeEnd)
					currentInstrumentRangeStart = currentInstrumentRangeEnd;
				update();
				break;
			}

			case TRANSPOSE_BUTTON_INS_RANGEEND_PLUS:
			{				
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(2));
				radioGroup->setChoice(2);

				if (currentInstrumentRangeEnd < 254)
					currentInstrumentRangeEnd++;
				if (currentInstrumentRangeStart > currentInstrumentRangeEnd)
					currentInstrumentRangeStart = currentInstrumentRangeEnd;
				update();
				break;
			}

			case TRANSPOSE_BUTTON_INS_RANGEEND_MINUS:
			{				
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(2));
				radioGroup->setChoice(2);

				if (currentInstrumentRangeEnd > 0)
					currentInstrumentRangeEnd--;
				if (currentInstrumentRangeEnd < currentInstrumentRangeStart)
					currentInstrumentRangeEnd = currentInstrumentRangeStart;
				update();
				break;
			}

			// note range
			case TRANSPOSE_BUTTON_NOTE_PLUS:
			{
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(3));
				radioGroup->setChoice(0);

				if (currentNote < 119)
					currentNote++;
				update();
				break;
			}

			case TRANSPOSE_BUTTON_NOTE_MINUS:
			{
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(3));
				radioGroup->setChoice(0);

				if (currentNote > 0)
					currentNote--;
				update();
				break;
			}
			
			case TRANSPOSE_BUTTON_NOTE_RANGESTART_PLUS:
			{				
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(3));
				radioGroup->setChoice(2);

				if (currentNoteRangeStart < 119)
					currentNoteRangeStart++;
				if (currentNoteRangeEnd < currentNoteRangeStart)
					currentNoteRangeStart = currentNoteRangeEnd;
				update();
				break;
			}

			case TRANSPOSE_BUTTON_NOTE_RANGESTART_MINUS:
			{				
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(3));
				radioGroup->setChoice(2);

				if (currentNoteRangeStart > 0)
					currentNoteRangeStart--;
				if (currentNoteRangeStart > currentNoteRangeEnd)
					currentNoteRangeStart = currentNoteRangeEnd;
				update();
				break;
			}

			case TRANSPOSE_BUTTON_NOTE_RANGEEND_PLUS:
			{				
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(3));
				radioGroup->setChoice(2);

				if (currentNoteRangeEnd < 119)
					currentNoteRangeEnd++;
				if (currentNoteRangeStart > currentNoteRangeEnd)
					currentNoteRangeStart = currentNoteRangeEnd;
				update();
				break;
			}

			case TRANSPOSE_BUTTON_NOTE_RANGEEND_MINUS:
			{				
				radioGroup = static_cast<PPRadioGroup*>(static_cast<PPContainer*>(sectionContainer)->getControlByID(3));
				radioGroup->setChoice(2);

				if (currentNoteRangeEnd > 0)
					currentNoteRangeEnd--;
				if (currentNoteRangeEnd < currentNoteRangeStart)
					currentNoteRangeEnd = currentNoteRangeStart;
				update();
				break;
			}
			
			case TRANSPOSE_BUTTON_AMOUNT_PLUS:
			{
				if (currentTransposeAmount < 48)
					currentTransposeAmount ++;
					
				if (currentTransposeAmount == 0)
					currentTransposeAmount = 1;

				update();
				break;
			}

			case TRANSPOSE_BUTTON_AMOUNT_MINUS:
			{
				if (currentTransposeAmount > -48)
					currentTransposeAmount --;
					
				if (currentTransposeAmount == 0)
					currentTransposeAmount = -1;

				update();
				break;
			}
			
			case TRANSPOSE_BUTTON_AMOUNT_NOTEUP:
				currentTransposeAmount = 1;
				update();
				break;
				
			case TRANSPOSE_BUTTON_AMOUNT_NOTEDOWN: 
				currentTransposeAmount = -1;
				update();
				break;

			case TRANSPOSE_BUTTON_AMOUNT_OCTAVEUP:
				currentTransposeAmount = 12;
				update();
				break;

			case TRANSPOSE_BUTTON_AMOUNT_OCTAVEDOWN:
				currentTransposeAmount = -12;
				update();
				break;
			
			case TRANSPOSE_BUTTON_EXIT:
				if (event->getID() != eCommand)
					break;

				show(false);
				break;
			
			case TRANSPOSE_BUTTON_USER1:
			case TRANSPOSE_BUTTON_USER2:
			case TRANSPOSE_BUTTON_USER3:
			case TRANSPOSE_BUTTON_USER4:
			{
				//tracker.ensureSongStopped();
				PPContainer* container = static_cast<PPContainer*>(sectionContainer);
				
				pp_int32 res;
				
				PPRadioGroup* radioGroupIns = static_cast<PPRadioGroup*>(container->getControlByID(2));
				PPRadioGroup* radioGroupNote = static_cast<PPRadioGroup*>(container->getControlByID(3));
				
				switch (radioGroupIns->getChoice())
				{
					case 0:
						tp.insRangeStart = currentInstrument + 1;
						tp.insRangeEnd = currentInstrument + 1;
						break;
					case 1:
						tp.insRangeStart = 0;
						tp.insRangeEnd = 255;
						break;
					case 2:
						tp.insRangeStart = currentInstrumentRangeStart + 1;
						tp.insRangeEnd = currentInstrumentRangeEnd + 1;
						break;
				}
				
				switch (radioGroupNote->getChoice())
				{
					case 0:
						tp.noteRangeStart = currentNote + 1;
						tp.noteRangeEnd = currentNote + 1;
						break;
					case 1:
						tp.noteRangeStart = 1;
						tp.noteRangeEnd = 96;
						break;
					case 2:
						tp.noteRangeStart = currentNoteRangeStart + 1;
						tp.noteRangeEnd = currentNoteRangeEnd + 1;
						break;
				}
				
				tp.amount = currentTransposeAmount;
				
				switch (reinterpret_cast<PPControl*>(sender)->getID())
				{
					case TRANSPOSE_BUTTON_USER1:
						res = tracker.getPatternEditorControl()->noteTransposeTrack(tp);
						break;
					case TRANSPOSE_BUTTON_USER2:
						res = tracker.getPatternEditorControl()->noteTransposePattern(tp);
						break;
					case TRANSPOSE_BUTTON_USER3:
					{
						handleTransposeSong();
						break;
					}					
					case TRANSPOSE_BUTTON_USER4:
						res = tracker.getPatternEditorControl()->noteTransposeSelection(tp);
						break;
				}
				
				//char buffer[100];
				//sprintf(buffer, "%i Notes have been transposed", res);
				//tracker.showMessageBox(MESSAGEBOX_UNIVERSAL, buffer, Tracker::MessageBox_OK);
				
				tracker.screen->paint();
				//tracker.ensureSongPlaying();
				return false;
			}
		}
		
	}

	return 0;
}

void SectionTranspose::init(pp_int32 px, pp_int32 py)
{
	pp_int32 i;

	PPScreen* screen = tracker.screen;

	// test
	PPContainer* container = new PPContainer(CONTAINER_TRANSPOSE, tracker.screen, this, PPPoint(px, py), PPSize(320,UPPERLEFTSECTIONHEIGHT), false);
	container->setColor(TrackerConfig::colorThemeMain);	
	tracker.screen->addControl(container);

	const pp_int32 buttonIDs[4] =  {TRANSPOSE_BUTTON_AMOUNT_NOTEUP, 
									TRANSPOSE_BUTTON_AMOUNT_NOTEDOWN, 
									TRANSPOSE_BUTTON_AMOUNT_OCTAVEUP, 
									TRANSPOSE_BUTTON_AMOUNT_OCTAVEDOWN};

	const char* buttonTexts[4] =  {"Note Up",
								   "Note Dn",
								   "Octave Up",
								   "Octave Dn"};

	const pp_int32 buttonIDs2[4] = {TRANSPOSE_BUTTON_USER1, 
									TRANSPOSE_BUTTON_USER2, 
									TRANSPOSE_BUTTON_USER3, 
									TRANSPOSE_BUTTON_USER4};

	const char* buttonTexts2[4] = {"Track",
								   "Pattern",
								   "Song",
								   "Block"};
	PPSize size = container->getSize();

	/*tracker.showMessageBox(MESSAGEBOX_TRANSPOSE, "Filter options", Tracker::MessageBox_OK, false);
	
	PPMessageBoxContainer* container = static_cast<PPMessageBoxContainer*>(screen->getModalControl());
	
	container->setCaption("Transpose");
	
	container->removeControl(container->getControlByID(MESSAGEBOX_BUTTON_YES));
	
	PPSize size = container->getSize();
	
	size.height = 216;
	
	PPPoint location = container->getLocation();
	
	location.y = screen->getHeight() / 2 - size.height / 2;
	
	container->setLocation(location);
	container->setSize(size);
	
	PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(1));
	ASSERT(text);
	
	text->setUnderlined(true);
	
	PPPoint location2 = text->getLocation();
	
	location2.y = location.y + 20;
	location2.x = location.x + 6;
	
	text->setLocation(location2);
	
	location2 = location;*/
	
	PPStaticText* text;
	
	PPPoint location(px,py);
	
	PPPoint location2 = location;
	
	PPRadioGroup* radioGroup;
	PPButton* button;
	
	// ----------------------- Instrument ----------------------
	location2.x += 2;
	location2.y += 2;
	
	text = new PPStaticText(0, NULL, NULL, location2, "Instrument:", true);
	container->addControl(text);		
	
	location2.y += 10;
	
	PPPoint temp = location2;
	
	radioGroup = new PPRadioGroup(2, screen, this, PPPoint(location2.x, location2.y-2), PPSize(8*9, 42));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->addItem("Single:");
	radioGroup->addItem("All");
	radioGroup->addItem("Range:");

	radioGroup->setChoice(1);

	container->addControl(radioGroup);		

	// plus minus
	pp_int32 h = location2.x + 8*9;
	pp_int32 hy = location2.y + 3;
	
	container->addControl(new PPStaticText(TRANSPOSE_TEXT_INS, NULL, NULL, PPPoint(h,hy), "xx", false));
	
	h+=8*2+4;
	
	button = new PPButton(TRANSPOSE_BUTTON_INS_PLUS, screen, this, PPPoint(h, hy), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	h+=button->getSize().width;
	button = new PPButton(TRANSPOSE_BUTTON_INS_MINUS, screen, this, PPPoint(h, hy), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);
	

	location2.y += radioGroup->getSize().height;
	//location2.x +=6*8 + 10;
	
	location2.x+=8*2-2;
	
	h = location2.x;
	
	text = new PPStaticText(TRANSPOSE_TEXT_INS_RANGESTART, NULL, NULL, location2, "From:xx", false);
	container->addControl(text);			

	location2.x += 8*8-4;
	button = new PPButton(TRANSPOSE_BUTTON_INS_RANGESTART_PLUS, screen, this, location2, PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	location2.x+=button->getSize().width;
	button = new PPButton(TRANSPOSE_BUTTON_INS_RANGESTART_MINUS, screen, this, location2, PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);
	//location2.x+=button->getSize().width + 2;

	location2.x = h;
	location2.y+=11;

	text = new PPStaticText(TRANSPOSE_TEXT_INS_RANGEEND, NULL, NULL, location2, "To:  xx", false);
	container->addControl(text);			
	location2.x+=8*8-4;

	button = new PPButton(TRANSPOSE_BUTTON_INS_RANGEEND_PLUS, screen, this, location2, PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	location2.x+=button->getSize().width;
	button = new PPButton(TRANSPOSE_BUTTON_INS_RANGEEND_MINUS, screen, this, location2, PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

	// ---------------- notes ---------------------
	location2.x += button->getSize().width + 26;
	location2.y = location.y + 2;
	
	container->addControl(new PPSeperator(0, screen, PPPoint(location2.x-6, location.y+2), (size.height)-42, container->getColor(), false));
	
	text = new PPStaticText(0, NULL, NULL, location2, "Note:", true);
	container->addControl(text);		
	
	location2.y += 10;
	
	temp = location2;
	
	radioGroup = new PPRadioGroup(3, screen, this, PPPoint(location2.x, location2.y-2), PPSize(8*9, 42));
	radioGroup->setColor(TrackerConfig::colorThemeMain);

	radioGroup->addItem("Single:");
	radioGroup->addItem("All");
	radioGroup->addItem("Range:");

	radioGroup->setChoice(1);

	container->addControl(radioGroup);		

	// plus minus
	h = location2.x + 8*9;
	hy = location2.y + 3;
	
	container->addControl(new PPStaticText(TRANSPOSE_TEXT_NOTE, NULL, NULL, PPPoint(h,hy), "xxx", false));
	
	h+=8*3+4;
	
	button = new PPButton(TRANSPOSE_BUTTON_NOTE_PLUS, screen, this, PPPoint(h, hy), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	h+=button->getSize().width;
	button = new PPButton(TRANSPOSE_BUTTON_NOTE_MINUS, screen, this, PPPoint(h, hy), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

	location2.y += radioGroup->getSize().height;
	//location2.x +=6*8 + 10;
	
	location2.x+=8*2-2;
	
	h = location2.x;
	
	text = new PPStaticText(TRANSPOSE_TEXT_NOTE_RANGESTART, NULL, NULL, location2, "From:xxx", false);
	container->addControl(text);			

	location2.x += 8*9-4;
	button = new PPButton(TRANSPOSE_BUTTON_NOTE_RANGESTART_PLUS, screen, this, location2, PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	location2.x+=button->getSize().width;
	button = new PPButton(TRANSPOSE_BUTTON_NOTE_RANGESTART_MINUS, screen, this, location2, PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);
	//location2.x+=button->getSize().width + 2;

	location2.x = h;
	location2.y+=11;

	text = new PPStaticText(TRANSPOSE_TEXT_NOTE_RANGEEND, NULL, NULL, location2, "To:  xxx", false);
	container->addControl(text);			
	location2.x+=8*9-4;

	button = new PPButton(TRANSPOSE_BUTTON_NOTE_RANGEEND_PLUS, screen, this, location2, PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	location2.x+=button->getSize().width;
	button = new PPButton(TRANSPOSE_BUTTON_NOTE_RANGEEND_MINUS, screen, this, location2, PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);

	// horizontal ruler
	location2.y += button->getSize().height+5;

	container->addControl(new PPSeperator(0, screen, PPPoint(location.x+2, location2.y), size.width-5, container->getColor(), true));	
	
	location2.x += button->getSize().width + 26;
	location2.y = location.y + 4;
	
	container->addControl(new PPSeperator(0, screen, PPPoint(location2.x-6, location.y+2), (size.height)-42, container->getColor(), false));

	const PPString str = "Amount:";

	location2.y-=2;
	
	text = new PPStaticText(6, NULL, NULL, location2, str, true);
	container->addControl(text);			
	
	const PPString str2 = "-xx";

	location2.y+=13;
	location2.x+=8;

	text = new PPStaticText(TRANSPOSE_TEXT_AMOUNT, NULL, NULL, location2, str2, false);
	container->addControl(text);			

	h = location2.x+3*8+4;
	hy = location2.y;

	button = new PPButton(TRANSPOSE_BUTTON_AMOUNT_PLUS, screen, this, PPPoint(h, hy), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonPlus);
	container->addControl(button);
	h+=button->getSize().width;
	button = new PPButton(TRANSPOSE_BUTTON_AMOUNT_MINUS, screen, this, PPPoint(h, hy), PPSize(10, 9));
	button->setText(TrackerConfig::stringButtonMinus);
	container->addControl(button);	

	hy+=11;
	h-=5*8;
	text = new PPStaticText(TRANSPOSE_TEXT_AMOUNT2, NULL, NULL, PPPoint(h+1, hy), "note(s)", true);
	container->addControl(text);			
	
	// preset buttons
	pp_int32 buttonWidth = 7*8+4;
	pp_int32 buttonHeight = 9;
	pp_int32 spacer = 10;
	
	pp_int32 y2 = hy+11;
	pp_int32 x = h-3;
	
	for (i = 0; i < 4; i++)
	{
		PPButton* button = new PPButton(buttonIDs[i], screen, this, PPPoint(x, y2), PPSize(buttonWidth, buttonHeight));
		button->setText(buttonTexts[i]);
		button->setFont(PPFont::getFont(PPFont::FONT_TINY));
		container->addControl(button);
		
		y2+= buttonHeight+1;
	}
	
	y2+=10;

	//container->addControl(new PPSeperator(0, screen, PPPoint(location.x+2, y2), size.width-5, container->getColor(), true));

	const PPString str3 = "Apply:";

	location2.x = location.x+6;
	location2.y = y2;
	
	text = new PPStaticText(6, NULL, NULL, location2, str3, true);
	container->addControl(text);			
	
	buttonWidth = 60;
	
	//y2+=14;

	spacer = 4;
	buttonHeight = 14;
	pp_int32 cx = ((buttonWidth) * 4 + (spacer*3))/2;
	x = location.x + (size.width-6*8)/2 - cx + 6*8 + 2;
	y2-=3;
	
	for (i = 0; i < 4; i++)
	{
		PPButton* button = new PPButton(buttonIDs2[i], screen, this, PPPoint(x, y2), PPSize(buttonWidth, buttonHeight));
		button->setText(buttonTexts2[i]);
		container->addControl(button);
		
		x+= buttonWidth + spacer;
	}

	y2 += buttonHeight+2;

	container->addControl(new PPSeperator(0, screen, PPPoint(location.x+2, y2), size.width-5, container->getColor(), true));	
	
	y2+=4;
	buttonWidth = 8*4+4;
	buttonHeight = 11;
	button = new PPButton(TRANSPOSE_BUTTON_EXIT, screen, this, PPPoint(x-buttonWidth-spacer, y2), PPSize(buttonWidth,buttonHeight));
	button->setText("Exit");
	container->addControl(button);
	
	sectionContainer = container;

	initialised = true;

	showSection(false);
}

void SectionTranspose::show(bool bShow)
{
	if (!initialised)
		currentNote = currentNoteRangeStart = currentNoteRangeEnd = 48;

	SectionUpperLeft::show(bShow);
}

void SectionTranspose::update(bool repaint/* = true*/)
{
	char buffer[80];

	PPContainer* container = static_cast<PPContainer*>(sectionContainer);
	
	ASSERT(sectionContainer);
	
	PPStaticText* text = static_cast<PPStaticText*>(container->getControlByID(TRANSPOSE_TEXT_INS));

	text->setHexValue(currentInstrument+1,2);

	text = static_cast<PPStaticText*>(container->getControlByID(TRANSPOSE_TEXT_INS_RANGESTART));

	strcpy(buffer, "From:");
	
	PPTools::convertToHex(buffer+5, currentInstrumentRangeStart+1, 2);
	
	text->setText(buffer);

	text = static_cast<PPStaticText*>(container->getControlByID(TRANSPOSE_TEXT_INS_RANGEEND));

	strcpy(buffer, "To:  ");
	
	PPTools::convertToHex(buffer+5, currentInstrumentRangeEnd+1, 2);
	
	text->setText(buffer);

	text = static_cast<PPStaticText*>(container->getControlByID(TRANSPOSE_TEXT_NOTE));

	PatternTools::getNoteName(buffer, currentNote+1);
	
	text->setText(buffer);

	text = static_cast<PPStaticText*>(container->getControlByID(TRANSPOSE_TEXT_NOTE_RANGESTART));

	strcpy(buffer, "From:");
	PatternTools::getNoteName(buffer+5, currentNoteRangeStart+1);	
	text->setText(buffer);

	text = static_cast<PPStaticText*>(container->getControlByID(TRANSPOSE_TEXT_NOTE_RANGEEND));

	strcpy(buffer, "To  :");
	PatternTools::getNoteName(buffer+5, currentNoteRangeEnd+1);	
	text->setText(buffer);

	text = static_cast<PPStaticText*>(container->getControlByID(TRANSPOSE_TEXT_AMOUNT));
	
	sprintf(buffer, "%i", currentTransposeAmount);

	text->setText(buffer);

	text = static_cast<PPStaticText*>(container->getControlByID(TRANSPOSE_TEXT_AMOUNT2));
	
	text->setText(currentTransposeAmount < 0 ? "note(s)" : "note(s)");
	
	tracker.screen->paintControl(container);
}

void SectionTranspose::handleTransposeSong()
{
	pp_int32 fuckups = tracker.moduleEditor->noteTransposeSong(tp, true);
	if (!fuckups)
		tracker.moduleEditor->noteTransposeSong(tp);
	else
	{
		char buffer[100];
		sprintf(buffer, "%i notes will be erased, continue?", fuckups);
		showMessageBox(MESSAGEBOX_TRANSPOSEPROCEED, buffer);
	}
}

void SectionTranspose::transposeSong()
{
	tracker.moduleEditor->noteTransposeSong(getTransposeParameters());
	tracker.screen->paint();				
}
