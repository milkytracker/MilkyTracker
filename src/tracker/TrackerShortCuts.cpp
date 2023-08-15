/*
 *  tracker/TrackerShortCuts.cpp
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
 *  TrackerShortCuts.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Thu May 19 2005.
 *
 */

#include "Tracker.h"
#include "ControlIDs.h"
#include "Screen.h"
#include "Event.h"
#include "PlayerController.h"
#include "PlayerLogic.h"
#include "RecorderLogic.h"

#include "Container.h"
#include "ListBox.h"
#include "PatternEditorControl.h"

#include "ModuleEditor.h"
#include "TrackerConfig.h"

#include "InputControlListener.h"
#include "SectionInstruments.h"
#include "SectionTranspose.h"
#include "SectionDiskMenu.h"

void Tracker::sendNoteDown(mp_sint32 note, pp_int32 volume/* = -1*/)
{
	if (volume != -1 && volume > 255)
		volume = 255;

	// Volume here is between 0 to 255, but don't forget to make the volume FT2 compatible (0..64)
	inputControlListener->sendNote(note | InputControlListener::KEY_PRESS,
								   volume == -1 ? -1 : (signed)PatternTools::normalizeVol(volume));
}

void Tracker::sendNoteUp(mp_sint32 note)
{
	// bit 16 indicates key release
	inputControlListener->sendNote(note | InputControlListener::KEY_RELEASE);
}

void Tracker::processShortcuts(PPEvent* event)
{
	if (processMessageBoxShortcuts(event))
		return;

	switch (editMode)
	{
		case EditModeMilkyTracker:
			processShortcutsMilkyTracker(event);
			break;

		case EditModeFastTracker:
			processShortcutsFastTracker(event);
			break;
			
		default:
			ASSERT(false);
	}
}

void Tracker::processShortcutsMilkyTracker(PPEvent* event)
{
	if (event->getID() == eKeyDown)
	{
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());
		pp_uint16 scanCode = *(((pp_uint16 *)event->getDataPtr()) + 1);

		if (::getKeyModifier() == (KeyModifierALT))
		{

			switch (scanCode)
			{
				// insert/delete  new order using  + / -
				case SC_TICK:
				{
					moduleEditor->insertNewOrderPosition(listBoxOrderList->getSelectedIndex());
					updateOrderlist();
					event->cancel();
					break;
				}

				case SC_SS:
				{
					moduleEditor->deleteOrderPosition(listBoxOrderList->getSelectedIndex());
					updateOrderlist();
					event->cancel();
					break;
				}
			}
		}

		switch (keyCode)
		{
			case VK_F1:
			case VK_F2:
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
			case VK_F7:
			case VK_F8:
			case VK_F9:
			case VK_F10:
			case VK_F11:
			case VK_F12:
			{
				if (::getKeyModifier())
					goto processBindings;
					
				if (static_cast<PPControl*>(getPatternEditorControl()) != screen->getFocusedControl())
				{
					getPatternEditorControl()->dispatchEvent(event);
				}
				break;
			}

			default:
			{
processBindings:
				pp_int32 keyModifier = ::getKeyModifier(); 
				bool res = executeBinding(eventKeyDownBindings, keyCode);

				if (res && !isActiveEditing())
					event->cancel();
					
				if (res || keyModifier)
					break;
			
				PatternEditorControl* patternEditorControl = getPatternEditorControl();

				// translate key to note
				pp_int32 note = patternEditorControl->ScanCodeToNote(scanCode);

				recorderLogic->sendNoteDownToPatternEditor(event, note, patternEditorControl);	
				break;
			}

			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_HOME:
			case VK_END:
			case VK_PRIOR:
			case VK_NEXT: {
				if (screen->getModalControl())
					break;

				if (!::getKeyModifier() ||
					::getKeyModifier() == KeyModifierALT ||
					::getKeyModifier() == (KeyModifierSHIFT | KeyModifierALT))
				{
					if (::getKeyModifier() == (KeyModifierALT))
					{
						switch (keyCode)
						{
							// Select nex/prev pattern in orderlist using Ctrl+Left/Right
							case VK_UP:
							{
								selectPreviousOrder();
								event->cancel();
								break;
							}
							case VK_DOWN:
							{
								selectNextOrder();
								event->cancel();
								break;
							}

							// Select pattern using Ctrl+SHIFT+Left/Right
							case VK_LEFT:
								moduleEditor->decreaseOrderPosition(listBoxOrderList->getSelectedIndex());
								updateOrderlist();
								event->cancel();
								break;

							case VK_RIGHT:
								moduleEditor->increaseOrderPosition(listBoxOrderList->getSelectedIndex());
								updateOrderlist();
								event->cancel();
								break;
						}
					}
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
				}
				else if (::getKeyModifier() == KeyModifierCTRL)
				{
					switch (keyCode)
					{

						// Select instrument using Ctrl+Up/Down when listbox unfocused
						case VK_UP:
						case VK_DOWN:
						case VK_NEXT:
						case VK_PRIOR:
							listBoxInstruments->dispatchEvent(event);
							event->cancel();
							break;
					}
				}
				else if (::getKeyModifier() == (KeyModifierSHIFT | KeyModifierCTRL))
				{
					switch (keyCode)
					{
						// Select sample using CTRL+Shift+Up/Down
						case VK_UP:
						case VK_DOWN:
						case VK_NEXT:
						case VK_PRIOR:
							listBoxSamples->dispatchEvent(event);
							event->cancel();
							break;

					}
				}
			}

		}
	}
	else if (event->getID() == eKeyUp)
	{
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr()); 
		pp_uint16 scanCode = *(((pp_uint16*)event->getDataPtr())+1);
		
		switch (keyCode)
		{
			case VK_SPACE:
			{
				playerLogic->finishTraceAndRowPlay();
				break;
			}
				
			default:
			{
				PatternEditorControl* patternEditorControl = getPatternEditorControl();
				
				pp_int32 note = patternEditorControl->ScanCodeToNote(scanCode);				
				
				recorderLogic->sendNoteUpToPatternEditor(event, note, patternEditorControl);	
			}
		}
		
	}
}

