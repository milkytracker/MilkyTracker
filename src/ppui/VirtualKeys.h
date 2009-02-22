/*
 *  ppui/VirtualKeys.h
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
 *  VirtualKeys.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Fri Feb 25 2005.
 *
 */

#ifndef VIRTUALKEYS__H
#define VIRTUALKEYS__H

// These might look familiar to windows programmers ;)
enum
{
	VK_UNDEFINED =     0x00,
	
	VK_LBUTTON =       0x01,
	VK_RBUTTON =       0x02,
	VK_CANCEL =        0x03,
	VK_MBUTTON =       0x04,   
	
	VK_BACK =          0x08,
	VK_TAB =           0x09,
	
	VK_CLEAR =         0x0C,
	VK_RETURN =        0x0D,
	
	VK_SHIFT =         0x10,
	VK_CONTROL =       0x11,
	VK_MENU =          0x12,
	VK_PAUSE =         0x13,
	VK_CAPITAL =       0x14,
	
	VK_KANA =          0x15,
	VK_HANGEUL =       0x15 , 
	VK_HANGUL =        0x15,
	VK_JUNJA =         0x17,
	VK_FINAL =         0x18,
	VK_HANJA =         0x19,
	VK_KANJI =         0x19,
	
	VK_ESCAPE =        0x1B,
	
	VK_CONVERT =       0x1C,
	VK_NONCONVERT =    0x1D,
	VK_ACCEPT =        0x1E,
	VK_MODECHANGE =    0x1F,
	
	VK_SPACE =         0x20,
	VK_PRIOR =         0x21,
	VK_NEXT =          0x22,
	VK_END =           0x23,
	VK_HOME =          0x24,
	VK_LEFT =          0x25,
	VK_UP =            0x26,
	VK_RIGHT =         0x27,
	VK_DOWN =          0x28,
	VK_SELECT =        0x29,
	VK_PRINT =         0x2A,
	VK_EXECUTE =       0x2B,
	VK_SNAPSHOT =      0x2C,
	VK_INSERT =        0x2D,
	VK_DELETE =        0x2E,
	VK_HELP =          0x2F,
	
	VK_LWIN =          0x5B,
	VK_RWIN =          0x5C,
	VK_APPS =          0x5D,
	
	VK_NUMPAD0 =       0x60,
	VK_NUMPAD1 =       0x61,
	VK_NUMPAD2 =       0x62,
	VK_NUMPAD3 =       0x63,
	VK_NUMPAD4 =       0x64,
	VK_NUMPAD5 =       0x65,
	VK_NUMPAD6 =       0x66,
	VK_NUMPAD7 =       0x67,
	VK_NUMPAD8 =       0x68,
	VK_NUMPAD9 =       0x69,
	VK_MULTIPLY =      0x6A,
	VK_ADD =           0x6B,
	VK_SEPARATOR =     0x6C,
	VK_SUBTRACT =      0x6D,
	VK_DECIMAL =       0x6E,
	VK_DIVIDE =        0x6F,
	VK_F1 =            0x70,
	VK_F2 =            0x71,
	VK_F3 =            0x72,
	VK_F4 =            0x73,
	VK_F5 =            0x74,
	VK_F6 =            0x75,
	VK_F7 =            0x76,
	VK_F8 =            0x77,
	VK_F9 =            0x78,
	VK_F10 =           0x79,
	VK_F11 =           0x7A,
	VK_F12 =           0x7B,
	VK_F13 =           0x7C,
	VK_F14 =           0x7D,
	VK_F15 =           0x7E,
	VK_F16 =           0x7F,
	VK_F17 =           0x80,
	VK_F18 =           0x81,
	VK_F19 =           0x82,
	VK_F20 =           0x83,
	VK_F21 =           0x84,
	VK_F22 =           0x85,
	VK_F23 =           0x86,
	VK_F24 =           0x87,
	
	VK_NUMLOCK =       0x90,
	VK_SCROLL =        0x91,
	
	VK_LSHIFT =        0xA0,
	VK_RSHIFT =        0xA1,
	VK_LCONTROL =      0xA2,
	VK_RCONTROL =      0xA3,
	VK_LMENU =         0xA4,
	VK_RMENU =         0xA5,
	
	VK_OEM_3 =        0xC0,
	VK_OEM_102 =       0xE2,
	
	VK_PROCESSKEY =    0xE5,
	
	VK_ATTN =          0xF6,
	VK_CRSEL =         0xF7,
	VK_EXSEL =         0xF8,
	VK_EREOF =         0xF9,
	VK_PLAY =          0xFA,
	VK_ZOOM =          0xFB,
	VK_NONAME =        0xFC,
	VK_PA1 =           0xFD,
	VK_OEM_CLEAR =     0xFE,
	
	VK_ALT =           0xFF
};

#endif
