/*
 *  tracker/PatternEditorControlKeyboard.cpp
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
 *  PatternEditorControlKeyboard.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Fri Mar 11 2005.
 *
 */
 
#include "PatternEditorControl.h"
#include "PatternTools.h"
#include "KeyBindings.h"

void PatternEditorControl::initKeyBindings()
{
	eventKeyDownBindingsMilkyTracker = new PPKeyBindings<TPatternEditorKeyBindingHandler>;

	// Key-down bindings MilkyTracker
	eventKeyDownBindingsMilkyTracker->addBinding(VK_LEFT, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_LEFT);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_RIGHT, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_RIGHT);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_UP, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_UP);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_DOWN, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_DOWN);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_PRIOR, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_PRIOR);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_NEXT, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_NEXT);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_HOME, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_HOME);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_END, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_END);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_TAB, 0, &PatternEditorControl::eventKeyDownBinding_NextChannel);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_TAB, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_NextChannel);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_TAB, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_PreviousChannel);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_TAB, KeyModifierSHIFT|KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_PreviousChannel);

	eventKeyDownBindingsMilkyTracker->addBinding(VK_DELETE, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_DeleteNoteVolumeAndEffect);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_DELETE, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_DeleteVolumeAndEffect);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_DELETE, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_DeleteEffect);

	eventKeyDownBindingsMilkyTracker->addBinding(VK_INSERT, 0, &PatternEditorControl::eventKeyDownBinding_InsertNote);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_INSERT, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_InsertLine);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_BACK, 0, &PatternEditorControl::eventKeyDownBinding_DeleteNoteSlot);
	eventKeyDownBindingsMilkyTracker->addBinding(VK_BACK, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_DeleteLine);

	eventKeyDownBindingsMilkyTracker->addBinding('Z', KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_Undo);
	eventKeyDownBindingsMilkyTracker->addBinding('Y', KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_Redo);
	eventKeyDownBindingsMilkyTracker->addBinding('X', KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_Cut);
	eventKeyDownBindingsMilkyTracker->addBinding('C', KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_Copy);
	eventKeyDownBindingsMilkyTracker->addBinding('V', KeyModifierSHIFT|KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_TransparentPaste);
	eventKeyDownBindingsMilkyTracker->addBinding('V', KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_Paste);
	eventKeyDownBindingsMilkyTracker->addBinding('A', KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_SelectAll);
	eventKeyDownBindingsMilkyTracker->addBinding('M', KeyModifierSHIFT, &PatternEditorControl::eventKeyCharBinding_MuteChannel);
	eventKeyDownBindingsMilkyTracker->addBinding('M', KeyModifierSHIFT|KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_InvertMuting);
	eventKeyDownBindingsMilkyTracker->addBinding('I', KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_Interpolate);

	// Scancode bindings
	scanCodeBindingsMilkyTracker = new PPKeyBindings<TPatternEditorKeyBindingHandler>;

	scanCodeBindingsMilkyTracker->addBinding(SC_SS, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_InsDecSelection);
	scanCodeBindingsMilkyTracker->addBinding(SC_TICK, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_InsIncSelection);
	scanCodeBindingsMilkyTracker->addBinding(SC_SS, KeyModifierSHIFT|KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_InsDecTrack);
	scanCodeBindingsMilkyTracker->addBinding(SC_TICK, KeyModifierSHIFT|KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_InsIncTrack);

	// Key-down bindings Fasttracker
	eventKeyDownBindingsFastTracker = new PPKeyBindings<TPatternEditorKeyBindingHandler>;

	eventKeyDownBindingsFastTracker->addBinding(VK_LEFT, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_LEFT);
	eventKeyDownBindingsFastTracker->addBinding(VK_RIGHT, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_RIGHT);
	eventKeyDownBindingsFastTracker->addBinding(VK_UP, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_UP);
	eventKeyDownBindingsFastTracker->addBinding(VK_DOWN, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_DOWN);
	eventKeyDownBindingsFastTracker->addBinding(VK_PRIOR, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_PRIOR);
	eventKeyDownBindingsFastTracker->addBinding(VK_NEXT, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_NEXT);
	eventKeyDownBindingsFastTracker->addBinding(VK_HOME, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_HOME);
	eventKeyDownBindingsFastTracker->addBinding(VK_END, 0xFFFF, &PatternEditorControl::eventKeyDownBinding_END);
	eventKeyDownBindingsFastTracker->addBinding(VK_TAB, 0, &PatternEditorControl::eventKeyDownBinding_NextChannel);
	eventKeyDownBindingsFastTracker->addBinding(VK_TAB, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_PreviousChannel);
	eventKeyDownBindingsFastTracker->addBinding(VK_TAB, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_NextChannel);
	eventKeyDownBindingsFastTracker->addBinding(VK_TAB, KeyModifierSHIFT|KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_PreviousChannel);

	eventKeyDownBindingsFastTracker->addBinding(VK_DELETE, 0, &PatternEditorControl::eventKeyDownBinding_DeleteNote);
	eventKeyDownBindingsFastTracker->addBinding(VK_DELETE, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_DeleteNoteVolumeAndEffect);
	eventKeyDownBindingsFastTracker->addBinding(VK_DELETE, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_DeleteVolumeAndEffect);
	eventKeyDownBindingsFastTracker->addBinding(VK_DELETE, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_DeleteEffect);

	eventKeyDownBindingsFastTracker->addBinding(VK_INSERT, 0, &PatternEditorControl::eventKeyDownBinding_InsertNote);
	eventKeyDownBindingsFastTracker->addBinding(VK_INSERT, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_InsertLine);
	eventKeyDownBindingsFastTracker->addBinding(VK_BACK, 0, &PatternEditorControl::eventKeyDownBinding_DeleteNoteSlot);
	eventKeyDownBindingsFastTracker->addBinding(VK_BACK, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_DeleteLine);

	eventKeyDownBindingsFastTracker->addBinding(VK_F3, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_CutTrack);
	eventKeyDownBindingsFastTracker->addBinding(VK_F4, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_CopyTrack);
	eventKeyDownBindingsFastTracker->addBinding(VK_F5, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_PasteTrack);
	eventKeyDownBindingsFastTracker->addBinding(VK_F6, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_TransparentPasteTrack);

	eventKeyDownBindingsFastTracker->addBinding(VK_F3, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_CutPattern);
	eventKeyDownBindingsFastTracker->addBinding(VK_F4, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_CopyPattern);
	eventKeyDownBindingsFastTracker->addBinding(VK_F5, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_PastePattern);
	eventKeyDownBindingsFastTracker->addBinding(VK_F6, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_TransparentPastePattern);

	eventKeyDownBindingsFastTracker->addBinding(VK_F3, KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_Cut);
	eventKeyDownBindingsFastTracker->addBinding(VK_F4, KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_Copy);
	eventKeyDownBindingsFastTracker->addBinding(VK_F5, KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_Paste);
	eventKeyDownBindingsFastTracker->addBinding(VK_F6, KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_TransparentPaste);

	// Typical:
	// Remember to check keyboard shortcuts which are always possible in file TrackerShortCuts.cpp 
	// (= keyboard handler) otherwise they might not be routed here
	eventKeyDownBindingsFastTracker->addBinding('Z', KeyModifierCTRL|KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_Undo);
	eventKeyDownBindingsFastTracker->addBinding('Y', KeyModifierCTRL|KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_Redo);
	eventKeyDownBindingsFastTracker->addBinding('X', KeyModifierCTRL|KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_Cut);
	eventKeyDownBindingsFastTracker->addBinding('C', KeyModifierCTRL|KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_Copy);
	eventKeyDownBindingsFastTracker->addBinding('V', KeyModifierCTRL|KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_Paste);
	eventKeyDownBindingsFastTracker->addBinding('A', KeyModifierCTRL|KeyModifierALT, &PatternEditorControl::eventKeyCharBinding_SelectAll);

	eventKeyDownBindingsFastTracker->addBinding('M', KeyModifierSHIFT, &PatternEditorControl::eventKeyCharBinding_MuteChannel);
	eventKeyDownBindingsFastTracker->addBinding('M', KeyModifierSHIFT|KeyModifierCTRL, &PatternEditorControl::eventKeyCharBinding_InvertMuting);
	eventKeyDownBindingsFastTracker->addBinding('I', KeyModifierSHIFT, &PatternEditorControl::eventKeyCharBinding_Interpolate);

	scanCodeBindingsFastTracker = new PPKeyBindings<TPatternEditorKeyBindingHandler>;

	// Scancode bindings
	scanCodeBindingsFastTracker->addBinding(SC_1, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro1);
	scanCodeBindingsFastTracker->addBinding(SC_2, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro2);
	scanCodeBindingsFastTracker->addBinding(SC_3, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro3);
	scanCodeBindingsFastTracker->addBinding(SC_4, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro4);
	scanCodeBindingsFastTracker->addBinding(SC_5, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro5);
	scanCodeBindingsFastTracker->addBinding(SC_6, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro6);
	scanCodeBindingsFastTracker->addBinding(SC_7, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro7);
	scanCodeBindingsFastTracker->addBinding(SC_8, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro8);
	scanCodeBindingsFastTracker->addBinding(SC_9, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro9);
	scanCodeBindingsFastTracker->addBinding(SC_0, KeyModifierALT|KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_ReadMacro0);

	scanCodeBindingsFastTracker->addBinding(SC_1, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro1);
	scanCodeBindingsFastTracker->addBinding(SC_2, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro2);
	scanCodeBindingsFastTracker->addBinding(SC_3, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro3);
	scanCodeBindingsFastTracker->addBinding(SC_4, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro4);
	scanCodeBindingsFastTracker->addBinding(SC_5, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro5);
	scanCodeBindingsFastTracker->addBinding(SC_6, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro6);
	scanCodeBindingsFastTracker->addBinding(SC_7, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro7);
	scanCodeBindingsFastTracker->addBinding(SC_8, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro8);
	scanCodeBindingsFastTracker->addBinding(SC_9, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro9);
	scanCodeBindingsFastTracker->addBinding(SC_0, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_WriteMacro0);
	
	scanCodeBindingsFastTracker->addBinding(SC_Q, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_Q);
	scanCodeBindingsFastTracker->addBinding(SC_W, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_W);
	scanCodeBindingsFastTracker->addBinding(SC_E, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_E);
	scanCodeBindingsFastTracker->addBinding(SC_R, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_R);
	scanCodeBindingsFastTracker->addBinding(SC_T, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_T);
	scanCodeBindingsFastTracker->addBinding(SC_Z, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_Z);
	scanCodeBindingsFastTracker->addBinding(SC_U, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_U);
	scanCodeBindingsFastTracker->addBinding(SC_I, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_I);

	scanCodeBindingsFastTracker->addBinding(SC_A, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_A);
	scanCodeBindingsFastTracker->addBinding(SC_S, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_S);
	scanCodeBindingsFastTracker->addBinding(SC_D, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_D);
	scanCodeBindingsFastTracker->addBinding(SC_F, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_F);
	scanCodeBindingsFastTracker->addBinding(SC_G, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_G);
	scanCodeBindingsFastTracker->addBinding(SC_H, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_H);
	scanCodeBindingsFastTracker->addBinding(SC_J, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_J);
	scanCodeBindingsFastTracker->addBinding(SC_K, KeyModifierALT, &PatternEditorControl::eventKeyDownBinding_SC_K);
	
	scanCodeBindingsFastTracker->addBinding(SC_WTF, 0, &PatternEditorControl::eventKeyDownBinding_SC_IncreaseRowInsertAdd);
	scanCodeBindingsFastTracker->addBinding(SC_WTF, KeyModifierSHIFT, &PatternEditorControl::eventKeyDownBinding_SC_DecreaseRowInsertAdd);

	scanCodeBindingsFastTracker->addBinding(SC_SS, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_InsDecSelection);
	scanCodeBindingsFastTracker->addBinding(SC_TICK, KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_InsIncSelection);
	scanCodeBindingsFastTracker->addBinding(SC_SS, KeyModifierSHIFT|KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_InsDecTrack);
	scanCodeBindingsFastTracker->addBinding(SC_TICK, KeyModifierSHIFT|KeyModifierCTRL, &PatternEditorControl::eventKeyDownBinding_InsIncTrack);

	eventKeyDownBindings = eventKeyDownBindingsMilkyTracker;
}

