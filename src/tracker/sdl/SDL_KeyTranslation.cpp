/*
 *  tracker/sdl/SDL_KeyTranslation.cpp
 *
 *  Copyright 2009 Peter Barth, Christopher O'Neill, Dale Whinham
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
 *  Created by Peter Barth on 19.11.05.
 *
 *  12/5/14 - Dale Whinham
 *    - Port to SDL2
 *    - Many new SDL2 scancodes added - keyboard shortcuts need thorough testing
 *    - OSX: Command now acts as CTRL (e.g. more native-style Command-C, Command-V copy and paste in MilkyTracker edit mode)
 *    - Attempted to fix mapping of RAlt/RCtrl (RCommand on OSX) for the FTII-style song play/pattern play shortcuts
 *    - CapsLock now acts as noteoff-insert as per MilkyTracker manual
 *
 *  14/8/06 - Christopher O'Neill
 *    - PC specific code added to toSC - allows none qwerty layouts to work
 *    - Small fix in toSC
 *    - Various keys added to toVK - rewritten to use array instead of select
 */

#include "SDL_KeyTranslation.h"

pp_uint16 toVK(const SDL_Keysym& keysym)
{
	switch (keysym.sym)
	{
		// Letters
		case SDLK_a:	return 'A';
		case SDLK_b:	return 'B';
		case SDLK_c:	return 'C';
		case SDLK_d:	return 'D';
		case SDLK_e:	return 'E';
		case SDLK_f:	return 'F';
		case SDLK_g:	return 'G';
		case SDLK_h:	return 'H';
		case SDLK_i:	return 'I';
		case SDLK_j:	return 'J';
		case SDLK_k:	return 'K';
		case SDLK_l:	return 'L';
		case SDLK_m:	return 'M';
		case SDLK_n:	return 'N';
		case SDLK_o:	return 'O';
		case SDLK_p:	return 'P';
		case SDLK_q:	return 'Q';
		case SDLK_r:	return 'R';
		case SDLK_s:	return 'S';
		case SDLK_t:	return 'T';
		case SDLK_u:	return 'U';
		case SDLK_v:	return 'V';
		case SDLK_w:	return 'W';
		case SDLK_x:	return 'X';
		case SDLK_y:	return 'Y';
		case SDLK_z:	return 'Z';

		// Numbers
		case SDLK_0:	return '0';
		case SDLK_1:	return '1';
		case SDLK_2:	return '2';
		case SDLK_3:	return '3';
		case SDLK_4:	return '4';
		case SDLK_5:	return '5';
		case SDLK_6:	return '6';
		case SDLK_7:	return '7';
		case SDLK_8:	return '8';
		case SDLK_9:	return '9';

		// Numeric keypad
		case SDLK_KP_0:			return VK_NUMPAD0;
		case SDLK_KP_1:			return VK_NUMPAD1;
		case SDLK_KP_2:			return VK_NUMPAD2;
		case SDLK_KP_3:			return VK_NUMPAD3;
		case SDLK_KP_4:			return VK_NUMPAD4;
		case SDLK_KP_5:			return VK_NUMPAD5;
		case SDLK_KP_6:			return VK_NUMPAD6;
		case SDLK_KP_7:			return VK_NUMPAD7;
		case SDLK_KP_8:			return VK_NUMPAD8;
		case SDLK_KP_9:			return VK_NUMPAD9;
		case SDLK_KP_DIVIDE:	return VK_DIVIDE;
		case SDLK_KP_MULTIPLY:	return VK_MULTIPLY;
		case SDLK_KP_MINUS:		return VK_SUBTRACT;
		case SDLK_KP_PLUS:		return VK_ADD;
		case SDLK_KP_ENTER:		return VK_SEPARATOR;
		case SDLK_KP_PERIOD:	return VK_DECIMAL;

		// Modifier keys
		case SDLK_LALT:		return VK_ALT;
		case SDLK_RALT:		return VK_RMENU;	// RAlt for 'play current pattern from beginning' in FastTracker II mode
		case SDLK_LSHIFT:	return VK_SHIFT;
		case SDLK_RSHIFT:	return VK_SHIFT;
#ifdef __APPLE__
		case SDLK_LGUI:		return VK_LCONTROL;	// OSX: LCommand = LCtrl for native-style Cut/Copy/Paste etc in MilkyTracker edit mode
		case SDLK_RGUI:		return VK_RCONTROL;	// OSX: RCommand = RCtrl for 'play song from current order' in FastTracker II edit mode
#else
		case SDLK_LCTRL:	return VK_LCONTROL;
		case SDLK_RCTRL:	return VK_RCONTROL;
#endif
		case SDLK_MODE:		return VK_RMENU;
		case SDLK_CAPSLOCK:	return VK_CAPITAL;

		// Other non-character keys
		case SDLK_RETURN:		return VK_RETURN;
		case SDLK_TAB:			return VK_TAB;
		case SDLK_SPACE:		return VK_SPACE;
		case SDLK_ESCAPE:		return VK_ESCAPE;
		case SDLK_INSERT:		return VK_INSERT;
		case SDLK_DELETE:		return VK_DELETE;
		case SDLK_BACKSPACE:	return VK_BACK;

		case SDLK_LEFT:			return VK_LEFT;
		case SDLK_RIGHT:		return VK_RIGHT;
		case SDLK_DOWN:			return VK_DOWN;
		case SDLK_UP:			return VK_UP;

		case SDLK_PAGEUP:		return VK_PRIOR;
		case SDLK_PAGEDOWN:		return VK_NEXT;
		case SDLK_HOME:			return VK_HOME;
		case SDLK_END:			return VK_END;

		// Function keys
		case SDLK_F1:	return VK_F1;
		case SDLK_F2:	return VK_F2;
		case SDLK_F3:	return VK_F3;
		case SDLK_F4:	return VK_F4;
		case SDLK_F5:	return VK_F5;
		case SDLK_F6:	return VK_F6;
		case SDLK_F7:	return VK_F7;
		case SDLK_F8:	return VK_F8;
		case SDLK_F9:	return VK_F9;
		case SDLK_F10:	return VK_F10;
		case SDLK_F11:	return VK_F11;
		case SDLK_F12:	return VK_F12;

		// TODO: Check if the following are required and remove if unused
		case SDLK_CARET:	return 0xC0;
		case SDLK_PLUS:		return 0xDD;
		case SDLK_HASH:		return 0xDC;
		case SDLK_COMMA:	return 0xBC;
		case SDLK_PERIOD:	return 0xBF;
		case SDLK_MINUS:	return 0xBE;
		case SDLK_LESS:		return 0xE2;

		default:			return 0;
	}
}

