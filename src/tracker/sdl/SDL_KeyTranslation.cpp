/*
 *  tracker/sdl/SDL_KeyTranslation.cpp
 *
 *  Copyright 2009 Peter Barth, Christopher O'Neill
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
 *  14/8/06 - Christopher O'Neill
 *    PC specific code added to toSC - allows none qwerty layouts to work
 *    Small fix in toSC
 *    Various keys added to toVK - rewritten to use array instead of select
 */

bool isX11 = false;
bool stdKb = true;
#include "SDL_KeyTranslation.h"

static const pp_uint16 sdl_keysym_to_vk[] = {
	SDLK_CARET, 0xC0,
	SDLK_WORLD_0, 0xBB,
	SDLK_WORLD_1, 0xBD,
	SDLK_PLUS, 0xDD,
	SDLK_WORLD_2, 0xDB,
	SDLK_RETURN, VK_RETURN,
	SDLK_WORLD_3, 0xDE,
	SDLK_WORLD_4, 0xBA,
	SDLK_HASH, 0xDC,
	SDLK_COMMA, 188,
	SDLK_PERIOD, 0xBF,
	SDLK_MINUS, 190,
	SDLK_TAB, VK_TAB,
	SDLK_SPACE, VK_SPACE,
	SDLK_LESS, 0xE2,
	SDLK_BACKSPACE, VK_BACK,
	SDLK_ESCAPE, VK_ESCAPE,
	SDLK_F1, VK_F1,
	SDLK_F2, VK_F2,
	SDLK_F3, VK_F3,
	SDLK_F4, VK_F4,
	SDLK_F5, VK_F5,
	SDLK_F6, VK_F6,
	SDLK_F7, VK_F7,
	SDLK_F8, VK_F8,
	SDLK_F9, VK_F9,
	SDLK_F10, VK_F10,
	SDLK_F11, VK_F11,
	SDLK_F12, VK_F12,
	SDLK_INSERT, VK_INSERT,
	SDLK_HOME, VK_HOME,
	SDLK_PAGEUP, VK_PRIOR,
	SDLK_DELETE, VK_DELETE,
	SDLK_END, VK_END,
	SDLK_PAGEDOWN, VK_NEXT,
	SDLK_LEFT, VK_LEFT,
	SDLK_RIGHT, VK_RIGHT,
	SDLK_DOWN, VK_DOWN,
	SDLK_UP, VK_UP,
	SDLK_LALT, VK_ALT,
	SDLK_LSHIFT, VK_SHIFT,
	SDLK_RSHIFT, VK_SHIFT,
	SDLK_LCTRL, VK_LCONTROL,
	SDLK_RCTRL, VK_RCONTROL,
	SDLK_MODE, VK_RMENU,

	// Numpad
	SDLK_KP0, VK_NUMPAD0,
	SDLK_KP1, VK_NUMPAD1,
	SDLK_KP2, VK_NUMPAD2,
	SDLK_KP3, VK_NUMPAD3,
	SDLK_KP4, VK_NUMPAD4,
	SDLK_KP5, VK_NUMPAD5,
	SDLK_KP6, VK_NUMPAD6,
	SDLK_KP7, VK_NUMPAD7,
	SDLK_KP8, VK_NUMPAD8,
	SDLK_KP9, VK_NUMPAD9,
	SDLK_KP_PERIOD, VK_DECIMAL,
	SDLK_KP_DIVIDE, VK_DIVIDE,
	SDLK_KP_MULTIPLY, VK_MULTIPLY,
	SDLK_KP_MINUS, VK_SUBTRACT,
	SDLK_KP_PLUS, VK_ADD,
	SDLK_KP_ENTER, VK_SEPARATOR
};


pp_uint16 toVK(const SDL_keysym& keysym)
{
	if (keysym.sym >= SDLK_a && keysym.sym <= SDLK_z)
		return (pp_uint16)keysym.sym-32;

	// azerty keyboards won't work with this, they get fixed in SDL_Main.cpp
	if (keysym.sym >= SDLK_0 && keysym.sym <= SDLK_9)
		return (pp_uint16)keysym.sym;

	for(int i=0; i < sizeof(sdl_keysym_to_vk)/2; i+=2)
		if(sdl_keysym_to_vk[i] == keysym.sym)
			return sdl_keysym_to_vk[i+1];
	
	return 0;
}

