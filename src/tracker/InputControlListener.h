/*
 *  tracker/InputControlListener.h
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
 *  InputControlListener.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 22.04.05.
 *
 */
#ifndef INPUTCONTROLLISTENER__H
#define INPUTCONTROLLISTENER__H

#include "BasicTypes.h"
#include "Event.h"

class Tracker;

class InputControlListener : public EventListenerInterface
{
private:
	Tracker& tracker;

	bool capsLockPressed;
	bool lShiftPressed;
	bool rShiftPressed;

	bool uppercase() { return !(capsLockPressed | lShiftPressed | rShiftPressed); }

	void refreshSIP();

public:
	enum
	{
		KEY_PRESS = 0,
		KEY_RELEASE = 0x10000
	};

	InputControlListener(Tracker& theTracker) :
		tracker(theTracker),
		capsLockPressed(false),
		lShiftPressed(false),
		rShiftPressed(false)
	{
	}

	void sendKey(EEventDescriptor event, pp_uint16 vk, pp_uint16 sc, pp_uint16 chr);
	void sendNote(pp_int32 note, pp_int32 volume = -1);

	// PPEvent listener
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);	

	void handleModifiers();
};

#endif

