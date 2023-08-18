/*
 *  tracker/PatternEditorControlEventListener.cpp
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
 *  PatternEditorControlEventListener.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Fri Mar 11 2005.
 *
 */

#include "PatternEditorControl.h"
#include "Event.h"
#include "Screen.h"
#include "Font.h"
#include "ScrollBar.h"
#include "Menu.h"
#include "XModule.h"
#include "ContextMenu.h"
#include "Undo.h"

#define SCROLLBARWIDTH  SCROLLBUTTONSIZE

pp_int32 PatternEditorControl::dispatchEvent(PPEvent* event)
{ 
	if (pattern == NULL)
		return -1;

	if (eventListener == NULL)
		return -1;

	switch (event->getID())
	{
		case eFocusGained:
		case eFocusGainedNoRepaint:
		{	
			hasFocus = true;
			if (menuInvokeChannel != -1)
			{
				lastMenuInvokeChannel = menuInvokeChannel;
				menuInvokeChannel = -1;
				//parentScreen->paintControl(this);			
			}
			
			if (event->getID() == eFocusGained)
				parentScreen->paintControl(this);			
			
			//resetKeyModifier();
			goto leave;
		}

		case eFocusLost:
		{	
			hasFocus = false;	
			parentScreen->paintControl(this);			
			//resetKeyModifier();
			goto leave;
		}

		case eFocusLostNoRepaint:
		{	
			hasFocus = false;
			//resetKeyModifier();
			goto leave;
		}

		case eMouseWheelMoved:
		{
			TMouseWheelEventParams* params = (TMouseWheelEventParams*)event->getDataPtr();
			
			// Vertical scrolling takes priority over horizontal scrolling and is mutually
			// exclusive from horizontal scrolling.
			if (params->deltaY)
			{
				pp_int32 dy = params->deltaY;
				if(properties.invertMouseVscroll) dy = 0 - dy;
				PPEvent e = dy < 0 ? PPEvent(eBarScrollDown, -dy) : PPEvent(eBarScrollUp, dy);
				handleEvent(reinterpret_cast<PPObject*>(vLeftScrollbar), &e);
			}
			
			else if (params->deltaX)
			{
				PPEvent e = params->deltaX < 0 ? PPEvent(eBarScrollDown, -params->deltaX) : PPEvent(eBarScrollUp, params->deltaX);
				handleEvent(reinterpret_cast<PPObject*>(hBottomScrollbar), &e);
			}
			
			event->cancel();
			
			break;
		}

		case eRMouseDown:
		{			
			PPPoint* p = (PPPoint*)event->getDataPtr();

			if (hTopScrollbar->hit(*p))
			{
				if (controlCaughtByLMouseButton)
					break;
				controlCaughtByRMouseButton = true;
				caughtControl = hTopScrollbar;
				caughtControl->dispatchEvent(event);
			}
			else if (hBottomScrollbar->hit(*p))
			{
				if (controlCaughtByLMouseButton)
					break;
				controlCaughtByRMouseButton = true;
				caughtControl = hBottomScrollbar;
				caughtControl->dispatchEvent(event);
			}
			// Clicked on vertical scrollbar -> route event to scrollbar and catch scrollbar control
			else if (vLeftScrollbar->hit(*p))
			{
				if (controlCaughtByLMouseButton)
					break;
				controlCaughtByRMouseButton = true;
				caughtControl = vLeftScrollbar;
				caughtControl->dispatchEvent(event);				
			}
			else if (vRightScrollbar->hit(*p))
			{
				if (controlCaughtByLMouseButton)
					break;
				controlCaughtByRMouseButton = true;
				caughtControl = vRightScrollbar;
				caughtControl->dispatchEvent(event);				
			}
			else 
			{
				RMouseDownInChannelHeading = -1;
				lastAction = RMouseDownActionFirstRClick;

				PPPoint cp = *p;
				
				cp.x-=location.x + SCROLLBARWIDTH + getRowCountWidth() + 4;
				cp.y-=location.y + SCROLLBARWIDTH + font->getCharHeight() + 4;
				
				if (cp.x < 0)
					break;

				// right click on channel number
				if ((RMouseDownInChannelHeading = pointInChannelHeading(cp)) != -1)
				{
					goto leave;
				}
				else if (cp.y < 0) break;

				pp_int32 i = (cp.x / slotSize) + startPos;
				if (i < 0 || i > patternEditor->getNumChannels()-1)
					break;

				invokeMenu(i, *p);
			}
			
			goto leave;
		}

		case eRMouseUp:
		{
			controlCaughtByRMouseButton = false;
			if (caughtControl && !controlCaughtByLMouseButton && !controlCaughtByRMouseButton)
			{
				caughtControl->dispatchEvent(event);
				caughtControl = NULL;			
				break;
			}

			PPPoint cp = *((PPPoint*)event->getDataPtr());
			
			cp.x-=location.x + SCROLLBARWIDTH + getRowCountWidth() + 4;
			cp.y-=location.y + SCROLLBARWIDTH + font->getCharHeight() + 4;
			
			if (cp.x < 0 || RMouseDownInChannelHeading == -1)
				break;
			
			// right click on channel number
			if ((menuInvokeChannel = pointInChannelHeading(cp)) != -1 && 
				menuInvokeChannel == RMouseDownInChannelHeading &&
				lastAction == RMouseDownActionFirstRClick)
			{
				RMouseDownInChannelHeading = -1;
				parentScreen->paintControl(this);
				executeMenuCommand(MenuCommandIDMuteChannel);
				menuInvokeChannel = lastMenuInvokeChannel = -1;
				parentScreen->paintControl(this);
				goto leave;
			}
			RMouseDownInChannelHeading = -1;
			menuInvokeChannel = -1;
			parentScreen->paintControl(this);

			break;
		}

		case eLMouseDown:
		{
			hasDragged = false;

			PPPoint* p = (PPPoint*)event->getDataPtr();

			// Clicked on horizontal scrollbar -> route event to scrollbar and catch scrollbar control
			if (hTopScrollbar->hit(*p))
			{
				if (controlCaughtByRMouseButton)
					break;
				controlCaughtByLMouseButton = true;
				caughtControl = hTopScrollbar;
				caughtControl->dispatchEvent(event);
			}
			else if (hBottomScrollbar->hit(*p))
			{
				if (controlCaughtByRMouseButton)
					break;
				controlCaughtByLMouseButton = true;
				caughtControl = hBottomScrollbar;
				caughtControl->dispatchEvent(event);
			}
			// Clicked on vertical scrollbar -> route event to scrollbar and catch scrollbar control
			else if (vLeftScrollbar->hit(*p))
			{
				if (controlCaughtByRMouseButton)
					break;
				controlCaughtByLMouseButton = true;
				caughtControl = vLeftScrollbar;
				caughtControl->dispatchEvent(event);				
			}
			else if (vRightScrollbar->hit(*p))
			{
				if (controlCaughtByRMouseButton)
					break;
				controlCaughtByLMouseButton = true;
				caughtControl = vRightScrollbar;
				caughtControl->dispatchEvent(event);				
			}
			// Clicked in client area
			else
			{
				PPPoint cp = *p;
				
				cp.x-=location.x + SCROLLBARWIDTH + getRowCountWidth() + 4;
				cp.y-=location.y + SCROLLBARWIDTH + font->getCharHeight() + 4;
				
				if (cp.y <  -((pp_int32)font->getCharHeight() + 6))
					break;

				if (cp.x < -(getRowCountWidth() + 4))
					break;
				
				// select new row by clicking on row number
				if (cp.x < -3)
				{			
					pp_int32 newStartIndex = cp.y / font->getCharHeight();					
					pp_int32 newStartPos = cp.x / slotSize;
					
					pp_int32 visibleRows = (visibleHeight) / font->getCharHeight();
					pp_int32 visibleChannels = (visibleWidth) / slotSize;
					
					// copy of current selection
					patternEditor->getSelection().backup();
					
					preCursor = patternEditor->getCursor();
					
					if (newStartIndex < visibleRows && newStartIndex >= 0)
					{
						if (newStartIndex + startIndex < 0)
							break;
						
						preCursor.row = newStartIndex + startIndex;
					}
					
					ppreCursor = &preCursor;
					break;
				}
				else if (cp.x < 0)
				{
					break;
				}

				pp_int32 i;
				if ((i = pointInChannelHeading(cp)) != -1)
				{
				
					// Special commands
					// Right button is pressed and left button is pressed
					if (::getKeyModifier() & KeyModifierALT)
					{
						RMouseDownInChannelHeading = i;
						lastAction = RMouseDownActionSecondLClick;
					}
					
					if (RMouseDownInChannelHeading == i)
					{
						if (lastAction == RMouseDownActionFirstRClick ||
							lastAction == RMouseDownActionSecondLClick)
						{
							if (isSoloChannel(i))
								goto unmuteAll;
							menuInvokeChannel = i;
							executeMenuCommand(MenuCommandIDSoloChannel);
							parentScreen->paintControl(this);
							lastAction = RMouseDownActionFirstLClick;
							hasDragged = true;
							goto leave;
						}
						else if (lastAction == RMouseDownActionFirstLClick)
						{
unmuteAll:
							menuInvokeChannel = i;
							executeMenuCommand(MenuCommandIDUnmuteAll);
							parentScreen->paintControl(this);
							lastAction = RMouseDownActionSecondLClick;
							hasDragged = true;
							goto leave;
						}
					}
				
					if (i == lastMenuInvokeChannel)
					{
						lastMenuInvokeChannel = -1;
						goto leave;
					}
				
					// show menu
					pp_int32 menuPosY = location.y + SCROLLBARWIDTH + 1 + font->getCharHeight();
					pp_int32 menuPosX = location.x + SCROLLBARWIDTH + getRowCountWidth() + 4 + slotSize * (i - startPos);
				
					PPPoint p2(menuPosX, menuPosY);

					invokeMenu(i, p2);
					goto leave;					
				}
				else if (cp.y < 0) break;

				pp_int32 newStartIndex = cp.y / font->getCharHeight();

				pp_int32 newStartPos = cp.x / slotSize;
				
				pp_int32 visibleRows = (visibleHeight) / font->getCharHeight();
				pp_int32 visibleChannels = (visibleWidth) / slotSize;
				
				// backup selection, so that it may be restored when context menu is activated by long-press
				patternEditor->getSelection().backup();
				
				if (!properties.advancedDnd) {
					// If we're pressing the shift key start selection
					// at current cursor position
					if (::getKeyModifier() & selectionKeyModifier)
					{
						if (patternEditor->getSelection().start.isValid())
							patternEditor->getSelection().end = patternEditor->getCursor();
						else
							patternEditor->getSelection().start = patternEditor->getCursor();
					}
				}
				
				preCursor = patternEditor->getCursor();

				if (newStartIndex < visibleRows && newStartIndex >= 0)
				{
					if (newStartIndex + startIndex < 0)
					{
						if (properties.advancedDnd) {
							patternEditor->resetSelection();
							preCursor.row = 0;
						} else {
							break;
						}
					}
					else if (properties.advancedDnd && newStartIndex + startIndex >= patternEditor->getNumRows())
					{
						preCursor.row = patternEditor->getNumRows() - 1;
					}
					else
					{
						preCursor.row = newStartIndex + startIndex;
					}
				}

				if (newStartPos < visibleHeight && newStartPos >= 0)
				{
					selectionTicker = 0;
					startSelection = true;

					if (newStartPos + startPos < 0)
						break;

					preCursor.channel = newStartPos + startPos;
					if (properties.advancedDnd) {
						preCursor.inner = 0;
					}
				
					if (preCursor.channel >= patternEditor->getNumChannels())
					{
						if (properties.advancedDnd) {
							// clicked beyond rightmost channel, start selection from the edge
							patternEditor->resetSelection();
							preCursor.channel = patternEditor->getNumChannels() - 1;
							preCursor.inner = 7;
						} else {
							break;
						}
					}
					else if (properties.advancedDnd)
					{
						// find which column in the channel was clicked
						pp_int32 innerPos = cp.x % slotSize;
						for (pp_uint32 i = 0; i < sizeof(cursorPositions) - 1; i++)
						{
							if (innerPos >= cursorPositions[i] &&
								innerPos < cursorPositions[i+1])
							{
								preCursor.inner = i;
								break;
							}
						}
					}
					
					if (properties.advancedDnd) {
						if (patternEditor->selectionContains(preCursor))
						{
							startSelection = false;
							moveSelection = true;
							moveSelectionInitialPos = preCursor;
							moveSelectionFinalPos = preCursor;
						}
						else
						{
							if (!(::getKeyModifier() & selectionKeyModifier))
							{
								// start selection from mouse cursor position
								patternEditor->getSelection().start = preCursor;
								patternEditor->getSelection().end = preCursor;
							}
							else
							{
								// resume selection from mouse cursor position
								if (!patternEditor->getSelection().start.isValid())
									patternEditor->getSelection().start = patternEditor->getCursor();
								patternEditor->getSelection().end = preCursor;
							}
						}
					} else {
						// start selecting row
						if (!(::getKeyModifier() & selectionKeyModifier))
						{
							patternEditor->getSelection().start.channel = patternEditor->getSelection().end.channel = preCursor.channel;
							patternEditor->getSelection().start.row = patternEditor->getSelection().end.row = preCursor.row;
						}
						else
						{
							patternEditor->getSelection().end.channel = preCursor.channel;
							patternEditor->getSelection().end.row = preCursor.row;
						}
						
						pp_int32 innerPos = cp.x % slotSize;
						
						preCursor.inner = 0;
						if (!(::getKeyModifier() & selectionKeyModifier))
							patternEditor->getSelection().start.inner = 0;
						patternEditor->getSelection().end.inner = 0;
						for (pp_uint32 i = 0; i < sizeof(cursorPositions) - 1; i++)
						{
							if (innerPos >= cursorPositions[i] &&
								innerPos < cursorPositions[i+1])
							{
								preCursor.inner = i;
								if (!(::getKeyModifier() & selectionKeyModifier))
									patternEditor->getSelection().start.inner = i;
								patternEditor->getSelection().end.inner = i;
								break;
							}
						}
					}

				}

				ppreCursor = &preCursor;

				//parentScreen->paintControl(this);
			}

			break;
		}

		case eLMouseUp:
			controlCaughtByLMouseButton = false;
			if (caughtControl && !controlCaughtByLMouseButton && !controlCaughtByRMouseButton)
			{
				caughtControl->dispatchEvent(event);
				caughtControl = NULL;			
				break;
			}
			
			menuInvokeChannel = -1;
			
			if (properties.advancedDnd && moveSelection && moveSelectionFinalPos != moveSelectionInitialPos)
			{
				pp_int32 moveSelectionRows = moveSelectionFinalPos.row - moveSelectionInitialPos.row;
				pp_int32 moveSelectionChannels = moveSelectionFinalPos.channel - moveSelectionInitialPos.channel;
				
				if (patternEditor->canMoveSelection(moveSelectionChannels, moveSelectionRows))
				{
					if (::getKeyModifier() & selectionKeyModifier)
						patternEditor->cloneSelection(moveSelectionChannels, moveSelectionRows);
					else
						patternEditor->moveSelection(moveSelectionChannels, moveSelectionRows);
				}
			}
			else if (!hasDragged && !(::getKeyModifier() & selectionKeyModifier) && ppreCursor)
			{
				if (properties.clickToCursor)
				{
					patternEditor->setCursor(*ppreCursor);
			
					notifyUpdate(AdvanceCodeSelectNewRow);
					
					switch (properties.scrollMode)
					{
						case ScrollModeToCenter:
						case ScrollModeStayInCenter:
							assureCursorVisible();
							break;
						case ScrollModeToEnd:
							break;
					}
				}
				
				patternEditor->resetSelection();
			}
			

			startSelection = false;
			moveSelection = false;

			parentScreen->paintControl(this);

			ppreCursor = NULL;

			break;

		case eLMouseDrag:
		{
			if (caughtControl && controlCaughtByLMouseButton)
			{
				caughtControl->dispatchEvent(event);
				break;
			}
			
			if (!moveSelection && !startSelection)
				break;
			
			hasDragged = true;
			goto markOrMoveSelection;
			//break;
		}

		case eRMouseDrag:
		{
			if (caughtControl && controlCaughtByRMouseButton)
				caughtControl->dispatchEvent(event);

			break;
		}
		
		case eRMouseRepeat:
		{
			if (caughtControl && controlCaughtByRMouseButton)
				caughtControl->dispatchEvent(event);

			break;
		}

		case eLMouseRepeat:
		{
			if (caughtControl && controlCaughtByLMouseButton)
			{
				caughtControl->dispatchEvent(event);
				break;
			}		

			// for slowing down mouse pressed events
			selectionTicker++;

			if (selectionTicker&1)
				break;
			
			// no selection has been made or mouse hasn't been dragged
			if (!startSelection || !hasDragged)
			{
				// mouse hasn't been dragged and selection ticker has reached threshold value
				if (!hasDragged && selectionTicker > 0)
				{
					// get point
					PPPoint* p = (PPPoint*)event->getDataPtr();
					
					// make sure scrollbars weren't pressed
					if (!hTopScrollbar->hit(*p) &&
						!hBottomScrollbar->hit(*p) &&
						!vLeftScrollbar->hit(*p) &&
						!vRightScrollbar->hit(*p))
					{
						// translate into local coordinate system
						PPPoint cp = *p;
						
						cp.x-=location.x + SCROLLBARWIDTH + getRowCountWidth() + 4;
						cp.y-=location.y + SCROLLBARWIDTH + font->getCharHeight() + 4;
						
						// not in our local CS
						if (cp.x < 0 || cp.y < 0)
							break;
						
						// valid channel?
						pp_int32 i = (cp.x / slotSize) + startPos;
						if (i < 0 || i > patternEditor->getNumChannels()-1)
							break;

						// restore last selection
						// will be made when mouse left mouse button is pressed
						if (patternEditor->getSelection().isCopyValid())
						{
							patternEditor->getSelection().restore();
						}
						
						// No more cursor positioning
						ppreCursor = NULL;
						
						invokeMenu(i, *p);
						// we're finished with event handling here
						goto leave;
					}
				}
				break;
			}

markOrMoveSelection:
			PPPoint cp = *((PPPoint*)event->getDataPtr());

			PPPoint cp2 = cp;

			cp.x-=location.x + SCROLLBARWIDTH + getRowCountWidth() + 4;
			cp.y-=location.y + SCROLLBARWIDTH + font->getCharHeight() + 4;
			
			cp2.x-=location.x;
			cp2.y-=location.y;

			if (cp2.x > size.width - SCROLLBARWIDTH /*- (slotSize>>1)*/)
			{
				if (startPos + (visibleWidth / slotSize) < patternEditor->getNumChannels())
					startPos++;
			}
			else if (cp2.x < SCROLLBARWIDTH + (signed)getRowCountWidth() + 4)
			{
				if (startPos > 0)
					startPos--;
			}
	
			if (cp2.y > size.height - SCROLLBARWIDTH /*- ((signed)font->getCharHeight()*3)*/)
			{
				if (properties.scrollMode != ScrollModeStayInCenter)
				{
					if (startIndex + (visibleHeight / font->getCharHeight()) < pattern->rows)
						startIndex++;
				}
				else
				{
					scrollCursorDown();
					assureCursorVisible(true, false);
				}
			}
			else if (cp2.y < SCROLLBARWIDTH + ((signed)font->getCharHeight()*4 /*+ 4*/))
			{
				if (properties.scrollMode != ScrollModeStayInCenter)
				{
					if (startIndex > 0)
						startIndex--;
				}
				else
				{
					scrollCursorUp();
					assureCursorVisible(true, false);
				}
			}

			if (cp.x < 0)
			{
				cp.x = 0;
				//goto leave;
			}

			if (cp.y < 0)
			{
				cp.y = 0;
				//goto leave;
			}

			pp_int32 newStartIndex = cp.y / font->getCharHeight();
			
			pp_int32 newStartPos = cp.x / slotSize;
			
			if (properties.advancedDnd) {
				pp_int32 visibleRows = (visibleHeight) / font->getCharHeight();
				pp_int32 visibleChannels = (visibleWidth) / slotSize;
				
				mp_sint32 cursorPositionRow = newStartIndex + startIndex;
				mp_sint32 cursorPositionChannel = newStartPos + startPos;
				mp_sint32 cursorPositionInner;
				
				if (moveSelection)
				{
					moveSelectionFinalPos.channel = cursorPositionChannel;
					moveSelectionFinalPos.row = cursorPositionRow;
				}
				else
				{
					if (cursorPositionRow < 0)
						cursorPositionRow = 0;
					else if (cursorPositionRow >= patternEditor->getNumRows())
						cursorPositionRow = patternEditor->getNumRows()-1;
					
					if (cursorPositionChannel < 0)
					{
						cursorPositionChannel = 0;
						cursorPositionInner = 0;
					}
					else if (cursorPositionChannel >= patternEditor->getNumChannels())
					{
						cursorPositionChannel = patternEditor->getNumChannels()-1;
						cursorPositionInner = 7;
					}
					else
					{
						pp_int32 innerPos = cp.x % slotSize;
						
						for (pp_uint32 i = 0; i < sizeof(cursorPositions) - 1; i++)
						{
							if (innerPos >= cursorPositions[i] &&
								innerPos < cursorPositions[i+1])
							{
								cursorPositionInner = i;
								break;
							}
						}
					}
					
					patternEditor->getSelection().end.row = cursorPositionRow;
					patternEditor->getSelection().end.channel = cursorPositionChannel;
					patternEditor->getSelection().end.inner = cursorPositionInner;
					
					setScrollbarPositions(startIndex, startPos);
				}
			} else {
				mp_sint32 cursorPositionRow = newStartIndex + startIndex;
				mp_sint32 cursorPositionChannel = newStartPos + startPos;
				
				if (cursorPositionRow < 0) cursorPositionRow = 0;
				if (cursorPositionChannel < 0) cursorPositionChannel = 0;
				
				if (cursorPositionChannel >= patternEditor->getNumChannels())
				{
					patternEditor->getSelection().end.channel = patternEditor->getNumChannels()-1;
					patternEditor->getSelection().end.inner = 7;
				}
				else
				{
					// start selecting row
					patternEditor->getSelection().end.channel = cursorPositionChannel;
					patternEditor->getSelection().end.row = cursorPositionRow;
					
					pp_int32 innerPos = cp.x % slotSize;
					
					for (pp_uint32 i = 0; i < sizeof(cursorPositions) - 1; i++)
					{
						if (innerPos >= cursorPositions[i] &&
							innerPos < cursorPositions[i+1])
						{
							patternEditor->getSelection().end.inner = i;
							break;
						}
					}
				}
				
				setScrollbarPositions(startIndex, startPos);
				
			}
			
			parentScreen->paintControl(this);

			break;
		}

		case eKeyDown:
		{	
			//assureCursorVisible();
			
			pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());
			pp_uint16 scanCode = *(((pp_uint16*)event->getDataPtr())+1);
			pp_uint16 character = *(((pp_uint16*)event->getDataPtr())+2);

			assureCursor = true;
			assureUpdate = false;

			bool keyboadStartSelectionFlipped = false;
			// start selection
			if (keyCode == VK_LEFT ||
				keyCode == VK_RIGHT ||
				keyCode == VK_UP ||
				keyCode == VK_DOWN ||
				keyCode == VK_PRIOR ||
				keyCode == VK_NEXT ||
				keyCode == VK_HOME ||
				keyCode == VK_END ||
				keyCode == VK_TAB)
			{
				if ((::getKeyModifier() == (unsigned)selectionKeyModifier) && keyboardStartSelection)
				{
					patternEditor->getSelection().start = patternEditor->getSelection().end = patternEditor->getCursor();
					keyboardStartSelection = false;
					if (keyCode == VK_LEFT ||
						keyCode == VK_RIGHT ||
						keyCode == VK_UP ||
						keyCode == VK_DOWN)
					{
						keyboadStartSelectionFlipped = true;
					}
				}
			}

			switch (keyCode)
			{
				// store key modifiers
				case VK_ALT:
					assureCursor = false;
					if (selectionKeyModifier & KeyModifierALT)
						selectionModifierKeyDown();
					break;
				case VK_SHIFT:
					assureCursor = false;
					if (selectionKeyModifier & KeyModifierSHIFT)
						selectionModifierKeyDown();
					break;
				case VK_CONTROL:
					assureCursor = false;
					if (selectionKeyModifier & KeyModifierCTRL)
						selectionModifierKeyDown();
					break;
			
				default:
				{
					bool res = executeBinding(scanCodeBindings, scanCode);

					if (!res)
						res = executeBinding(eventKeyDownBindings, keyCode);
					
					if (!res)
						handleKeyDown(keyCode, scanCode, character);
					
					break;
				}
			}
			
			// normal selection
			if (keyCode == VK_LEFT ||
				keyCode == VK_RIGHT ||
				keyCode == VK_UP ||
				keyCode == VK_DOWN ||
				keyCode == VK_PRIOR ||
				keyCode == VK_NEXT ||
				keyCode == VK_HOME ||
				keyCode == VK_END ||
				keyCode == VK_TAB)
			{
				if ((::getKeyModifier() == (unsigned)selectionKeyModifier) && 
					!keyboardStartSelection && 
					!keyboadStartSelectionFlipped)
				{
					patternEditor->getSelection().end = patternEditor->getCursor();
				}
				else if (::getKeyModifier() == (KeyModifierALT|KeyModifierSHIFT) &&
						 patternEditor->getSelection().start.isValid())
				{
					patternEditor->getSelection().end = patternEditor->getCursor();
				}
			}						
												
			if (assureCursor)
			{
				assureCursorVisible();
				parentScreen->paintControl(this);
			}
			else if (assureUpdate)
			{
				parentScreen->paintControl(this);
			}

			break;
		}

		case eKeyChar:
		{
			assureUpdate = false;

			pp_uint8 character = *((pp_uint8*)event->getDataPtr());

			handleKeyChar(character);

			if (assureUpdate)
				parentScreen->paintControl(this);

			break;
		}

		case eKeyUp:
		{	
			pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());

			switch (keyCode)
			{
				case VK_SHIFT:
					if (selectionKeyModifier & KeyModifierSHIFT)
						selectionModifierKeyUp();
					break;
				case VK_ALT:
					if (selectionKeyModifier & KeyModifierALT)
						selectionModifierKeyUp();
					break;
				case VK_CONTROL:
					if (selectionKeyModifier & KeyModifierCTRL)
						selectionModifierKeyUp();
					break;
			}
			break;
		}

		default:
			if (caughtControl == NULL)
				break;

			caughtControl->dispatchEvent(event);
			goto leave;
		
	}

