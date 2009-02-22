/*
 *  ppui/DialogBase.cpp
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
 *  DialogBase.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 06.10.05.
 *
 */

#include "DialogBase.h"
#include "Screen.h"
#include "StaticText.h"
#include "MessageBoxContainer.h"
#include "Font.h"
#include "SimpleVector.h"

// parse string and create a string vector with substrings
// all fitting into width when drawn with font f
static void wrapString(PPSimpleVector<PPString>& stringParts, PPFont& f, const PPString& str, pp_int32 width)
{
	if (f.getStrWidth(str) <= (unsigned)width)
	{ 
		stringParts.add(new PPString(str));		
		return;
	}
	
	PPString strCopy = str;
	
	stringParts.clear();
	
	while (strCopy.length() > 0)
	{
		pp_int32 i = 0;
		pp_int32 j = -1;
		do
		{
			if (i < (signed)strCopy.length() && 
			    strCopy.charAt(i) == ' ')
				j = i;
				
			i++;
		} while (i <= (signed)strCopy.length() && 
			     (signed)f.getStrWidth(strCopy.subString(0,i)) <= width);
		i--;
		
		if (i != (signed)strCopy.length() && j >= 0)
			i = j+1;
		
		stringParts.add(new PPString(strCopy.subString(0,i)));		
		strCopy = strCopy.subString(i,strCopy.length());			
	}
}

void PPDialogBase::initCommon(PPScreen* screen, 
								   DialogResponder* responder,
								   pp_int32 id, 
								   const PPString& caption, 
								   pp_int32 width, 
								   pp_int32 height,
								   pp_int32& x, pp_int32& y)
{
	respondListener = responder;
	parentScreen = screen;
	messageBoxContainerGeneric = NULL;

/*#ifdef __LOWRES__
	height+=15;
#endif*/
	x = screen->getWidth() / 2 - width/2;
	y = screen->getHeight() / 2 - height/2;

	messageBoxContainerGeneric = new PPMessageBoxContainer(id, screen, this, PPPoint(x, y), PPSize(width,height), "System request");
	messageBoxContainerGeneric->show(false);

	pp_int32 x2 = x + width / 2 - (PPFont::getFont(PPFont::FONT_SYSTEM)->getStrWidth(caption) / 2);
	pp_int32 y2 = y + 28;

	messageBoxContainerGeneric->addControl(new PPStaticText(MESSAGEBOX_STATICTEXT_MAIN_CAPTION, screen, this, PPPoint(x2, y2), caption, true));	
}	

void PPDialogBase::initDialog(PPScreen* screen, 
							  DialogResponder* responder,
							  pp_int32 id,
							  const PPString& caption,
							  const PPString& message,
							  pp_int32 captionOffset,
							  const PPString& buttonYesCaption)
{
	respondListener = responder;
	parentScreen = screen;
	messageBoxContainerGeneric = NULL;

	pp_uint32 width = 290;
	
	PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);
	
	PPSimpleVector<PPString> stringList;
	wrapString(stringList, *font, message, width - (font->getCharWidth()*2 + 4));

	pp_uint32 height = 62 + stringList.size()*12;

/*#ifdef __LOWRES__
	height+=15;
#endif*/
	pp_int32 x = screen->getWidth() / 2 - width/2;
	pp_int32 y = screen->getHeight() / 2 - height/2;

	messageBoxContainerGeneric = new PPMessageBoxContainer(id, screen, this, PPPoint(x, y), PPSize(width,height), caption);
	messageBoxContainerGeneric->show(false);

	pp_int32 y2 = y + 28;

	for (pp_int32 i = 0; i < stringList.size(); i++)
	{
		pp_int32 x2 = x + width / 2 - (font->getStrWidth(*stringList.get(i)) / 2);
		
		messageBoxContainerGeneric->addControl(new PPStaticText(MESSAGEBOX_STATICTEXT_MAIN_CAPTION+i, screen, this, PPPoint(x2, y2), *stringList.get(i), true));
		y2+=12;
	}

