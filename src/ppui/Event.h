/*
 *  ppui/Event.h
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

/////////////////////////////////////////////////////////////////
//
//	PPEvent classes
//
/////////////////////////////////////////////////////////////////
#ifndef EVENT__H
#define EVENT__H

#include "Object.h"
#include "BasicTypes.h"

// key states
enum KeyModifiers
{
	KeyModifierALT = 1,
	KeyModifierSHIFT = 2,
	KeyModifierCTRL = 4
};

struct TMouseWheelEventParams
{
	PPPoint pos;
	pp_int32 deltaX;
	pp_int32 deltaY;
};

void setKeyModifier(KeyModifiers eModifier);
void clearKeyModifier(KeyModifiers eModifier);
void setForceKeyModifier(KeyModifiers eModifier);
void clearForceKeyModifier(KeyModifiers eModifier);
pp_uint32 getKeyModifier();

enum EEventDescriptor
{
	eInvalid = 0,
	eLMouseDown,
	eLMouseUp,
	eLMouseDoubleClick,
	eLMouseDrag,
	eLMouseRepeat,
	eRMouseDown,
	eRMouseUp,
	eRMouseDoubleClick,
	eRMouseDrag,
	eRMouseRepeat,
	eMMouseDown,
	eMMouseUp,
	eMMouseDoubleClick,
	eMMouseDrag,
	eMMouseRepeat,
	eMouseMoved,
	eMouseEntered,
	eMouseLeft,
	eMouseWheelMoved,
	eBarPosChanged,
	eBarScrollUp,
	eBarScrollDown,
	eKeyDown,
	eKeyChar,
	eKeyUp,
	eFileDragDropped,
	eFileSystemChanged,
	eFocusGained,
	eFocusLost,
	eFocusGainedNoRepaint,
	eFocusLostNoRepaint,
	eRemovedContextMenu,
	eCommand,					// e.g. button pressed once
	eCommandRight,				// e.g. right button pressed once
	eCommandRepeat,				// e.g. button stays pressed
	ePreSelection,				// e.g. list box selection is about to change
	eSelection,					// e.g. list box selection has been made
	eValueChanged,
	eUpdated,
	eUpdateChanged,
	eConfirmed,					// e.g. ok-press
	eCanceled,					// e.g. cancel-press
	eTimer,
	eFullScreen,
	eAppQuit
};

/////////////////////////////////////////////////////////////////
//	Basic event class
/////////////////////////////////////////////////////////////////
class PPEvent : public PPObject
{
private:
	EEventDescriptor ID;

	unsigned char userData[256];
	pp_int32 dataSize;
	pp_int32 metaData;

public:
	PPEvent() :
		ID(eInvalid),
		dataSize(0),
		metaData(0)
	{
	}

	PPEvent(EEventDescriptor ID, pp_int32 theMetaData = 0) :
		ID(ID),
		dataSize(0),
		metaData(theMetaData)
	{
	}

	PPEvent(EEventDescriptor ID, void* dataPtr, pp_int32 dSize, pp_int32 theMetaData = 0) :
		ID(ID),
		dataSize(dSize),
		metaData(theMetaData)
	{
		if (dSize <= (signed)sizeof(userData))
			memcpy(userData, dataPtr, dataSize);
		else
			exit(0);
	}

	PPEvent(const PPEvent& event) :
		ID(event.ID),
		dataSize(event.dataSize),
		metaData(event.metaData)
	{
		memcpy(this->userData, event.userData, event.dataSize);
	}

	EEventDescriptor getID() const { return ID; }

	const void* getDataPtr() const { return userData; }
	pp_int32 getDataSize() const { return dataSize; }

	pp_int32 getMetaData() const { return metaData; }

	void cancel() { ID = eInvalid; }

	bool isMouseEvent() const
	{
		switch (ID)
		{
			case eLMouseDown:
			case eLMouseUp:
			case eLMouseDoubleClick:
			case eLMouseDrag:
			case eLMouseRepeat:
			case eRMouseDown:
			case eRMouseUp:
			case eRMouseDoubleClick:
			case eRMouseDrag:
			case eRMouseRepeat:
			case eMMouseDown:
			case eMMouseUp:
			case eMMouseDoubleClick:
			case eMMouseDrag:
			case eMMouseRepeat:
			case eMouseMoved:
			case eMouseEntered:
			case eMouseLeft:
			case eMouseWheelMoved:
				return true;
			default:
				return false;
		}
	}
};

/////////////////////////////////////////////////////////////////
//	Interface for EventListenerInterface
/////////////////////////////////////////////////////////////////
class EventListenerInterface : public PPObject
{
public:
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event) = 0;
};

enum
{
	PP_MESSAGEBOX_BUTTON_YES		= 31000,
	PP_MESSAGEBOX_BUTTON_OK			= 31000,
	PP_MESSAGEBOX_BUTTON_NO			= 31001,
	PP_MESSAGEBOX_BUTTON_CANCEL		= 31002,
	PP_MESSAGEBOX_BUTTON_USER1		= 31003,
	PP_MESSAGEBOX_BUTTON_USER2		= 31004,
	PP_MESSAGEBOX_BUTTON_USER3		= 31005,
	PP_MESSAGEBOX_BUTTON_USER4		= 31006,
	PP_MESSAGEBOX_BUTTON_USER5		= 31007,
	PP_MESSAGEBOX_BUTTON_USER6		= 31008,
	PP_MESSAGEBOX_BUTTON_USER7		= 31009,
	PP_MESSAGEBOX_BUTTON_USER8		= 31010,
	PP_MESSAGEBOX_BUTTON_USER9		= 31011,
	PP_MESSAGEBOX_BUTTON_USER10		= 31012,
	PP_MESSAGEBOX_BUTTON_USER11		= 31013,
	PP_MESSAGEBOX_BUTTON_USER12		= 31014,
	PP_MESSAGEBOX_BUTTON_USER13		= 31015,
	PP_MESSAGEBOX_BUTTON_USER14		= 31016,
	PP_MESSAGEBOX_BUTTON_USER15		= 31017,

	PP_DEFAULT_ID					= 0x12345678
};

class DialogResponder
{
public:
	virtual pp_int32 ActionOkay(PPObject* sender)	{ return 0; }
	virtual pp_int32 ActionCancel(PPObject* sender) { return 0; }
	virtual pp_int32 ActionNo(PPObject* sender)		{ return 0; }
	virtual pp_int32 ActionUser1(PPObject* sender)	{ return 0; }
	virtual pp_int32 ActionUser2(PPObject* sender)	{ return 0; }
	virtual pp_int32 ActionUser3(PPObject* sender)	{ return 0; }
	virtual pp_int32 ActionUser4(PPObject* sender)	{ return 0; }
};

#endif
