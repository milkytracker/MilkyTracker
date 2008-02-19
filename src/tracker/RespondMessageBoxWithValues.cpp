/*
 *  tracker/RespondMessageBoxWithValues.cpp
 *
 *  Copyright 2008 Peter Barth
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
 *  RespondMessageBoxWithValues.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *
 */

#include "RespondMessageBoxWithValues.h"
#include "Screen.h"
#include "StaticText.h"
#include "MessageBoxContainer.h"
#include "Font.h"
#include "ListBox.h"
#include "Seperator.h"
#include "ControlIDs.h"

RespondMessageBoxWithValues::RespondMessageBoxWithValues(PPScreen* screen, 
					  RespondListenerInterface* responder,
					  pp_int32 id,
					  const PPString& caption,
					  ValueStyles style) :
	RespondMessageBox()
{
	valueOne = 0;
	valueTwo = 0;
	valueOneRangeStart = (float)-0x7FFFFFFF;
	valueOneRangeEnd = (float)0x7FFFFFFE;
	valueTwoRangeStart = (float)-0x7FFFFFFF;
	valueTwoRangeEnd = (float)0x7FFFFFFE;
	valueOneIncreaseStep = 1.0f;
	valueTwoIncreaseStep = 1.0f;
	numValueOneDecimals = 0; 
	numValueTwoDecimals = 0;

	switch (style)
	{
		case ValueStyleEnterOneValue:
#ifdef __LOWRES__
			initRespondMessageBox(screen, responder, id, caption, 290, 110+15, 26+15, "Ok", "Cancel");
#else
			initRespondMessageBox(screen, responder, id, caption, 290, 110, 26, "Ok", "Cancel");
#endif
			break;
			
		case ValueStyleEnterTwoValues:
#ifdef __LOWRES__
			initRespondMessageBox(screen, responder, id, caption, 290, 142+15, 26+15, "Ok", "Cancel");
#else
			initRespondMessageBox(screen, responder, id, caption, 290, 142, 26, "Ok", "Cancel");
#endif
			break;
	}

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	
	pp_int32 width = getMessageBoxContainer()->getSize().width;
	
	pp_int32 x2 = x;
	pp_int32 y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y;

	PPButton* button;

	if (style == ValueStyleEnterOneValue || 
		style == ValueStyleEnterTwoValues)  
	{
		//setValueOneCaption("Enter new volume in percent");
		y2 +=20+12;
		
		// enter edit field 1
		x2 = x + width / 2 - (100+35)/2;
		
		PPListBox* listBox = new PPListBox(MESSAGEBOX_LISTBOX_VALUE_ONE, screen, this, PPPoint(x2, y2), PPSize(100,12), true, true, false);
		listBox->showSelection(false);
		listBox->setBorderColor(messageBoxContainerGeneric->getColor());
		listBox->setMaxEditSize(8);
		messageBoxContainerGeneric->addControl(listBox);	
		
		button = new PPButton(MESSAGEBOX_BUTTON_INCREASE_VALUEONE, screen, this, PPPoint(x2 + listBox->getSize().width + 2, y2), PPSize(16, 11));
		button->setText("+");
		messageBoxContainerGeneric->addControl(button);

		button = new PPButton(MESSAGEBOX_BUTTON_DECREASE_VALUEONE, screen, this, PPPoint(x2 + listBox->getSize().width + 2 + button->getSize().width + 1, y2), PPSize(16, 11));
		button->setText("-");
		messageBoxContainerGeneric->addControl(button);

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
	}
	
	if (style == ValueStyleEnterTwoValues)
	{
		//setValueTwoCaption("Enter new volume in percent");
		y2 += 32;
		
		// enter edit field 2
		x2 = x + width / 2 - (100+35)/2;
		
		PPListBox* listBox = new PPListBox(MESSAGEBOX_LISTBOX_VALUE_TWO, screen, this, PPPoint(x2, y2), PPSize(100,12), true, true, false);
		listBox->showSelection(false);
		listBox->setBorderColor(messageBoxContainerGeneric->getColor());
		listBox->setMaxEditSize(8);
		messageBoxContainerGeneric->addControl(listBox);

		button = new PPButton(MESSAGEBOX_BUTTON_INCREASE_VALUETWO, screen, this, PPPoint(x2 + listBox->getSize().width + 2, y2), PPSize(16, 11));
		button->setText("+");
		messageBoxContainerGeneric->addControl(button);

		button = new PPButton(MESSAGEBOX_BUTTON_DECREASE_VALUETWO, screen, this, PPPoint(x2 + listBox->getSize().width + 2 + button->getSize().width + 1, y2), PPSize(16, 11));
		button->setText("-");
		messageBoxContainerGeneric->addControl(button);
	}
		
	fitListBoxes();
	updateListBoxes();
}