/*#ifdef __LOWRES__
	pp_int32 y3 = y + height - (26 + 15);
#else
	pp_int32 y3 = y + height - 26;
#endif*/
	pp_int32 y3 = y + height - captionOffset;

	PPButton* button;

	button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, screen, this, PPPoint(x+width/2 - 30, y3), PPSize(60, 11));
	button->setText(buttonYesCaption);
	messageBoxContainerGeneric->addControl(button);
}

void PPDialogBase::initDialog(PPScreen* screen, 
											  DialogResponder* responder,
											  pp_int32 id,
											  const PPString& caption,
											  pp_int32 width,
											  pp_int32 height,
											  pp_int32 captionOffset,
											  const PPString& buttonYesCaption) 
{
	pp_int32 x,y;
	initCommon(screen, responder, id, caption, width, height, x, y);

/*#ifdef __LOWRES__
	pp_int32 y3 = y + height - (26 + 15);
#else
	pp_int32 y3 = y + height - 26;
#endif*/
	pp_int32 y3 = y + height - captionOffset;

	PPButton* button;

	button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, screen, this, PPPoint(x+width/2-30, y3), PPSize(60, 11));
	button->setText(buttonYesCaption);
	messageBoxContainerGeneric->addControl(button);
}

void PPDialogBase::initDialog(PPScreen* screen, 
											  DialogResponder* responder,
											  pp_int32 id,
											  const PPString& caption,
											  pp_int32 width,
											  pp_int32 height,
											  pp_int32 captionOffset,
											  const PPString& buttonYesCaption,
											  const PPString& buttonCancelCaption) 
{
	pp_int32 x,y;
	initCommon(screen, responder, id, caption, width, height, x, y);

/*#ifdef __LOWRES__
	pp_int32 y3 = y + height - (26 + 15);
#else
	pp_int32 y3 = y + height - 26;
#endif*/
	pp_int32 y3 = y + height - captionOffset;

	PPButton* button;

	button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, screen, this, PPPoint(x+width/2-65, y3), PPSize(60, 11));
	button->setText(buttonYesCaption);
	messageBoxContainerGeneric->addControl(button);
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_CANCEL, screen, this, PPPoint(x+width/2+5, y3), PPSize(60, 11));
	button->setText(buttonCancelCaption);
	messageBoxContainerGeneric->addControl(button);
}

void PPDialogBase::initDialog(PPScreen* screen, 
											  DialogResponder* responder,
											  pp_int32 id,
											  const PPString& caption,
											  pp_int32 width,
											  pp_int32 height,
											  pp_int32 captionOffset,
											  const PPString& buttonYesCaption,
											  const PPString& buttonNoCaption,
											  const PPString& buttonCancelCaption) 
{
	pp_int32 x,y;
	initCommon(screen, responder, id, caption, width, height, x, y);

/*#ifdef __LOWRES__
	pp_int32 y3 = y + height - (26 + 15);
#else
	pp_int32 y3 = y + height - 26;
#endif*/
	pp_int32 y3 = y + height - captionOffset;

	PPButton* button;

	button = new PPButton(PP_MESSAGEBOX_BUTTON_YES, screen, this, PPPoint(x+width/2-100, y3), PPSize(60, 11));
	button->setText(buttonYesCaption);
	messageBoxContainerGeneric->addControl(button);
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_NO, screen, this, PPPoint(x+width/2-30, y3), PPSize(60, 11));
	button->setText(buttonNoCaption);
	messageBoxContainerGeneric->addControl(button);
	
	button = new PPButton(PP_MESSAGEBOX_BUTTON_CANCEL, screen, this, PPPoint(x+width/2+40, y3), PPSize(60, 11));
	button->setText(buttonCancelCaption);
	messageBoxContainerGeneric->addControl(button);
}

