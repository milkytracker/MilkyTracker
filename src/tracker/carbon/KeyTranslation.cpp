/*
 *  tracker/carbon/KeyTranslation.cpp
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
 *  KeyTranslation.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.11.05.
 *
 */

#include <Carbon/Carbon.h>
#include "KeyTranslation.h"
#include "iGetKeys.h"
#include "Event.h"

static Ascii2KeyCodeTable ascKeyCodeTable;
static InsertKeyShortcuts insertKeyEmulation = InsertKeyShortcutNone;

void InitKeyCodeTranslation()
{
	// keyboard-translator init ------------------------------------	
	InitAscii2KeyCodeTable(&ascKeyCodeTable);
}

void QueryKeyModifiers()
{
	if (TestForKeyDown(kVirtualShiftKey))
		setKeyModifier(KeyModifierSHIFT);
	else
		clearKeyModifier(KeyModifierSHIFT);

	if (TestForKeyDown(kVirtualOptionKey))
		setKeyModifier(KeyModifierALT);
	else
		clearKeyModifier(KeyModifierALT);

	if (TestForKeyDown(kVirtualCommandKey))
		setKeyModifier(KeyModifierCTRL);
	else
		clearKeyModifier(KeyModifierCTRL);
}

// filter key codes which have corresponding ASCII values
pp_uint16 FilterVK(pp_uint16 keyCode)
{
	if ((keyCode >= 'A' && keyCode <= 'Z') ||
		(keyCode >= '0' && keyCode <= '9'))
	{
		return keyCode;
	}
	
	switch (keyCode)
	{
		case VK_BACK:
		case VK_TAB:
		case VK_CLEAR:
		case VK_RETURN:
		case VK_ESCAPE:
		case VK_SPACE:
			return keyCode;
	}
	
	return 0;
}

pp_uint16 KeyCodeToVK(UInt32 keyCode)
{

	switch (keyCode)
	{
		// Strange NUM-lock key or whatever this is
		case 71:
			return VK_UNDEFINED;
	
		// NUMPAD
		case 81:
			return VK_DIVIDE;
		case 75:
			return VK_MULTIPLY;
		case 67:
			return VK_SUBTRACT;
		case 78:
			return VK_ADD;
		case 69:
			return VK_SEPARATOR;
		case 76:
			return VK_DECIMAL;
		//case 65:
		//	return VK_INSERT;

		case 82:
			return VK_NUMPAD0;
		case 83:
			return VK_NUMPAD1;
		case 84:
			return VK_NUMPAD2;
		case 85:
			return VK_NUMPAD3;
		case 86:
			return VK_NUMPAD4;
		case 87:
			return VK_NUMPAD5;
		case 88:
			return VK_NUMPAD6;
		case 89:
			return VK_NUMPAD7;
		case 91:
			return VK_NUMPAD8;
		case 92:
			return VK_NUMPAD9;
	
		// WTF
		case 10:
			return 0xC0;
		
		// key "1" is capslock
		//case 18:
		//	return VK_CAPITAL;
		// Tick
		case 24:
			return 0xBB;
		// SS
		case 27:
			return 0xBD;
		// +
		case 30:
			return 0xDD;
		// Ü
		case 33:
			return 0xDB;
		// Return isn't it?
		case 36:
			// allow using ctrl+enter as replacement for insert
			if (insertKeyEmulation == InsertKeyShortcutCtrlEnter && 
				TestForKeyDown(kVirtualControlKey))
				return VK_INSERT;
			return VK_RETURN;
		// Ä
		case 39:
			return 0xDE;
		// Ö
		case 41:
			return 0xBA;
		// Sharp
		case 42:
			return 0xDC;
		// ,
		case 43:
			return 188;
		// .
		case 44:
			return 0xBF;
		// -
		case 47:
			return 190;
		// Tab
		case 48:
			return VK_TAB;
		// Space
		case 49:
			return VK_SPACE;
		// <, >
		case 50:
			return 0xE2;

		case 51:
			// allow using ctrl+backspace as replacement for insert
			if (insertKeyEmulation == InsertKeyShortcutCtrlBackspace && 
				TestForKeyDown(kVirtualControlKey))
				return VK_INSERT;
			return VK_BACK;

		case 53:
			return VK_ESCAPE;

		case 96:
			return VK_F5;
			
		case 97:
			return VK_F6;

		case 98:
			return VK_F7;

		case 99:
			return VK_F3;

		case 100:
			return VK_F8;

		case 101:
			return VK_F9;

		case 109:
			return VK_F10;

		case 103:
			return VK_F11;
		
		case 111:
			return VK_F12;
		
		case 114:
			return VK_INSERT;

		case 115:
			return VK_HOME;

		case 116:
			return VK_PRIOR;

		case 117:
			return VK_DELETE;

		case 118:
			return VK_F4;

		case 119:
			return VK_END;

		case 120:
			return VK_F2;
		
		case 121:
			return VK_NEXT;
		
		case 122:
			return VK_F1;
		
		case 123:
			return VK_LEFT;

		case 124:
			return VK_RIGHT;

		case 125:
			return VK_DOWN;

		case 126:
			// allow using ctrl+up as replacement for insert
			if (insertKeyEmulation == InsertKeyShortcutCtrlUp && 
				TestForKeyDown(kVirtualControlKey))
				return VK_INSERT;
			return VK_UP;
			
		default:
			if (KeyCodeToAscii(keyCode) >= 'a' && KeyCodeToAscii(keyCode) <= 'z')
				return FilterVK(KeyCodeToAscii(keyCode)-32);
			else
				return FilterVK(KeyCodeToAscii(keyCode));

	}

	return 0;
}

