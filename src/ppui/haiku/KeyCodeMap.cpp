/*
 *  Copyright 2012 Julian Harnath
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

#include "KeyCodeMap.h"
#include "VirtualKeys.h"

// Translation tables for Haiku key codes to SC/VK values as expected by
// MilkyTracker. The comments below use US keyboard layout, but the key
// codes are layout-invariant
// Scancodes expected by MilkyTracker are Scancode Set 1
// For more information on Haiku key code assignments see:
// BeBook -> Special Topics -> The Keyboard -> Key Codes

uint16 gKeyCodeToSC[] = {
	        // Code | Corresponding key
	        //      | in US layout
	        // -----+------------------
	0x0,    // 0x00 | [unassigned]
	0x01,   // 0x01 | Esc
	0x3b,   // 0x02 | F1
	0x3c,   // 0x03 | F2
	0x3d,   // 0x04 | F3
	0x3e,   // 0x05 | F4
	0x3f,   // 0x06 | F5
	0x40,   // 0x07 | F6
	0x41,   // 0x08 | F7
	0x42,   // 0x09 | F8
	0x43,   // 0x0a | F9
	0x44,   // 0x0b | F10
	0x57,   // 0x0c | F11
	0x58,   // 0x0d | F12
	0x0,    // 0x0e | Print Screen  NOTE: 4 byte scancode, can't use it here
	0x46,   // 0x0f | Scroll Lock
	0x0 ,   // 0x10 | Pause / Break NOTE: 4 byte scancode, can't use it here
	0x29,   // 0x11 | `
	0x02,   // 0x12 | 1
	0x03,   // 0x13 | 2
	0x04,   // 0x14 | 3
	0x05,   // 0x15 | 4
	0x06,   // 0x16 | 5
	0x07,   // 0x17 | 6
	0x08,   // 0x18 | 7
	0x09,   // 0x19 | 8
	0x0a,   // 0x1a | 9
	0x0b,   // 0x1b | 0
	0x0c,   // 0x1c | -
	0x0d,   // 0x1d | =
	0x0e,   // 0x1e | Backspace
	0xe152, // 0x1f | Insert
	0xe147, // 0x20 | Home
	0xe149, // 0x21 | Page Up
	0xe147, // 0x22 | Num Lock
	0xe135, // 0x23 | / (keypad)
	0x37,   // 0x24 | * (keypad)
	0x4a,   // 0x25 | - (keypad)
	0x0f,   // 0x26 | Tab
	0x10,   // 0x27 | Q
	0x11,   // 0x28 | W
	0x12,   // 0x29 | E
	0x13,   // 0x2a | R
	0x14,   // 0x2b | T
	0x15,   // 0x2c | Y
	0x16,   // 0x2d | U
	0x17,   // 0x2e | I
	0x18,   // 0x2f | O
	0x19,   // 0x30 | P
	0x1a,   // 0x31 | [
	0x1b,   // 0x32 | ]
	0x2b,   // 0x33 | \ |
	0xe153, // 0x34 | Del
	0xe14f, // 0x35 | End
	0xe151, // 0x36 | Page Down
	0x47,   // 0x37 | 7 (keypad)
	0x48,   // 0x38 | 8 (keypad)
	0x49,   // 0x39 | 9 (keypad)
	0x4e,   // 0x3a | + (keypad)
	0x3a,   // 0x3b | Caps Lock
	0x1e,   // 0x3c | A
	0x1f,   // 0x3d | S
	0x20,   // 0x3e | D
	0x21,   // 0x3f | F
	0x22,   // 0x40 | G
	0x23,   // 0x41 | H
	0x24,   // 0x42 | J
	0x25,   // 0x43 | K
	0x26,   // 0x44 | L
	0x27,   // 0x45 | ;
	0x28,   // 0x46 | '
	0x1c,   // 0x47 | Enter
	0x4b,   // 0x48 | 4 (keypad)
	0x4c,   // 0x49 | 5 (keypad)
	0x4d,   // 0x4a | 6 (keypad)
	0x2a,   // 0x4b | Shift (left)
	0x2c,   // 0x4c | Z
	0x2d,   // 0x4d | X
	0x2e,   // 0x4e | C
	0x2f,   // 0x4f | V
	0x30,   // 0x50 | B
	0x31,   // 0x51 | N
	0x32,   // 0x52 | M
	0x33,   // 0x53 | ,
	0x34,   // 0x54 | .
	0x35,   // 0x55 | /
	0x36,   // 0x56 | Shift (right)
	0xe148, // 0x57 | Arrow Up
	0x4f,   // 0x58 | 1 (keypad)
	0x50,   // 0x59 | 2 (keypad)
	0x51,   // 0x5a | 3 (keypad)
	0xe01c, // 0x5b | Enter (keypad)
	0x1d,   // 0x5c | Control (left)
	0x38,   // 0x5d | Alt (left)
	0x39,   // 0x5e | Spacebar
	0xe038, // 0x5f | Alt (right)
	0xe01d, // 0x60 | Control (right)
	0xe14b, // 0x61 | Arrow Left
	0xe150, // 0x62 | Arrow Down
	0xe14d, // 0x63 | Arrow Right
	0x52,   // 0x64 | 0 (keypad)
	0x53,   // 0x65 | . (keypad)
	0xe05b, // 0x66 | Option (left)
	0xe05c, // 0x67 | Option (right)
	0xe05d, // 0x68 | Menu
	0x56    // 0x69 | Euro
};

uint16 gKeyCodeToVK[] = {
	             // Code | Corresponding key
	             //      | in US layout
	             // -----+------------------
	0,           // 0x00 |
	VK_ESCAPE,   // 0x01 | Esc
	VK_F1,       // 0x02 | F1
	VK_F2,       // 0x03 | F2
	VK_F3,       // 0x04 | F3
	VK_F4,       // 0x05 | F4
	VK_F5,       // 0x06 | F5
	VK_F6,       // 0x07 | F6
	VK_F7,       // 0x08 | F7
	VK_F8,       // 0x09 | F8
	VK_F9,       // 0x0a | F9
	VK_F10,      // 0x0b | F10
	VK_F11,      // 0x0c | F11
	VK_F12,      // 0x0d | F12
	VK_PRINT,    // 0x0e | Print Screen
	VK_SCROLL,   // 0x0f | Scroll Lock
	VK_PAUSE,    // 0x10 | Pause / Break
	0,           // 0x11 | `
	0,           // 0x12 | 1
	0,           // 0x13 | 2
	0,           // 0x14 | 3
	0,           // 0x15 | 4
	0,           // 0x16 | 5
	0,           // 0x17 | 6
	0,           // 0x18 | 7
	0,           // 0x19 | 8
	0,           // 0x1a | 9
	0,           // 0x1b | 0
	0,           // 0x1c | -
	0,           // 0x1d | =
	VK_BACK,     // 0x1e | Backspace
	VK_INSERT,   // 0x1f | Insert
	VK_HOME,     // 0x20 | Home
	VK_PRIOR,    // 0x21 | Page Up
	VK_NUMLOCK,  // 0x22 | Num Lock
	VK_DIVIDE,   // 0x23 | / (keypad)
	VK_MULTIPLY, // 0x24 | * (keypad)
	VK_SUBTRACT, // 0x25 | - (keypad)
	VK_TAB,      // 0x26 | Tab
	0,           // 0x27 | Q
	0,           // 0x28 | W
	0,           // 0x29 | E
	0,           // 0x2a | R
	0,           // 0x2b | T
	0,           // 0x2c | Y
	0,           // 0x2d | U
	0,           // 0x2e | I
	0,           // 0x2f | O
	0,           // 0x30 | P
	0,           // 0x31 | [
	0,           // 0x32 | ]
	0,           // 0x33 | \ |
	VK_DELETE,   // 0x34 | Del
	VK_END,      // 0x35 | End
	VK_NEXT,     // 0x36 | Page Down
	VK_NUMPAD7,  // 0x37 | 7 (keypad)
	VK_NUMPAD8,  // 0x38 | 8 (keypad)
	VK_NUMPAD9,  // 0x39 | 9 (keypad)
	VK_ADD,      // 0x3a | + (keypad)
	0,           // 0x3b | Caps Lock
	0,           // 0x3c | A
	0,           // 0x3d | S
	0,           // 0x3e | D
	0,           // 0x3f | F
	0,           // 0x40 | G
	0,           // 0x41 | H
	0,           // 0x42 | J
	0,           // 0x43 | K
	0,           // 0x44 | L
	0,           // 0x45 | ;
	0,           // 0x46 | '
	VK_RETURN,   // 0x47 | Enter
	VK_NUMPAD4,  // 0x48 | 4 (keypad)
	VK_NUMPAD5,  // 0x49 | 5 (keypad)
	VK_NUMPAD6,  // 0x4a | 6 (keypad)
	VK_LSHIFT,   // 0x4b | Shift (left)
	0,           // 0x4c | Z
	0,           // 0x4d | X
	0,           // 0x4e | C
	0,           // 0x4f | V
	0,           // 0x50 | B
	0,           // 0x51 | N
	0,           // 0x52 | M
	0,           // 0x53 | ,
	0,           // 0x54 | .
	0,           // 0x55 | /
	VK_RSHIFT,   // 0x56 | Shift (right)
	VK_UP,       // 0x57 | Arrow Up
	VK_NUMPAD1,  // 0x58 | 1 (keypad)
	VK_NUMPAD2,  // 0x59 | 2 (keypad)
	VK_NUMPAD3,  // 0x5a | 3 (keypad)
	VK_RETURN,   // 0x5b | Enter (keypad)
	VK_LCONTROL, // 0x5c | Control (left)
	VK_LMENU,    // 0x5d | Alt (left)
	VK_SPACE,    // 0x5e | Spacebar
	VK_RMENU,    // 0x5f | Alt (right)
	VK_RCONTROL, // 0x60 | Control (right)
	VK_LEFT,     // 0x61 | Arrow Left
	VK_DOWN,     // 0x62 | Arrow Down
	VK_RIGHT,    // 0x63 | Arrow Right
	VK_NUMPAD0,  // 0x64 | 0 (keypad)
	VK_DECIMAL,  // 0x65 | . (keypad)   TODO TEST
	VK_LWIN,     // 0x66 | Option (left)
	VK_RWIN,     // 0x67 | Option (right)
	VK_APPS,     // 0x68 | Menu
	0            // 0x69 | Euro
};


uint16 KeyCodeToVK(int32 keyCode, char character)
{
	uint16 vk = gKeyCodeToVK[keyCode];

	if (vk != 0)
		return vk;

	if (character >= 'a' && character <= 'z')
		return (uint16)(character - 'a') + 0x41;

	if (character >= '0' && character <= '9')
		return (uint16)(character - 'a') + 0x30;

	return 0;
}


bool gSwapCommandControl = false;

ModifierData gModifierDataShift = {
	B_SHIFT_KEY, B_LEFT_SHIFT_KEY, B_RIGHT_SHIFT_KEY,
	KeyModifierSHIFT, VK_SHIFT, VK_LSHIFT, VK_RSHIFT
};

ModifierData gModifierDataControl = {
	B_CONTROL_KEY, B_LEFT_CONTROL_KEY, B_RIGHT_CONTROL_KEY,
	KeyModifierCTRL, VK_CONTROL, VK_LCONTROL, VK_RCONTROL
};

ModifierData gModifierDataCommand = {
	B_COMMAND_KEY, B_LEFT_COMMAND_KEY, B_RIGHT_COMMAND_KEY,
	KeyModifierALT, VK_ALT, VK_LMENU, VK_RMENU
};