void Tracker::selectNextOrder(bool wrap/* = false*/)
{
	if (wrap && listBoxOrderList->isLastEntry())
	{
		setOrderListIndex(0);
		return;
	}

	pp_uint16 vk[3] = {VK_DOWN, 0, 0};
	PPEvent e(eKeyDown, &vk, sizeof(vk));
	listBoxOrderList->dispatchEvent(&e);
}

void Tracker::selectPreviousOrder(bool wrap/* = false*/)
{
	if (wrap && listBoxOrderList->isFirstEntry())
	{
		setOrderListIndex(listBoxOrderList->getNumItems()-1);
		return;
	}

	pp_uint16 vk[3] = {VK_UP, 0, 0};
	PPEvent e(eKeyDown, &vk, sizeof(vk));
	listBoxOrderList->dispatchEvent(&e);
}

void Tracker::selectNextInstrument()
{
	pp_uint16 vk[3] = {VK_DOWN, 0, 0};
	PPEvent e(eKeyDown, &vk, sizeof(vk));
	listBoxInstruments->dispatchEvent(&e);
}

void Tracker::selectPreviousInstrument()
{
	pp_uint16 vk[3] = {VK_UP, 0, 0};
	PPEvent e(eKeyDown, &vk, sizeof(vk));
	listBoxInstruments->dispatchEvent(&e);
}

