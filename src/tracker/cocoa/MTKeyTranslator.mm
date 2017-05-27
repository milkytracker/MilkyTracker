/*
 *  tracker/cocoa/MTKeyTranslator.mm
 *
 *  Copyright 2014 Dale Whinham
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

#import "MTKeyTranslator.h"

@implementation MTKeyTranslator

// ---------------------------------------------------------
//  Converts Cocoa Event keycode to MilkyTracker VirtualKey
// ---------------------------------------------------------
+ (pp_uint16)toVK:(unsigned short) keyCode
{
	switch (keyCode)
	{
		// Letters
		case kVK_ANSI_A:	return 'A';
		case kVK_ANSI_B:	return 'B';
		case kVK_ANSI_C:	return 'C';
		case kVK_ANSI_D:	return 'D';
		case kVK_ANSI_E:	return 'E';
		case kVK_ANSI_F:	return 'F';
		case kVK_ANSI_G:	return 'G';
		case kVK_ANSI_H:	return 'H';
		case kVK_ANSI_I:	return 'I';
		case kVK_ANSI_J:	return 'J';
		case kVK_ANSI_K:	return 'K';
		case kVK_ANSI_L:	return 'L';
		case kVK_ANSI_M:	return 'M';
		case kVK_ANSI_N:	return 'N';
		case kVK_ANSI_O:	return 'O';
		case kVK_ANSI_P:	return 'P';
		case kVK_ANSI_Q:	return 'Q';
		case kVK_ANSI_R:	return 'R';
		case kVK_ANSI_S:	return 'S';
		case kVK_ANSI_T:	return 'T';
		case kVK_ANSI_U:	return 'U';
		case kVK_ANSI_V:	return 'V';
		case kVK_ANSI_W:	return 'W';
		case kVK_ANSI_X:	return 'X';
		case kVK_ANSI_Y:	return 'Y';
		case kVK_ANSI_Z:	return 'Z';
			
		// Numbers
		case kVK_ANSI_0:	return '0';
		case kVK_ANSI_1:	return '1';
		case kVK_ANSI_2:	return '2';
		case kVK_ANSI_3:	return '3';
		case kVK_ANSI_4:	return '4';
		case kVK_ANSI_5:	return '5';
		case kVK_ANSI_6:	return '6';
		case kVK_ANSI_7:	return '7';
		case kVK_ANSI_8:	return '8';
		case kVK_ANSI_9:	return '9';
			
		// Numeric keypad
		case kVK_ANSI_Keypad0:			return VK_NUMPAD0;
		case kVK_ANSI_Keypad1:			return VK_NUMPAD1;
		case kVK_ANSI_Keypad2:			return VK_NUMPAD2;
		case kVK_ANSI_Keypad3:			return VK_NUMPAD3;
		case kVK_ANSI_Keypad4:			return VK_NUMPAD4;
		case kVK_ANSI_Keypad5:			return VK_NUMPAD5;
		case kVK_ANSI_Keypad6:			return VK_NUMPAD6;
		case kVK_ANSI_Keypad7:			return VK_NUMPAD7;
		case kVK_ANSI_Keypad8:			return VK_NUMPAD8;
		case kVK_ANSI_Keypad9:			return VK_NUMPAD9;
		case kVK_ANSI_KeypadEquals:		return VK_DIVIDE;	// See numpad Mac key mappings
		case kVK_ANSI_KeypadDivide:		return VK_MULTIPLY; // in MilkyTracker docs to understand
		case kVK_ANSI_KeypadMultiply:	return VK_SUBTRACT; // why this looks messed up... :)
		case kVK_ANSI_KeypadMinus:		return VK_ADD;
		case kVK_ANSI_KeypadPlus:		return VK_SEPARATOR;
		case kVK_ANSI_KeypadEnter:		return VK_DECIMAL;
			
		// Modifier keys
		case kVK_Command:		return VK_CONTROL;
		case kVK_Option:		return VK_ALT;
		case kVK_RightCommand:	return VK_RMENU;	// MilkyTracker uses VK_RMENU for Play Pattern
		case kVK_RightOption:	return VK_RCONTROL; // and VK_RCONTROL for Play Song
		
		case kVK_Shift:
		case kVK_RightShift:	return VK_SHIFT;
			
		case kVK_CapsLock:		return VK_CAPITAL;
			
		// Other non-character keys
		case kVK_Return:		return VK_RETURN;
		case kVK_Tab:			return VK_TAB;
		case kVK_Space:			return VK_SPACE;
		case kVK_Escape:		return VK_ESCAPE;
			
		case kVK_ForwardDelete:	return VK_DELETE;
		case kVK_Delete:		return VK_BACK;
			
		case kVK_LeftArrow:		return VK_LEFT;
		case kVK_RightArrow:	return VK_RIGHT;
		case kVK_DownArrow:		return VK_DOWN;
		case kVK_UpArrow:		return VK_UP;
			
		case kVK_PageUp:		return VK_PRIOR;
		case kVK_PageDown:		return VK_NEXT;
		case kVK_Home:			return VK_HOME;
		case kVK_End:			return VK_END;
			
		// Function keys
		case kVK_F1:			return VK_F1;
		case kVK_F2:			return VK_F2;
		case kVK_F3:			return VK_F3;
		case kVK_F4:			return VK_F4;
		case kVK_F5:			return VK_F5;
		case kVK_F6:			return VK_F6;
		case kVK_F7:			return VK_F7;
		case kVK_F8:			return VK_F8;
		case kVK_F9:			return VK_F9;
		case kVK_F10:			return VK_F10;
		case kVK_F11:			return VK_F11;
		case kVK_F12:			return VK_F12;
		case kVK_F13:			return VK_INSERT;

		default:				return VK_UNDEFINED;
	}
}

// -------------------------------------------------------
//  Converts Cocoa Event keycode to MilkyTracker Scancode
// -------------------------------------------------------
+ (pp_uint16)toSC:(unsigned short) keyCode
{
	switch (keyCode)
	{
		// Letters
		case kVK_ANSI_A:	return SC_A;
		case kVK_ANSI_B:	return SC_B;
		case kVK_ANSI_C:	return SC_C;
		case kVK_ANSI_D:	return SC_D;
		case kVK_ANSI_E:	return SC_E;
		case kVK_ANSI_F:	return SC_F;
		case kVK_ANSI_G:	return SC_G;
		case kVK_ANSI_H:	return SC_H;
		case kVK_ANSI_I:	return SC_I;
		case kVK_ANSI_J:	return SC_J;
		case kVK_ANSI_K:	return SC_K;
		case kVK_ANSI_L:	return SC_L;
		case kVK_ANSI_M:	return SC_M;
		case kVK_ANSI_N:	return SC_N;
		case kVK_ANSI_O:	return SC_O;
		case kVK_ANSI_P:	return SC_P;
		case kVK_ANSI_Q:	return SC_Q;
		case kVK_ANSI_R:	return SC_R;
		case kVK_ANSI_S:	return SC_S;
		case kVK_ANSI_T:	return SC_T;
		case kVK_ANSI_U:	return SC_U;
		case kVK_ANSI_V:	return SC_V;
		case kVK_ANSI_W:	return SC_W;
		case kVK_ANSI_X:	return SC_X;
		case kVK_ANSI_Y:	return SC_Z; // MilkyTracker scancodes are based on German
		case kVK_ANSI_Z:	return SC_Y; // keyboard layout; so Y <-> Z swap is correct here.
			
		// Numbers
		case kVK_ANSI_0:	return SC_0;
		case kVK_ANSI_1:	return SC_1;
		case kVK_ANSI_2:	return SC_2;
		case kVK_ANSI_3:	return SC_3;
		case kVK_ANSI_4:	return SC_4;
		case kVK_ANSI_5:	return SC_5;
		case kVK_ANSI_6:	return SC_6;
		case kVK_ANSI_7:	return SC_7;
		case kVK_ANSI_8:	return SC_8;
		case kVK_ANSI_9:	return SC_9;
			
		// Special characters
		case kVK_ANSI_Minus:		return SC_SS;
		case kVK_ANSI_Equal:		return SC_TICK;
		case kVK_ANSI_LeftBracket:	return SC_UE;
		case kVK_ANSI_RightBracket:	return SC_PLUS;
		
		case kVK_ANSI_Semicolon:	return SC_OE;
		case kVK_ANSI_Quote:		return SC_AE;
		case kVK_ANSI_Backslash:	return SC_SHARP;
			
		case kVK_ANSI_Grave:		return SC_SMALLERGREATER;
		case kVK_ANSI_Comma:		return SC_COMMA;
		case kVK_ANSI_Period:		return SC_PERIOD;
		case kVK_ANSI_Slash:		return SC_MINUS;
		case kVK_ISO_Section:		return SC_WTF;	// WTF is actually a Section sign
			
		// Modifiers
		case kVK_CapsLock:			return SC_CAPSLOCK;

		default:					return 0;
	}
}

@end