/*
 *  KeyboardBindingHandler.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 29.04.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef KEYBOARDBINDINGHANDLER__H
#define KEYBOARDBINDINGHANDLER__H

#include "BasicTypes.h"
#include "Event.h"

class KeyboardBindingHandler
{
private:
	pp_uint32 keyModifier;

public:
	KeyboardBindingHandler() :
		keyModifier(0)
	{
	}
	
protected:
	bool processKeyEvents(PPEvent* event);

	void resetKeyModifier() { keyModifier = 0; }
	pp_uint32 getKeyModifier() { return keyModifier; }
};

#endif