///////////////////////////////////////////////////////////////////////////////
// The Fasttracker II compatibility mode is really just a big hack, because
// MilkyTracker uses focus handling on most editable controls while FT2 doesn't
// ----------------------------------------------------------------------------
// 1. a defined set of keys always are always routed to to the pattern editor
// 2. If record mode is ON all keyboard events are also routed to pattern editor 
//    (no matter if it can handle them or not)
// 3. Keys are not routed into any other control except for editing
///////////////////////////////////////////////////////////////////////////////
void Tracker::processShortcutsFastTracker(PPEvent* event)
{
	if (isActiveEditing())
		return;

	/*if (screen->getFocusedControl() != static_cast<PPControl*>(getPatternEditorControl()))
	{
		screen->setFocus(getPatternEditorControl());
		screen->paintControl(getPatternEditorControl());
	}*/

	if (event->getID() == eKeyDown)
	{
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());
		pp_uint16 scanCode = *(((pp_uint16*)event->getDataPtr())+1);
	
		switch (scanCode)
		{
			case SC_WTF:
				if (!::getKeyModifier() || ::getKeyModifier() == KeyModifierSHIFT)
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
					keyCode = 0;
				}
				break;

			// Place cursor in channel
			case SC_Q:
			case SC_W:
			case SC_E:
			case SC_R:
			case SC_T:
			case SC_Z:
			case SC_U:
			case SC_I:
			case SC_A:
			case SC_S:
			case SC_D:
			case SC_F:
			case SC_G:
			case SC_H:
			case SC_J:
			case SC_K:
				if (screen->getModalControl())
					break;
				
				if (::getKeyModifier() == KeyModifierALT)
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
					keyCode = 0;
				}
				break;

			// Increment/decrement instrument
			case SC_SS:
			case SC_TICK:
				if (screen->getModalControl())
					break;

				if (::getKeyModifier() == KeyModifierCTRL ||
					::getKeyModifier() == (KeyModifierSHIFT|KeyModifierCTRL))
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
					keyCode = 0;
				}
				break;
		}
	
		switch (keyCode)
		{
			case VK_SPACE:
			{
				if (screen->getModalControl())
					break;
					
				if (::getKeyModifier())
					goto processOthers;
			
				if (playerController->isPlaying() || playerController->isPlayingPattern())
				{
					playerLogic->stopSong();
					event->cancel();
					break;
				}
				
				playerLogic->stopSong();

				eventKeyDownBinding_ToggleFT2Edit();

				event->cancel();
				break;
			}

			// Those are the key combinations which are always routed to pattern editor control as long
			// as we're in Fasttracker editing mode
			case VK_ALT:
			case VK_SHIFT:
			case VK_CONTROL:
				if (screen->getModalControl())
					break;

				getPatternEditorControl()->dispatchEvent(event);
				event->cancel();
				break;
			
			// Transpose (regardless of modifers)
			case VK_F1:
			case VK_F2:
			case VK_F7:
			case VK_F8:
			case VK_F9:
			case VK_F10:
			case VK_F11:
			case VK_F12:
				processShortcutsMilkyTracker(event);
				break;

			// Cut copy paste
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
				// Global meaning here
				if (::getKeyModifier())
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
					break;
				}
				processShortcutsMilkyTracker(event);
				break;
				
			// Some special keys always going to the pattern editor (like undo, redo, mute etc.)
			case 'A':
			case 'C':
			case 'V':
			case 'X':
			case 'Z':
			case 'Y':
				if (screen->getModalControl())
				{
					// those seem to be piano keys, they're used in some
					// modal dialogs for instrument preview playback
					if (!::getKeyModifier())
						goto processOthers;
						
					break;
				}
				
				if (::getKeyModifier() == (KeyModifierCTRL|KeyModifierALT))
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
				}
				else goto processOthers;
				break;

			case 'I':
				if (screen->getModalControl())
					break;
				
				if (::getKeyModifier() == KeyModifierSHIFT)
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
				}
				else goto processOthers;
				break;

			case 'M':
				if (screen->getModalControl())
					break;
				
				if (::getKeyModifier() == KeyModifierSHIFT ||
					::getKeyModifier() == (KeyModifierSHIFT|KeyModifierCTRL))
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
				}
				else goto processOthers;
				break;

			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_HOME:
			case VK_END:
			case VK_PRIOR:
			case VK_NEXT:
				if (screen->getModalControl())
					break;

				if (!::getKeyModifier() ||
					::getKeyModifier() == KeyModifierALT ||
					::getKeyModifier() == (KeyModifierSHIFT|KeyModifierALT))
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
				}
				else if (::getKeyModifier() == KeyModifierSHIFT)
				{
					switch (keyCode)
					{
						// Select instrument using Shift+Up/Down
						case VK_UP:
						case VK_DOWN:
						case VK_NEXT:
						case VK_PRIOR:
							listBoxInstruments->dispatchEvent(event);
							event->cancel();
							break;
						
						// Select new order using Shift+Left/Right
						case VK_LEFT:
						{
							selectPreviousOrder();
							event->cancel();
							break;
						}
						case VK_RIGHT:
						{
							selectNextOrder();
							event->cancel();
							break;
						}
					}
				}
				else if (::getKeyModifier() == (KeyModifierSHIFT|KeyModifierCTRL))
				{
					switch (keyCode)
					{
						// Select sample using Shift+Alt+Up/Down
						case VK_UP:
						case VK_DOWN:
						case VK_NEXT:
						case VK_PRIOR:
							listBoxSamples->dispatchEvent(event);
							event->cancel();
							break;
					}
				}
				else if (::getKeyModifier() == KeyModifierCTRL)
				{
					switch (keyCode)
					{
						// Select pattern using Ctrl+Left/Right
						case VK_LEFT:
							eventKeyDownBinding_PreviousPattern();
							event->cancel();
							break;
							
						case VK_RIGHT:
							eventKeyDownBinding_NextPattern();
							event->cancel();
							break;
					}
				}
				goto processOthers;
				break;

			case VK_TAB:
				if (screen->getModalControl())
					break;

				getPatternEditorControl()->dispatchEvent(event);
				event->cancel();
				break;

			default:
processOthers:
				processShortcutsMilkyTracker(event);

				if (screen->getModalControl())
					break;

				if (recorderLogic->getRecordMode())
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
				}
				// if recordMode is false and focus is on pattern editor
				// we need to cancel the event in order to prevent it
				// from going into the pattern editor
				else if (screen->getFocusedControl() == static_cast<PPControl*>(getPatternEditorControl()))
				{
					event->cancel();
				}
		}
	}
	else if (event->getID() == eKeyChar)
	{
		if (recorderLogic->getRecordMode())
		{
			getPatternEditorControl()->dispatchEvent(event);
			event->cancel();
		}
		// if recordMode is false and focus is on pattern editor
		// we need to cancel the event in order to prevent it
		// from going into the pattern editor
		else if (screen->getFocusedControl() == static_cast<PPControl*>(getPatternEditorControl()))
		{
			event->cancel();
		}
	}
	else if (event->getID() == eKeyUp)
	{
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());
		//pp_uint16 scanCode = *(((pp_uint16*)event->getDataPtr())+1);
	
		switch (keyCode)
		{
			// Those are the keykombinations which are always routed to pattern editor control as long
			// as we're in Fasttracker editing mode
			case VK_ALT:
			case VK_SHIFT:
			case VK_CONTROL:
				if (screen->getModalControl())
					break;

				getPatternEditorControl()->dispatchEvent(event);
				event->cancel();
				break;

			default:
				processShortcutsMilkyTracker(event);

				if (screen->getModalControl())
					/*break;*/return;

				if (recorderLogic->getRecordMode())
				{
					getPatternEditorControl()->dispatchEvent(event);
					event->cancel();
				}
				// if recordMode is false and focus is on pattern editor
				// we need to cancel the event in order to prevent it
				// from going into the pattern editor
				else if (screen->getFocusedControl() == static_cast<PPControl*>(getPatternEditorControl()))
				{
					event->cancel();
				}
		}
	}
}


void Tracker::switchEditMode(EditModes mode)
{
	bool b = (mode == EditModeMilkyTracker);

	PPContainer* container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_MENU));
	ASSERT(container);
	
	// Assign keyboard bindings
	getPatternEditorControl()->setShowFocus(b);
	listBoxInstruments->setShowFocus(b);
	listBoxSamples->setShowFocus(b);
	listBoxOrderList->setShowFocus(b);
	sectionDiskMenu->setFileBrowserShowFocus(b);
	sectionDiskMenu->setCycleFilenames(b);
	container = static_cast<PPContainer*>(screen->getControlByID(CONTAINER_ABOUT));
	ASSERT(container);
	static_cast<PPListBox*>(container->getControlByID(LISTBOX_SONGTITLE))->setShowFocus(b);
	
	if (b)
	{
		eventKeyDownBindings = eventKeyDownBindingsMilkyTracker;	
		screen->setFocus(listBoxInstruments, false);
	}
	else
	{
		eventKeyDownBindings = eventKeyDownBindingsFastTracker;	
		recorderLogic->setRecordMode(true);
		eventKeyDownBinding_ToggleFT2Edit();
	}
	
	getPatternEditorControl()->switchEditMode(mode);

	editMode = mode;
}

// Process messagebox shortcuts (RETURN & ESC)
// the modal dialogs only appear to be modal, we're still getting
// keyboard events here in case a modal dialog box appears
// this is the handler which allows for esc + return handling in case
// of a modal dialog
static void simulateMouseClickEvent(PPControl* ctrl)
{
	PPPoint p = ctrl->getLocation();
	p.x+=ctrl->getSize().width >> 1;
	p.y+=ctrl->getSize().height >> 1;
	
	PPEvent e1(eLMouseDown, &p, sizeof(PPPoint));
	PPEvent e2(eLMouseUp, &p, sizeof(PPPoint));
	
	ctrl->dispatchEvent(&e1);
	ctrl->dispatchEvent(&e2);
}

bool Tracker::processMessageBoxShortcuts(PPEvent* event)
{
	PPControl* ctrl = screen->getModalControl();

	if (ctrl == NULL || !ctrl->isContainer() || (event->getID() != eKeyDown
			&& event->getID() != eKeyChar))
		return false;

	PPSimpleVector<PPControl>& controls = static_cast<PPContainer*>(ctrl)->getControls();

	pp_int32 i;
	// if dialog contains list (list can also be an edit field btw.)
	// and something is being edited in that list we don't simulate
	// yes/no/cancel button presses
	for (i = 0; i < controls.size(); i++)
	{
		PPControl* ctrl = controls.get(i);
		if (ctrl->isListBox() && static_cast<PPListBox*>(ctrl)->isEditing())
			return true;
	}

	// iterate over controls in dialog and see whether we can find
	// yes/no/cancel buttons
	// if that's the case we simulate mouse button press
	if (event->getID() == eKeyDown)
	{
		pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());

		for (i = 0; i < controls.size(); i++)
		{
			PPControl* ctrl = controls.get(i);
			switch (ctrl->getID())
			{
				case PP_MESSAGEBOX_BUTTON_YES:
					if (keyCode == VK_RETURN)
					{
						simulateMouseClickEvent(ctrl);
						return true;
					}
					break;

				case PP_MESSAGEBOX_BUTTON_CANCEL:
				case PP_MESSAGEBOX_BUTTON_NO:
					if (keyCode == VK_ESCAPE)
					{
						simulateMouseClickEvent(ctrl);
						return true;
					}
					break;
			}
		}
	}

	return false;
}