void RespondMessageBoxWithValues::setValueOneCaption(const PPString& caption)
{
	PPControl* ctrl = messageBoxContainerGeneric->getControlByID(MESSAGEBOX_STATICTEXT_VALUE_ONE_CAPTION);
	if (ctrl)
		messageBoxContainerGeneric->removeControl(ctrl);

	pp_int32 width = messageBoxContainerGeneric->getSize().width;

	pp_int32 x = messageBoxContainerGeneric->getLocation().x;
	pp_int32 y = messageBoxContainerGeneric->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y;

	y+=20;

	x += width / 2 - (PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(caption) / 2);

	messageBoxContainerGeneric->addControl(new PPStaticText(MESSAGEBOX_STATICTEXT_VALUE_ONE_CAPTION, parentScreen, this, PPPoint(x, y), caption, true));
}

void RespondMessageBoxWithValues::setValueTwoCaption(const PPString& caption)
{
	PPControl* ctrl = messageBoxContainerGeneric->getControlByID(MESSAGEBOX_STATICTEXT_VALUE_TWO_CAPTION);
	if (ctrl)
		messageBoxContainerGeneric->removeControl(ctrl);

	pp_int32 width = messageBoxContainerGeneric->getSize().width;

	pp_int32 x = messageBoxContainerGeneric->getLocation().x;
	pp_int32 y = messageBoxContainerGeneric->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y;

	y+=20+32;

	x += width / 2 - (PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(caption) / 2);

	messageBoxContainerGeneric->addControl(new PPStaticText(MESSAGEBOX_STATICTEXT_VALUE_TWO_CAPTION, parentScreen, this, PPPoint(x, y), caption, true));
}

void RespondMessageBoxWithValues::show()
{
	listBoxEnterEditState(MESSAGEBOX_LISTBOX_VALUE_ONE);
	RespondMessageBox::show();	
}

pp_int32 RespondMessageBoxWithValues::handleEvent(PPObject* sender, PPEvent* event)
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
				setValueOne(valueOne+valueOneIncreaseStep);
				parentScreen->paintControl(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
				break;
			}

			case MESSAGEBOX_BUTTON_DECREASE_VALUEONE:
			{
				setValueOne(valueOne-valueOneIncreaseStep);
				parentScreen->paintControl(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
				break;
			}

			case MESSAGEBOX_BUTTON_INCREASE_VALUETWO:
			{
				setValueTwo(valueTwo+valueOneIncreaseStep);
				parentScreen->paintControl(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_TWO));
				break;
			}

			case MESSAGEBOX_BUTTON_DECREASE_VALUETWO:
			{
				setValueTwo(valueTwo-valueOneIncreaseStep);
				parentScreen->paintControl(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_TWO));
				break;
			}
			
		}
	}
	else if (event->getID() == eValueChanged)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			// song title
			case MESSAGEBOX_LISTBOX_VALUE_ONE:
			{
				PPString* str = *(reinterpret_cast<PPString**>(event->getDataPtr()));
				setValueOne((float)atof(*str));
				break;
			}

			case MESSAGEBOX_LISTBOX_VALUE_TWO:
			{
				PPString* str = *(reinterpret_cast<PPString**>(event->getDataPtr()));
				setValueTwo((float)atof(*str));
				break;
			}
		}
	}
	
	return RespondMessageBox::handleEvent(sender, event);
}

