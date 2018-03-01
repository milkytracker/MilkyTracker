/*
 *  tracker/DialogResample.cpp
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
 *  DialogResample.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *
 */

#include "DialogResample.h"
#include "Screen.h"
#include "StaticText.h"
#include "CheckBox.h"
#include "CheckBoxLabel.h"
#include "PPUIConfig.h"
#include "MessageBoxContainer.h"
#include "Font.h"
#include "ListBox.h"
#include "Seperator.h"
#include "XModule.h"
#include "ResamplerHelper.h"

float getc4spd(mp_sint32 relnote,mp_sint32 finetune)
{
	static const mp_sint32	table[] = {65536,69432,73561,77935,82570,87480,92681,98193,104031,110217,116771,123715,
						   65536,65565,65595,65624,65654,65684,65713,65743,65773,65802,65832,65862,65891,
						   65921,65951,65981,66010,66040,66070,66100,66130,66160,66189,66219,66249,66279,
						   66309,66339,66369,66399,66429,66459,66489,66519,66549,66579,66609,66639,66669,
						   66699,66729,66759,66789,66820,66850,66880,66910,66940,66971,67001,67031,67061,
						   67092,67122,67152,67182,67213,67243,67273,67304,67334,67365,67395,67425,67456,
						   67486,67517,67547,67578,67608,67639,67669,67700,67730,67761,67792,67822,67853,
						   67883,67914,67945,67975,68006,68037,68067,68098,68129,68160,68190,68221,68252,
						   68283,68314,68344,68375,68406,68437,68468,68499,68530,68561,68592,68623,68654,
						   68685,68716,68747,68778,68809,68840,68871,68902,68933,68964,68995,69026,69057,
						   69089,69120,69151,69182,69213,69245,69276,69307,69339,69370,69401};

	mp_sint32 c4spd = 8363;
	mp_sbyte xmfine = finetune;

	mp_sbyte octave = (relnote+96)/12;
	mp_sbyte note = (relnote+96)%12;
	
	mp_sbyte o2 = octave-8;
	
	if (xmfine<0)
	{
		xmfine+=(mp_sbyte)128;
		note--;
		if (note<0)
		{
			note+=12;
			o2--;
		}
	}

	if (o2>=0)
	{
		c4spd<<=o2;		
	}
	else
	{
		c4spd>>=-o2;
	}

	float f = table[(mp_ubyte)note]*(1.0f/65536.0f) * c4spd;
	return f * (table[(mp_ubyte)xmfine+12]*(1.0f/65536.0f));
}