PPDialogBase::PPDialogBase(PPScreen* screen, 
						   DialogResponder* responder,
						   pp_int32 id, 
						   const PPString& caption,
						   MessageBoxTypes type/* = MessageBox_OKCANCEL*/) :
	keyDownInvokeKeyCode(-1)
{
	switch (type)
	{
		case MessageBox_OK:
			initDialog(screen, responder, id, "System Request", caption, 26, "Okay");
			break;
		case MessageBox_OKCANCEL:
			initDialog(screen, responder, id, caption, 290, 74, 26, "Yes", "No");
			break;
		case MessageBox_YESNOCANCEL:
			initDialog(screen, responder, id, caption, 290, 74, 26, "Yes", "No", "Cancel");
			break;
	}
}

PPDialogBase::PPDialogBase(PPScreen* screen, 
						   DialogResponder* responder,
						   pp_int32 id, 
						   const PPString& caption,
						   const PPString& message) :
	keyDownInvokeKeyCode(-1)
{
	initDialog(screen, responder, id, caption, message, 26, "Okay");
}

PPDialogBase::~PPDialogBase()
{
	delete messageBoxContainerGeneric;
}

void PPDialogBase::show(bool b/* = true*/)
{
	if (b)
		parentScreen->setModalControl(messageBoxContainerGeneric);
	else
		parentScreen->setModalControl(NULL);
}

void PPDialogBase::sendKey(EEventDescriptor event, pp_uint16 vk, pp_uint16 sc, pp_uint16 chr)
{
	if (event == eKeyDown || event == eKeyUp)
	{
		pp_uint16 vksc[3] = {vk, sc, chr};

		PPEvent keyDown(event, &vksc, sizeof(vksc));

		PPControl* ctrl = messageBoxContainerGeneric->getFocusedControl();
		if (ctrl && ctrl->isListBox())
			ctrl->dispatchEvent(&keyDown);
	}
	else if (event == eKeyChar)
	{
		pp_uint16 vksc[2] = {vk, sc};

		PPEvent keyDown(event, &vksc, sizeof(vksc));

		PPControl* ctrl = messageBoxContainerGeneric->getFocusedControl();
		if (ctrl && ctrl->isListBox())
			ctrl->dispatchEvent(&keyDown);
	}
}

#define CALLHANDLER(HANDLERFUNC) \
	if (event->getID() != eCommand) \
		break; \
	if (respondListener && respondListener->HANDLERFUNC(reinterpret_cast<PPObject*>(this)) == 0) \
		show(false); \
	else if (respondListener == NULL) \
		show(false);

