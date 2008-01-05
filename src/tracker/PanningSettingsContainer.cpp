/*
 *  PanningSettingsContainer.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.03.06.
 *  Copyright 2006 milkytracker.net, All rights reserved.
 *
 */

#include "PanningSettingsContainer.h"
#include "Tracker.h"
#include "Screen.h"
#include "ControlIDs.h"
#include "MessageBoxContainer.h"
#include "Button.h"
#include "Slider.h"
#include "StaticText.h"
#include "Seperator.h"

#define SLIDER_PANNING_BASE		1000
#define BUTTON_PANNING_AMIGA	1001
#define BUTTON_PANNING_MILKY	1002
#define BUTTON_PANNING_MONO		1003

PanningSettingsContainer::PanningSettingsContainer(PPScreen* theScreen, EventListenerInterface* theEventListener, pp_uint32 channels) :
	screen(theScreen),
	eventListener(theEventListener),
	container(NULL),
	panning(NULL),
	numChannels(channels)
{
	init();
}

PanningSettingsContainer::~PanningSettingsContainer()
{
	delete[] panning;
	delete container;
}


// PPEvent listener
pp_int32 PanningSettingsContainer::handleEvent(PPObject* sender, PPEvent* event)
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
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case SLIDER_PANNING_BASE:
			case SLIDER_PANNING_BASE+1:
			case SLIDER_PANNING_BASE+2:
			case SLIDER_PANNING_BASE+3:
			case SLIDER_PANNING_BASE+4:
			case SLIDER_PANNING_BASE+5:
			case SLIDER_PANNING_BASE+6:
			case SLIDER_PANNING_BASE+7:
			case SLIDER_PANNING_BASE+8:
			case SLIDER_PANNING_BASE+9:
			case SLIDER_PANNING_BASE+10:
			case SLIDER_PANNING_BASE+11:
			case SLIDER_PANNING_BASE+12:
			case SLIDER_PANNING_BASE+13:
			case SLIDER_PANNING_BASE+14:
			case SLIDER_PANNING_BASE+15:
			case SLIDER_PANNING_BASE+16:
			case SLIDER_PANNING_BASE+17:
			case SLIDER_PANNING_BASE+18:
			case SLIDER_PANNING_BASE+19:
			case SLIDER_PANNING_BASE+20:
			case SLIDER_PANNING_BASE+21:
			case SLIDER_PANNING_BASE+22:
			case SLIDER_PANNING_BASE+23:
			case SLIDER_PANNING_BASE+24:
			case SLIDER_PANNING_BASE+25:
			case SLIDER_PANNING_BASE+26:
			case SLIDER_PANNING_BASE+27:
			case SLIDER_PANNING_BASE+28:
			case SLIDER_PANNING_BASE+29:
			case SLIDER_PANNING_BASE+30:
			case SLIDER_PANNING_BASE+31:
			{
				pp_int32 i = reinterpret_cast<PPControl*>(sender)->getID() - SLIDER_PANNING_BASE;
				panning[i] = (pp_uint8)reinterpret_cast<PPSlider*>(sender)->getCurrentValue();
				PPEvent e(eValueChanged, &i, sizeof(i));				
				eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
				break;
			}
		}
	}
	return 0;
}

void PanningSettingsContainer::init()
{
	pp_int32 width = 290;
	pp_int32 height = 234;

	pp_int32 x = screen->getWidth() / 2 - width/2;
	pp_int32 y = screen->getHeight() / 2 - height/2;

	container = new PPMessageBoxContainer(MESSAGEBOX_PANNINGSELECT, screen, this, PPPoint(x, y), PPSize(width,height), "Global Panning");

	pp_int32 y2 = y + 18;

	char buffer[80];
	
	pp_int32 i;
	for (i = 0; i < 16; i++)
	{		
		PPSlider* slider = new PPSlider(SLIDER_PANNING_BASE+i, screen, this, PPPoint(x + 4+6*8+4+4+1, y2), (width>>1)-(6*8+8+4+4+1+10), true);
		slider->setMaxValue(255);
		slider->setBarSize(16384);
		container->addControl(slider);

		sprintf(buffer, "Ch:%02d L         R", i+1);
		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 4, y2), buffer, true));

		y2+=12;
	}
	y2 = y + 18;

	pp_int32 x2 = x+(width>>1);

	for (i = 0; i < 16; i++)
	{
		PPSlider* slider = new PPSlider(SLIDER_PANNING_BASE+i+16, screen, this, PPPoint(x2 + 4+6*8+4+4+1, y2), (width>>1)-(6*8+8+4+4+1+10), true);
		slider->setMaxValue(255);
		slider->setBarSize(16384);
		container->addControl(slider);

		sprintf(buffer, "Ch:%02d L         R", i+1+16);
		container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x2 + 4, y2), buffer, true));

		y2+=12;
	}

	y2 = y + height - 17;

	PPButton* button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, screen, this, PPPoint(x+width - 60 - 57, y2), PPSize(54, 11));
	button->setText("Okay");
	container->addControl(button);
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_NO, screen, this, PPPoint(x+width - 60, y2), PPSize(54, 11));
	button->setText("Cancel");
	container->addControl(button);

	container->addControl(new PPStaticText(0, NULL, NULL, PPPoint(x + 7, y2+2), "Predef", true));
	
	button = new PPButton(BUTTON_PANNING_AMIGA, screen, this, PPPoint(x+7+6*8+2, y2), PPSize(32, 11));
	button->setText("Amiga");
	container->addControl(button);

	button = new PPButton(BUTTON_PANNING_MILKY, screen, this, PPPoint(x+7+6*8+2+33, y2), PPSize(32, 11));
	button->setText("Milky");
	container->addControl(button);

	button = new PPButton(BUTTON_PANNING_MONO, screen, this, PPPoint(x+7+6*8+2+33*2, y2), PPSize(32, 11));
	button->setText("Mono");
	container->addControl(button);
	
	container->addControl(new PPSeperator(0, screen, PPPoint(x+5+6*8+2+33*3 + 8, y2 - 5), 18, container->getColor(), false));
	container->addControl(new PPSeperator(0, screen, PPPoint(x+2, y2 - 6), container->getSize().width - 5, container->getColor(), true));

	panning = new pp_uint8[numChannels];
}

void PanningSettingsContainer::show(bool b)
{
	if (b)
	{
		screen->setModalControl(container);
	}
	else
		screen->setModalControl(NULL);
}

void PanningSettingsContainer::setPanning(pp_uint32 chn, pp_uint8 pan, bool repaint/* = true*/)
{
	if (panning == NULL)
		return;
		
	if (chn >= numChannels)
		return;
		
	panning[chn] = pan;
	
	PPSlider* slider = static_cast<PPSlider*>(container->getControlByID(SLIDER_PANNING_BASE+chn));
	
	if (slider == NULL)
		return;
		
	slider->setCurrentValue(pan);
	
	if (repaint && slider->isVisible())
		screen->paintControl(slider);
}

void PanningSettingsContainer::applyPanningAmiga()
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

	screen->paintControl(container);
}

void PanningSettingsContainer::applyPanningMilky()
{
	for (pp_uint32 i = 0; i < numChannels; i++)
	{
		setPanning(i, i & 1 ? 192 : 64, false);
		PPEvent e(eValueChanged, &i, sizeof(i));				
		eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}

	screen->paintControl(container);
}

void PanningSettingsContainer::applyPanningMono()
{
	for (pp_uint32 i = 0; i < numChannels; i++)
	{
		setPanning(i, 128, false);
		PPEvent e(eValueChanged, &i, sizeof(i));				
		eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}

	screen->paintControl(container);
}