DialogResample::DialogResample(PPScreen* screen, 
							   DialogResponder* responder,
							   pp_int32 id) :
	PPDialogBase(),
	count(0),
	resamplerHelper(new ResamplerHelper()),
	interpolationType(1),
	adjustFtAndRelnote(true)
{
#ifdef __LOWRES__
	initDialog(screen, responder, id, "Resample" PPSTR_PERIODS, 290, 142+15+20+16, 26+15, "Ok", "Cancel");
#else
	initDialog(screen, responder, id, "Resample" PPSTR_PERIODS, 290, 142+20+16, 26, "Ok", "Cancel");
#endif

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	
	pp_int32 width = getMessageBoxContainer()->getSize().width;
	
	pp_int32 x2 = x;
	pp_int32 y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y;

	PPButton* button;

	y2 +=16;
	
	// enter edit field 1
	x2 = x + width / 2 - (10*8+35 + 14*8)/2;
	
	messageBoxContainerGeneric->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2+2), "Relative note", true));	
	
	x2+=17*8;
	PPListBox* listBox = new PPListBox(MESSAGEBOX_LISTBOX_VALUE_ONE, screen, this, PPPoint(x2, y2), PPSize(7*8,12), true, true, false);
	listBox->showSelection(false);
	listBox->setBorderColor(messageBoxContainerGeneric->getColor());
	listBox->setMaxEditSize(8);
	listBoxes[0] = listBox;
	messageBoxContainerGeneric->addControl(listBox);	
	
	button = new PPButton(MESSAGEBOX_BUTTON_INCREASE_VALUEONE, screen, this, PPPoint(x2 + listBox->getSize().width + 2, y2), PPSize(16, 11));
	button->setText("+");
	messageBoxContainerGeneric->addControl(button);
	
	button = new PPButton(MESSAGEBOX_BUTTON_DECREASE_VALUEONE, screen, this, PPPoint(x2 + listBox->getSize().width + 2 + button->getSize().width + 1, y2), PPSize(16, 11));
	button->setText("-");
	messageBoxContainerGeneric->addControl(button);

	y2+=16;

	x2 = x + width / 2 - (10*8+35 + 14*8)/2;
	
	messageBoxContainerGeneric->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2+2), "Fine tune", true));	
	
	x2+=17*8;
	listBox = new PPListBox(MESSAGEBOX_LISTBOX_VALUE_TWO, screen, this, PPPoint(x2, y2), PPSize(7*8,12), true, true, false);
	listBox->showSelection(false);
	listBox->setBorderColor(messageBoxContainerGeneric->getColor());
	listBox->setMaxEditSize(8);
	listBoxes[1] = listBox;
	messageBoxContainerGeneric->addControl(listBox);	
	
	button = new PPButton(MESSAGEBOX_BUTTON_INCREASE_VALUETWO, screen, this, PPPoint(x2 + listBox->getSize().width + 2, y2), PPSize(16, 11));
	button->setText("+");
	messageBoxContainerGeneric->addControl(button);
	
	button = new PPButton(MESSAGEBOX_BUTTON_DECREASE_VALUETWO, screen, this, PPPoint(x2 + listBox->getSize().width + 2 + button->getSize().width + 1, y2), PPSize(16, 11));
	button->setText("-");
	messageBoxContainerGeneric->addControl(button);

	y2+=16;

	x2 = x + width / 2 - (10*8+35 + 14*8)/2;
	
	messageBoxContainerGeneric->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2+2), "C4 speed (Hz)", true));	
	
	x2+=14*8;
	listBox = new PPListBox(MESSAGEBOX_LISTBOX_VALUE_THREE, screen, this, PPPoint(x2, y2), PPSize(10*8,12), true, true, false);
	listBox->showSelection(false);
	listBox->setBorderColor(messageBoxContainerGeneric->getColor());
	listBox->setMaxEditSize(8);
	listBoxes[2] = listBox;
	messageBoxContainerGeneric->addControl(listBox);	
	
	button = new PPButton(MESSAGEBOX_BUTTON_INCREASE_VALUETHREE, screen, this, PPPoint(x2 + listBox->getSize().width + 2, y2), PPSize(16, 11));
	button->setText("+");
	messageBoxContainerGeneric->addControl(button);
	
	button = new PPButton(MESSAGEBOX_BUTTON_DECREASE_VALUETHREE, screen, this, PPPoint(x2 + listBox->getSize().width + 2 + button->getSize().width + 1, y2), PPSize(16, 11));
	button->setText("-");
	messageBoxContainerGeneric->addControl(button);

	y2+=16;
	x2 = x + width / 2 - (10*8+35 + 14*8)/2;
	messageBoxContainerGeneric->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2+2), "New size:", true));	
	x2+=18*8;
	messageBoxContainerGeneric->addControl(new PPStaticText(MESSAGEBOX_STATICTEXT_USER1, screen, this, PPPoint(x2, y2+2), "XXXXXXXX"));	

	y2+=16;

	x2 = x + width / 2 - (10*8+35 + 14*8)/2;
	messageBoxContainerGeneric->addControl(new PPStaticText(0, screen, this, PPPoint(x2, y2+2), "Interpolation:", true));	
	
	x2+=15*8;
	button = new PPButton(MESSAGEBOX_CONTROL_USER1, screen, this, PPPoint(x2, y2), PPSize(button->getLocation().x + button->getSize().width - x2, 11), false);
	button->setText(resamplerHelper->getResamplerName(interpolationType, true));
	button->setColor(messageBoxContainerGeneric->getColor());
	button->setTextColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));

	messageBoxContainerGeneric->addControl(button);

	y2+=16;

	x2 = x + width / 2 - (10 * 8 + 35 + 14 * 8) / 2 + 21 * 8;
	checkBox = new PPCheckBox(MESSAGEBOX_CONTROL_USER2, screen, this, PPPoint(x2, y2 + 1));
	checkBox->checkIt(adjustFtAndRelnote);
	messageBoxContainerGeneric->addControl(checkBox);

	x2 -= 21 * 8;
	messageBoxContainerGeneric->addControl(new PPCheckBoxLabel(0, screen, this, PPPoint(x2, y2 + 2), "Adjust Ft/Rel.Note:", checkBox, true));	
	
	y2+=16;

