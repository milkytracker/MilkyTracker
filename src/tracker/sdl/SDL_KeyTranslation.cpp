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

static const pp_uint32 sdl_keysym_to_vk[] = {
	SDLK_CARET,		0xC0,
	SDLK_PLUS,		0xDD,
	SDLK_RETURN,	VK_RETURN,
	SDLK_HASH,		0xDC,
	SDLK_COMMA,		188,
	SDLK_PERIOD,	0xBF,
	SDLK_MINUS,		190,
	SDLK_TAB,		VK_TAB,
	SDLK_SPACE,		VK_SPACE,
	SDLK_LESS,		0xE2,
	SDLK_BACKSPACE,	VK_BACK,
	SDLK_ESCAPE,	VK_ESCAPE,
	SDLK_F1,		VK_F1,
	SDLK_F2,		VK_F2,
	SDLK_F3,		VK_F3,
	SDLK_F4,		VK_F4,
	SDLK_F5,		VK_F5,
	SDLK_F6,		VK_F6,
	SDLK_F7,		VK_F7,
	SDLK_F8,		VK_F8,
	SDLK_F9,		VK_F9,
	SDLK_F10,		VK_F10,
	SDLK_F11,		VK_F11,
	SDLK_F12,		VK_F12,
	SDLK_INSERT,	VK_INSERT,
	SDLK_HOME,		VK_HOME,
	SDLK_PAGEUP,	VK_PRIOR,
	SDLK_DELETE,	VK_DELETE,
	SDLK_END,		VK_END,
	SDLK_PAGEDOWN,	VK_NEXT,
	SDLK_LEFT,		VK_LEFT,
	SDLK_RIGHT,		VK_RIGHT,
	SDLK_DOWN,		VK_DOWN,
	SDLK_UP,		VK_UP,
	SDLK_LALT,		VK_ALT,
	SDLK_RALT,		VK_RMENU,		// RAlt for 'play current pattern from beginning' in FastTracker II mode
	SDLK_LSHIFT,	VK_SHIFT,
	SDLK_RSHIFT,	VK_SHIFT,
#ifndef __APPLE__
	SDLK_LCTRL,		VK_LCONTROL,
	SDLK_RCTRL,		VK_RCONTROL,
#else
	SDLK_LGUI,		VK_LCONTROL,	// OSX: LCommand = LCtrl for native-style Cut/Copy/Paste etc in MilkyTracker edit mode
	SDLK_RGUI,		VK_RCONTROL,	// OSX: RCommand = RCtrl for 'play song from current order' in FastTracker II edit mode
#endif
	SDLK_MODE,		VK_RMENU,
	SDLK_CAPSLOCK,	VK_CAPITAL,

	// Numpad
	SDLK_KP_0,			VK_NUMPAD0,
	SDLK_KP_1,			VK_NUMPAD1,
	SDLK_KP_2,			VK_NUMPAD2,
	SDLK_KP_3,			VK_NUMPAD3,
	SDLK_KP_4,			VK_NUMPAD4,
	SDLK_KP_5,			VK_NUMPAD5,
	SDLK_KP_6,			VK_NUMPAD6,
	SDLK_KP_7,			VK_NUMPAD7,
	SDLK_KP_8,			VK_NUMPAD8,
	SDLK_KP_9,			VK_NUMPAD9,
	SDLK_KP_PERIOD,		VK_DECIMAL,
	SDLK_KP_DIVIDE,		VK_DIVIDE,
	SDLK_KP_MULTIPLY,	VK_MULTIPLY,
	SDLK_KP_MINUS,		VK_SUBTRACT,
	SDLK_KP_PLUS,		VK_ADD,
	SDLK_KP_ENTER,		VK_SEPARATOR
};

