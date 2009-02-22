/*
 *  tracker/SectionAbstract.cpp
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
 *  SectionAbstract.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 06.01.06.
 *
 */

#include "SectionAbstract.h"
#include "ControlIDs.h"
#include "Control.h"
#include "Container.h"
#include "ListBox.h"
#include "Tracker.h"
#include "DialogBase.h"
#include "SectionSwitcher.h"

SectionAbstract::~SectionAbstract()
{
	delete responder;
	delete dialog;
}

void SectionAbstract::showMessageBox(pp_uint32 id, const PPString& text, bool yesnocancel/* = false*/)
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}

	dialog = new PPDialogBase(tracker.screen, responder, 
							  id, text, 
							  yesnocancel ? 
							  PPDialogBase::MessageBox_YESNOCANCEL :
							  PPDialogBase::MessageBox_OKCANCEL); 	
											  
	dialog->show();
}

#ifdef __LOWRES__
void SectionAbstract::replaceAndResizeInstrumentListContainer(pp_int32 listBoxContainerHeight)
{
	PPScreen* screen = tracker.screen;
	PPContainer* ctrl = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INSTRUMENTLIST));

	ctrl->getControlByID(BUTTON_INSTRUMENT)->setLocation(PPPoint(3,1));
	ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->setLocation(PPPoint(160+3,3));
	ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->setLocation(PPPoint(97,2));
	ctrl->getControlByID(BUTTON_INSTRUMENTS_MINUS)->setLocation(PPPoint(97 + ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->getSize().width+1,2));
	
	PPSize size = tracker.listBoxInstruments->getSize();
	size.height = listBoxContainerHeight - 15;
	size.width = screen->getWidth() / 2 - 4;
	tracker.listBoxInstruments->setSize(size);
	
	PPPoint p = tracker.listBoxInstruments->getLocation();
	p.x = 2;
	p.y = 13;
	tracker.listBoxInstruments->setLocation(p);
	
	size = tracker.listBoxSamples->getSize();
	size.height = listBoxContainerHeight - 15;
	size.width = screen->getWidth() / 2 - 4;
	tracker.listBoxSamples->setSize(size);
	
	p = tracker.listBoxSamples->getLocation();
	p.x = 2 + (screen->getWidth() / 2);
	p.y = 13;
	tracker.listBoxSamples->setLocation(p);
	// samples listbox always visible
	tracker.listBoxSamples->hide(false);
	
	size = ctrl->getSize();
	size.height = listBoxContainerHeight;
	size.width = screen->getWidth();
	ctrl->setSize(size);
	
	ctrl->setLocation(PPPoint(0,0));
}

