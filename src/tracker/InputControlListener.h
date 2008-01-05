/*
 *  InputControlListener.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 22.04.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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