pp_uint16 toSC(const SDL_Keysym& keysym)
{
	switch (keysym.scancode)
	{
		// Letters
		case SDL_SCANCODE_A:	return SC_A;
		case SDL_SCANCODE_B:	return SC_B;
		case SDL_SCANCODE_C:	return SC_C;
		case SDL_SCANCODE_D:	return SC_D;
		case SDL_SCANCODE_E:	return SC_E;
		case SDL_SCANCODE_F:	return SC_F;
		case SDL_SCANCODE_G:	return SC_G;
		case SDL_SCANCODE_H:	return SC_H;
		case SDL_SCANCODE_I:	return SC_I;
		case SDL_SCANCODE_J:	return SC_J;
		case SDL_SCANCODE_K:	return SC_K;
		case SDL_SCANCODE_L:	return SC_L;
		case SDL_SCANCODE_M:	return SC_M;
		case SDL_SCANCODE_N:	return SC_N;
		case SDL_SCANCODE_O:	return SC_O;
		case SDL_SCANCODE_P:	return SC_P;
		case SDL_SCANCODE_Q:	return SC_Q;
		case SDL_SCANCODE_R:	return SC_R;
		case SDL_SCANCODE_S:	return SC_S;
		case SDL_SCANCODE_T:	return SC_T;
		case SDL_SCANCODE_U:	return SC_U;
		case SDL_SCANCODE_V:	return SC_V;
		case SDL_SCANCODE_W:	return SC_W;
		case SDL_SCANCODE_X:	return SC_X;
		case SDL_SCANCODE_Y:	return SC_Z; // MilkyTracker scancodes are based on German
		case SDL_SCANCODE_Z:	return SC_Y; // keyboard layout; so Y <-> Z swap is correct here.

		// Numbers
		case SDL_SCANCODE_0:	return SC_0;
		case SDL_SCANCODE_1:	return SC_1;
		case SDL_SCANCODE_2:	return SC_2;
		case SDL_SCANCODE_3:	return SC_3;
		case SDL_SCANCODE_4:	return SC_4;
		case SDL_SCANCODE_5:	return SC_5;
		case SDL_SCANCODE_6:	return SC_6;
		case SDL_SCANCODE_7:	return SC_7;
		case SDL_SCANCODE_8:	return SC_8;
		case SDL_SCANCODE_9:	return SC_9;

		// Special characters
		case SDL_SCANCODE_MINUS:			return SC_SS;
		case SDL_SCANCODE_EQUALS:			return SC_TICK;
		case SDL_SCANCODE_LEFTBRACKET:		return SC_UE;
		case SDL_SCANCODE_RIGHTBRACKET:		return SC_PLUS;

		case SDL_SCANCODE_SEMICOLON:		return SC_OE;
		case SDL_SCANCODE_APOSTROPHE:		return SC_AE;
		case SDL_SCANCODE_BACKSLASH:		return SC_SHARP;

		case SDL_SCANCODE_NONUSBACKSLASH:	return SC_SMALLERGREATER;
		case SDL_SCANCODE_COMMA:			return SC_COMMA;
		case SDL_SCANCODE_PERIOD:			return SC_PERIOD;
		case SDL_SCANCODE_SLASH:			return SC_MINUS;
		case SDL_SCANCODE_GRAVE:			return SC_WTF;	// WTF is actually a Section sign

		// Modifiers
		case SDL_SCANCODE_CAPSLOCK:			return SC_CAPSLOCK;

		default:							return 0;
	}
}
