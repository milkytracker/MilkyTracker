/*
 *  tracker/InputControlListener.cpp
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
 *  InputControlListener.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 22.04.05.
 *
 */

#include <ctype.h>
#include "InputControlListener.h"
#include "PPUI.h"
#include "Tracker.h"
#include "Screen.h"
#include "ControlIDs.h"
#include "SIPButtons.h"
#include "PatternEditorControl.h"
#include "RecorderLogic.h"

static const pp_uint16 scanCodesLetters[]	= {SC_A, SC_B, SC_C, SC_D, SC_E, SC_F, SC_G, SC_H, SC_I, SC_J, SC_K, SC_L, SC_M,
											   SC_N, SC_O, SC_P, SC_Q, SC_R, SC_S, SC_T, SC_U, SC_V, SC_W, SC_X, SC_Z, SC_Y};

static const pp_uint16 scanCodesNumbers[]	= {SC_0, SC_1, SC_2, SC_3, SC_4, SC_5, SC_6, SC_7, SC_8, SC_9};

static const pp_uint8 shiftNumbers[]		= {')','!','@','#','$','%','^','&','*','('};

void InputControlListener::sendKey(EEventDescriptor event, pp_uint16 vk, pp_uint16 sc, pp_uint16 chr)
{
	PPScreen* screen = tracker.screen;

	if (event == eKeyDown || eKeyUp)
	{
		// Send keydown to simulate note key press	
		pp_uint16 vksc[3] = {vk, sc, chr};

		PPEvent keyDown(event, &vksc, sizeof(vksc));
		screen->raiseEvent(&keyDown);
	}
	else if (event == eKeyChar)
	{
		// Send keydown to simulate note key press	
		pp_uint16 vksc[2] = {vk, sc};

		PPEvent keyDown(event, &vksc, sizeof(vksc));
		screen->raiseEvent(&keyDown);
	}
}

void InputControlListener::sendNote(pp_int32 v, pp_int32 volume/* = -1*/)
{
	PPScreen* screen = tracker.screen;

	pp_int32 note = v & 0xFFFF;				
				
	pp_int32 octave = (note / 12) + 1;
	pp_int32 realNote = note % 12;
				
	// virtual keys are actually not relevant for note input
	static const pp_uint16 noteKeys[12]			= {0x5a, 0x53, 0x58, 0x44, 0x43, 0x56, 0x47, 0x42, 0x48, 0x4e, 0x4a, 0x4d};
	static const pp_uint16 noteKeyScancodes[12]	= {SC_Y, SC_S, SC_X, SC_D, SC_C, SC_V, SC_G, SC_B, SC_H, SC_N, SC_J, SC_M};
	// ascii keys are actually not relevant for note input
	static const pp_uint16 noteKeyAscii[12]		= {'z', 's', 'x', 'd', 'c', 'v', 'g', 'b', 'h', 'n', 'j', 'm'};
				
	pp_uint16 vk[3];

	// Save current octave
	pp_int32 currentOctave = tracker.getPatternEditor()->getCurrentOctave();	
	tracker.getPatternEditor()->setCurrentOctave(octave);
	tracker.recorderLogic->setKeyVolume(volume);
	
	vk[0] = noteKeys[realNote];
	vk[1] = noteKeyScancodes[realNote];
	vk[2] = noteKeyAscii[realNote];

	// Send keydown/keyup to simulate note key press
	PPEvent event((v >> 16) ? eKeyUp : eKeyDown, &vk, sizeof(vk));
	screen->raiseEvent(&event);	

	tracker.recorderLogic->setKeyVolume(-1);
	// restore octave
	tracker.getPatternEditor()->setCurrentOctave(currentOctave);
}

