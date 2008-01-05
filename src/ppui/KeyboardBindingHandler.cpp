/*
 *  KeyboardBindingHandler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 29.04.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "KeyboardBindingHandler.h"

bool KeyboardBindingHandler::processKeyEvents(PPEvent* event)
{
	switch (event->getID())
	{
		case eKeyDown:
		{	
			//assureCursorVisible();
			
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
	}

	return false;
}