#ifdef __LOWRES__
	pp_int32 height = getMessageBoxContainer()->getSize().height;
	pp_int32 y = getMessageBoxContainer()->getLocation().y;

	const char buttonTexts[] = {'1','2','3','4','5','6','7','8','9','0','+','-','.','<','>'};
	
	pp_int32 bWidth = (width - 22*2 - 2*3) / sizeof(buttonTexts);
	pp_int32 x2_2 = x+4;
	
	pp_int32 y2_2 = y+height - 4 - 13;
	
	messageBoxContainerGeneric->addControl(new PPSeperator(0, screen, PPPoint(x2_2-1, y2_2-3), width-2*3, messageBoxContainerGeneric->getColor(), true));
	
	pp_uint32 i = 0;
	for (i = 0; i < sizeof(buttonTexts); i++)
	{
		button = new PPButton(MESSAGEBOX_BUTTON_KEYS_BASE+i, screen, this, PPPoint(x2_2, y2_2), PPSize(bWidth+1, 13));
		button->setText(buttonTexts[i]);
		messageBoxContainerGeneric->addControl(button);
		x2_2+=bWidth;
	}
	
	bWidth = 22;
	
	button = new PPButton(MESSAGEBOX_BUTTON_KEYS_BASE+i, screen, this, PPPoint(x2_2, y2_2), PPSize(bWidth+1-3, 13));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Del");
	messageBoxContainerGeneric->addControl(button);
	x2_2+=bWidth-3;
	i++;
	button = new PPButton(MESSAGEBOX_BUTTON_KEYS_BASE+i, screen, this, PPPoint(x2_2, y2_2), PPSize(bWidth+1, 13));
	button->setFont(PPFont::getFont(PPFont::FONT_TINY));
	button->setText("Back");
	messageBoxContainerGeneric->addControl(button);
#endif

	currentSelectedListBox = 0;
	relnote = finetune = 0;
	c4spd = 0.0f;
}

DialogResample::~DialogResample()
{
	delete resamplerHelper;
}

void DialogResample::show(bool b/* = true*/)
{
	if (b)
	{
		currentSelectedListBox = 0;
		updateListBoxes();
		listBoxEnterEditState(MESSAGEBOX_LISTBOX_VALUE_ONE);
		
		PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_CONTROL_USER1));
		button->setText(resamplerHelper->getResamplerName(interpolationType, true));
	}
	PPDialogBase::show(b);	
}

pp_int32 DialogResample::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eKeyDown)
	{
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());
		if (keyCode == VK_TAB)
		{
			switchListBox();
			event->cancel();
		}
	}
	else if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case PP_MESSAGEBOX_BUTTON_YES:
			{
				commitChanges();
				break;
			}

			case MESSAGEBOX_BUTTON_INCREASE_VALUEONE:
			{
				relnote++;
				toC4Speed();
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}

			case MESSAGEBOX_BUTTON_DECREASE_VALUEONE:
			{
				relnote--;
				toC4Speed();
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}

			case MESSAGEBOX_BUTTON_INCREASE_VALUETWO:
			{
				finetune++;
				toC4Speed();
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}

			case MESSAGEBOX_BUTTON_DECREASE_VALUETWO:
			{
				finetune--;
				toC4Speed();
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}

			case MESSAGEBOX_BUTTON_INCREASE_VALUETHREE:
			{
				c4spd++;
				fromC4Speed();
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}

			case MESSAGEBOX_BUTTON_DECREASE_VALUETHREE:
			{
				c4spd--;
				fromC4Speed();
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}
			
			case MESSAGEBOX_CONTROL_USER1:
			{	
				if (event->getID() != eCommand)
					break;
				
				interpolationType = (interpolationType + 1) % resamplerHelper->getNumResamplers();
				
				PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_CONTROL_USER1));
				button->setText(resamplerHelper->getResamplerName(interpolationType, true));
				parentScreen->paintControl(messageBoxContainerGeneric);							
				break;
			}

			case MESSAGEBOX_CONTROL_USER2:
			{
				if (event->getID() != eCommand)
					break;
					
				this->adjustFtAndRelnote = reinterpret_cast<PPCheckBox*>(sender)->isChecked();
				break;
			}

		}
	}
	else if (event->getID() == eValueChanged)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case MESSAGEBOX_LISTBOX_VALUE_ONE:
			{
				const PPString* str = *(reinterpret_cast<PPString* const*>(event->getDataPtr()));
				setRelNote((pp_int32)atoi(*str));
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}

			case MESSAGEBOX_LISTBOX_VALUE_TWO:
			{
				const PPString* str = *(reinterpret_cast<PPString* const*>(event->getDataPtr()));
				setFineTune((pp_int32)atoi(*str));
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}

			case MESSAGEBOX_LISTBOX_VALUE_THREE:
			{
				const PPString* str = *(reinterpret_cast<PPString* const*>(event->getDataPtr()));
				setC4Speed((float)atof(*str));
				calcSize();
				updateListBoxes();
				parentScreen->paintControl(messageBoxContainerGeneric);
				break;
			}
		}
	}
	
	return PPDialogBase::handleEvent(sender, event);
}