pp_int32 InputControlListener::handleEvent(PPObject* sender, PPEvent* event)
{
	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case INPUT_BUTTON_CAPSLOCK:
			{	
				capsLockPressed = !capsLockPressed;
				PPButton* button = reinterpret_cast<PPButton*>(sender);
				button->setPressed(capsLockPressed);
				refreshSIP();
				break;
			}

			case INPUT_BUTTON_LSHIFT:
			{	
				lShiftPressed = !lShiftPressed;
				PPButton* button = reinterpret_cast<PPButton*>(sender);
				button->setPressed(lShiftPressed);
				refreshSIP();
				break;
			}

			case INPUT_BUTTON_RSHIFT:
			{	
				rShiftPressed = !rShiftPressed;
				PPButton* button = reinterpret_cast<PPButton*>(sender);
				button->setPressed(rShiftPressed);
				refreshSIP();
				break;
			}
		
			case INPUT_BUTTON_0:
			case INPUT_BUTTON_1:
			case INPUT_BUTTON_2:
			case INPUT_BUTTON_3:
			case INPUT_BUTTON_4:
			case INPUT_BUTTON_5:
			case INPUT_BUTTON_6:
			case INPUT_BUTTON_7:
			case INPUT_BUTTON_8:
			case INPUT_BUTTON_9:
			{
				pp_uint16 key[2];
				key[0] = (pp_uint16)(reinterpret_cast<PPControl*>(sender)->getID() - INPUT_BUTTON_0 + '0');
				key[1] = scanCodesNumbers[key[0] - '0'];
				
				sendKey(eKeyDown, key[0], key[1], uppercase() ? key[0] : shiftNumbers[key[0]-'0']);
				sendKey(eKeyChar, uppercase() ? key[0] : shiftNumbers[key[0]-'0'], key[1], 0);
				sendKey(eKeyUp, key[0], key[1], uppercase() ? key[0] : shiftNumbers[key[0]-'0']);

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_A:
			case INPUT_BUTTON_B:
			case INPUT_BUTTON_C:
			case INPUT_BUTTON_D:
			case INPUT_BUTTON_E:
			case INPUT_BUTTON_F:
			case INPUT_BUTTON_G:
			case INPUT_BUTTON_H:
			case INPUT_BUTTON_I:
			case INPUT_BUTTON_J:
			case INPUT_BUTTON_K:
			case INPUT_BUTTON_L:
			case INPUT_BUTTON_M:
			case INPUT_BUTTON_N:
			case INPUT_BUTTON_O:
			case INPUT_BUTTON_P:
			case INPUT_BUTTON_Q:
			case INPUT_BUTTON_R:
			case INPUT_BUTTON_S:
			case INPUT_BUTTON_T:
			case INPUT_BUTTON_U:
			case INPUT_BUTTON_V:
			case INPUT_BUTTON_W:
			case INPUT_BUTTON_X:
			case INPUT_BUTTON_Y:
			case INPUT_BUTTON_Z:
			{
				pp_uint16 key[2];
				key[0] = (pp_uint16)(reinterpret_cast<PPControl*>(sender)->getID() - INPUT_BUTTON_A + 'A');
				key[1] = scanCodesLetters[key[0] - 'A'];

				sendKey(eKeyDown, key[0], key[1], !uppercase() ? key[0] : tolower(key[0]));
				sendKey(eKeyChar, !uppercase() ? key[0] : tolower(key[0]), key[1], 0);
				sendKey(eKeyUp, key[0], key[1], !uppercase() ? key[0] : tolower(key[0]));

				handleModifiers();
				break;
			}
			
			case INPUT_BUTTON_MINUS:
			{
				sendKey(eKeyDown, 189, 12, !uppercase() ? '_' : '-');
				sendKey(eKeyChar, !uppercase() ? '_' : '-', 12, 0);
				sendKey(eKeyUp, 189, 12, !uppercase() ? '_' : '-');

				handleModifiers();
				break;
			}
			
			case INPUT_BUTTON_PLUS:
			{
				sendKey(eKeyDown, 187, 13, !uppercase() ? '+' : '=');
				sendKey(eKeyChar, !uppercase() ? '+' : '=', 13, 0);
				sendKey(eKeyUp, 187, 13, !uppercase() ? '+' : '=');

				handleModifiers();
				break;
			}
			
			case INPUT_BUTTON_BRACKETOPEN:
			{
				sendKey(eKeyDown, 219, 26, !uppercase() ? '{' : '[');
				sendKey(eKeyChar,  !uppercase() ? '{' : '[', 26, 0);
				sendKey(eKeyUp, 219, 26, !uppercase() ? '{' : '[');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_BRACKETCLOSE:
			{
				sendKey(eKeyDown, 221, 27, !uppercase() ? '}' : ']');
				sendKey(eKeyChar, !uppercase() ? '}' : ']', 27, 0);
				sendKey(eKeyUp, 221, 27, !uppercase() ? '}' : ']');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_SEMICOLON:
			{
				sendKey(eKeyDown, 186, 39, !uppercase() ? ':' : ';');
				sendKey(eKeyChar, !uppercase() ? ':' : ';', 39, 0);
				sendKey(eKeyUp, 186, 39, !uppercase() ? ':' : ';');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_TICK:
			{
				sendKey(eKeyDown, 222, 40, !uppercase() ? '\"' : '\'');
				sendKey(eKeyChar, !uppercase() ? '\"' : '\'', 40, 0);
				sendKey(eKeyUp, 222, 40, !uppercase() ? '\"' : '\'');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_BACKSLASH:
			{
				sendKey(eKeyDown, 220, 43, !uppercase() ? '|' : '\\');
				sendKey(eKeyChar, !uppercase() ? '|' : '\\', 43, 0);
				sendKey(eKeyUp, 220, 43, !uppercase() ? '|' : '\\');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_TILDE:
			{
				sendKey(eKeyDown, 226, 86, !uppercase() ? '|' : '~');
				sendKey(eKeyChar, !uppercase() ? '|' : '~', 86, 0);
				sendKey(eKeyUp, 226, 86, !uppercase() ? '|' : '~');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_COMMA:
			{
				sendKey(eKeyDown, 188, 51, !uppercase() ? '<' : ',');
				sendKey(eKeyChar, !uppercase() ? '<' : ',', 51, 0);
				sendKey(eKeyUp, 188, 51, !uppercase() ? '<' : ',');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_PERIOD:
			{
				sendKey(eKeyDown, 190, 52, !uppercase() ? '>' : '.');
				sendKey(eKeyChar, !uppercase() ? '>' : '.', 52, 0);
				sendKey(eKeyUp, 190, 52, !uppercase() ? '>' : '.');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_SLASH:
			{
				sendKey(eKeyDown, 191, 53, !uppercase() ? '?' : '/');
				sendKey(eKeyChar, !uppercase() ? '?' : '/', 53, 0);
				sendKey(eKeyUp, 191, 53, !uppercase() ? '?' : '/');

				handleModifiers();
				break;
			}

			case INPUT_BUTTON_INS:
				sendKey(eKeyDown, VK_INSERT, 0, 0);
				sendKey(eKeyUp, VK_INSERT, 0, 0);
				break;

			case INPUT_BUTTON_DEL:
				sendKey(eKeyDown, VK_DELETE, 0, 0);
				sendKey(eKeyUp, VK_DELETE, 0, 0);
				break;

			case INPUT_BUTTON_BACK:
				sendKey(eKeyDown, VK_BACK, 0, 0);
				sendKey(eKeyUp, VK_BACK, 0, 0);
				break;

			case INPUT_BUTTON_INSLINE:
				sendKey(eKeyDown, VK_SHIFT, 0, 0);
				::setForceKeyModifier(KeyModifierSHIFT);
				sendKey(eKeyDown, VK_INSERT, 0, 0);
				sendKey(eKeyUp, VK_INSERT, 0, 0);
				::clearForceKeyModifier(KeyModifierSHIFT);
				sendKey(eKeyUp, VK_SHIFT, 0, 0);
				break;

			case INPUT_BUTTON_BACKLINE:
				sendKey(eKeyDown, VK_SHIFT, 0, 0);
				::setForceKeyModifier(KeyModifierSHIFT);
				sendKey(eKeyDown, VK_BACK, 0, 0);
				sendKey(eKeyUp, VK_BACK, 0, 0);
				::clearForceKeyModifier(KeyModifierSHIFT);
				sendKey(eKeyUp, VK_SHIFT, 0, 0);
				break;

			case INPUT_BUTTON_WTF:
				if (!uppercase())
				{
					::setForceKeyModifier(KeyModifierSHIFT);
					sendKey(eKeyDown, VK_SHIFT, 0, 0);
				}
				sendKey(eKeyDown, VK_OEM_3, SC_WTF, '\x60');
				sendKey(eKeyChar, '\x60', 53, 0);
				sendKey(eKeyUp, VK_OEM_3, SC_WTF, '\x60');

				if (!uppercase())
				{
					::clearForceKeyModifier(KeyModifierSHIFT);
					sendKey(eKeyUp, VK_SHIFT, 0, 0);
				}
				handleModifiers();
				break;

			case INPUT_BUTTON_KEYOFF:
				sendKey(eKeyDown, 
						tracker.editMode == EditModeMilkyTracker ? VK_OEM_3 : VK_OEM_102, 
						tracker.editMode == EditModeMilkyTracker ? SC_WTF : SC_SMALLERGREATER, 0);
				sendKey(eKeyUp, 
						tracker.editMode == EditModeMilkyTracker ? VK_OEM_3 : VK_OEM_102, 
						tracker.editMode == EditModeMilkyTracker ? SC_WTF : SC_SMALLERGREATER, 0);	
				break;

			case INPUT_BUTTON_SPACE:
				sendKey(eKeyDown, VK_SPACE, 666, ' ');
				sendKey(eKeyChar, ' ', 666, 0);
				sendKey(eKeyUp, VK_SPACE, 666, ' ');

				handleModifiers();
				break;

			case INPUT_BUTTON_ENTER:
				sendKey(eKeyDown, VK_RETURN, 777, 0x0A);
				sendKey(eKeyChar, 0x0A, 777, 0);
				sendKey(eKeyUp, VK_RETURN, 777, 0x0A);

				handleModifiers();
				break;

			case INPUT_BUTTON_TAB:
				sendKey(eKeyDown, VK_TAB, 888, 0x08);
				sendKey(eKeyChar, 0x08, 888, 0);
				sendKey(eKeyUp, VK_TAB, 888, 0x08);

				handleModifiers();
				break;
				
			case INPUT_BUTTON_SHRINK:
			{
				if (event->getID() != eCommand)
					break;
				tracker.setInputControl(Tracker::SIPDefault);
				tracker.screen->paint();
				break;
			}

			case INPUT_BUTTON_EXPAND:
			{
				if (event->getID() != eCommand)
					break;
				tracker.setInputControl(Tracker::SIPExtended);
				tracker.screen->paint();
				break;
			}
		}
	}
	else if (event->getID() == eSelection)
	{
		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case PIANO_CONTROL:
			{
				pp_int32 v = *((pp_int32*)event->getDataPtr());

				sendNote(v);
				break;
			}
		}
	}
	
	return 0;	
}

void InputControlListener::handleModifiers()
{
	PPButton* button = static_cast<PPButton*>(tracker.inputContainerExtended->getControlByID(INPUT_BUTTON_LSHIFT));
	
	ASSERT(button);
	
	if (lShiftPressed && button->isPressed())
	{
		button->setPressed(false);
		lShiftPressed = false;
	}

	refreshSIP();
}

void InputControlListener::refreshSIP()
{
	pp_int32 i;

	PPContainer* container = tracker.inputContainerExtended;

	for (i = 0; i < (signed)sizeof(keyLine_0_lowerCase); i++)
	{
		PPButton* button = static_cast<PPButton*>(container->getControlByID(keyLineIDs_0[i]));
		button->setText(uppercase() ? keyLine_0_lowerCase[i] : keyLine_0_upperCase[i]);
	}

	for (i = 0; i < (signed)sizeof(keyLine_1_lowerCase); i++)
	{
		PPButton* button = static_cast<PPButton*>(container->getControlByID(keyLineIDs_1[i]));
		button->setText(uppercase() ? keyLine_1_lowerCase[i] : keyLine_1_upperCase[i]);
	}

	for (i = 0; i < (signed)sizeof(keyLine_2_lowerCase); i++)
	{
		PPButton* button = static_cast<PPButton*>(container->getControlByID(keyLineIDs_2[i]));
		button->setText(uppercase() ? keyLine_2_lowerCase[i] : keyLine_2_upperCase[i]);
	}

	for (i = 0; i < (signed)sizeof(keyLine_3_lowerCase); i++)
	{
		PPButton* button = static_cast<PPButton*>(container->getControlByID(keyLineIDs_3[i]));
		button->setText(uppercase() ? keyLine_3_lowerCase[i] : keyLine_3_upperCase[i]);
	}

	tracker.screen->paintControl(container);
}