// the following translation is not complete, only keys which are necessary
// are translated
pp_uint16 KeyCodeToSC(UInt32 keyCode)
{
	/*printf("%i\n",keyCode);
	
	return 0;*/
	switch (keyCode)
	{
		case 10:
			return 0x29;
		// 1
		case 18:
			//return SC_CAPSLOCK;
			return 0x02;
		// 2
		case 19:
			return 0x03;
		// 3
		case 20:
			return 0x04;
		// 4
		case 21:
			return 0x05;
		// 5
		case 23:
			return 0x06;
		// 6
		case 22:
			return 0x07;
		// 7
		case 26:
			return 0x08;
		// 8
		case 28:
			return 0x09;
		// 9
		case 25:
			return 0x0A;
		// 0
		case 29:
			return 0x0B;
		// SS
		case 27:
			return 0x0c;
		// Tick
		case 24:
			return 0x0d;
		// Q
		case 12:
			return 0x10;
		case 13:
			return 0x11;
		case 14:
			return 0x12;
		case 15:
			return 0x13;
		case 17:
			return 0x14;
		case 16:
			return 0x15;
		case 32:
			return 0x16;
		case 34:
			return 0x17;
		case 31:
			return 0x18;
		case 35:
			return 0x19;
		case 33:
			return 0x1a;
		case 30:
			return 0x1b;
		case 0:
			return 0x1e;
		case 1:
			return 0x1f;
		case 2:
			return 0x20;
		case 3:
			return 0x21;
		case 5:
			return 0x22;
		case 4:
			return 0x23;
		case 38:
			return 0x24;
		case 40:
			return 0x25;
		case 37:
			return 0x26;
		case 41:
			return 0x27;
		case 39:
			return 0x28;
		case 42:
			return 0x2b;
		case 50:
			return 0x56;
		case 6:
			return 0x2c;
		case 7:
			return 0x2d;
		case 8:
			return 0x2e;
		case 9:
			return 0x2f;
		case 11:
			return 0x30;
		case 45:
			return 0x31;
		case 46:
			return 0x32;
		case 43:
			return 0x33;
		case 47:
			return 0x34;
		case 44:
			return 0x35;

		// RETURN
		case 36:
			return 0;

		// BACK
		case 48:
			return 0;
			
		// SPACE
		case 49:
			return 0;

		// BACKSPACE
		case 51:
			return 0;

		// ESCAPE
		case 53:
			return 0;

		// F5
		case 96:
			return 0;
			
		// F6
		case 97:
			return 0;

		// F7
		case 98:
			return 0;

		// F3
		case 99:
			return 0;

		// F9
		case 101:
			return 0;

		// F10
		case 109:
			return 0;

		// F11
		case 103:
			return 0;

		// F8
		case 100:
			return 0;

		// F12
		case 111:
			return 0;

		// INSERT
		case 114:
			return 0;

		// HOME
		case 115:
			return 0;

		// PAGE UP
		case 116:
			return 0;

		// DELETE
		case 117:
			return 0;

		// F4
		case 118:
			return 0;

		// END
		case 119:
			return 0;

		// F2
		case 120:
			return 0;
		
		// PAGE DOWN
		case 121:
			return 0;
		
		// F1
		case 122:
			return 0;
		
		// RIGHT
		case 123:
			return 0;

		// LEFT
		case 124:
			return 0;

		// DOWN
		case 125:
			return 0;

		// UP
		case 126:
			return 0;
			
		// Strange NUM-lock key or whatever this is
		case 71:
			return 0;
	
		// NUMPAD
		case 81:
			return 0;
		case 75:
			return 0;
		case 67:
			return 0;
		case 78:
			return 0;
		case 76:
			return 0;
		case 65:
			return 0;
		case 69:
			return 0;
		case 82:
			return 0;
		case 83:
			return 0;
		case 84:
			return 0;
		case 85:
			return 0;
		case 86:
			return 0;
		case 87:
			return 0;
		case 88:
			return 0;
		case 89:
			return 0;
		case 91:
			return 0;
		case 92:
			return 0;
		
		default:
			printf("Pressed unknown key: %u\n",static_cast<unsigned int>(keyCode));
			return 0;

	}

	return 0;
}

void enableInsertKeyEmulation(InsertKeyShortcuts shortcut)
{
	insertKeyEmulation = shortcut;
}