void RespondMessageBoxWithValues::setValueOneRange(float start, float end, pp_int32 numDecimals)
{ 
	valueOneRangeStart = start; valueOneRangeEnd = end;  numValueOneDecimals = numDecimals;
	fitListBoxes();
	setValueOne(valueOne);
}
void RespondMessageBoxWithValues::setValueTwoRange(float start, float end, pp_int32 numDecimals) 
{ 
	valueTwoRangeStart = start; valueTwoRangeEnd = end; numValueTwoDecimals = numDecimals;
	fitListBoxes();
	setValueTwo(valueTwo);
}

void RespondMessageBoxWithValues::fitListBoxes()
{
	fitListBox(MESSAGEBOX_LISTBOX_VALUE_ONE, valueOneRangeStart, valueOneRangeEnd, numValueOneDecimals);
	fitListBox(MESSAGEBOX_LISTBOX_VALUE_TWO, valueTwoRangeStart, valueTwoRangeEnd, numValueTwoDecimals);
}
	
void RespondMessageBoxWithValues::fitListBox(pp_int32 id, float valueOneRangeStart, float valueOneRangeEnd, pp_int32 numDecimals)
{
	pp_int32 width = messageBoxContainerGeneric->getSize().width;
	pp_int32 x = messageBoxContainerGeneric->getLocation().x;

	char buffer1[100];
	char buffer2[100];

	sprintf(buffer1, "%%.%if", numDecimals);

	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(id));
	if (listBox)
	{
		sprintf(buffer2, buffer1, valueOneRangeStart);
		pp_int32 len = (pp_int32)strlen(buffer2);
		sprintf(buffer2, buffer1,  valueOneRangeEnd);
		if ((pp_int32)strlen(buffer2) > len) len = (pp_int32)strlen(buffer2);
		
		pp_int32 y2 = listBox->getLocation().y;
		
		pp_int32 x2 = x + width / 2 - ((len*8)+10+35)/2;
		
		listBox->setLocation(PPPoint(x2, y2));
		listBox->setSize(PPSize(len*8+10, listBox->getSize().height));
	
		x2+=listBox->getSize().width + 2;
		
		PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(id+1));
		y2 = button->getLocation().y;
		button->setLocation(PPPoint(x2, y2));
	
		x2+=button->getSize().width+1;
		button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(id+2));
		y2 = button->getLocation().y;
		button->setLocation(PPPoint(x2, y2));
	}
}

void RespondMessageBoxWithValues::updateListBoxes()
{
	updateListBox(MESSAGEBOX_LISTBOX_VALUE_ONE, valueOne, numValueOneDecimals);
	updateListBox(MESSAGEBOX_LISTBOX_VALUE_TWO, valueTwo, numValueTwoDecimals);
}

void RespondMessageBoxWithValues::updateListBox(pp_int32 id, float val, pp_int32 numDecimals)
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

void RespondMessageBoxWithValues::commitChanges()
{
	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
	if (listBox)
		listBox->commitChanges();

	listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_TWO));
	if (listBox)
		listBox->commitChanges();
}

void RespondMessageBoxWithValues::listBoxEnterEditState(pp_int32 id)
{
	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(id));
	if (listBox)
		listBox->placeCursorAtEnd();
}

void RespondMessageBoxWithValues::switchListBox()
{
	if (messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_TWO) == NULL)
		return;

	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
	if (listBox->isEditing())
	{
		listBox->commitChanges();
		listBoxEnterEditState(MESSAGEBOX_LISTBOX_VALUE_TWO);
		messageBoxContainerGeneric->setFocus(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_TWO));
		parentScreen->paintControl(messageBoxContainerGeneric);
		return;
	}
	
	listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_TWO));
	if (listBox->isEditing())
	{
		listBox->commitChanges();
		listBoxEnterEditState(MESSAGEBOX_LISTBOX_VALUE_ONE);
		messageBoxContainerGeneric->setFocus(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
		parentScreen->paintControl(messageBoxContainerGeneric);
		return;
	}

	listBoxEnterEditState(MESSAGEBOX_LISTBOX_VALUE_ONE);
	messageBoxContainerGeneric->setFocus(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
	parentScreen->paintControl(messageBoxContainerGeneric);
}