pp_int32 PPDialogBase::handleEvent(PPObject* sender, PPEvent* event)
{
#ifdef __LOWRES__
	static const pp_uint16 scanCodesNumbers[] = {SC_1, SC_2, SC_3, SC_4, SC_5, SC_6, SC_7, SC_8, SC_9, SC_0, SC_PLUS, SC_MINUS, SC_PERIOD, 0, 0, 0, 0};
	static const pp_uint16 asciiCodesNumbers[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+', '-', '.', VK_LEFT, VK_RIGHT, VK_DELETE, VK_BACK};
#endif

	if (event->getID() == eKeyDown && keyDownInvokeKeyCode > 0 && *(((pp_int16*)event->getDataPtr())) == keyDownInvokeKeyCode)
	{
		event->cancel();
		return 0;
	}
	else if (event->getID() == eKeyChar && keyDownInvokeKeyCode > 0)
	{
		event->cancel();
	}
	else if (event->getID() == eKeyUp && keyDownInvokeKeyCode > 0 && *(((pp_int16*)event->getDataPtr())) == keyDownInvokeKeyCode)
	{
		keyDownInvokeKeyCode = -1;
		event->cancel();
	}

	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case PP_MESSAGEBOX_BUTTON_YES:
			{
				CALLHANDLER(ActionOkay);
				break;
			}

			case PP_MESSAGEBOX_BUTTON_CANCEL:
			{
				CALLHANDLER(ActionCancel);
				break;
			}

			case PP_MESSAGEBOX_BUTTON_NO:
			{
				CALLHANDLER(ActionNo);
				break;
			}

			case PP_MESSAGEBOX_BUTTON_USER1:
			{
				CALLHANDLER(ActionUser1);
				break;
			}

			case PP_MESSAGEBOX_BUTTON_USER2:
			{
				CALLHANDLER(ActionUser2);
				break;
			}

			case PP_MESSAGEBOX_BUTTON_USER3:
			{
				CALLHANDLER(ActionUser3);
				break;
			}

			case PP_MESSAGEBOX_BUTTON_USER4:
			{
				CALLHANDLER(ActionUser4);
				break;
			}
						
			// Input keys
#ifdef __LOWRES__
			case MESSAGEBOX_BUTTON_KEYS_BASE:	// 1
			case MESSAGEBOX_BUTTON_KEYS_BASE+1:	// 2
			case MESSAGEBOX_BUTTON_KEYS_BASE+2: // 3
			case MESSAGEBOX_BUTTON_KEYS_BASE+3: // 4
			case MESSAGEBOX_BUTTON_KEYS_BASE+4: // 5
			case MESSAGEBOX_BUTTON_KEYS_BASE+5: // 6
			case MESSAGEBOX_BUTTON_KEYS_BASE+6: // 7
			case MESSAGEBOX_BUTTON_KEYS_BASE+7: // 8
			case MESSAGEBOX_BUTTON_KEYS_BASE+8: // 9
			case MESSAGEBOX_BUTTON_KEYS_BASE+9: // 0
			case MESSAGEBOX_BUTTON_KEYS_BASE+10: // +
			case MESSAGEBOX_BUTTON_KEYS_BASE+11: // -
			case MESSAGEBOX_BUTTON_KEYS_BASE+12: // .
			{
				pp_int32 i = (reinterpret_cast<PPControl*>(sender)->getID() - MESSAGEBOX_BUTTON_KEYS_BASE);
				pp_uint16 key[3];
				key[0] = key[2] = (pp_uint16)asciiCodesNumbers[i];
				key[1] = scanCodesNumbers[i];
				sendKey(eKeyDown, key[0], key[1], key[2]);
				
				sendKey(eKeyChar, key[0], key[1], 0);
				sendKey(eKeyUp, key[0], key[1], key[2]);
				break;
			}
			
			case MESSAGEBOX_BUTTON_KEYS_BASE+13: // LEFT
			case MESSAGEBOX_BUTTON_KEYS_BASE+14: // RIGHT
			case MESSAGEBOX_BUTTON_KEYS_BASE+15: // DEL
			case MESSAGEBOX_BUTTON_KEYS_BASE+16: // BACK
			{
				pp_int32 i = (reinterpret_cast<PPControl*>(sender)->getID() - MESSAGEBOX_BUTTON_KEYS_BASE);
				pp_uint16 key[2];
				key[0] = (pp_uint16)asciiCodesNumbers[i];
				key[1] = scanCodesNumbers[i];
				key[2] = 0;
				sendKey(eKeyDown, key[0], key[1], key[2]);				
				sendKey(eKeyUp, key[0], key[1], key[2]);
				break;
			}
#endif
		}
	}
	
	return 0;
}

void PPDialogBase::setUserButtonText(pp_int32 index, const PPString& caption)
{
	PPButton* button = static_cast<PPButton*>(messageBoxContainerGeneric->getControlByID(PP_MESSAGEBOX_BUTTON_USER1+index));
	if (button)
		button->setText(caption);
}


pp_int32 PPDialogBase::getID() const
{
	if (messageBoxContainerGeneric)
		return messageBoxContainerGeneric->getID();
		
	return 0;
}


void PPDialogBase::setMessageBoxContainer(PPMessageBoxContainer* container)
{
	delete messageBoxContainerGeneric;
	messageBoxContainerGeneric = container;
}


