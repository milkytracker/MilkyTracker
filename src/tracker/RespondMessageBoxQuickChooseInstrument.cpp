/*
 *  RespondMessageBoxWithValues.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "RespondMessageBoxQuickChooseInstrument.h"
#include "Screen.h"
#include "StaticText.h"
#include "MessageBoxContainer.h"
#include "Font.h"
#include "ListBox.h"
#include "Seperator.h"
#include "ControlIDs.h"

RespondMessageBoxQuickChooseInstrument::RespondMessageBoxQuickChooseInstrument(PPScreen* screen, 
					  RespondListenerInterface* responder,
					  pp_int32 id,
					  const PPString& caption) :
	RespondMessageBox()
{
	value = 0;
	valueRangeStart = 0;
	valueRangeEnd = 0xFF;
	valueIncreaseStep = 1;

#ifdef __LOWRES__
	initRespondMessageBox(screen, responder, id, caption, 290, 110+15, 26+15, "Ok", "Cancel");
#else
	initRespondMessageBox(screen, responder, id, caption, 290, 110, 26, "Ok", "Cancel");
#endif

	pp_int32 x = getMessageBoxContainer()->getLocation().x;
	pp_int32 y = getMessageBoxContainer()->getLocation().y;
	
	pp_int32 width = getMessageBoxContainer()->getSize().width;
	pp_int32 height = getMessageBoxContainer()->getSize().height;
	
	pp_int32 x2 = x;
	pp_int32 y2 = getMessageBoxContainer()->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y;

	PPButton* button;

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
	const char buttonTexts[] = {'1','2','3','4','5','6','7','8','9','0','+','-','.','<','>'};
	
	pp_int32 bWidth = (width - 22*2 - 2*3) / sizeof(buttonTexts);
	pp_int32 x2_2 = x+4;
	
	pp_int32 y2_2 = y+height - 4 - 13;
	
	messageBoxContainerGeneric->addControl(new PPSeperator(0, screen, PPPoint(x2_2-1, y2_2-3), width-2*3, messageBoxContainerGeneric->getColor(), true));
	
	pp_int32 i = 0;
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
	
	fitListBoxes();
	updateListBoxes();
}

void RespondMessageBoxQuickChooseInstrument::setValueCaption(const PPString& caption)
{
	PPControl* ctrl = messageBoxContainerGeneric->getControlByID(MESSAGEBOX_STATICTEXT_VALUE_ONE_CAPTION);
	if (ctrl)
		messageBoxContainerGeneric->removeControl(ctrl);

	pp_int32 width = messageBoxContainerGeneric->getSize().width;
	pp_int32 height = messageBoxContainerGeneric->getSize().height;

	pp_int32 x = messageBoxContainerGeneric->getLocation().x;
	pp_int32 y = messageBoxContainerGeneric->getControlByID(MESSAGEBOX_STATICTEXT_MAIN_CAPTION)->getLocation().y;

	y+=20;

	x += width / 2 - (PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(caption) / 2);

	messageBoxContainerGeneric->addControl(new PPStaticText(MESSAGEBOX_STATICTEXT_VALUE_ONE_CAPTION, parentScreen, this, PPPoint(x, y), caption, true));
}

void RespondMessageBoxQuickChooseInstrument::show()
{
	listBoxEnterEditState(MESSAGEBOX_LISTBOX_VALUE_ONE);
	RespondMessageBox::show();	
}

pp_uint16 RespondMessageBoxQuickChooseInstrument::numPadKeyToValue(pp_uint16 keyCode)
{
	switch (keyCode)
	{
		case VK_NUMPAD0:
			return 0;
		case VK_NUMPAD1:
			return 1;
		case VK_NUMPAD2:
			return 2;
		case VK_NUMPAD3:
			return 3;
		case VK_NUMPAD4:
			return 4;
		case VK_NUMPAD5:
			return 5;
		case VK_NUMPAD6:
			return 6;
		case VK_NUMPAD7:
			return 7;
		case VK_NUMPAD8:
			return 8;
		case VK_NUMPAD9:
			return 9;			
		case VK_DIVIDE:
			return 0x0A;
		case VK_MULTIPLY:
			return 0x0B;
		case VK_SUBTRACT:
			return 0x0C;
		case VK_ADD:
			return 0x0D;
		case VK_SEPARATOR:
			return 0x0E;
		case VK_DECIMAL:
			return 0x0F;
		default:
			return 0xFFFF;
	}
}

static pp_uint8 getNibble(const char* str)
{
	if (*str >= '0' && *str <= '9')
		return (*str - '0');
	if (*str >= 'A' && *str <= 'F')
		return (*str - 'A' + 10);
	if (*str >= 'a' && *str <= 'f')
		return (*str - 'a' + 10);
		
	return 0;
}

static pp_uint8 getByte(const char* str)
{
	return (getNibble(str)<<4) + getNibble(str+1);
}

pp_int32 RespondMessageBoxQuickChooseInstrument::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eKeyChar)
	{
		event->cancel();
	}
	else if (event->getID() == eKeyDown)
	{
		RespondMessageBox::handleEvent(sender, event);
		
		if (event->getID() == eInvalid)
			return 0;
		
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());
		
		pp_uint16 chr = numPadKeyToValue(keyCode);
		if (chr <= 0xF)
		{
			PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
			
			const char* transTab = "0123456789abcdef";
			chr = transTab[chr];
			listBox->callEventListener(event);
			PPEvent event2(eKeyChar, &chr, sizeof(chr));
			listBox->callEventListener(&event2);
			
			if (listBox->getItem(0).length() == 2)
			{
				commitChanges();
				PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_YES));
				
				if (button)
				{
					PPEvent event(eCommand);
					RespondMessageBox::handleEvent(reinterpret_cast<PPObject*>(button), &event);
				}
			}
				
			return 0;
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
				setValue(value+valueIncreaseStep);
				parentScreen->paintControl(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
				break;
			}

			case MESSAGEBOX_BUTTON_DECREASE_VALUEONE:
			{
				setValue(value-valueIncreaseStep);
				parentScreen->paintControl(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
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
				
				setValue(getByte(*str));
				break;
			}
		}
	}
	
	return RespondMessageBox::handleEvent(sender, event);
}

void RespondMessageBoxQuickChooseInstrument::fitListBoxes()
{
	fitListBox(MESSAGEBOX_LISTBOX_VALUE_ONE, valueRangeStart, valueRangeEnd);
}
	
void RespondMessageBoxQuickChooseInstrument::fitListBox(pp_int32 id, pp_int32 valueOneRangeStart, pp_int32 valueOneRangeEnd)
{
	pp_int32 width = messageBoxContainerGeneric->getSize().width;
	pp_int32 x = messageBoxContainerGeneric->getLocation().x;

	char buffer1[100];
	char buffer2[100];

	sprintf(buffer1, "%%x");

	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(id));
	if (listBox)
	{
		sprintf(buffer2, buffer1, valueRangeStart);
		pp_int32 len = (pp_int32)strlen(buffer2)+1;
		sprintf(buffer2, buffer1,  valueRangeEnd);
		if ((pp_int32)strlen(buffer2)+1 > len) len = (pp_int32)strlen(buffer2)+1;
		
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

void RespondMessageBoxQuickChooseInstrument::updateListBoxes()
{
	updateListBox(MESSAGEBOX_LISTBOX_VALUE_ONE, value);
}

void RespondMessageBoxQuickChooseInstrument::updateListBox(pp_int32 id, pp_int32 val)
{
	pp_int32 width = messageBoxContainerGeneric->getSize().width;
	pp_int32 x = messageBoxContainerGeneric->getLocation().x;

	char buffer1[100];
	char buffer2[100];

	sprintf(buffer1, "%%x");

	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(id));
	if (listBox)
	{
		sprintf(buffer2, buffer1, val);
		listBox->clear();
		listBox->addItem(buffer2);
	}
}

void RespondMessageBoxQuickChooseInstrument::commitChanges()
{
	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(MESSAGEBOX_LISTBOX_VALUE_ONE));
	if (listBox)
		listBox->commitChanges();
}

void RespondMessageBoxQuickChooseInstrument::listBoxEnterEditState(pp_int32 id)
{
	PPListBox* listBox = static_cast<PPListBox*>(messageBoxContainerGeneric->getControlByID(id));
	if (listBox)
		listBox->placeCursorAtEnd();
}
