/*
 *  ppui/KeyboardBindingHandler.cpp
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
 *  KeyboardBindingHandler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 29.04.05.
 *
 */

#include "KeyboardBindingHandler.h"

bool KeyboardBindingHandler::processKeyEvents(PPEvent* event)
{
	switch (event->getID())
	{
		case eKeyDown:
		{	
			pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());

			switch (keyCode)
			{
				// store key modifiers
				case VK_ALT:
					keyModifier |= KeyModifierALT;
					return true;
				case VK_SHIFT:
					keyModifier |= KeyModifierSHIFT;
					return true;
				case VK_CONTROL:
					keyModifier |= KeyModifierCTRL;
					return true;
			}

			break;
		}

		case eKeyUp:
		{	
			pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());

			switch (keyCode)
			{
				// store key modifiers
				case VK_ALT:
					keyModifier &= ~KeyModifierALT;
					return true;
				case VK_SHIFT:
					keyModifier &= ~KeyModifierSHIFT;
					return true;
				case VK_CONTROL:
					keyModifier &= ~KeyModifierCTRL;
					return true;
			}
			break;
		}
		default:
			break;
	}

	return false;
}