leave:
	return 0;
}

void PatternEditorControl::selectionModifierKeyDown()
{
	keyboardStartSelection = true;
	if (moveSelection)
		parentScreen->paintControl(this);
}

void PatternEditorControl::selectionModifierKeyUp()
{
	keyboardStartSelection = false;
	if (moveSelection)
		parentScreen->paintControl(this);
}


pp_int32 PatternEditorControl::handleEvent(PPObject* sender, PPEvent* event)
{
	PatternEditorTools::Position& cursor = patternEditor->getCursor();
	// Vertical scrollbar, scroll down
	if ((sender == reinterpret_cast<PPObject*>(vLeftScrollbar) || 
		sender == reinterpret_cast<PPObject*>(vRightScrollbar)) &&
		event->getID() == eBarScrollDown)
	{
		pp_int32 scrollAmount = event->getMetaData();
		if (properties.scrollMode != ScrollModeStayInCenter)
		{
			pp_int32 visibleItems = (visibleHeight) / font->getCharHeight();
			startIndex += scrollAmount;
			if (startIndex + visibleItems > pattern->rows)
				startIndex = pattern->rows - visibleItems;

			float v = (float)(pattern->rows - visibleItems);

			vLeftScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
			vRightScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
		}
		else
		{
			cursor.row += scrollAmount;
			if (cursor.row >= pattern->rows)
				cursor.row = pattern->rows - 1;
			assureCursorVisible(true, false);
		}
	}
	// Vertical scrollbar, scroll up
	else if ((sender == reinterpret_cast<PPObject*>(vLeftScrollbar) ||
			 sender == reinterpret_cast<PPObject*>(vRightScrollbar)) &&
			 event->getID() == eBarScrollUp)
	{
		pp_int32 scrollAmount = event->getMetaData();
		if (properties.scrollMode != ScrollModeStayInCenter)
		{
			startIndex -= scrollAmount;
			if (startIndex < 0)
				startIndex = 0;
		
			pp_int32 visibleItems = (visibleHeight) / font->getCharHeight();

			float v = (float)(pattern->rows - visibleItems);

			vLeftScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
			vRightScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
		}
		else
		{
			cursor.row -= scrollAmount;
			if (cursor.row < 0)
				cursor.row = 0;
			assureCursorVisible(true, false);
		}
	}
	// Vertical scrollbar, position changed
	else if ((sender == reinterpret_cast<PPObject*>(vLeftScrollbar) ||
			 sender == reinterpret_cast<PPObject*>(vRightScrollbar)) &&
			 event->getID() == eBarPosChanged)
	{

		float pos = reinterpret_cast<PPScrollbar*>(sender)->getBarPosition()/65536.0f;

		switch (properties.scrollMode)
		{
			case ScrollModeToEnd:
			case ScrollModeToCenter:
			{
				pp_int32 visibleItems = (visibleHeight) / font->getCharHeight();		
				float v = (float)(pattern->rows - visibleItems);
				startIndex = (pp_uint32)(v*pos);
				vLeftScrollbar->setBarPosition((pp_int32)(pos*65536.0f));
				vRightScrollbar->setBarPosition((pp_int32)(pos*65536.0f));
				break;
			}
			case ScrollModeStayInCenter:
				patternEditor->getCursor().row = (pp_int32)(pos*(pattern->rows-1));
				assureCursorVisible(true, false);
				break;
		}
	}
	// Horizontal scrollbar, scroll up (=left)
	else if ((sender == reinterpret_cast<PPObject*>(hBottomScrollbar) ||
			 sender == reinterpret_cast<PPObject*>(hTopScrollbar)) &&
			 event->getID() == eBarScrollUp)
	{
		if (startPos)
			startPos--;

		pp_int32 visibleItems = (visibleWidth) / slotSize;

		float v = (float)(patternEditor->getNumChannels() - visibleItems);

		hTopScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
		hBottomScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
	// Horizontal scrollbar, scroll down (=right)
	else if ((sender == reinterpret_cast<PPObject*>(hBottomScrollbar) ||
			 sender == reinterpret_cast<PPObject*>(hTopScrollbar)) &&
			 event->getID() == eBarScrollDown)
	{
		pp_int32 visibleItems = (visibleWidth) / slotSize;

		if (startPos + visibleItems < (signed)patternEditor->getNumChannels())
			startPos++;

		float v = (float)(patternEditor->getNumChannels() - visibleItems);

		if (v < 0.0f)
			v = 0.0f;

		hTopScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
		hBottomScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
	// Horizontal scrollbar, position changed
	else if ((sender == reinterpret_cast<PPObject*>(hBottomScrollbar) ||
			 sender == reinterpret_cast<PPObject*>(hTopScrollbar)) &&
			 event->getID() == eBarPosChanged)
	{

		float pos = reinterpret_cast<PPScrollbar*>(sender)->getBarPosition()/65536.0f;

		pp_int32 visibleItems = (visibleWidth) / slotSize;
		
		float v = (float)(patternEditor->getNumChannels() - visibleItems);

		if (v < 0.0f)
			v = 0.0f;

		startPos = (pp_uint32)(v*pos);

		hTopScrollbar->setBarPosition((pp_int32)(pos*65536.0f));
		hBottomScrollbar->setBarPosition((pp_int32)(pos*65536.0f));
	}
	else if (sender == reinterpret_cast<PPObject*>(editMenuControl) ||
	         sender == reinterpret_cast<PPObject*>(moduleMenuControl) ||
	         sender == reinterpret_cast<PPObject*>(instrumentMenuControl) ||
	         sender == reinterpret_cast<PPObject*>(channelMenuControl) )
	{
		switch (event->getID())
		{
			case eCommand:
				executeMenuCommand(*((pp_int32*)event->getDataPtr()));
				break;
			case eRemovedContextMenu:
			{
				struct MetaData
				{
					pp_int32 id;
					PPPoint p;
				} *metaData;
				
				metaData = (MetaData*)event->getDataPtr();
				
				PPPoint cp(metaData->p);
				
				cp.x-=location.x + SCROLLBARWIDTH + getRowCountWidth() + 4;
				cp.y-=location.y + SCROLLBARWIDTH + font->getCharHeight() + 4;
				
				pp_int32 menuInvokeChannel = pointInChannelHeading(cp);
				if (menuInvokeChannel != -1 && menuInvokeChannel != this->menuInvokeChannel) 
				{
					PPEvent e((EEventDescriptor)metaData->id, &metaData->p, sizeof(PPPoint));
					dispatchEvent(&e);
				}
				// clicked somewhere else in the control, no more channel header
				// highlight and repaint without updating
				else
				{
					this->menuInvokeChannel = -1;
					parentScreen->paintControl(this, false);
				}
				
				break;
			}
			default:
				break;
		}
		return 0;
	}
	else
	{
		return 0;
	}

	// Little happy workaround for scroll to center scrollmode
	if (properties.scrollMode == ScrollModeToCenter)
	{
		pp_int32 visibleItems = (visibleHeight) / font->getCharHeight();
		if ((vLeftScrollbar->getBarPosition() == 65536 || 
			 vRightScrollbar->getBarPosition() == 65536) &&
			startIndex + visibleItems <= pattern->rows)
		{
			startIndex++;
		}
	}

	parentScreen->paintControl(this);

	return 0;
}


pp_int32 PatternEditorControl::pointInChannelHeading(PPPoint& cp)
{
	if (cp.x < 0)
		return -1;

	// right click on channel numbber
	if (cp.y < 0 && cp.y < -2 && cp.y > -4-(pp_int32)font->getCharHeight())
	{
		pp_int32 i = (cp.x / slotSize) + startPos;
		if (i < 0 || i > 31)
			return -1;
		
		return i;
	}
	
	return -1;
}

void PatternEditorControl::invokeMenu(pp_int32 channel, const PPPoint& p)
{
	menuInvokeChannel = channel;

	editMenuControl->setLocation(p);
	editMenuControl->setState(MenuCommandIDCut, !hasValidSelection());
	editMenuControl->setState(MenuCommandIDCopy, !hasValidSelection());
	editMenuControl->setState(MenuCommandIDPaste, patternEditor->clipBoardSelectionIsEmpty());
	editMenuControl->setState(MenuCommandIDPorousPaste, patternEditor->clipBoardSelectionIsEmpty());
	editMenuControl->setState(MenuCommandIDUndo, !patternEditor->canUndo());
	editMenuControl->setState(MenuCommandIDRedo, !patternEditor->canRedo());
	editMenuControl->setState(MenuCommandIDSwapChannels, menuInvokeChannel == patternEditor->getCursor().channel);
	parentScreen->setContextMenuControl(editMenuControl);
}

bool PatternEditorControl::isSoloChannel(pp_int32 c)
{
	pp_int32 i = 0;
	for (pp_int32 j = 0; j < patternEditor->getNumChannels(); j++)
		if (muteChannels[j])
			i++;
			
	if (!muteChannels[c] && i == patternEditor->getNumChannels()-1)
		return true;
		
	return false;
}
