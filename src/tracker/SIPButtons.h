/*
 *  tracker/SIPButtons.h
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

#ifndef __SIPBUTTONS_H__
#define __SIPBUTTONS_H__

// ---------------------------- First row of buttons ----------------------------
static const char keyLine_0_lowerCase[] = {'\x60','1','2','3','4','5','6','7','8','9','0','-','='};
static const char keyLine_0_upperCase[] = {'\x60','!','@','#','$','%','^','&','*','(',')','_','+'};
static const pp_int32 keyLineIDs_0[sizeof(keyLine_0_lowerCase)] = {INPUT_BUTTON_WTF, INPUT_BUTTON_1, INPUT_BUTTON_2, INPUT_BUTTON_3, 
INPUT_BUTTON_4, INPUT_BUTTON_5, INPUT_BUTTON_6, INPUT_BUTTON_7,
INPUT_BUTTON_8, INPUT_BUTTON_9, INPUT_BUTTON_0, INPUT_BUTTON_MINUS, INPUT_BUTTON_PLUS};

// ---------------------------- Second row of buttons ----------------------------
static const char keyLine_1_lowerCase[] = {'q','w','e','r','t','y','u','i','o','p','[',']'};
static const char keyLine_1_upperCase[] = {'Q','W','E','R','T','Y','U','I','O','P','{','}'};
static const pp_int32 keyLineIDs_1[sizeof(keyLine_1_lowerCase)] = {INPUT_BUTTON_Q, INPUT_BUTTON_W, INPUT_BUTTON_E, INPUT_BUTTON_R, 
INPUT_BUTTON_T, INPUT_BUTTON_Y, INPUT_BUTTON_U, INPUT_BUTTON_I,
INPUT_BUTTON_O, INPUT_BUTTON_P, INPUT_BUTTON_BRACKETOPEN, INPUT_BUTTON_BRACKETCLOSE};

static const pp_int32 keyLineSizes_1[sizeof(keyLine_1_lowerCase)] = {11,11,11,11,11,11,12,11,11,11,12,12};

// ---------------------------- Third row of buttons ----------------------------
static const char keyLine_2_lowerCase[] = {'a','s','d','f','g','h','j','k','l',';','\'','\\'};
static const char keyLine_2_upperCase[] = {'A','S','D','F','G','H','J','K','L',':','\"','|'};
static const pp_int32 keyLineIDs_2[sizeof(keyLine_2_lowerCase)] = {INPUT_BUTTON_A, INPUT_BUTTON_S, INPUT_BUTTON_D, INPUT_BUTTON_F, 
INPUT_BUTTON_G, INPUT_BUTTON_H, INPUT_BUTTON_J, INPUT_BUTTON_K,
INPUT_BUTTON_L, INPUT_BUTTON_SEMICOLON, INPUT_BUTTON_TICK, INPUT_BUTTON_BACKSLASH};

// ---------------------------- Fourth row of buttons ----------------------------
static const char keyLine_3_lowerCase[] = {'~','z','x','c','v','b','n','m',',','.','/'};
static const char keyLine_3_upperCase[] = {'|','Z','X','C','V','B','N','M','<','>','?'};
static const pp_int32 keyLineIDs_3[sizeof(keyLine_3_lowerCase)] = {INPUT_BUTTON_TILDE, INPUT_BUTTON_Z, INPUT_BUTTON_X, INPUT_BUTTON_C, INPUT_BUTTON_V, 
INPUT_BUTTON_B, INPUT_BUTTON_N, INPUT_BUTTON_M, INPUT_BUTTON_COMMA, INPUT_BUTTON_PERIOD, INPUT_BUTTON_SLASH};

#endif