static pp_int32 asciiToHex(pp_uint8 ascii)
{
	pp_int32 number = -1;
	
	switch (ascii)
	{
			// 0: 
		case '0':
			number = 0;
			break;

			// 1: 
		case '1':
			number = 1;
			break;
			
			// 2: 
		case '2':
			number = 2;
			break;
			
			// 3: 
		case '3':
			number = 3;
			break;
			
			// 4: 
		case '4':
			number = 4;
			break;
			
			// 5: 
		case '5':
			number = 5;
			break;
			
			// 6: 
		case '6':
			number = 6;
			break;
			
			// 7: 
		case '7':
			number = 7;
			break;

			// 8: 
		case '8':
			number = 8;
			break;
			
			// 9: 
		case '9':
			number = 9;
			break;
			
			// A: 
		case 'A':
		case 'a':
			number = 0xA;
			break;

			// B: 
		case 'B':
		case 'b':
			number = 0xB;
			break;

			// C: 
		case 'C':
		case 'c':
			number = 0xC;
			break;
			
			// D: 
		case 'D':
		case 'd':
			number = 0xD;
			break;
			
			// E: 
		case 'E':
		case 'e':
			number = 0xE;
			break;
			
			// F: 
		case 'F':
		case 'f':
			number = 0xF;
			break;
	}

	return number;
}