void SectionAbstract::replaceInstrumentListBoxes(bool b, pp_int32 listBoxContainerHeight/* = REPLACEDINSTRUMENTLISTBOXESHEIGHT*/)
{
	PPScreen* screen = tracker.screen;
	
	if (b)
	{
		tracker.showMainMenu(false, true);
		tracker.sectionSwitcher->showSubMenu(SectionSwitcher::ActiveLowerSectionPageInstruments, false);

		screen->getControlByID(CONTAINER_LOWRES_TINYMENU)->show(false);
		
		PPContainer* ctrl = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INSTRUMENTLIST));
		
		oldInstrumentListContainerSize = ctrl->getSize();
		oldInstrumentListContainerLocation = ctrl->getLocation();
		
		oldInstrumentListSize = tracker.listBoxInstruments->getSize();
		oldInstrumentListLocation = tracker.listBoxInstruments->getLocation();
		oldSampleListSize = tracker.listBoxSamples->getSize();
		oldSampleListLocation = tracker.listBoxSamples->getLocation();

		oldControlLocations[0] = ctrl->getControlByID(BUTTON_INSTRUMENT)->getLocation();
		oldControlLocations[1] = ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->getLocation();
		oldControlLocations[2] = ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->getLocation();
		oldControlLocations[3] = ctrl->getControlByID(BUTTON_INSTRUMENTS_MINUS)->getLocation();

		visibility[0] = ctrl->getControlByID(LISTBOX_INSTRUMENTS)->isHidden();
		visibility[1] = ctrl->getControlByID(LISTBOX_SAMPLES)->isHidden();
		visibility[2] = ctrl->getControlByID(BUTTON_INSTRUMENT)->isHidden();
		visibility[3] = ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->isHidden();
		visibility[4] = ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->isHidden();
		visibility[5] = ctrl->getControlByID(BUTTON_INSTRUMENTS_MINUS)->isHidden();
		visibility[6] = ctrl->getControlByID(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER)->isHidden();
		visibility[7] = ctrl->getControlByID(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER2)->isHidden();
		visibility[8] = ctrl->getControlByID(BUTTON_JAMMENU_NEXTINSTRUMENT)->isHidden();
		visibility[9] = ctrl->getControlByID(BUTTON_JAMMENU_PREVINSTRUMENT)->isHidden();

		replaceAndResizeInstrumentListContainer(listBoxContainerHeight);

		// flip button is always hidden
		ctrl->getControlByID(BUTTON_INSTRUMENTS_FLIP)->hide(true);
		// so is alternative header ("Samples / Ins:xx")
		ctrl->getControlByID(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER)->hide(true);
		ctrl->getControlByID(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER2)->hide(true);
		// up/down buttons as well
		ctrl->getControlByID(BUTTON_JAMMENU_NEXTINSTRUMENT)->hide(true);
		ctrl->getControlByID(BUTTON_JAMMENU_PREVINSTRUMENT)->hide(true);

		// instrument button always visible
		ctrl->getControlByID(BUTTON_INSTRUMENT)->hide(false);
		ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->hide(false);
		// instrument "+" visible
		ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->hide(false);
		// instrument "-" visible
		ctrl->getControlByID(BUTTON_INSTRUMENTS_MINUS)->hide(false);
		// listbox is always visible
		tracker.listBoxInstruments->hide(false);
		
		// add another button
		if (ctrl->getControlByID(BUTTON_SAMPLES_INVOKEHDRECORDER) == NULL)
		{
			PPPoint p = ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->getLocation();
			p.y = ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->getLocation().y;
			pp_int32 width = ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->getSize().width;
			pp_int32 height = ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->getSize().height;
			PPButton* button = new PPButton(BUTTON_SAMPLES_INVOKEHDRECORDER, screen, &tracker, PPPoint(p.x + width + 2, p.y), PPSize(7*5, height));
			button->setFont(PPFont::getFont(PPFont::FONT_TINY));
			button->setText("Render");
			ctrl->addControl(button);		
		}
		else
		{
			PPPoint p = ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->getLocation();
			p.y = ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->getLocation().y;
			pp_int32 width = ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->getSize().width;
			pp_int32 height = ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->getSize().height;
			PPButton* button = static_cast<PPButton*>(ctrl->getControlByID(BUTTON_SAMPLES_INVOKEHDRECORDER));
			button->setLocation(PPPoint(p.x + width + 2, p.y));
			button->setSize(PPSize(7*5, height));
			button->hide(false);
		}
	}
	else
	{
		tracker.showMainMenu(true, true);

		tracker.listBoxInstruments->setSize(oldInstrumentListSize);
		tracker.listBoxInstruments->setLocation(oldInstrumentListLocation);
		
		tracker.listBoxSamples->setSize(oldSampleListSize);
		tracker.listBoxSamples->setLocation(oldSampleListLocation);
		
		PPContainer* ctrl = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_INSTRUMENTLIST));
		ctrl->setSize(oldInstrumentListContainerSize);
		ctrl->setLocation(oldInstrumentListContainerLocation);
		
		ctrl->getControlByID(BUTTON_INSTRUMENT)->setLocation(oldControlLocations[0]);
		ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->setLocation(oldControlLocations[1]);
		ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->setLocation(oldControlLocations[2]);
		ctrl->getControlByID(BUTTON_INSTRUMENTS_MINUS)->setLocation(oldControlLocations[3]);

		// this is always visible
		ctrl->getControlByID(BUTTON_INSTRUMENTS_FLIP)->hide(false);

		// for the rest just restore the old visibility flags
		ctrl->getControlByID(LISTBOX_INSTRUMENTS)->hide(visibility[0]);
		ctrl->getControlByID(LISTBOX_SAMPLES)->hide(visibility[1]);
		ctrl->getControlByID(BUTTON_INSTRUMENT)->hide(visibility[2]);
		ctrl->getControlByID(STATICTEXT_SAMPLEHEADER)->hide(visibility[3]);
		ctrl->getControlByID(BUTTON_INSTRUMENTS_PLUS)->hide(visibility[4]);
		ctrl->getControlByID(BUTTON_INSTRUMENTS_MINUS)->hide(visibility[5]);
		ctrl->getControlByID(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER)->hide(visibility[6]);
		ctrl->getControlByID(STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER2)->hide(visibility[7]);
		ctrl->getControlByID(BUTTON_JAMMENU_NEXTINSTRUMENT)->hide(visibility[8]);
		ctrl->getControlByID(BUTTON_JAMMENU_PREVINSTRUMENT)->hide(visibility[9]);
		
		PPControl* dummy = ctrl->getControlByID(BUTTON_SAMPLES_INVOKEHDRECORDER);
		if (dummy)
			dummy->hide(true);
	}
}
#endif
