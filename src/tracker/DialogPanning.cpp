/*
 *  tracker/DialogPanning.cpp
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
 *  DialogPanning.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.03.06.
 *
 */

#include "DialogPanning.h"
#include "Tracker.h"
#include "TrackerConfig.h"
#include "Screen.h"
#include "ControlIDs.h"
#include "MessageBoxContainer.h"
#include "Button.h"
#include "Slider.h"
#include "StaticText.h"
#include "Seperator.h"

enum
{
	BUTTON_PANNING_AMIGA = 1000,
	BUTTON_PANNING_MILKY,
	BUTTON_PANNING_MONO,
	SLIDER_PANNING_BASE
};

DialogPanning::DialogPanning(PPScreen* theScreen, EventListenerInterface* theEventListener, pp_uint32 channels) :
	PPDialogBase(),
	eventListener(theEventListener),
	panning(NULL),
	numChannels(channels)
{
	parentScreen = theScreen;
	init();
}

DialogPanning::~DialogPanning()
{
	delete[] panning;
}


// PPEvent listener
pp_int32 DialogPanning::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eCommand)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case PP_MESSAGEBOX_BUTTON_YES:
			{
				PPEvent e(eConfirmed);				
				eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
				show(false);
				break;
			}

			case PP_MESSAGEBOX_BUTTON_NO:
			{
				PPEvent e(eCanceled);				
				eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
				show(false);
				break;
			}

			case BUTTON_PANNING_AMIGA:
				applyPanningAmiga();
				break;

			case BUTTON_PANNING_MILKY:
				applyPanningMilky();
				break;

			case BUTTON_PANNING_MONO:
				applyPanningMono();
				break;
		}
	}
	else if (event->getID() == eValueChanged)
	{
		pp_uint32 id = reinterpret_cast<PPControl*>(sender)->getID();
		if (id >= SLIDER_PANNING_BASE &&
			id < (SLIDER_PANNING_BASE + numChannels))
		{
			pp_int32 i = id - SLIDER_PANNING_BASE;
			panning[i] = (pp_uint8)reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
			PPEvent e(eValueChanged, &i, sizeof(i));				
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
		}
	}
	return 0;
}

void DialogPanning::init()
{
	const pp_int32 maxChannels = numChannels;

	pp_int32 width = 290;
	pp_int32 height = 234;

	pp_int32 x = parentScreen->getWidth() / 2 - width/2;
	pp_int32 y = parentScreen->getHeight() / 2 - height/2;

	messageBoxContainerGeneric = new PPMessageBoxContainer(MESSAGEBOX_PANNINGSELECT, parentScreen, this, PPPoint(x, y), PPSize(width,height), "Global Panning");

	pp_int32 y2 = y + 18;

	char buffer[80];
	
	pp_int32 i;
	for (i = 0; i < maxChannels/2; i++)
	{		
		PPSlider* slider = new PPSlider(SLIDER_PANNING_BASE+i, parentScreen, this, PPPoint(x + 4+6*8+4+4+1, y2), (width>>1)-(6*8+8+4+4+1+10), true);
		slider->setMaxValue(255);
		slider->setBarSize(16384);
		messageBoxContainerGeneric->addControl(slider);

		sprintf(buffer, "Ch:%02d L         R", i+1);
		messageBoxContainerGeneric->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y2), buffer, true));

		y2+=12;
	}
	y2 = y + 18;

	pp_int32 x2 = x+(width>>1);

	for (i = 0; i < maxChannels/2; i++)
	{
		PPSlider* slider = new PPSlider(SLIDER_PANNING_BASE+i+(maxChannels/2), parentScreen, this, PPPoint(x2 + 4+6*8+4+4+1, y2), (width>>1)-(6*8+8+4+4+1+10), true);
		slider->setMaxValue(255);
		slider->setBarSize(16384);
		messageBoxContainerGeneric->addControl(slider);

		sprintf(buffer, "Ch:%02d L         R", i+1+(maxChannels/2));
		messageBoxContainerGeneric->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 4, y2), buffer, true));

		y2+=12;
	}

	y2 = y + height - 17;

	PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, parentScreen, this, PPPoint(x+width - 60 - 57, y2), PPSize(54, 11));
	button->setText("Okay");
	messageBoxContainerGeneric->addControl(button);
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_NO, parentScreen, this, PPPoint(x+width - 60, y2), PPSize(54, 11));
	button->setText("Cancel");
	messageBoxContainerGeneric->addControl(button);

	messageBoxContainerGeneric->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 7, y2+2), "Predef", true));
	
	button = new PPButton(BUTTON_PANNING_AMIGA, parentScreen, this, PPPoint(x+7+6*8+2, y2), PPSize(32, 11));
	button->setText("Amiga");
	messageBoxContainerGeneric->addControl(button);

	button = new PPButton(BUTTON_PANNING_MILKY, parentScreen, this, PPPoint(x+7+6*8+2+33, y2), PPSize(32, 11));
	button->setText("Milky");
	messageBoxContainerGeneric->addControl(button);

	button = new PPButton(BUTTON_PANNING_MONO, parentScreen, this, PPPoint(x+7+6*8+2+33*2, y2), PPSize(32, 11));
	button->setText("Mono");
	messageBoxContainerGeneric->addControl(button);
	
	messageBoxContainerGeneric->addControl(new PPSeperator(0, parentScreen, PPPoint(x+5+6*8+2+33*3 + 8, y2 - 5), 18, messageBoxContainerGeneric->getColor(), false));
	messageBoxContainerGeneric->addControl(new PPSeperator(0, parentScreen, PPPoint(x+2, y2 - 6), messageBoxContainerGeneric->getSize().width - 5, messageBoxContainerGeneric->getColor(), true));

	panning = new pp_uint8[maxChannels];
}

void DialogPanning::setPanning(pp_uint32 chn, pp_uint8 pan, bool repaint/* = true*/)
{
	if (panning == NULL)
		return;
		
	if (chn >= numChannels)
		return;
		
	panning[chn] = pan;
	
	PPSlider* slider = static_cast<PPSlider*>(messageBoxContainerGeneric->getControlByID(SLIDER_PANNING_BASE+chn));
	
	if (slider == NULL)
		return;
		
	slider->setCurrentValue(pan);
	
	if (repaint && slider->isVisible())
		parentScreen->paintControl(slider);
}

void DialogPanning::applyPanningAmiga()
{
	for (pp_uint32 i = 0; i < numChannels; i++)
	{
		pp_uint8 pan = 0;
		switch (i & 3)
		{
			case 0:
				pan = 0;
				break;
			case 1:
				pan = 255;
				break;
			case 2:
				pan = 255;
				break;
			case 3:
				pan = 0;
				break;
		}		
		
		setPanning(i, pan, false);
		PPEvent e(eValueChanged, &i, sizeof(i));				
		eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}

	parentScreen->paintControl(messageBoxContainerGeneric);
}

void DialogPanning::applyPanningMilky()
{
	for (pp_uint32 i = 0; i < numChannels; i++)
	{
		setPanning(i, i & 1 ? 192 : 64, false);
		PPEvent e(eValueChanged, &i, sizeof(i));				
		eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}

	parentScreen->paintControl(messageBoxContainerGeneric);
}

void DialogPanning::applyPanningMono()
{
	for (pp_uint32 i = 0; i < numChannels; i++)
	{
		setPanning(i, 128, false);
		PPEvent e(eValueChanged, &i, sizeof(i));				
		eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}

	parentScreen->paintControl(messageBoxContainerGeneric);
}