static pp_int32 asciiToHexExtended(pp_uint8 ascii)
{
	pp_int32 number = -1;

	switch (ascii)
	{
		case 'G':
		case 'g':
			number = 0x10;
			break;
		case 'H':
		case 'h':
			number = 0x11;
			break;
		case 'I':
		case 'i':
			number = 0x12;
			break;
		case 'J':
		case 'j':
			number = 0x13;
			break;
		case 'K':
		case 'k':
			number = 0x14;
			break;
		case 'L':
		case 'l':
			number = 0x15;
			break;
		case 'M':
		case 'm':
			number = 0x16;
			break;
		case 'N':
		case 'n':
			number = 0x17;
			break;
		case 'O':
		case 'o':
			number = 0x18;
			break;
		case 'P':
		case 'p':
			number = 0x19;
			break;
		case 'Q':
		case 'q':
			number = 0x1A;
			break;
		case 'R':
		case 'r':
			number = 0x1B;
			break;
		case 'S':
		case 's':
			number = 0x1C;
			break;
		case 'T':
		case 't':
			number = 0x1D;
			break;
		case 'U':
		case 'u':
			number = 0x1E;
			break;
		case 'V':
		case 'v':
			number = 0x1F;
			break;
		case 'W':
		case 'w':
			number = 0x20;
			break;
		case 'X':
		case 'x':
			number = 0x21;
			break;
		case 'Y':
		case 'y':
			number = 0x22;
			break;
		case 'Z':
		case 'z':
			number = 0x23;
			break;
	}

	return number;
}