static const pp_uint32 sdl_keycode_to_pc_keycode[] = {
	// -----------------------------------------
	//  Keys that generate printable characters
	// -----------------------------------------

	0x00,				// 0 SDLK_UNKNOWN
	0x00,				// 1
	0x00,				// 2
	0x00,				// 3
	0x00,				// 4
	0x00,				// 5
	0x00,				// 6
	0x00,				// 7
	0x0e,				// 8 SDLK_BACKSPACE
	0x0f,				// 9 SDLK_TAB
	0x00,				// 10
	0x00,				// 11
	0x00,				// 12
	0x1c,				// 13 SDLK_RETURN
	0x00,				// 14
	0x00,				// 15
	0x00,				// 16
	0x00,				// 17
	0x00,				// 18
	0x00,				// 19
	0x00,				// 20
	0x00,				// 21
	0x00,				// 22
	0x00,				// 23
	0x00,				// 24
	0x00,				// 25
	0x00,				// 26
	0x01,				// 27 SDLK_ESCAPE
	0x00,				// 28
	0x00,				// 29
	0x00,				// 30
	0x00,				// 31
	0x39,				// 32 SDLK_SPACE
	0x02,				// 33 SDLK_EXCLAIM
	0x28,				// 34 SDLK_QUOTEDBL
	0x04,				// 35 SDLK_HASH
	0x05,				// 36 SDLK_DOLLAR
	0x06,				// 37 SDLK_PERCENT
	0x08,				// 38 SDLK_AMPERSAND
	0x28,				// 39 SDLK_QUOTE
	0x0a,				// 40 SDLK_LEFTPAREN
	0x0b,				// 41 SDLK_RIGHTPAREN
	0x09,				// 42 SDLK_ASTERISK
	0x0d,				// 43 SDLK_PLUS
	0x33,				// 44 SDLK_COMMA
	0x0c,				// 45 SDLK_MINUS
	0x34,				// 46 SDLK_PERIOD
	0x35,				// 47 SDLK_SLASH
	0x0b,				// 48 SDLK_0
	0x02,				// 49 SDLK_1
	0x03,				// 50 SDLK_2
	0x04,				// 51 SDLK_3
	0x05,				// 52 SDLK_4
	0x06,				// 53 SDLK_5
	0x07,				// 54 SDLK_6
	0x08,				// 55 SDLK_7
	0x09,				// 56 SDLK_8
	0x0a,				// 57 SDLK_9
	0x27,				// 58 SDLK_COLON
	0x27,				// 59 SDLK_SEMICOLON
	0x33,				// 60 SDLK_LESS
	0x0d,				// 61 SDLK_EQUALS
	0x34,				// 62 SDLK_GREATER
	0x35,				// 63 SDLK_QUESTION
	0x03,				// 64 SDLK_AT
	0x00,				// 65
	0x00,				// 66
	0x00,				// 67
	0x00,				// 68
	0x00,				// 69
	0x00,				// 70
	0x00,				// 71
	0x00,				// 72
	0x00,				// 73
	0x00,				// 74
	0x00,				// 75
	0x00,				// 76
	0x00,				// 77
	0x00,				// 78
	0x00,				// 79
	0x00,				// 80
	0x00,				// 81
	0x00,				// 82
	0x00,				// 83
	0x00,				// 84
	0x00,				// 85
	0x00,				// 86
	0x00,				// 87
	0x00,				// 88
	0x00,				// 89
	0x00,				// 90
	0x1a,				// 91 SDLK_LEFTBRACKET
	0x2b,				// 92 SDLK_BACKSLASH
	0x1b,				// 93 SDLK_RIGHTBRACKET
	0x07,				// 94 SDLK_CARET
	0x0c,				// 95 SDLK_UNDERSCORE
	0x29,				// 96 SDLK_BACKQUOTE
	0x1e,				// 97 SDLK_a
	0x30,				// 98 SDLK_b
	0x2e,				// 99 SDLK_c
	0x20,				// 100 SDLK_d
	0x12,				// 101 SDLK_e
	0x21,				// 102 SDLK_f
	0x22,				// 103 SDLK_g
	0x23,				// 104 SDLK_h
	0x17,				// 105 SDLK_i
	0x24,				// 106 SDLK_j
	0x25,				// 107 SDLK_k
	0x26,				// 108 SDLK_l
	0x32,				// 109 SDLK_m
	0x31,				// 110 SDLK_n
	0x18,				// 111 SDLK_o
	0x19,				// 112 SDLK_p
	0x10,				// 113 SDLK_q
	0x13,				// 114 SDLK_r
	0x1f,				// 115 SDLK_s
	0x14,				// 116 SDLK_t
	0x16,				// 117 SDLK_u
	0x2f,				// 118 SDLK_v
	0x11,				// 119 SDLK_w
	0x2d,				// 120 SDLK_x
	0x15,				// 121 SDLK_y
	0x2c,				// 122 SDLK_z
	0x00,				// 123
	0x00,				// 124
	0x00,				// 125
	0x00,				// 126
	0x00,				// 127
	0x00,				// 128
	0x00,				// 129
	0x00,				// 130
	0x00,				// 131
	0x00,				// 132
	0x00,				// 133
	0x00,				// 134
	0x00,				// 135
	0x00,				// 136
	0x00,				// 137
	0x00,				// 138
	0x00,				// 139
	0x00,				// 140
	0x00,				// 141
	0x00,				// 142
	0x00,				// 143
	0x00,				// 144
	0x00,				// 145
	0x00,				// 146
	0x00,				// 147
	0x00,				// 148
	0x00,				// 149
	0x00,				// 150
	0x00,				// 151
	0x00,				// 152
	0x00,				// 153
	0x00,				// 154
	0x00,				// 155
	0x00,				// 156
	0x00,				// 157
	0x00,				// 158
	0x00,				// 159
	0x00,				// 160
	0x00,				// 161
	0x00,				// 162
	0x00,				// 163
	0x00,				// 164
	0x00,				// 165
	0x00,				// 166
	SC_SMALLERGREATER,	// 167 -- 'Smaller/Greater' on German keyboard
	0x00,				// 168
	0x00,				// 169
	0x00,				// 170
	0x00,				// 171
	0x00,				// 172
	0x00,				// 173
	0x00,				// 174
	0x00,				// 175
	0x00,				// 176
	0x00,				// 177 SDLK_DELETE

	// -----------------------------------------------
	//  Keys that don't generate printable characters
	// -----------------------------------------------

	SC_CAPSLOCK,	// 1073741881 SDLK_CAPSLOCK
	0x3b,			// 1073741882 SDLK_F1
	0x3c,			// 1073741883 SDLK_F2
	0x3d,			// 1073741884 SDLK_F3
	0x3e,			// 1073741885 SDLK_F4
	0x3f,			// 1073741886 SDLK_F5
	0x40,			// 1073741887 SDLK_F6
	0x41,			// 1073741888 SDLK_F7
	0x42,			// 1073741889 SDLK_F8
	0x43,			// 1073741890 SDLK_F9
	0x44,			// 1073741891 SDLK_F10
	0x57,			// 1073741892 SDLK_F11
	0x58,			// 1073741893 SDLK_F12
	0x00,			// 1073741894 SDLK_PRINTSCREEN
	0x46,			// 1073741895 SDLK_SCROLLLOCK
	0x00,			// 1073741896 SDLK_PAUSE
	0x52e0,			// 1073741897 SDLK_INSERT
	0x47e0,			// 1073741898 SDLK_HOME
	0x49e0,			// 1073741899 SDLK_PAGEUP
	0x00,			// 1073741900
	0x4fe0,			// 1073741901 SDLK_END
	0x51e0,			// 1073741902 SDLK_PAGEDOWN
	0x4de0,			// 1073741903 SDLK_RIGHT
	0x4be0,			// 1073741904 SDLK_LEFT
	0x50e0,			// 1073741905 SDLK_DOWN
	0x48e0,			// 1073741906 SDLK_UP
	0x45,			// 1073741907 SDLK_NUMLOCKCLEAR
	0x35e0,			// 1073741908 SDLK_KP_DIVIDE
	0x37,			// 1073741909 SDLK_KP_MULTIPLY
	0x4a,			// 1073741910 SDLK_KP_MINUS
	0x4e,			// 1073741911 SDLK_KP_PLUS
	0x1ce0,			// 1073741912 SDLK_KP_ENTER
	0x4f,			// 1073741913 SDLK_KP_1
	0x50,			// 1073741914 SDLK_KP_2
	0x51,			// 1073741915 SDLK_KP_3
	0x4b,			// 1073741916 SDLK_KP_4
	0x4c,			// 1073741917 SDLK_KP_5
	0x4d,			// 1073741918 SDLK_KP_6
	0x47,			// 1073741919 SDLK_KP_7
	0x48,			// 1073741920 SDLK_KP_8
	0x49,			// 1073741921 SDLK_KP_9
	0x52,			// 1073741922 SDLK_KP_0
	0x53,			// 1073741923 SDLK_KP_PERIOD
	0x00,			// 1073741924
	0x00,			// 1073741925 SDLK_APPLICATION
	0x00,			// 1073741926 SDLK_POWER
	0x00,			// 1073741927 SDLK_KP_EQUALS
	0x00,			// 1073741928 SDLK_F13
	0x00,			// 1073741929 SDLK_F14
	0x00,			// 1073741930 SDLK_F15
	0x00,			// 1073741931 SDLK_F16
	0x00,			// 1073741932 SDLK_F17
	0x00,			// 1073741933 SDLK_F18
	0x00,			// 1073741934 SDLK_F19
	0x00,			// 1073741935 SDLK_F20
	0x00,			// 1073741936 SDLK_F21
	0x00,			// 1073741937 SDLK_F22
	0x00,			// 1073741938 SDLK_F23
	0x00,			// 1073741939 SDLK_F24
	0x00,			// 1073741940 SDLK_EXECUTE
	0x00,			// 1073741941 SDLK_HELP
	0x5de0,			// 1073741942 SDLK_MENU
	0x00,			// 1073741943 SDLK_SELECT
	0x00,			// 1073741944 SDLK_STOP
	0x00,			// 1073741945 SDLK_AGAIN
	0x00,			// 1073741946 SDLK_UNDO
	0x00,			// 1073741947 SDLK_CUT
	0x00,			// 1073741948 SDLK_COPY
	0x00,			// 1073741949 SDLK_PASTE
	0x00,			// 1073741950 SDLK_FIND
	0x00,			// 1073741951 SDLK_MUTE
	0x00,			// 1073741952 SDLK_VOLUMEUP
	0x00,			// 1073741953 SDLK_VOLUMEDOWN
	0x00,			// 1073741954
	0x00,			// 1073741955
	0x00,			// 1073741956
	0x00,			// 1073741957 SDLK_KP_COMMA
	0x00,			// 1073741958 SDLK_KP_EQUALSAS400
	0x00,			// 1073741959
	0x00,			// 1073741960
	0x00,			// 1073741961
	0x00,			// 1073741962
	0x00,			// 1073741963
	0x00,			// 1073741964
	0x00,			// 1073741965
	0x00,			// 1073741966
	0x00,			// 1073741967
	0x00,			// 1073741968
	0x00,			// 1073741969
	0x00,			// 1073741970
	0x00,			// 1073741971
	0x00,			// 1073741972
	0x00,			// 1073741973
	0x00,			// 1073741974
	0x00,			// 1073741975
	0x00,			// 1073741976
	0x00,			// 1073741977 SDLK_ALTERASE
	0x00,			// 1073741978 SDLK_SYSREQ
	0x00,			// 1073741979 SDLK_CANCEL
	0x00,			// 1073741980 SDLK_CLEAR
	0x00,			// 1073741981 SDLK_PRIOR
	0x00,			// 1073741982 SDLK_RETURN2
	0x00,			// 1073741983 SDLK_SEPARATOR
	0x00,			// 1073741984 SDLK_OUT
	0x00,			// 1073741985 SDLK_OPER
	0x00,			// 1073741986 SDLK_CLEARAGAIN
	0x00,			// 1073741987 SDLK_CRSEL
	0x00,			// 1073741988 SDLK_EXSEL
	0x00,			// 1073741989
	0x00,			// 1073741990
	0x00,			// 1073741991
	0x00,			// 1073741992
	0x00,			// 1073741993
	0x00,			// 1073741994
	0x00,			// 1073741995
	0x00,			// 1073741996
	0x00,			// 1073741997
	0x00,			// 1073741998
	0x00,			// 1073741999
	0x00,			// 1073742000 SDLK_KP_00
	0x00,			// 1073742001 SDLK_KP_000
	0x00,			// 1073742002 SDLK_THOUSANDSSEPARATOR
	0x00,			// 1073742003 SDLK_DECIMALSEPARATOR
	0x00,			// 1073742004 SDLK_CURRENCYUNIT
	0x00,			// 1073742005 SDLK_CURRENCYSUBUNIT
	0x00,			// 1073742006 SDLK_KP_LEFTPAREN
	0x00,			// 1073742007 SDLK_KP_RIGHTPAREN
	0x00,			// 1073742008 SDLK_KP_LEFTBRACE
	0x00,			// 1073742009 SDLK_KP_RIGHTBRACE
	0x00,			// 1073742010 SDLK_KP_TAB
	0x00,			// 1073742011 SDLK_KP_BACKSPACE
	0x00,			// 1073742012 SDLK_KP_A
	0x00,			// 1073742013 SDLK_KP_B
	0x00,			// 1073742014 SDLK_KP_C
	0x00,			// 1073742015 SDLK_KP_D
	0x00,			// 1073742016 SDLK_KP_E
	0x00,			// 1073742017 SDLK_KP_F
	0x00,			// 1073742018 SDLK_KP_XOR
	0x00,			// 1073742019 SDLK_KP_POWER
	0x00,			// 1073742020 SDLK_KP_PERCENT
	0x00,			// 1073742021 SDLK_KP_LESS
	0x00,			// 1073742022 SDLK_KP_GREATER
	0x00,			// 1073742023 SDLK_KP_AMPERSAND
	0x00,			// 1073742024 SDLK_KP_DBLAMPERSAND
	0x00,			// 1073742025 SDLK_KP_VERTICALBAR
	0x00,			// 1073742026 SDLK_KP_DBLVERTICALBAR
	0x00,			// 1073742027 SDLK_KP_COLON
	0x00,			// 1073742028 SDLK_KP_HASH
	0x00,			// 1073742029 SDLK_KP_SPACE
	0x00,			// 1073742030 SDLK_KP_AT
	0x00,			// 1073742031 SDLK_KP_EXCLAM
	0x00,			// 1073742032 SDLK_KP_MEMSTORE
	0x00,			// 1073742033 SDLK_KP_MEMRECALL
	0x00,			// 1073742034 SDLK_KP_MEMCLEAR
	0x00,			// 1073742035 SDLK_KP_MEMADD
	0x00,			// 1073742036 SDLK_KP_MEMSUBTRACT
	0x00,			// 1073742037 SDLK_KP_MEMMULTIPLY
	0x00,			// 1073742038 SDLK_KP_MEMDIVIDE
	0x00,			// 1073742039 SDLK_KP_PLUSMINUS
	0x00,			// 1073742040 SDLK_KP_CLEAR
	0x00,			// 1073742041 SDLK_KP_CLEARENTRY
	0x00,			// 1073742042 SDLK_KP_BINARY
	0x00,			// 1073742043 SDLK_KP_OCTAL
	0x00,			// 1073742044 SDLK_KP_DECIMAL
	0x00,			// 1073742045 SDLK_KP_HEXADECIMAL
	0x00,			// 1073742046
	0x00,			// 1073742047
#ifndef __APPLE__
	0x1d,			// 1073742048 SDLK_LCTRL
#else
	0x00,			// 1073742048 SDLK_LCTRL -- OSX: LCtrl disabled in favour of LCommand
#endif
	0x2a,			// 1073742049 SDLK_LSHIFT
	0x38,			// 1073742050 SDLK_LALT
#ifndef __APPLE__
	0x5ce0,			// 1073742051 SDLK_LGUI -- Windows Key/Apple Command
#else
	0x1d,			// 1073742051 SDLK_LGUI -- OSX: LCommand becomes LCtrl
#endif
	0x1de0,			// 1073742052 SDLK_RCTRL
	0x36,			// 1073742053 SDLK_RSHIFT
	0x38e0,			// 1073742054 SDLK_RALT
	0x5be0,			// 1073742055 SDLK_RGUI -- Windows Key/Apple Command
	0x00,			// 1073742056
	0x00,			// 1073742057
	0x00,			// 1073742058
	0x00,			// 1073742059
	0x00,			// 1073742060
	0x00,			// 1073742061
	0x00,			// 1073742062
	0x00,			// 1073742063
	0x00,			// 1073742064
	0x00,			// 1073742065
	0x00,			// 1073742066
	0x00,			// 1073742067
	0x00,			// 1073742068
	0x00,			// 1073742069
	0x00,			// 1073742070
	0x00,			// 1073742071
	0x00,			// 1073742072
	0x00,			// 1073742073
	0x00,			// 1073742074
	0x00,			// 1073742075
	0x00,			// 1073742076
	0x00,			// 1073742077
	0x00,			// 1073742078
	0x00,			// 1073742079
	0x00,			// 1073742080
	0x00,			// 1073742081
	0x00,			// 1073742082 SDLK_AUDIONEXT
	0x00,			// 1073742083 SDLK_AUDIOPREV
	0x00,			// 1073742084 SDLK_AUDIOSTOP
	0x00,			// 1073742085 SDLK_AUDIOPLAY
	0x00,			// 1073742086 SDLK_AUDIOMUTE
	0x00,			// 1073742087 SDLK_MEDIASELECT
	0x00,			// 1073742088 SDLK_WWW
	0x00,			// 1073742089 SDLK_MAIL
	0x00,			// 1073742090 SDLK_CALCULATOR
	0x00,			// 1073742091 SDLK_COMPUTER
	0x00,			// 1073742092 SDLK_AC_SEARCH
	0x00,			// 1073742093 SDLK_AC_HOME
	0x00,			// 1073742094 SDLK_AC_BACK
	0x00,			// 1073742095 SDLK_AC_FORWARD
	0x00,			// 1073742096 SDLK_AC_STOP
	0x00,			// 1073742097 SDLK_AC_REFRESH
	0x00,			// 1073742098 SDLK_AC_BOOKMARKS
	0x00,			// 1073742099 SDLK_BRIGHTNESSDOWN
	0x00,			// 1073742100 SDLK_BRIGHTNESSUP
	0x00,			// 1073742101 SDLK_DISPLAYSWITCH
	0x00,			// 1073742102 SDLK_KBDILLUMTOGGLE
	0x00,			// 1073742103 SDLK_KBDILLUMDOWN
	0x00,			// 1073742104 SDLK_KBDILLUMUP
	0x00,			// 1073742105 SDLK_EJECT
	0x00,			// 1073742106 SDLK_SLEEP
};

pp_uint16 toVK(const SDL_Keysym& keysym)
{
	if (keysym.sym >= SDLK_a && keysym.sym <= SDLK_z)
		return (pp_uint16)keysym.sym-32;

	if (keysym.sym >= SDLK_0 && keysym.sym <= SDLK_9)
		return (pp_uint16)keysym.sym;

	for (int i = 0; i < sizeof sdl_keysym_to_vk / sizeof (pp_uint32); i += 2)
		if(sdl_keysym_to_vk[i] == keysym.sym)
			return sdl_keysym_to_vk[i+1];

	return 0;
}

pp_uint16 toSC(const SDL_Keysym& keysym)
{
	int index = keysym.sym;

	// Shift to upper range if key is not a printable character
	if (index > 177)
	{
		index -= 1073741880;
		index += 177 + 1;
	}

	// Range check
	if (index > sizeof sdl_keycode_to_pc_keycode / sizeof (pp_uint32) || index < 0)
	{
		return 0;
	}

	return sdl_keycode_to_pc_keycode[index];
}
