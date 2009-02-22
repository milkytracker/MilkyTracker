/*
 *  ppui/KeyboardBindingHandler.h
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
 *  KeyboardBindingHandler.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 29.04.05.
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
	pp_uint32 getKeyModifier() const { return keyModifier; }
};

#endif