pp_int32 PatternEditorControl::ScanCodeToNote(pp_int16 scanCode)
{
	switch (scanCode)
	{
		// 2,L: C#
		case 0x03:
		case 0x26:
			return TONOTE(patternEditor->getCurrentOctave(),1);
		
		// 3,÷: D#
		case 0x04:
		case 0x27:
			return TONOTE(patternEditor->getCurrentOctave(),3);
		
		// 5,': F#
		case 0x06:
		case 0x2B:
			return TONOTE(patternEditor->getCurrentOctave(),6);
		
		// 6: G#
		case 0x07:
			return TONOTE(patternEditor->getCurrentOctave(),8);

		// 7: A#
		case 0x08:
			return TONOTE(patternEditor->getCurrentOctave(),10);
		
		// 9: C# + 1
		case 0x0A:
			return TONOTE(patternEditor->getCurrentOctave() + 1,1);
		
		// 0: D# + 1
		case 0x0B:
			return TONOTE(patternEditor->getCurrentOctave() + 1,3);
		
		// ¥: F# + 1
		case 0x0D:
			return TONOTE(patternEditor->getCurrentOctave() + 1,6);
		
		// S: C# - 1
		case 0x1F:
			return TONOTE(patternEditor->getCurrentOctave() - 1,1);
		
		// D: D# - 1
		case 0x20:
			return TONOTE(patternEditor->getCurrentOctave() - 1,3);
		
		// G: F# - 1
		case 0x22:
			return TONOTE(patternEditor->getCurrentOctave() - 1,6);
		
		// H: G# - 1
		case 0x23:
			return TONOTE(patternEditor->getCurrentOctave() - 1,8);

		// J: A# - 1
		case 0x24:
			return TONOTE(patternEditor->getCurrentOctave() - 1,10);

		// Q: C
		case 0x10:
			return TONOTE(patternEditor->getCurrentOctave(),0);

		// W: D
		case 0x11:
			return TONOTE(patternEditor->getCurrentOctave(),2);

		// E: E
		case 0x12:
			return TONOTE(patternEditor->getCurrentOctave(),4);

		// R: F
		case 0x13:
			return TONOTE(patternEditor->getCurrentOctave(),5);

		// T: G
		case 0x14:
			return TONOTE(patternEditor->getCurrentOctave(),7);

		// Z: A
		case 0x15:
			return TONOTE(patternEditor->getCurrentOctave(),9);

		// U: B
		case 0x16:
			return TONOTE(patternEditor->getCurrentOctave(),11);
		
		// Y: C - 1
		case 0x2C:
			return TONOTE(patternEditor->getCurrentOctave() - 1,0);
		
		// X: D - 1
		case 0x2D:
			return TONOTE(patternEditor->getCurrentOctave() - 1,2);
		
		// C: E - 1 
		case 0x2E:
			return TONOTE(patternEditor->getCurrentOctave() - 1,4);

		// R: F - 1
		case 0x2F:
			return TONOTE(patternEditor->getCurrentOctave() - 1,5);

		// B: G - 1
		case 0x30:
			return TONOTE(patternEditor->getCurrentOctave() - 1,7);

		// N: A - 1
		case 0x31:
			return TONOTE(patternEditor->getCurrentOctave() - 1,9);
		// M: B - 1
		case 0x32:
			return TONOTE(patternEditor->getCurrentOctave() - 1,11);
		
		// ,: C
		case 0x33:
			return TONOTE(patternEditor->getCurrentOctave(),0);

		// .: D
		case 0x34:
			return TONOTE(patternEditor->getCurrentOctave(),2);

		// -: E
		case 0x35:
			return TONOTE(patternEditor->getCurrentOctave(),4);

		// I
		case 0x17:
			return TONOTE(patternEditor->getCurrentOctave()+1,0);

		// O
		case 0x18:
			return TONOTE(patternEditor->getCurrentOctave()+1,2);

		// P
		case 0x19:
			return TONOTE(patternEditor->getCurrentOctave()+1,4);

		// ‹
		case 0x1A:
			return TONOTE(patternEditor->getCurrentOctave()+1,5);

		// +
		case 0x1B:
			return TONOTE(patternEditor->getCurrentOctave()+1,7);

		// ^: Key off
		case SC_WTF:
			if (editMode != EditModeMilkyTracker)
				break;
			return PatternTools::getNoteOffNote();

		// CAPS-lock
		case SC_CAPSLOCK:
#ifndef WIN32
		case SC_1:
#endif
			return PatternTools::getNoteOffNote();

		case SC_SMALLERGREATER:
			return PatternTools::getNoteOffNote();
		
		default:
			return -1;
	}

	return -1;
}

void PatternEditorControl::handleDeleteKey(pp_uint16 keyCode, pp_int32& result)
{
	if (result == -1 &&  ::getKeyModifier() == 0)
	{	
		switch (keyCode)
		{
			// Delete (only works in MilkyTracker edit mode)
			// in FT2 mode this key combination is bound to a method
			case VK_DELETE:
				if (hasValidSelection())
					patternEditor->clearSelection();
				else
				{
					result = 0xFF;
				}
				break;
		}			
	}
}

