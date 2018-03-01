/*
 *  tracker/DialogEQ.cpp
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
 *  DialogEQ.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.04.07.
 *
 */

#include "DialogEQ.h"
#include "Screen.h"
#include "StaticText.h"
#include "MessageBoxContainer.h"
#include "ScrollBar.h"
#include "Slider.h"
#include "Seperator.h"
#include "EQConstants.h"

DialogEQ::DialogEQ(PPScreen* screen, 
				   DialogResponder* responder,
				   pp_int32 id,
				   EQNumBands numBands) :
	PPDialogBase(),
	numBands(numBands)
{
	switch (numBands)
	{
		case EQ10Bands:
			numSliders = 10;
			break;
		case EQ3Bands:
			numSliders = 3;
	}
	
	char dummy[100];
	
	sprintf(dummy, "%i Band Equalizer" PPSTR_PERIODS, numSliders);
	
	initDialog(screen, responder, id, dummy, 300, 230, 26, "Ok", "Cancel");

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	pp_int32 y = getMessageBoxContainer()->getLocation().y;
	
	pp_int32 width = getMessageBoxContainer()->getSize().width;
	pp_int32 height = getMessageBoxContainer()->getSize().height;

	/*PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_YES));	
	pp_int32 y2 = button->getLocation().y;
	pp_int32 x2 = x + width / 2 - 30;*/
	
	pp_uint32 borderSpace = 12;
	
	pp_uint32 scalaSpace = 6*5+8;
	
	pp_int32 y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y + 26;
	pp_int32 x2 = x + borderSpace + scalaSpace;
	
	pp_int32 size = (y+height) - (y2 + 70);
	
	const pp_uint32 tickSize = 4;
	
	const float* values = ((numSliders == 3) ? EQConstants::EQ3bands : EQConstants::EQ10bands);
	
	for (pp_uint32 i = 0; i < numSliders; i++)
	{
		PPFont* font = PPFont::getFont(PPFont::FONT_TINY);
		char dummy[100];
		pp_int32 value = (pp_int32)values[i];
		sprintf(dummy, value >= 1000 ? "%ik" : "%i", value >= 1000 ? value / 1000 : value);
		PPStaticText* staticText = new PPStaticText(0, screen, this, PPPoint(x2 + (SCROLLBUTTONSIZE/2) - font->getStrWidth(dummy) / 2, y2 - 8), dummy, true);
		staticText->setFont(font);
		getMessageBoxContainer()->addControl(staticText);
	
		PPSlider* slider = new PPSlider(MESSAGEBOX_CONTROL_USER1+i, screen, this, PPPoint(x2, y2), size, false, true);
		slider->setBarSize(8192);
		slider->setMinValue(0);
		slider->setMaxValue(240);
		slider->setCurrentValue(120);
		getMessageBoxContainer()->addControl(slider);

		float y2f = (float)y2;
		y2f+=SCROLLBUTTONSIZE + 4;
		for (pp_int32 j = 0; j <= 8; j++)
		{
			PPSeperator* seperator = new PPSeperator(0, parentScreen, PPPoint(x2-(tickSize+1), (pp_int32)y2f), tickSize, getMessageBoxContainer()->getColor(), true);
			getMessageBoxContainer()->addControl(seperator);
			y2f+=((float)(size-SCROLLBUTTONSIZE*3)/8.0f);
		}
		
		x2+=(width-borderSpace*2-scalaSpace-SCROLLBUTTONSIZE)/(numSliders-1);
	}
	
	x2 = x + borderSpace;
	float y2f = (float)y2;
	y2f+=SCROLLBUTTONSIZE + 2;
	for (pp_int32 j = 0; j <= 8; j++)
	{
		PPFont* font = PPFont::getFont(PPFont::FONT_TINY);
	
		char dummy[100];
		sprintf(dummy, j < 4 ? "+%i dB" : "%i dB", (8-j)*3 - 12);
		PPStaticText* staticText = new PPStaticText(0, screen, this, PPPoint(x2 + font->getStrWidth("+12 dB") - font->getStrWidth(dummy), (pp_int32)y2f), dummy, true);
		staticText->setFont(font);
		getMessageBoxContainer()->addControl(staticText);
	
		y2f+=((float)(size-SCROLLBUTTONSIZE*3)/8.0f);
	}
	
	y2 = (pp_int32)y2f + 16;

	getMessageBoxContainer()->addControl(new PPSeperator(0, screen, PPPoint(x+2, y2), width - 5, getMessageBoxContainer()->getColor(), true));

	x2 = x+6;
	y2+=6;
	
	PPButton* button = new PPButton(MESSAGEBOX_LISTBOX_VALUE_ONE, screen, this, PPPoint(x2 + (width-5)/2 - 25, y2), PPSize(50, 14));
	button->setText("Reset");
	getMessageBoxContainer()->addControl(button);

	getMessageBoxContainer()->addControl(new PPSeperator(0, screen, PPPoint(x+2, y2+18), width - 5, getMessageBoxContainer()->getColor(), true));	
}


void DialogEQ::resetSliders()
{
	for (pp_uint32 i = 0; i < numSliders; i++)
	{
		PPSlider* slider = static_cast<PPSlider*>(getMessageBoxContainer()->getControlByID(MESSAGEBOX_CONTROL_USER1+i));
		slider->setCurrentValue(120);		
	}
}

void DialogEQ::update()
{
	parentScreen->paintControl(messageBoxContainerGeneric);
}

pp_int32 DialogEQ::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eCommand)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			// reset sliders
			case MESSAGEBOX_LISTBOX_VALUE_ONE:
			{
				resetSliders();
				update();
				break;
			}
		}
	}

	return PPDialogBase::handleEvent(sender, event);
}

void DialogEQ::setBandParam(pp_uint32 index, float param)
{
	if (index >= numSliders)
		return;

	PPSlider* slider = static_cast<PPSlider*>(getMessageBoxContainer()->getControlByID(MESSAGEBOX_CONTROL_USER1+index));

	pp_int32 value = (pp_int32)((1.0f - param) * 240.0f);
	
	slider->setCurrentValue(value);
}

float DialogEQ::getBandParam(pp_uint32 index) const
{
	if (index >= numSliders)
		return 0.0f;

	PPSlider* slider = static_cast<PPSlider*>(getMessageBoxContainer()->getControlByID(MESSAGEBOX_CONTROL_USER1+index));

	float v = 1.0f - (slider->getCurrentValue() / 240.0f);

	return v;
}