static const pp_uint32 sdl_keycode_to_pc_keycode[] = {
     0x00, /* 0 SDLK_UNKNOWN */
     0x00, /* 1 */
     0x00, /* 2 */
     0x00, /* 3 */
     0x00, /* 4 */
     0x00, /* 5 */
     0x00, /* 6 */
     0x00, /* 7 */
     0x0e, /* 8 SDLK_BACKSPACE */
     0x0f, /* 9 SDLK_TAB */
     0x00, /* 10 */
     0x00, /* 11 */
     0x00, /* 12 SDLK_CLEAR */
     0x1c, /* 13 SDLK_RETURN */
     0x00, /* 14 */
     0x00, /* 15 */
     0x00, /* 16 */
     0x00, /* 17 */
     0x00, /* 18 */
     0x451de1, /* 19 SDLK_PAUSE */
     0x00, /* 20 */
     0x00, /* 21 */
     0x00, /* 22 */
     0x00, /* 23 */
     0x00, /* 24 */
     0x00, /* 25 */
     0x00, /* 26 */
     0x01, /* 27 SDLK_ESCAPE */
     0x00, /* 28 */
     0x00, /* 29 */
     0x00, /* 30 */
     0x00, /* 31 */
     0x39, /* 32 SDLK_SPACE */
     0x02, /* 33 SDLK_EXCLAIM */
     0x28, /* 34 SDLK_QUOTEDBL */
     0x04, /* 35 SDLK_HASH */
     0x05, /* 36 SDLK_DOLLAR */
     0x00, /* 37 */
     0x08, /* 38 SDLK_AMPERSAND */
     0x28, /* 39 SDLK_QUOTE */
     0x0a, /* 40 SDLK_LEFTPAREN */
     0x0b, /* 41 SDLK_RIGHTPAREN */
     0x09, /* 42 SDLK_ASTERISK */
     0x0d, /* 43 SDLK_PLUS */
     0x33, /* 44 SDLK_COMMA */
     0x0c, /* 45 SDLK_MINUS */
     0x34, /* 46 SDLK_PERIOD */
     0x35, /* 47 SDLK_SLASH */
     0x0b, /* 48 SDLK_0 */
     0x02, /* 49 SDLK_1 */
     0x03, /* 50 SDLK_2 */
     0x04, /* 51 SDLK_3 */
     0x05, /* 52 SDLK_4 */
     0x06, /* 53 SDLK_5 */
     0x07, /* 54 SDLK_6 */
     0x08, /* 55 SDLK_7 */
     0x09, /* 56 SDLK_8 */
     0x0a, /* 57 SDLK_9 */
     0x27, /* 58 SDLK_COLON */
     0x27, /* 59 SDLK_SEMICOLON */
     0x33, /* 60 SDLK_LESS */
     0x0d, /* 61 SDLK_EQUALS */
     0x34, /* 62 SDLK_GREATER */
     0x35, /* 63 SDLK_QUESTION */
     0x03, /* 64 SDLK_AT */
     0x00, /* 65 */
     0x00, /* 66 */
     0x00, /* 67 */
     0x00, /* 68 */
     0x00, /* 69 */
     0x00, /* 70 */
     0x00, /* 71 */
     0x00, /* 72 */
     0x00, /* 73 */
     0x00, /* 74 */
     0x00, /* 75 */
     0x00, /* 76 */
     0x00, /* 77 */
     0x00, /* 78 */
     0x00, /* 79 */
     0x00, /* 80 */
     0x00, /* 81 */
     0x00, /* 82 */
     0x00, /* 83 */
     0x00, /* 84 */
     0x00, /* 85 */
     0x00, /* 86 */
     0x00, /* 87 */
     0x00, /* 88 */
     0x00, /* 89 */
     0x00, /* 90 */
     0x1a, /* 91 SDLK_LEFTBRACKET */
     0x2b, /* 92 SDLK_BACKSLASH */
     0x1b, /* 93 SDLK_RIGHTBRACKET */
     0x07, /* 94 SDLK_CARET */
     0x0c, /* 95 SDLK_UNDERSCORE */
     0x29, /* 96 SDLK_BACKQUOTE */
     0x1e, /* 97 SDLK_a */
     0x30, /* 98 SDLK_b */
     0x2e, /* 99 SDLK_c */
     0x20, /* 100 SDLK_d */
     0x12, /* 101 SDLK_e */
     0x21, /* 102 SDLK_f */
     0x22, /* 103 SDLK_g */
     0x23, /* 104 SDLK_h */
     0x17, /* 105 SDLK_i */
     0x24, /* 106 SDLK_j */
     0x25, /* 107 SDLK_k */
     0x26, /* 108 SDLK_l */
     0x32, /* 109 SDLK_m */
     0x31, /* 110 SDLK_n */
     0x18, /* 111 SDLK_o */
     0x19, /* 112 SDLK_p */
     0x10, /* 113 SDLK_q */
     0x13, /* 114 SDLK_r */
     0x1f, /* 115 SDLK_s */
     0x14, /* 116 SDLK_t */
     0x16, /* 117 SDLK_u */
     0x2f, /* 118 SDLK_v */
     0x11, /* 119 SDLK_w */
     0x2d, /* 120 SDLK_x */
     0x15, /* 121 SDLK_y */
     0x2c, /* 122 SDLK_z */
     0x00, /* 123 */
     0x00, /* 124 */
     0x00, /* 125 */
     0x00, /* 126 */
     0x00, /* 127 SDLK_DELETE */
     0x00, /* 128 */
     0x00, /* 129 */
     0x00, /* 130 */
     0x00, /* 131 */
     0x00, /* 132 */
     0x00, /* 133 */
     0x00, /* 134 */
     0x00, /* 135 */
     0x00, /* 136 */
     0x00, /* 137 */
     0x00, /* 138 */
     0x00, /* 139 */
     0x00, /* 140 */
     0x00, /* 141 */
     0x00, /* 142 */
     0x00, /* 143 */
     0x00, /* 144 */
     0x00, /* 145 */
     0x00, /* 146 */
     0x00, /* 147 */
     0x00, /* 148 */
     0x00, /* 149 */
     0x00, /* 150 */
     0x00, /* 151 */
     0x00, /* 152 */
     0x00, /* 153 */
     0x00, /* 154 */
     0x00, /* 155 */
     0x00, /* 156 */
     0x00, /* 157 */
     0x00, /* 158 */
     0x00, /* 159 */
     0x00, /* 160 SDLK_WORLD_0 */
     0x00, /* 161 SDLK_WORLD_1 */
     0x00, /* 162 SDLK_WORLD_2 */
     0x00, /* 163 SDLK_WORLD_3 */
     0x00, /* 164 SDLK_WORLD_4 */
     0x00, /* 165 SDLK_WORLD_5 */
     0x00, /* 166 SDLK_WORLD_6 */
     0x00, /* 167 SDLK_WORLD_7 */
     0x00, /* 168 SDLK_WORLD_8 */
     0x00, /* 169 SDLK_WORLD_9 */
     0x00, /* 170 SDLK_WORLD_10 */
     0x00, /* 171 SDLK_WORLD_11 */
     0x00, /* 172 SDLK_WORLD_12 */
     0x00, /* 173 SDLK_WORLD_13 */
     0x00, /* 174 SDLK_WORLD_14 */
     0x00, /* 175 SDLK_WORLD_15 */
     0x00, /* 176 SDLK_WORLD_16 */
     0x00, /* 177 SDLK_WORLD_17 */
     0x00, /* 178 SDLK_WORLD_18 */
     0x00, /* 179 SDLK_WORLD_19 */
     0x00, /* 180 SDLK_WORLD_20 */
     0x00, /* 181 SDLK_WORLD_21 */
     0x00, /* 182 SDLK_WORLD_22 */
     0x00, /* 183 SDLK_WORLD_23 */
     0x00, /* 184 SDLK_WORLD_24 */
     0x00, /* 185 SDLK_WORLD_25 */
     0x00, /* 186 SDLK_WORLD_26 */
     0x00, /* 187 SDLK_WORLD_27 */
     0x00, /* 188 SDLK_WORLD_28 */
     0x00, /* 189 SDLK_WORLD_29 */
     0x00, /* 190 SDLK_WORLD_30 */
     0x00, /* 191 SDLK_WORLD_31 */
     0x00, /* 192 SDLK_WORLD_32 */
     0x00, /* 193 SDLK_WORLD_33 */
     0x00, /* 194 SDLK_WORLD_34 */
     0x00, /* 195 SDLK_WORLD_35 */
     0x00, /* 196 SDLK_WORLD_36 */
     0x00, /* 197 SDLK_WORLD_37 */
     0x00, /* 198 SDLK_WORLD_38 */
     0x00, /* 199 SDLK_WORLD_39 */
     0x00, /* 200 SDLK_WORLD_40 */
     0x00, /* 201 SDLK_WORLD_41 */
     0x00, /* 202 SDLK_WORLD_42 */
     0x00, /* 203 SDLK_WORLD_43 */
     0x00, /* 204 SDLK_WORLD_44 */
     0x00, /* 205 SDLK_WORLD_45 */
     0x00, /* 206 SDLK_WORLD_46 */
     0x00, /* 207 SDLK_WORLD_47 */
     0x00, /* 208 SDLK_WORLD_48 */
     0x00, /* 209 SDLK_WORLD_49 */
     0x00, /* 210 SDLK_WORLD_50 */
     0x00, /* 211 SDLK_WORLD_51 */
     0x00, /* 212 SDLK_WORLD_52 */
     0x00, /* 213 SDLK_WORLD_53 */
     0x00, /* 214 SDLK_WORLD_54 */
     0x00, /* 215 SDLK_WORLD_55 */
     0x00, /* 216 SDLK_WORLD_56 */
     0x00, /* 217 SDLK_WORLD_57 */
     0x00, /* 218 SDLK_WORLD_58 */
     0x00, /* 219 SDLK_WORLD_59 */
     0x00, /* 220 SDLK_WORLD_60 */
     0x00, /* 221 SDLK_WORLD_61 */
     0x00, /* 222 SDLK_WORLD_62 */
     0x00, /* 223 SDLK_WORLD_63 */
     0x00, /* 224 SDLK_WORLD_64 */
     0x00, /* 225 SDLK_WORLD_65 */
     0x00, /* 226 SDLK_WORLD_66 */
     0x00, /* 227 SDLK_WORLD_67 */
     0x00, /* 228 SDLK_WORLD_68 */
     0x00, /* 229 SDLK_WORLD_69 */
     0x00, /* 230 SDLK_WORLD_70 */
     0x00, /* 231 SDLK_WORLD_71 */
     0x00, /* 232 SDLK_WORLD_72 */
     0x00, /* 233 SDLK_WORLD_73 */
     0x00, /* 234 SDLK_WORLD_74 */
     0x00, /* 235 SDLK_WORLD_75 */
     0x00, /* 236 SDLK_WORLD_76 */
     0x00, /* 237 SDLK_WORLD_77 */
     0x00, /* 238 SDLK_WORLD_78 */
     0x00, /* 239 SDLK_WORLD_79 */
     0x00, /* 240 SDLK_WORLD_80 */
     0x00, /* 241 SDLK_WORLD_81 */
     0x00, /* 242 SDLK_WORLD_82 */
     0x00, /* 243 SDLK_WORLD_83 */
     0x00, /* 244 SDLK_WORLD_84 */
     0x00, /* 245 SDLK_WORLD_85 */
     0x00, /* 246 SDLK_WORLD_86 */
     0x00, /* 247 SDLK_WORLD_87 */
     0x00, /* 248 SDLK_WORLD_88 */
     0x00, /* 249 SDLK_WORLD_89 */
     0x00, /* 250 SDLK_WORLD_90 */
     0x00, /* 251 SDLK_WORLD_91 */
     0x00, /* 252 SDLK_WORLD_92 */
     0x00, /* 253 SDLK_WORLD_93 */
     0x00, /* 254 SDLK_WORLD_94 */
     0x00, /* 255 SDLK_WORLD_95 */
     
     0x52,   /* 256 SDLK_KP0 */
     0x4f,   /* 257 SDLK_KP1 */
     0x50,   /* 258 SDLK_KP2 */
     0x51,   /* 259 SDLK_KP3 */
     0x4b,   /* 260 SDLK_KP4 */
     0x4c,   /* 261 SDLK_KP5 */
     0x4d,   /* 262 SDLK_KP6 */
     0x47,   /* 263 SDLK_KP7 */
     0x48,   /* 264 SDLK_KP8 */
     0x49,   /* 265 SDLK_KP9 */
     0x53,   /* 266 SDLK_KP_PERIOD */
     0x35e0, /* 267 SDLK_KP_DIVIDE */
     0x37,   /* 268 SDLK_KP_MULTIPLY */
     0x4a,   /* 269 SDLK_KP_MINUS */
     0x4e,   /* 270 SDLK_KP_PLUS */
     0x1ce0, /* 271 SDLK_KP_ENTER */
     0x00,   /* 272 SDLK_KP_EQUALS */
 
     0x48e0, /* 273 SDLK_UP */
     0x50e0, /* 274 SDLK_DOWN */
     0x4de0, /* 275 SDLK_RIGHT */
     0x4be0, /* 276 SDLK_LEFT */
     0x52e0, /* 277 SDLK_INSERT */
     0x47e0, /* 278 SDLK_HOME */
     0x4fe0, /* 279 SDLK_END */
     0x49e0, /* 280 SDLK_PAGEUP */
     0x51e0, /* 281 SDLK_PAGEDOWN */
 
     0x3b, /* 282 SDLK_F1 */
     0x3c, /* 283 SDLK_F2 */
     0x3d, /* 284 SDLK_F3 */
     0x3e, /* 285 SDLK_F4 */
     0x3f, /* 286 SDLK_F5 */
     0x40, /* 287 SDLK_F6 */
     0x41, /* 288 SDLK_F7 */
     0x42, /* 289 SDLK_F8 */
     0x43, /* 290 SDLK_F9 */
     0x44, /* 291 SDLK_F10 */
     0x57, /* 292 SDLK_F11 */
     0x58, /* 293 SDLK_F12 */
     0x00, /* 294 SDLK_F13 */
     0x00, /* 295 SDLK_F14 */
     0x00, /* 296 SDLK_F15 */
     0x00, /* 297 */
     0x00, /* 298 */
     0x00, /* 299 */
 
     0x45,   /* 300 SDLK_NUMLOCK */
     0x3a,   /* 301 SDLK_CAPSLOCK */
     0x46,   /* 302 SDLK_SCROLLOCK */
     0x36,   /* 303 SDLK_RSHIFT */
     0x2a,   /* 304 SDLK_LSHIFT */
     0x1de0, /* 305 SDLK_RCTRL */
     0x1d,   /* 306 SDLK_LCTRL */
     0x38e0, /* 307 SDLK_RALT */
     0x38,   /* 308 SDLK_LALT */
     0x5be0, /* 309 SDLK_RMETA -- Apple Command keys*/
     0x5ce0, /* 310 SDLK_LMETA */
     0x5be0, /* 311 SDLK_LSUPER -- Windows keys */
     0x5ce0, /* 312 SDLK_RSUPER */
     0x00,   /* 313 SDLK_MODE -- Alt Gr */
     0x00,   /* 314 SDLK_COMPOSE */
 
     0x00,   /* 315 SDLK_HELP */
     0x00,   /* 316 SDLK_PRINT */
     0x00,   /* 317 SDLK_SYSREQ */
     0x00,   /* 318 SDLK_BREAK */
     0x5de0, /* 319 SDLK_MENU */
     0x00,   /* 320 SDLK_POWER */
     0x00,   /* 321 SDLK_EURO */
     0x00,   /* 322 SDLK_UNDO */
 };
 
pp_uint16 toSC(const SDL_keysym& keysym)
{
#ifndef NOT_PC_KB
	// WARNING!!!!
	// The next line will only work on X11 systems - I've no idea what will happen
	// with anything else
	if(isX11 && stdKb)
		return keysym.scancode - 8;
	else if(stdKb)
		return keysym.scancode;
#endif

	int keycode, v;

    if (keysym.sym > SDLK_LAST) 
	{
		keycode = 0;
	} 
	else 
	{
        keycode = sdl_keycode_to_pc_keycode[keysym.sym];
	}

	return keycode;
}