void DialogResample::updateListBoxes()
{
	updateListBox(MESSAGEBOX_LISTBOX_VALUE_ONE, (float)relnote, 0);
	updateListBox(MESSAGEBOX_LISTBOX_VALUE_TWO, (float)finetune, 0);
	updateListBox(MESSAGEBOX_LISTBOX_VALUE_THREE, (float)c4spd, 2);

	PPStaticText* staticText = static_cast<PPStaticText*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_STATICTEXT_USER1));
	staticText->setHexValue(finalSize, 8);	

	checkBox->checkIt(adjustFtAndRelnote);
}

void DialogResample::updateListBox(pp_int32 id, float val, pp_int32 numDecimals)
{
	char buffer1[100];
	char buffer2[100];

	sprintf(buffer1, "%%.%if", numDecimals);

	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(id));
	if (listBox)
	{
		sprintf(buffer2, buffer1, val);
		listBox->clear();
		listBox->addItem(buffer2);
	}
}

void DialogResample::commitChanges()
{
	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
	if (listBox)
		listBox->commitChanges();

	listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_TWO));
	if (listBox)
		listBox->commitChanges();
}

void DialogResample::listBoxEnterEditState(pp_int32 id)
{
	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(id));
	if (listBox)
		listBox->placeCursorAtEnd();
}

void DialogResample::switchListBox()
{
	if (listBoxes[currentSelectedListBox]->isEditing())
		listBoxes[currentSelectedListBox]->commitChanges();
		
	currentSelectedListBox = (currentSelectedListBox+1) % 3;
	
	listBoxEnterEditState(listBoxes[currentSelectedListBox]->getID());
	messageBoxContainerGeneric->setFocus(listBoxes[currentSelectedListBox]);
	parentScreen->paintControl(messageBoxContainerGeneric);
}

void DialogResample::setRelNote(pp_int32 note)
{
	relnote = note;
	toC4Speed();

	if (count < 2)
	{
		count++;
		if (count == 2)
			originalc4spd = c4spd;
	}
}

void DialogResample::setFineTune(pp_int32 ft)
{
	finetune = ft;
	toC4Speed();

	if (count < 2)
	{
		count++;
		if (count == 2)
			originalc4spd = c4spd;
	}
}

void DialogResample::setC4Speed(float c4spd)
{
	this->c4spd = c4spd;
	fromC4Speed();
}

void DialogResample::setSize(pp_uint32 size)
{
	this->size = size;

	calcSize();
}

void DialogResample::toC4Speed()
{
	validate();
	c4spd = getc4spd(relnote, finetune);
	validate();
}

void DialogResample::fromC4Speed()
{
	validate();
	mp_sbyte rn, ft;
	XModule::convertc4spd((mp_uint32)c4spd, &ft, &rn);
	relnote = rn;
	finetune = ft;
	validate();
}

void DialogResample::calcSize()
{
	float c4spd = getc4spd(relnote, finetune);
	float step = originalc4spd / c4spd;

	finalSize = (mp_uint32)(size / step);
}

void DialogResample::validate()
{
	if (relnote > 48)
		relnote = 48;
	if (relnote < -48)
		relnote = -48;

	if (finetune > 127) 
		finetune = 127;
	if (finetune < -128) 
		finetune = -128;
		
	if (c4spd < 0)
		c4spd = 0;

	if (c4spd > 65535*4)
		c4spd = 65535*4;
}