// "C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"
void PatternEditorControl::handleKeyDown(pp_uint16 keyCode, pp_uint16 scanCode, pp_uint16 character)
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();
	pp_int32 number = -1;

	if (::getKeyModifier() == 0)
	{
		switch (keyCode)
		{
			// F1
			case VK_F1:
				patternEditor->setCurrentOctave(1);
				assureCursor = false;
				notifyUpdate();
				return;
			// F2
			case VK_F2:
				patternEditor->setCurrentOctave(2);
				assureCursor = false;
				notifyUpdate();
				return;
			// F3
			case VK_F3:
				patternEditor->setCurrentOctave(3);
				assureCursor = false;
				notifyUpdate();
				return;
			// F4
			case VK_F4:
				patternEditor->setCurrentOctave(4);
				assureCursor = false;
				notifyUpdate();
				return;
			// F5
			case VK_F5:
				patternEditor->setCurrentOctave(5);
				assureCursor = false;
				notifyUpdate();
				return;
			// F6
			case VK_F6:
				patternEditor->setCurrentOctave(6);
				assureCursor = false;
				notifyUpdate();
				return;
			// F7
			case VK_F7:
				patternEditor->setCurrentOctave(7);
				assureCursor = false;
				notifyUpdate();
				return;
			// F8
			case VK_F8:
				patternEditor->setCurrentOctave(8);
				assureCursor = false;
				notifyUpdate();
				return;

			// F9
			case VK_F9:
				eventKeyDownBinding_HOME();
				return;

			// F10
			case VK_F10:
				eventKeyDownBinding_FIRSTQUARTER();
				return;

			// F11
			case VK_F11:
				eventKeyDownBinding_SECONDQUARTER();
				return;

			// F12
			case VK_F12:
				eventKeyDownBinding_THIRDQUARTER();
				return;
		}
	}

	// prevent unnecessary screen refreshing through listener callback
	// remember to reset this when leaving this function
	patternEditor->setLazyUpdateNotifications(true);

	if (cursor.inner == 0)
	{
		handleDeleteKey(keyCode, number);

		if (number == -1 && ::getKeyModifier() == 0)
			number = ScanCodeToNote(scanCode);

		patternEditor->writeNote(number, true, this);
	}
	else
		handleDeleteKey(keyCode, number);

	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::handleKeyChar(pp_uint8 character)
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();
	pp_int32 number = -1;

	// prevent unnecessary screen refreshing through listener callback
	// remember to reset this when leaving this function
	patternEditor->setLazyUpdateNotifications(true);

	if ((cursor.inner == 1 || cursor.inner == 2))
	{
		number = asciiToHex(character);

		if (number == 0xFF)
			patternEditor->writeInstrument(PatternEditor::NibbleTypeBoth, 0, true, this);
		else if (number >= 0 && number <= 0xF)
		{
			if (cursor.inner == 1)
				patternEditor->writeInstrument(PatternEditor::NibbleTypeHigh, number, true, this);
			else if (cursor.inner == 2)
				patternEditor->writeInstrument(PatternEditor::NibbleTypeLow, number, true, this);
		}
	}
	else if (cursor.inner == 3 || cursor.inner == 4)
	{
		if (cursor.inner == 3)
		{
			switch (character)
			{
				// -: Volslide down
				case '-':
					number = 5;
					break;

				// +: Volslide up
				case '+':
					number = 6;
					break;

				// D: Fine volslide down
				case 'd':
					number = 7;
					break;

				// U: Fine volslide up
				case 'u':
					number = 8;
					break;

				// S: Vibrato rate
				case 's':
					number = 9;
					break;

				// V: Vibrato depth
				case 'v':
					number = 0xA;
					break;

				// P: Set Panning
				case 'p':
					number = 0xB;
					break;

				// L: Panning slide left
				case 'l':
				case '<':
					number = 0xC;
					break;

				// R: Panning slide right
				case 'r':
				case '>':
					number = 0xD;
					break;

				// M: Portamento
				case 'm':
					number = 0xE;
					break;
			}
		}

		// still not assigned, try if we map to a hexadecimal digit
		if (number == -1)
		{
			number = asciiToHex(character);
			if (cursor.inner == 3 && number > 4)
				goto cleanUp;
		}

		if (number == 0xFF || (number >= 0 && number <= 0xF))
		{
			if (number == 0xFF)
				patternEditor->writeFT2Volume(PatternEditor::NibbleTypeBoth, number, true, this);
			else
				patternEditor->writeFT2Volume(cursor.inner == 3 ? PatternEditor::NibbleTypeHigh : PatternEditor::NibbleTypeLow,
						number, true, this);
		}
	}
	else if (cursor.inner == 5)
	{
		number = asciiToHex(character);

		if (number == -1)
			number = asciiToHexExtended(character);

		if (number == 0xFF || (number >= 0 && number <= 0x23))
			patternEditor->writeEffectNumber(number, true, this);
	}
	else if ((cursor.inner == 6 || cursor.inner == 7))
	{
		number = asciiToHex(character);

		if (number == 0xFF || (number >= 0 && number <= 0xF))
		{
			if (number == 0xFF)
				patternEditor->writeEffectOperand(PatternEditor::NibbleTypeBoth, number, true, this);
			else
				patternEditor->writeEffectOperand(cursor.inner == 6 ? PatternEditor::NibbleTypeHigh : PatternEditor::NibbleTypeLow,
												  number, true, this);
		}
	}

	// If the input had an effect, ensure the PatternEditorControl is repainted
	if (number != -1)
		assureUpdate = true;

cleanUp:
	patternEditor->setLazyUpdateNotifications(false);
}

bool PatternEditorControl::executeBinding(const PPKeyBindings<TPatternEditorKeyBindingHandler>* bindings, pp_uint16 keyCode)
{
	if (bindings == NULL)
		return false;

	TPatternEditorKeyBindingHandler handlerFunc;
	bool res = bindings->getBinding(keyCode, ::getKeyModifier(), handlerFunc);

	if (res)
	{
		(this->*handlerFunc)();		
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////
// key-down bindings								//
//////////////////////////////////////////////////////
void PatternEditorControl::eventKeyDownBinding_LEFT()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	cursor.inner--;
	if (cursor.inner < 0)
	{
		if (cursor.channel > 0)
		{
			cursor.channel--;
			cursor.inner = 7;
		}
		else 
		{
			// When selecting don't wrap around (not intuitive)
			if (::getKeyModifier() & selectionKeyModifier)
			{
				cursor.inner = 0;
			}
			else
			{
				cursor.inner = 7;
				cursor.channel = patternEditor->getNumChannels()-1;
			}
		}
	}
}

void PatternEditorControl::eventKeyDownBinding_RIGHT()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	cursor.inner++;
	if (cursor.inner == 8)
	{
		if (cursor.channel < patternEditor->getNumChannels() - 1)
		{
			cursor.channel++;
			cursor.inner = 0;
		}
		else
		{
			// When selecting don't wrap around (not intuitive)
			if (::getKeyModifier() & selectionKeyModifier)
			{
				cursor.inner = 7;
			}
			else
			{
				cursor.inner = 0;
				cursor.channel = 0;
			}
		}
	}
}

void PatternEditorControl::eventKeyDownBinding_UP()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	pp_int32 res = 0;

	cursor.row--;
	
	// When selecting don't wrap around (not intuitive)
	if (cursor.row < 0 && ((::getKeyModifier() & selectionKeyModifier) || startSelection))
	{
		cursor.row = 0;
		return;
	}
	else if (cursor.row < 0)
	{
		res = notifyUpdate(AdvanceCodeCursorUpWrappedStart);
	}

	if (!res && cursor.row < 0)
	{
		//cursor.row = wrapAround ? pattern->rows-1 : 0;
		cursor.row = pattern->rows-1;
	}
	
	if (!res)
		notifyUpdate(AdvanceCodeSelectNewRow);
}

