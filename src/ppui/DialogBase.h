/*
 *  ppui/DialogBase.h
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
 *  DialogBase.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 06.10.05.
 *
 */

#ifndef __DIALOGBASE_H__
#define __DIALOGBASE_H__

#include "BasicTypes.h"
#include "Event.h"

class PPScreen;
class PPMessageBoxContainer;

class PPDialogBase : public EventListenerInterface
{
public:
	enum MessageBoxTypes
	{
		MessageBox_OK,
		MessageBox_OKCANCEL,
		MessageBox_YESNOCANCEL
	};

private:
	DialogResponder* respondListener;
	pp_int16 keyDownInvokeKeyCode;
	
protected:
	PPScreen* parentScreen;
	PPMessageBoxContainer* messageBoxContainerGeneric;

	enum
	{
		MESSAGEBOX_STATICTEXT_MAIN_CAPTION		=	1,
		MESSAGEBOX_STATICTEXT_VALUE_ONE_CAPTION	=	2,
		MESSAGEBOX_STATICTEXT_VALUE_TWO_CAPTION	=	3,
		MESSAGEBOX_STATICTEXT_USER1				=	4,
		MESSAGEBOX_STATICTEXT_USER2				=	5,
		MESSAGEBOX_LISTBOX_VALUE_ONE			=	100,
		MESSAGEBOX_LISTBOX_VALUE_TWO			=	200,
		MESSAGEBOX_LISTBOX_VALUE_THREE			=	300,
		MESSAGEBOX_LISTBOX_USER1				=	400,
		MESSAGEBOX_CONTROL_USER1				=	500,
		MESSAGEBOX_CONTROL_USER2,
		MESSAGEBOX_CONTROL_USER3,

		MESSAGEBOX_BUTTON_INCREASE_VALUEONE		=	(MESSAGEBOX_LISTBOX_VALUE_ONE+1),
		MESSAGEBOX_BUTTON_DECREASE_VALUEONE		=	(MESSAGEBOX_LISTBOX_VALUE_ONE+2),

		MESSAGEBOX_BUTTON_INCREASE_VALUETWO		=	(MESSAGEBOX_LISTBOX_VALUE_TWO+1),
		MESSAGEBOX_BUTTON_DECREASE_VALUETWO		=	(MESSAGEBOX_LISTBOX_VALUE_TWO+2),

		MESSAGEBOX_BUTTON_INCREASE_VALUETHREE	=	(MESSAGEBOX_LISTBOX_VALUE_THREE+1),
		MESSAGEBOX_BUTTON_DECREASE_VALUETHREE	=	(MESSAGEBOX_LISTBOX_VALUE_THREE+2),

		MESSAGEBOX_BUTTON_KEYS_BASE				=	1000
	};

	void initDialog(PPScreen* screen, 
					DialogResponder* responder,
					pp_int32 id,
					const PPString& caption,
					const PPString& message,
					pp_int32 captionOffset,
					const PPString& buttonYesCaption);
	
	void initDialog(PPScreen* screen, 
					DialogResponder* responder,
					pp_int32 id,
					const PPString& caption,
					pp_int32 width,
					pp_int32 height,
					pp_int32 captionOffset,
					const PPString& buttonYesCaption);
	
	void initDialog(PPScreen* screen, 
					DialogResponder* responder,
					pp_int32 id,
					const PPString& caption,
					pp_int32 width,
					pp_int32 height,
					pp_int32 captionOffset,
					const PPString& buttonYesCaption,
					const PPString& buttonCancelCaption);
	
	void initDialog(PPScreen* screen, 
					DialogResponder* responder,
					pp_int32 id,
					const PPString& caption,
					pp_int32 width,
					pp_int32 height,
					pp_int32 captionOffset,
					const PPString& buttonYesCaption,
					const PPString& buttonNoCaption,
					const PPString& buttonCancelCaption);
	
protected:
	PPDialogBase() :
		respondListener(NULL),
		keyDownInvokeKeyCode(-1),
		parentScreen(NULL),
		messageBoxContainerGeneric(NULL)
	{ 
	}
		
public:
	PPDialogBase(PPScreen* screen, 
				 DialogResponder* responder,
				 pp_int32 id,
				 const PPString& caption,
				 MessageBoxTypes type = MessageBox_OKCANCEL);
	
	PPDialogBase(PPScreen* screen, 
				 DialogResponder* responder,
				 pp_int32 id, 
				 const PPString& caption,
				 const PPString& message);
	
	virtual ~PPDialogBase();
	
	void setResponder(DialogResponder* responder) { respondListener = responder; }
	
	PPMessageBoxContainer* getMessageBoxContainer() const { return messageBoxContainerGeneric; }
	void setMessageBoxContainer(PPMessageBoxContainer* container);
	
	virtual void show(bool b = true);
	
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	
		
	void setUserButtonText(pp_int32 index, const PPString& caption);
	
	void setKeyDownInvokeKeyCode(pp_int16 keyCode) { keyDownInvokeKeyCode = keyCode; }

	pp_int32 getID() const;
		
private:
	void initCommon(PPScreen* screen, 
					DialogResponder* responder,
					pp_int32 id, 
					const PPString& caption, 
					pp_int32 width, 
					pp_int32 height,
					pp_int32& x, pp_int32& y);

	void sendKey(EEventDescriptor event, pp_uint16 vk, pp_uint16 sc, pp_uint16 chr);
};

#endif