void PatternEditorControl::eventKeyDownBinding_DOWN()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	pp_int32 res = 0;

	cursor.row++;
	
	if (cursor.row > pattern->rows-1 && ((::getKeyModifier() & selectionKeyModifier) || startSelection))
	{
		cursor.row = pattern->rows-1;
		return;
	}
	else if (cursor.row > pattern->rows-1)
	{
		res = notifyUpdate(AdvanceCodeCursorDownWrappedEnd);
	}

	if (!res && cursor.row > pattern->rows-1)
	{
		//cursor.row = wrapAround ? 0 : pattern->rows-1;
		cursor.row = 0;
	}
	
	if (!res)
		notifyUpdate(AdvanceCodeSelectNewRow);
}

void PatternEditorControl::eventKeyDownBinding_PRIOR()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	pp_int32 res = 0;

	cursor.row-=16;
	
	// When selecting don't wrap around (not intuitive)
	if (cursor.row < 0 && ((::getKeyModifier() & selectionKeyModifier) || startSelection))
	{
		cursor.row = 0;
		return;
	}
	else if (cursor.row < 0)
	{
		res = notifyUpdate(AdvanceCodeCursorPageUpWrappedStart);
	}

	if (!res && cursor.row < 0)
	{
		cursor.row = /*wrapAround ? pattern->rows-1 : */0;
	}
	
	if (!res)
		notifyUpdate(AdvanceCodeSelectNewRow);
}

void PatternEditorControl::eventKeyDownBinding_NEXT()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	pp_int32 res = 0;

	cursor.row+=16;
	
	if (cursor.row > pattern->rows-1 && ((::getKeyModifier() & selectionKeyModifier) || startSelection))
	{
		cursor.row = pattern->rows-1;
		return;
	}
	else if (cursor.row > pattern->rows-1)
	{
		res = notifyUpdate(AdvanceCodeCursorPageDownWrappedEnd);
	}

	if (!res && cursor.row > pattern->rows-1)
	{
		cursor.row = /*wrapAround ? 0 : */pattern->rows-1;
	}
	
	if (!res)
		notifyUpdate(AdvanceCodeSelectNewRow);
}

void PatternEditorControl::eventKeyDownBinding_HOME()
{
	patternEditor->getCursor().row = 0;
	notifyUpdate(AdvanceCodeSelectNewRow);
}

void PatternEditorControl::eventKeyDownBinding_END()
{
	patternEditor->getCursor().row = pattern->rows-1;
	notifyUpdate(AdvanceCodeSelectNewRow);
}

void PatternEditorControl::eventKeyDownBinding_FIRSTQUARTER()
{
	patternEditor->getCursor().row = (pattern->rows >> 2);
	notifyUpdate(AdvanceCodeSelectNewRow);
}

void PatternEditorControl::eventKeyDownBinding_SECONDQUARTER()
{
	patternEditor->getCursor().row = (pattern->rows >> 2)*2;
	notifyUpdate(AdvanceCodeSelectNewRow);
}

void PatternEditorControl::eventKeyDownBinding_THIRDQUARTER()
{
	patternEditor->getCursor().row = (pattern->rows >> 2)*3;
	notifyUpdate(AdvanceCodeSelectNewRow);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//											Damn FT2 shortcuts
////////////////////////////////////////////////////////////////////////////////////////////////////////
void PatternEditorControl::eventKeyDownBinding_ReadMacro1()
{
	patternEditor->storeMacroFromCursor(1);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro2()
{
	patternEditor->storeMacroFromCursor(2);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro3()
{
	patternEditor->storeMacroFromCursor(3);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro4()
{
	patternEditor->storeMacroFromCursor(4);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro5()
{
	patternEditor->storeMacroFromCursor(5);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro6()
{
	patternEditor->storeMacroFromCursor(6);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro7()
{
	patternEditor->storeMacroFromCursor(7);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro8()
{
	patternEditor->storeMacroFromCursor(8);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro9()
{
	patternEditor->storeMacroFromCursor(9);
}

void PatternEditorControl::eventKeyDownBinding_ReadMacro0()
{
	patternEditor->storeMacroFromCursor(0);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro1()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(1, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro2()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(2, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro3()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(3, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro4()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(4, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro5()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(5, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro6()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(6, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro7()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(7, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro8()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(8, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro9()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(9, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_WriteMacro0()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->writeMacroToCursor(0, this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_SC_Q()
{
	patternEditor->getCursor().channel = 0 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_W()
{
	patternEditor->getCursor().channel = 1 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_E()
{
	patternEditor->getCursor().channel = 2 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_R()
{
	patternEditor->getCursor().channel = 3 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_T()
{
	patternEditor->getCursor().channel = 4 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_Z()
{
	patternEditor->getCursor().channel = 5 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_U()
{
	patternEditor->getCursor().channel = 6 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_I()
{
	patternEditor->getCursor().channel = 7 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_A()
{
	patternEditor->getCursor().channel = 8 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_S()
{
	patternEditor->getCursor().channel = 9 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_D()
{
	patternEditor->getCursor().channel = 10 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_F()
{
	patternEditor->getCursor().channel = 11 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_G()
{
	patternEditor->getCursor().channel = 12 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_H()
{
	patternEditor->getCursor().channel = 13 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_J()
{
	patternEditor->getCursor().channel = 14 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_K()
{
	patternEditor->getCursor().channel = 15 % patternEditor->getNumChannels();
}

void PatternEditorControl::eventKeyDownBinding_SC_IncreaseRowInsertAdd()
{
	increaseRowInsertAdd();
	notifyUpdate();
}

void PatternEditorControl::eventKeyDownBinding_SC_DecreaseRowInsertAdd()
{
	decreaseRowInsertAdd();
	notifyUpdate();
}

void PatternEditorControl::eventKeyDownBinding_DeleteNote()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->deleteCursorSlotData(this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_DeleteNoteVolumeAndEffect()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->deleteCursorSlotDataEntire(this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_DeleteVolumeAndEffect()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->deleteCursorSlotDataVolumeAndEffect(this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_DeleteEffect()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->deleteCursorSlotDataEffect(this);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_PreviousChannel()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	// if the track inner position is not the note column, we will first 
	// set the position to the note before decrementing the track result
	if (properties.tabToNote && cursor.inner > 0)
	{
		cursor.inner = 0;
		return;
	}

	if (cursor.channel >  0)
	{
		cursor.channel--;
		if (properties.tabToNote)
			cursor.inner = 0;
	}
	else
	{					
		// When selecting don't wrap around (not intuitive)
		if (::getKeyModifier() & selectionKeyModifier)
		{
			cursor.channel = 0;
			cursor.inner = 0;
		}
		else
		{
			cursor.channel = patternEditor->getNumChannels() - 1;
			if (properties.tabToNote)
				cursor.inner = 0;
		}
	}
}

void PatternEditorControl::eventKeyDownBinding_NextChannel()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	if (cursor.channel < patternEditor->getNumChannels() - 1)
	{
		cursor.channel++;
		if (properties.tabToNote)
			cursor.inner = 0;
	}
	else
	{					
		// When selecting don't wrap around (not intuitive)
		if (::getKeyModifier() & selectionKeyModifier)
		{
			cursor.channel = patternEditor->getNumChannels()-1;
			cursor.inner = 7;
		}
		else
		{
			cursor.channel = 0;
			if (properties.tabToNote)
				cursor.inner = 0;
		}
	}
}

void PatternEditorControl::eventKeyDownBinding_InsertNote()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->insertNote(patternEditor->getCursor().channel, patternEditor->getCursor().row);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_InsertLine()
{
	// prevent unnecessary screen refreshing through listener callback
	patternEditor->setLazyUpdateNotifications(true);
	patternEditor->insertLine(patternEditor->getCursor().row);
	patternEditor->setLazyUpdateNotifications(false);
}

void PatternEditorControl::eventKeyDownBinding_DeleteNoteSlot()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();
	
	if (cursor.row)
	{
		cursor.row--;
		// prevent unnecessary screen refreshing through listener callback
		patternEditor->setLazyUpdateNotifications(true);
		patternEditor->deleteNote(cursor.channel, cursor.row);
		patternEditor->setLazyUpdateNotifications(false);
	}
}

void PatternEditorControl::eventKeyDownBinding_DeleteLine()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	if (cursor.row)
	{
		cursor.row--;
		// prevent unnecessary screen refreshing through listener callback
		patternEditor->setLazyUpdateNotifications(true);
		patternEditor->deleteLine(cursor.row);
		patternEditor->setLazyUpdateNotifications(false);
	}
}

void PatternEditorControl::eventKeyDownBinding_InsIncSelection()
{
	patternEditor->insIncSelection();
}

void PatternEditorControl::eventKeyDownBinding_InsDecSelection()
{
	patternEditor->insDecSelection();
}

void PatternEditorControl::eventKeyDownBinding_InsIncTrack()
{
	patternEditor->insIncTrack();
}

void PatternEditorControl::eventKeyDownBinding_InsDecTrack()
{
	patternEditor->insDecTrack();
}

void PatternEditorControl::eventKeyDownBinding_CutTrack()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	PatternEditorTools::Position ss = patternEditor->getSelection().start, se = patternEditor->getSelection().end, cc = cursor;	

	markChannel(cursor.channel, false);

	// if invoked by key combination, the view shouldn't be adjusted
	// to ensure a visible cursor, just stay where you are
	assureCursor = false;
	// but an update might be useful
	assureUpdate = true;
	// remember cursor position
	cursorCopy = patternEditor->getCursor();
	
	patternEditor->cut(PatternEditor::ClipBoardTypeTrack);

	patternEditor->getSelection().start = ss;
	patternEditor->getSelection().end = se;
	cursor = cc;
}

void PatternEditorControl::eventKeyDownBinding_CopyTrack()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	PatternEditorTools::Position ss = patternEditor->getSelection().start, se = patternEditor->getSelection().end, cc = cursor;	

	markChannel(cursor.channel, false);
	
	// if invoked by key combination, the view shouldn't be adjusted
	// to ensure a visible cursor, just stay where you are
	assureCursor = false;
	// but an update might be useful
	assureUpdate = true;
	// remember cursor position
	cursorCopy = patternEditor->getCursor();

	patternEditor->copy(PatternEditor::ClipBoardTypeTrack);

	patternEditor->getSelection().start = ss;
	patternEditor->getSelection().end = se;
	cursor = cc;	
}

void PatternEditorControl::eventKeyDownBinding_PasteTrack()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	PatternEditorTools::Position ss = patternEditor->getSelection().start, se = patternEditor->getSelection().end, cc = cursor;	

	// Start at row 0
	cursor.row = 0;
	cursor.inner = 0;

	patternEditor->paste(PatternEditor::ClipBoardTypeTrack, false);

	patternEditor->getSelection().start = ss;
	patternEditor->getSelection().end = se;
	cursor = cc;		
}

void PatternEditorControl::eventKeyDownBinding_TransparentPasteTrack()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	PatternEditorTools::Position ss = patternEditor->getSelection().start, se = patternEditor->getSelection().end, cc = cursor;	

	// Start at row 0
	cursor.row = 0;
	cursor.inner = 0;
	patternEditor->paste(PatternEditor::ClipBoardTypeTrack, true);

	patternEditor->getSelection().start = ss;
	patternEditor->getSelection().end = se;
	cursor = cc;		
}

void PatternEditorControl::eventKeyDownBinding_CutPattern()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	PatternEditorTools::Position ss = patternEditor->getSelection().start, se = patternEditor->getSelection().end, cc = cursor;	

	selectAll();
	
	// if invoked by key combination, the view shouldn't be adjusted
	// to ensure a visible cursor, just stay where you are
	assureCursor = false;
	// but an update might be useful
	assureUpdate = true;
	// remember cursor position
	cursorCopy = patternEditor->getCursor();
	
	patternEditor->cut(PatternEditor::ClipBoardTypePattern);

	patternEditor->getSelection().start = ss;
	patternEditor->getSelection().end = se;
	cursor = cc;		
}

void PatternEditorControl::eventKeyDownBinding_CopyPattern()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	PatternEditorTools::Position ss = patternEditor->getSelection().start, se = patternEditor->getSelection().end, cc = cursor;	

	selectAll();
	
	// if invoked by key combination, the view shouldn't be adjusted
	// to ensure a visible cursor, just stay where you are
	assureCursor = false;
	// but an update might be useful
	assureUpdate = true;
	// remember cursor position
	cursorCopy = patternEditor->getCursor();

	patternEditor->copy(PatternEditor::ClipBoardTypePattern);

	patternEditor->getSelection().start = ss;
	patternEditor->getSelection().end = se;
	cursor = cc;		
}

void PatternEditorControl::eventKeyDownBinding_PastePattern()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	PatternEditorTools::Position ss = patternEditor->getSelection().start, se = patternEditor->getSelection().end, cc = cursor;	

	// start at row 0/channel 0
	cursor.row = 0;
	cursor.channel = 0;
	cursor.inner = 0;
	
	patternEditor->paste(PatternEditor::ClipBoardTypePattern, false);

	patternEditor->getSelection().start = ss;
	patternEditor->getSelection().end = se;
	cursor = cc;		
}

void PatternEditorControl::eventKeyDownBinding_TransparentPastePattern()
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	PatternEditorTools::Position ss = patternEditor->getSelection().start, se = patternEditor->getSelection().end, cc = cursor;	

	// start at row 0/channel 0
	cursor.row = 0;
	cursor.channel = 0;
	cursor.inner = 0;
	
	patternEditor->paste(PatternEditor::ClipBoardTypePattern, true);

	patternEditor->getSelection().start = ss;
	patternEditor->getSelection().end = se;
	cursor = cc;		
}

void PatternEditorControl::eventKeyCharBinding_Undo()
{
	patternEditor->undo();
}

void PatternEditorControl::eventKeyCharBinding_Redo()
{
	patternEditor->redo();
}

void PatternEditorControl::eventKeyCharBinding_Cut()
{
	// if invoked by key combination, the view shouldn't be adjusted
	// to ensure a visible cursor, just stay where you are
	assureCursor = false;
	// but an update might be useful
	assureUpdate = true;
	// remember cursor position
	cursorCopy = patternEditor->getCursor();
	
	patternEditor->cut(PatternEditor::ClipBoardTypeSelection);
}

void PatternEditorControl::eventKeyCharBinding_Copy()
{
	// if invoked by key combination, the view shouldn't be adjusted
	// to ensure a visible cursor, just stay where you are
	assureCursor = false;
	// but an update might be useful
	assureUpdate = true;
	// remember cursor position
	cursorCopy = patternEditor->getCursor();

	patternEditor->copy(PatternEditor::ClipBoardTypeSelection);
}

void PatternEditorControl::eventKeyCharBinding_Paste()
{
	if (patternEditor->getCursor() == cursorCopy && menuInvokeChannel != -1)
		patternEditor->paste(PatternEditor::ClipBoardTypeSelection, false, menuInvokeChannel);
	else
		patternEditor->paste(PatternEditor::ClipBoardTypeSelection, false);
	// If row count changed, call notifyUpdate to refresh pattern length indicator
	if (patternEditor->getLastOperationDidChangeRows())
		notifyUpdate();
}

void PatternEditorControl::eventKeyCharBinding_TransparentPaste()
{
	if (patternEditor->getCursor() == cursorCopy && menuInvokeChannel != -1)
		patternEditor->paste(PatternEditor::ClipBoardTypeSelection, true, menuInvokeChannel);
	else
		patternEditor->paste(PatternEditor::ClipBoardTypeSelection, true);
	// If row count changed, call notifyUpdate to refresh pattern length indicator
	if (patternEditor->getLastOperationDidChangeRows())
		notifyUpdate();
}

void PatternEditorControl::eventKeyCharBinding_SelectAll()
{
	PatternEditor::Selection currentSelection = patternEditor->getSelection();

	if (currentSelection.start.channel == 0 &&
		currentSelection.start.row == 0 &&
		currentSelection.end.channel == patternEditor->getNumChannels()-1 &&
		currentSelection.end.row == pattern->rows-1)
	{
		deselectAll();
	}
	else
	{
		selectAll();
	}
}

void PatternEditorControl::eventKeyCharBinding_MuteChannel()
{
	muteChannels[patternEditor->getCursor().channel] = !muteChannels[patternEditor->getCursor().channel];
	PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels));						
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);	
}

void PatternEditorControl::eventKeyCharBinding_InvertMuting()
{
	for (pp_int32 i = 0; i < patternEditor->getNumChannels(); i++)
		muteChannels[i] = !muteChannels[i];
	PPEvent e(eValueChanged, &muteChannels, sizeof(muteChannels));						
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);	
}

void PatternEditorControl::eventKeyCharBinding_Interpolate()
{
	patternEditor->interpolateValuesInSelection();
}

