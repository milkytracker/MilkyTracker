/*
 *  ppui/ListBox.cpp
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

#include "ListBox.h"
#include "GraphicsAbstract.h"
#include "Event.h"
#include "Screen.h"
#include "Font.h"
#include "ScrollBar.h"
#include "Button.h"
#include "PPUIConfig.h"
#include "Tools.h"

#define SCROLLBARWIDTH SCROLLBUTTONSIZE

#define BLINKINTERVAL 40

void PPListBox::initialize()
{
	// create background button
	//pp_int32 w = size.width - (scrollable?SCROLLBARWIDTH-2:0)-2;
	//pp_int32 h = horizontalScrollbar? size.height - (scrollable?SCROLLBARWIDTH-2:0)-2 : size.height-2;
	
	pp_int32 w = size.width - 2;
	pp_int32 h = size.height - 2;	

	backgroundButton = new PPButton(0, parentScreen, NULL, PPPoint(location.x+1, location.y+1), PPSize(w, h), border, false);
	backgroundButton->setColor(*backGroundButtonColor);
	backgroundButton->setInvertShading(true);

	vScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x + size.width - SCROLLBARWIDTH, location.y), size.height, false);
	hScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x, location.y + size.height - SCROLLBARWIDTH), size.width - SCROLLBARWIDTH, true);
	
	if (vScrollbar)
		vScrollbar->show(!autoHideVScroll);
	if (hScrollbar)
		hScrollbar->show(!autoHideHScroll);
}

PPListBox::PPListBox(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
					 const PPPoint& location, const PPSize& size, 
					 bool border/*= true*/, 
					 bool editable/*= false*/, 
					 bool scrollable/* = true*/,
					 bool showSelectionAlways/*= false*/) :
	PPControl(id, parentScreen, eventListener, location, size),
	borderColor(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorListBoxBorder)),
	backGroundButtonColor(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorListBoxBackground)),
	// default textcolor 
	textColor(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText)),
	autoHideVScroll(true),
	autoHideHScroll(true),
	selectionVisible(true),
	onlyShowIndexSelection(false),
	keepsFocus(true),
	showFocus(true),
	centerSelection(false),
	selectOnScroll(false),
	singleButtonClickEdit(false),
	allowDragSelection(true),
	rightButtonConfirm(false),
	hScrollbar(NULL),
	vScrollbar(NULL),
	colorQueryListener(NULL)
{
	this->border = border;

	this->editable = editable;
	this->scrollable = scrollable;

	this->showSelectionAlways = showSelectionAlways;

	showIndex = false;
	indexBaseCount = 1;

	// "unlimited" editing
	maxEditSize = -1;

	//this->clickable = clickable;

	// create char vector with 16 initial entries
	items = new PPSimpleVector<PPString>(16);		

	// create background button
	initialize();

	caughtControl = NULL;
	controlCaughtByLMouseButton = controlCaughtByRMouseButton = false;
	lMouseDown = rMouseDown = false;

	font = PPFont::getFont(PPFont::FONT_SYSTEM);

	startIndex = 0;	
	startPos = 0;
	timerTicker = 0;

	selectionIndex = showSelectionAlways ? 0 : -1;
	
	columnSelectionStart = -1;

	adjustScrollbars();

	editCopy = NULL;	
}

PPListBox::~PPListBox()
{
	delete vScrollbar;
	delete hScrollbar;
		
	delete backgroundButton;
		
	delete items;
}

void PPListBox::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	ColorQueryListener* colorQueryListener = this->colorQueryListener;

	PPColor bColor = *borderColor, dColor = *borderColor;
	// adjust dark color
	dColor.scaleFixed(32768);

	// adjust bright color
	bColor.scaleFixed(87163);

	pp_int32 xOffset = 2;

	pp_uint32 maxDigits = PPTools::getHexNumDigits(items->size() - 1 + indexBaseCount);

	if (maxDigits == 0)
		maxDigits++;

	if (showIndex)
		xOffset += (maxDigits * font->getCharWidth() + 5);
	
	pp_int32 yOffset = 2;

	backgroundButton->paint(g);

	if (border)
	{
		drawBorder(g, *borderColor);
	}

	g->setRect(location.x, location.y, location.x + size.width-2, location.y + size.height-2);

	g->setFont(font);

	pp_int32 pos = location.y + yOffset;

	pp_int32 selectionWidth = onlyShowIndexSelection ? (maxDigits * font->getCharWidth() + 5) : (size.width - 2);

	g->setColor(*textColor);

	for (pp_int32 i = startIndex; i < items->size(); i++)
	{
		if (i < 0)
			continue;

		if (pos >= location.y + size.height)
			break;
		
		if (i == selectionIndex /*&& columnSelectionStart < 0*/ && selectionVisible)
		{
			PPRect currentRect = g->getRect();

			PPRect rect = PPRect(location.x + 2, pos, location.x + selectionWidth, pos + getItemHeight());

			if (rect.y1 < currentRect.y1)
				rect.y1 = currentRect.y1;
			
			if (rect.x1 < currentRect.x1)
				rect.x1 = currentRect.x1;

			if (rect.y2 > currentRect.y2)
				rect.y2 = currentRect.y2;
			
			if (rect.x2 > currentRect.x2)
				rect.x2 = currentRect.x2;

			g->setRect(rect);

			if (hasFocus || !showFocus)
				g->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorSelection));
			else
			{
				PPColor c = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorGrayedOutSelection);
				c.scale(0.5f);
				g->setColor(c);
			}

			g->fill();

			g->setColor(*textColor);

			g->setRect(currentRect);
		}

		if (showIndex)
		{
			char hexIndex[10];
		
			PPTools::convertToHex(hexIndex, i + indexBaseCount, maxDigits);
			
			g->drawString(hexIndex, location.x + 2, pos);
		}
		
		PPRect currentRect = g->getRect();
		
		g->setRect(location.x + xOffset, 
				   location.y + yOffset, 
				   location.x + size.width-2, location.y + size.height-2);

		if (columnSelectionStart>=0 && selectionIndex == i)
		{
			if ((timerTicker % BLINKINTERVAL) < (BLINKINTERVAL>>1))
			{
				if (hasFocus)
				{
					PPColor c = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorSelection);
					if (showSelectionAlways)
						c.scale(0.65f);
					g->setColor(c);
				}
				else
					g->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorGrayedOutSelection));
				for (pp_int32 j = 0; j < 8; j++)
					g->drawVLine(pos, pos + getItemHeight(), location.x + xOffset - ((startPos-columnSelectionStart)*font->getCharWidth()) + j);
			}
			
			g->setColor(*textColor);
		}

		g->setColor(colorQueryListener ? colorQueryListener->getColor(i, *this) : *textColor);

		g->drawString(*items->get(i), location.x + xOffset - (startPos*font->getCharWidth()), pos);

		g->setRect(currentRect);
		
		pos+=getItemHeight();
	}

	g->setRect(location.x, location.y, location.x + size.width, location.y + size.height-1);

	if (showIndex)
	{
		g->setColor(bColor);
		g->drawVLine(location.y + 1, location.y + size.height - 1, location.x + xOffset - 3);
		g->setColor(*borderColor);
		g->drawVLine(location.y, location.y + size.height, location.x + xOffset - 2);
		g->setColor(dColor);
		g->drawVLine(location.y + 1, location.y + size.height - 1, location.x + xOffset - 1);
	}

	if (hScrollbar)
		hScrollbar->paint(g);

	if (vScrollbar)
		vScrollbar->paint(g);
}

PPListBox::SelectReturnCodes PPListBox::select(const PPPoint* p)
{
	if (singleButtonClickEdit)
		return SelectReturnCodePlaceCursor;

	timerTicker = 0;
	
	PPPoint cp = *p;
	cp.x-=location.x;
	cp.y-=location.y;
	
	pp_int32 xOffset = 2;
	pp_uint32 maxDigits = PPTools::getHexNumDigits(items->size() - 1 + indexBaseCount);
	if (maxDigits == 0)
		maxDigits++;
	
	if (showIndex)
		xOffset += (maxDigits * font->getCharWidth() + 5);
	
	pp_int32 yOffset = 2;
	
	cp.x -= xOffset;
	cp.y -= yOffset;
	
	if (cp.y > visibleHeight - getItemHeight())
		cp.y = visibleHeight - getItemHeight();
	
	pp_int32 selectionIndex = (cp.y / getItemHeight()) + startIndex;
	
	if (selectionIndex < 0 || selectionIndex >= items->size())
		selectionIndex = -1;
	
	if (showSelectionAlways && selectionIndex == -1)
		return SelectReturnCodeBreak;
	
	// selected line is different from current line so commit changes first
	// then select other line
	if (selectionIndex != this->selectionIndex)
	{
		commitChanges();
		//this->selectionIndex = selectionIndex;
		if (selectionIndex < 0 || selectionIndex >= items->size())
			selectionIndex = -1;
		
		if (selectionIndex != -1)
		{
			PPEvent e(ePreSelection, &selectionIndex, sizeof(selectionIndex));
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
		}
		
		this->selectionIndex = selectionIndex;
		
		if (selectionIndex != -1)
		{
			PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
		}
		
		columnSelectionStart = -1;	
		
		if (centerSelection)
			assureCursorVisible();
	}
	else if (columnSelectionStart > 0 && this->selectionIndex >= 0)
	{
		pp_int32 columnSelStart = cp.x / font->getCharWidth() + startPos;
		
		if (columnSelStart <= (signed)getItem(selectionIndex).length())
			columnSelectionStart = columnSelStart;
		else
			columnSelectionStart = (signed)getItem(selectionIndex).length();
	}
	
	parentScreen->paintControl(this);
	
	return SelectReturnCodeDefault;
}

pp_int32 PPListBox::dispatchEvent(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	//if (!visible)
	//	return 0;

	switch (event->getID())
	{
		case eTimer:
		{
			if (!isEditing())
				break;

			timerTicker++;
			bool timerState = (timerTicker % BLINKINTERVAL) < (BLINKINTERVAL>>1);
			if (timerState != lastTimerState)
			{
				lastTimerState = timerState;
				parentScreen->paintControl(this);
			}
			break;
		}
	
		case eFocusGainedNoRepaint:
		{
			timerTicker = 0;
			hasFocus = true;
			break;
		}

		case eFocusGained:
		{
			timerTicker = 0;
			hasFocus = true;
			parentScreen->paintControl(this);
			break;
		}
	
		case eFocusLost:
		{
			commitChanges();
			hasFocus = false;
			parentScreen->paintControl(this);
			break;
		}

		case eFocusLostNoRepaint:
		{
			commitChanges();
			hasFocus = false;
			break;
		}
		case eMouseWheelMoved:
		{
			TMouseWheelEventParams* params = (TMouseWheelEventParams*)event->getDataPtr();
			
			// Vertical scrolling takes priority over horizontal scrolling and is mutually
			// exclusive from horizontal scrolling.
			if (vScrollbar && params->deltaY)
			{
				PPEvent e = params->deltaY < 0 ? PPEvent(eBarScrollDown) : PPEvent(eBarScrollUp);
				handleEvent(reinterpret_cast<PPObject*>(vScrollbar), &e);
			}
			
			else if (hScrollbar && params->deltaX)
			{
				PPEvent e = params->deltaX > 0 ? PPEvent(eBarScrollDown) : PPEvent(eBarScrollUp);
				handleEvent(reinterpret_cast<PPObject*>(hScrollbar), &e);
			}

			event->cancel();
			
			break;
		}
		
		case eRMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();

			rMouseDown = true;

			// Clicked on horizontal scrollbar -> route event to scrollbar and catch scrollbar control
			if (hScrollbar && hScrollbar->isVisible() && hScrollbar->hit(*p))
			{
				if (controlCaughtByLMouseButton)
					break;
				caughtControl = hScrollbar;
				caughtControl->dispatchEvent(event);
				controlCaughtByRMouseButton = true;
			}
			// Clicked on vertical scrollbar -> route event to scrollbar and catch scrollbar control
			else if (vScrollbar && vScrollbar->isVisible() && vScrollbar->hit(*p))
			{
				if (controlCaughtByLMouseButton)
					break;
				caughtControl = vScrollbar;
				caughtControl->dispatchEvent(event);				
				controlCaughtByRMouseButton = true;
			}
			else if (rightButtonConfirm)
			{
				switch (select(p))
				{
					case SelectReturnCodeBreak:
						return 0;
					case SelectReturnCodePlaceCursor:
						goto placeCursor;
					case SelectReturnCodeDefault:
						break;
				}
			}
			break;
		}

		// implement some FT2 list box functionality
		// in FT2 you could click on a row and drag down, the while dragging
		// the list box would let you select other single rows
		case eRMouseRepeat:
		case eRMouseDrag:
			if (!rMouseDown)
				break;
			goto dragorautoscroll;
		case eLMouseRepeat:
		case eLMouseDrag:
		{
			if (!lMouseDown)
				break;
dragorautoscroll:
			if (caughtControl == NULL)
			{
				PPPoint* p = (PPPoint*)event->getDataPtr();

				PPPoint cp = *p;

				// check if we hit the items area:
				
				// below area => scroll up
				if (p->y < getVisibleRect().y1)
				{
					PPEvent e(eBarScrollUp);
					handleEvent(reinterpret_cast<PPObject*>(vScrollbar), &e);
					// and limit the hit point to the start of the area
					cp.y = getVisibleRect().y1;
				}
				// above area => scroll down
				else if (p->y > getVisibleRect().y2)
				{
					PPEvent e(eBarScrollDown);
					handleEvent(reinterpret_cast<PPObject*>(vScrollbar), &e);
					// and limit the hit point to the end of the area					
					cp.y = getVisibleRect().y2 - getItemHeight();
				}
				
				// handle selection by click point
				switch (select(&cp))
				{
					case SelectReturnCodeBreak:
						return 0;
					case SelectReturnCodePlaceCursor:
						goto placeCursor;
					case SelectReturnCodeDefault:
						break;
				}
			}
			else
				caughtControl->dispatchEvent(event);

			break;
		}
		
		case eLMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();

			lMouseDown = true;

			// Clicked on horizontal scrollbar -> route event to scrollbar and catch scrollbar control
			if (hScrollbar &&  hScrollbar->isVisible() && hScrollbar->hit(*p))
			{
				if (controlCaughtByRMouseButton)
					break;
				caughtControl = hScrollbar;
				caughtControl->dispatchEvent(event);
				controlCaughtByLMouseButton = true;
			}
			// Clicked on vertical scrollbar -> route event to scrollbar and catch scrollbar control
			else if (vScrollbar && vScrollbar->isVisible() && vScrollbar->hit(*p))
			{
				if (controlCaughtByRMouseButton)
					break;
				caughtControl = vScrollbar;
				caughtControl->dispatchEvent(event);				
				controlCaughtByLMouseButton = true;
			}
			// Clicked on text -> select text
			else
			{
				switch (select(p))
				{
					case SelectReturnCodeBreak:
						return 0;
					case SelectReturnCodePlaceCursor:
						goto placeCursor;
					case SelectReturnCodeDefault:
						break;
				}
			}

			break;
		}

		case eRMouseUp:
			controlCaughtByRMouseButton = false;
			if (caughtControl && !controlCaughtByLMouseButton && !controlCaughtByRMouseButton)
			{
				caughtControl->dispatchEvent(event);
				caughtControl = NULL;			
				break;
			}
			
			if (rMouseDown)
				rMouseDown = false;
			else
				break;
		case eLMouseDoubleClick:
		{
placeCursor:			
			timerTicker = 0;

			PPPoint cp = *((PPPoint*)event->getDataPtr());
			cp.x-=location.x;
			cp.y-=location.y;

			pp_int32 xOffset = 2;
			
			pp_uint32 maxDigits = PPTools::getHexNumDigits(items->size() - 1 + indexBaseCount);
			if (maxDigits == 0)
				maxDigits++;
			
			if (showIndex)
				xOffset += (maxDigits * font->getCharWidth() + 5);
			
			pp_int32 yOffset = 2;

			cp.x-=xOffset;
			cp.y-=yOffset;

			bool b = (cp.x >= 0 &&
					  cp.y >= 0 &&
					  cp.x <= visibleWidth &&
					  cp.y <= visibleHeight);
			
			if (!editable && b)
			{
				pp_int32 selectionIndex = (cp.y / getItemHeight()) + startIndex;				
				if (caughtControl == NULL && selectionIndex >= 0 && selectionIndex < items->size())
				{
					PPEvent e(eConfirmed, &selectionIndex, sizeof(selectionIndex));
					eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
				}
				break;
			}
			
			if (b)
			{
				pp_int32 selectionIndex = (cp.y / getItemHeight()) + startIndex;
				
				if (selectionIndex < 0 || selectionIndex >= items->size())
				{
					if (!showSelectionAlways)
						this->selectionIndex = -1;
				}
				else if (this->selectionIndex != selectionIndex || columnSelectionStart < 0)
				{
					// selected line is different from current line so commit changes first
					// then select other line
					commitChanges();
				
					if (selectionIndex != -1)
					{
						PPEvent e(ePreSelection, &selectionIndex, sizeof(selectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
					}
					
					this->selectionIndex = selectionIndex;

					if (selectionIndex != -1)
					{
						PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
					}
					
					pp_int32 columnSelStart = cp.x / font->getCharWidth() + startPos;
					
					if (columnSelStart <= (signed)getItem(selectionIndex).length())
						columnSelectionStart = columnSelStart;
					else
						columnSelectionStart = (signed)getItem(selectionIndex).length();
				
					if (editCopy)
						delete editCopy;

					editCopy = new PPString(getItem(selectionIndex));
				}
				else if (this->selectionIndex == selectionIndex && columnSelectionStart >= 0)
				{
					pp_int32 columnSelStart = cp.x / font->getCharWidth() + startPos;
					
					if (columnSelStart <= (signed)getItem(selectionIndex).length())
						columnSelectionStart = columnSelStart;
					else
						columnSelectionStart = (signed)getItem(selectionIndex).length();				
				}
				
				parentScreen->paintControl(this);
			
			}

			break;
		}

		case eLMouseUp:
			controlCaughtByLMouseButton = false;
			
			lMouseDown = false;

			if (caughtControl == NULL)
				break;

			if (!controlCaughtByLMouseButton && !controlCaughtByRMouseButton)
			{
				caughtControl->dispatchEvent(event);
				caughtControl = NULL;			
				break;
			}

			break;

		case eKeyChar:
		{
			if (caughtControl)
				break;

			if (!isEditing())
				break;
		
			if (selectionIndex < 0 ||
				selectionIndex >= items->size()/* ||
				columnSelectionStart < 0*/)
				break;
			
			pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());

			if (selectionIndex >= 0 &&
				columnSelectionStart > (signed)getItem(selectionIndex).length())
				columnSelectionStart = (signed)getItem(selectionIndex).length();

			assureCursorVisible();

			switch (keyCode)
			{
				default:
					if ((maxEditSize > -1) && 
						((signed)items->get(selectionIndex)->length() >= maxEditSize))
						break;

					if (columnSelectionStart < 0 || 
						selectionIndex < 0)
						break;

					items->get(selectionIndex)->insertAt(columnSelectionStart,(char)keyCode);
					columnSelectionStart++;
					break;
			}

			if (selectionIndex >= 0 &&
				columnSelectionStart > (signed)getItem(selectionIndex).length())
				columnSelectionStart = (signed)getItem(selectionIndex).length();

			assureCursorVisible();

			adjustScrollbars();
			adjustScrollbarPositions();

			parentScreen->paintControl(this);			

			break;
		}


		case eKeyDown:
		{	
			if (caughtControl)
				break;
		
			if (selectionIndex < 0 ||
				selectionIndex >= items->size()/* ||
				columnSelectionStart < 0*/)
				break;
			
			pp_uint16 keyCode = *((pp_uint16*)event->getDataPtr());

			if (selectionIndex >= 0 &&
				columnSelectionStart > (signed)getItem(selectionIndex).length())
				columnSelectionStart = (signed)getItem(selectionIndex).length();

			timerTicker = 0;

			switch (keyCode)
			{
				case VK_DELETE:
					if (columnSelectionStart < 0 || selectionIndex < 0)
						goto skiprefresh;

					assureCursorVisible();
					items->get(selectionIndex)->deleteAt(columnSelectionStart, 1);
					break;

				case VK_BACK:
					if (columnSelectionStart < 0 || selectionIndex < 0)
						goto skiprefresh;

					if (columnSelectionStart)
					{
						assureCursorVisible();
						columnSelectionStart--;
						items->get(selectionIndex)->deleteAt(columnSelectionStart, 1);
					}
					break;

				case VK_ESCAPE:
					if (!editable)
						goto skiprefresh;

					// discard changes
					assureCursorVisible();
					discardChanges();
					break;

				case VK_RETURN:
					if (!editable)
						goto skiprefresh;

					// commit changes
					assureCursorVisible();
					commitChanges();
					break;

				//case VK_ESCAPE:
				//	selectionIndex = -1;
				//	columnSelectionStart = 0;
				//	break;

				case VK_LEFT:
					assureCursorVisible();
					// Not editing, try to scroll left
					// might be handy on pocketpcs with very tight listboxes
					if (columnSelectionStart < 0)
					{
						PPEvent e(eBarScrollUp);
						handleEvent(reinterpret_cast<PPObject*>(hScrollbar), &e);
						break;
					}

					if (columnSelectionStart)
						columnSelectionStart--;
					break;
				
				case VK_RIGHT:
					assureCursorVisible();
					// Not editing, try to scroll right
					// might be handy on pocketpcs with very tight listboxes
					if (columnSelectionStart < 0)
					{
						PPEvent e(eBarScrollDown);
						handleEvent(reinterpret_cast<PPObject*>(hScrollbar), &e);
						break;
					}

					if (columnSelectionStart < (signed)getItem(selectionIndex).length())
						columnSelectionStart++;
					break;
				
				case VK_UP:
					assureCursorVisible();
					if (columnSelectionStart >= 0)
						break;

					if (selectionIndex)
					{
						pp_int32 newSelectionIndex = selectionIndex-1;
						PPEvent ePre(ePreSelection, &newSelectionIndex, sizeof(newSelectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &ePre);
						selectionIndex = newSelectionIndex;
						PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
					}
					break;
				
				case VK_DOWN:
					assureCursorVisible();
					if (columnSelectionStart >= 0)
						break;

					if (selectionIndex < items->size() - 1)
					{
						pp_int32 newSelectionIndex = selectionIndex+1;
						PPEvent ePre(ePreSelection, &newSelectionIndex, sizeof(newSelectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &ePre);
						selectionIndex = newSelectionIndex;
						PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
					}
					break;

				case VK_NEXT:
				{
					assureCursorVisible();
					pp_int32 lastSelectionIndex = selectionIndex;
					pp_int32 newSelectionIndex = selectionIndex;
					pp_int32 visibleItems = getNumVisibleItems();
					
					if (newSelectionIndex + visibleItems >= items->size() - 1)
					{
						newSelectionIndex = items->size() - 1;
						startIndex = newSelectionIndex - visibleItems;	
					}
					else
					{
						newSelectionIndex+=visibleItems;
						if (newSelectionIndex > items->size() - 1)
							newSelectionIndex = items->size() - 1;

						if (newSelectionIndex != lastSelectionIndex)
						{
							startIndex = newSelectionIndex;
							if (startIndex + visibleItems > items->size())
							{
								startIndex-=(startIndex + visibleItems)-(items->size());
								if (startIndex < 0)
									startIndex = 0;
								newSelectionIndex = startIndex;
							}
						}
					}

					if (newSelectionIndex != lastSelectionIndex)
					{
						PPEvent ePre(ePreSelection, &newSelectionIndex, sizeof(newSelectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &ePre);
						this->selectionIndex = newSelectionIndex;
						PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
					}

					break;
				}

				case VK_PRIOR:
				{
					assureCursorVisible();
					pp_int32 lastSelectionIndex = selectionIndex;
					pp_int32 newSelectionIndex = selectionIndex;
					pp_int32 visibleItems = getNumVisibleItems();
					
					newSelectionIndex-=visibleItems;
					if (newSelectionIndex < 0)
						newSelectionIndex = 0;
					
					if (newSelectionIndex != lastSelectionIndex)
					{
						PPEvent ePre(ePreSelection, &newSelectionIndex, sizeof(newSelectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &ePre);
						this->selectionIndex = newSelectionIndex;
						PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
						eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
					}
					break;
				}
				
				default:
					goto skiprefresh;
			}

			if (selectionIndex >= 0 &&
				columnSelectionStart > (signed)getItem(selectionIndex).length())
				columnSelectionStart = (signed)getItem(selectionIndex).length();

			assureCursorVisible();

			adjustScrollbars();
			adjustScrollbarPositions();

			parentScreen->paintControl(this);			
skiprefresh:
			break;
		}

		//case eLMouseDrag:
		default:
			if (caughtControl == NULL)
				break;

			caughtControl->dispatchEvent(event);
			break;

	}

	return 0;
}

pp_int32 PPListBox::handleEvent(PPObject* sender, PPEvent* event)
{
	pp_int32 lastSelectionIndex = selectionIndex;

	// Vertical scrollbar, scroll down
	if (sender == reinterpret_cast<PPObject*>(vScrollbar) && 
		vScrollbar &&
		vScrollbar->isVisible() &&
		event->getID() == eBarScrollDown)
	{
		if (selectOnScroll)
		{
			pp_int32 newSelectionIndex = selectionIndex;
			if (newSelectionIndex < items->size()-1)
			{
				newSelectionIndex++;
			}
			if (newSelectionIndex != lastSelectionIndex)
			{
				PPEvent ePre(ePreSelection, &newSelectionIndex, sizeof(newSelectionIndex));
				eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &ePre);
				this->selectionIndex = newSelectionIndex;
				PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
				eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			}
			assureCursorVisible();
		}
		else
		{
			pp_int32 visibleItems = getNumVisibleItems();
			
			if (startIndex + visibleItems < items->size())
				startIndex++;
			
			float v = (float)(items->size() - visibleItems);
			
			vScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
		}
	}
	// Vertical scrollbar, scroll up
	else if (sender == reinterpret_cast<PPObject*>(vScrollbar) && 
			 vScrollbar &&
			 vScrollbar->isVisible() &&
			 event->getID() == eBarScrollUp)
	{
		if (selectOnScroll)
		{
			pp_int32 newSelectionIndex = selectionIndex;
			if (newSelectionIndex > 0)
			{
				newSelectionIndex--;
			}
			if (newSelectionIndex != lastSelectionIndex)
			{
				PPEvent ePre(ePreSelection, &newSelectionIndex, sizeof(newSelectionIndex));
				eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &ePre);
				this->selectionIndex = newSelectionIndex;
				PPEvent e(eSelection, &selectionIndex, sizeof(selectionIndex));
				eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
			}
			assureCursorVisible();
		}
		else
		{
			if (startIndex)
				startIndex--;
			
			pp_int32 visibleItems = getNumVisibleItems();
			
			float v = (float)(items->size() - visibleItems);
			
			vScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
		}
	}
	// Vertical scrollbar, position changed
	else if (sender == reinterpret_cast<PPObject*>(vScrollbar) && 
			 vScrollbar &&
			 vScrollbar->isVisible() &&
			 event->getID() == eBarPosChanged)
	{

		float pos = vScrollbar->getBarPosition()/65536.0f;

		pp_int32 visibleItems = getNumVisibleItems();
		
		float v = (float)(items->size() - visibleItems);

		startIndex = (pp_uint32)(v*pos);
	}
	// Horizontal scrollbar, scroll up (=left)
	else if (sender == reinterpret_cast<PPObject*>(hScrollbar) && 
			 hScrollbar &&
			 hScrollbar->isVisible() &&
			 event->getID() == eBarScrollUp)
	{
		if (startPos)
			startPos--;

		pp_int32 visibleItems = (visibleWidth) / font->getCharWidth();

		float v = (float)(getMaxWidth() - visibleItems);

		hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
	// Horizontal scrollbar, scroll down (=right)
	else if (sender == reinterpret_cast<PPObject*>(hScrollbar) && 
			 hScrollbar &&
			 hScrollbar->isVisible() &&
			 event->getID() == eBarScrollDown)
	{
		pp_int32 visibleItems = (visibleWidth) / font->getCharWidth();

		if (startPos + visibleItems < (signed)getMaxWidth())
			startPos++;

		float v = (float)(getMaxWidth() - visibleItems);

		hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
	// Horizontal scrollbar, position changed
	else if (sender == reinterpret_cast<PPObject*>(hScrollbar) && 
			 hScrollbar &&
			 hScrollbar->isVisible() &&
			 event->getID() == eBarPosChanged)
	{

		float pos = hScrollbar->getBarPosition()/65536.0f;

		pp_int32 visibleItems = (visibleWidth) / font->getCharWidth();
		
		float v = (float)(getMaxWidth() - visibleItems);

		startPos = (pp_uint32)(v*pos);
	}

	parentScreen->paintControl(this);
	
	return 0;
}

void PPListBox::setSize(const PPSize& size)
{
	PPControl::setSize(size);
	
	delete backgroundButton;
	
	if (vScrollbar)
		delete vScrollbar;

	if (hScrollbar)
		delete hScrollbar;
	
	initialize();

	adjustScrollbars();
	
	assureCursorVisible();
}

void PPListBox::setLocation(const PPPoint& p)
{
	PPControl::setLocation(p);

	delete backgroundButton;

	if (vScrollbar)
		delete vScrollbar;

	if (hScrollbar)
		delete hScrollbar;
	
	initialize();

	adjustScrollbars();
	
	assureCursorVisible();
}

void PPListBox::addItem(const PPString& item)
{
	items->add(new PPString(item)); 

	adjustScrollbars();
}

const PPString& PPListBox::getItem(pp_int32 index) const
{
	return *items->get(index);
}

void PPListBox::updateItem(pp_int32 index, const PPString& item)
{
	items->replace(index, new PPString(item));
}

void PPListBox::clear()
{
	items->clear();

	startIndex = 0;	
	startPos = 0;

	selectionIndex = showSelectionAlways ? 0 : -1;
	columnSelectionStart = -1;

	adjustScrollbars();

	if (vScrollbar)
		vScrollbar->setBarPosition(0);
	
	if (hScrollbar)
		hScrollbar->setBarPosition(0);
}

void PPListBox::setSelectedIndex(pp_int32 index, bool adjustStartIndex/* = true*/, bool assureCursor/* = true*/)
{
	selectionIndex = index < items->size() ? index : items->size()-1;

	if (adjustStartIndex)
		startIndex = selectionIndex;

	if (assureCursor)
		assureCursorVisible();
}

void PPListBox::setSelectedIndexByItem(const PPString& item, bool adjustStartIndex/* = true*/)
{
	for (pp_int32 i = 0; i < items->size(); i++)
	{
		if (items->get(i)->compareTo(item) == 0)
		{
			setSelectedIndex(i, adjustStartIndex);
			break;
		}
	}
}

void PPListBox::placeCursorAtEnd()
{
	if (selectionIndex >= 0 && selectionIndex < items->size())		
		columnSelectionStart = getItem(selectionIndex).length();
	else if (items->size())
	{
		selectionIndex = 0;
		columnSelectionStart = getItem(0).length();

		if (editCopy)
			delete editCopy;
		
		editCopy = new PPString(getItem(selectionIndex));
	}
}

void PPListBox::placeCursorAtStart()
{
	if (items->size())
	{
		selectionIndex = 0;
		columnSelectionStart = 0;

		if (editCopy)
			delete editCopy;
		
		editCopy = new PPString(getItem(selectionIndex));
	}
}

void PPListBox::setShowIndex(bool showIndex)
{ 
	this->showIndex = showIndex; 

	calcVisible();
}

void PPListBox::calcVisible()
{
	// calculate visible area

	if (hScrollbar->isVisible())
		visibleHeight = size.height - (scrollable?SCROLLBARWIDTH:0);
	else
		visibleHeight = size.height;
	
	if (vScrollbar->isVisible())	
		visibleWidth = size.width - (scrollable?SCROLLBARWIDTH:0);
	else
		visibleWidth = size.width;
	
	if (border)
	{
		if (!hScrollbar->isVisible())
			visibleHeight--;
		if (!vScrollbar->isVisible())
			visibleWidth--;
	}
	
	if (showIndex)
	{
		pp_uint32 maxDigits = PPTools::getHexNumDigits(items->size() - 1 + indexBaseCount);
		
		if (maxDigits == 0)
			maxDigits++;
		visibleWidth -= (maxDigits * font->getCharWidth() + 5);
	}
}

void PPListBox::adjustScrollbarPositions()
{
	// adjust scrollbar positions
	if (vScrollbar)
	{
		pp_int32 visibleItems = getNumVisibleItems();
	
		float v = (float)(items->size() - visibleItems);
	
		vScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
	}
	
	if (hScrollbar)
	{
		pp_int32 visibleItems = (visibleWidth) / font->getCharWidth();
		
		float v = (float)(getMaxWidth() - visibleItems);
		
		hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
}

void PPListBox::adjustScrollbars()
{
	if (!scrollable)
	{
		calcVisible();
		return;
	}
		
	vScrollbar->show(!autoHideVScroll);
	hScrollbar->show(!autoHideHScroll);
	calcVisible();
	
	if (items->size() == 0/* || getMaxWidth()*font->getCharWidth() == 0*/)
	{
		return;
	}
	
	const pp_int32 maxWidth = font->getCharWidth() * getMaxWidth();
	
	// number of items fit into the current visible area (y direction)
	if (items->size() <= getNumVisibleItems())
	{
		// if they exceed the current visible area in x direction
		// we need to activate the horizontal scroll bar
		if (maxWidth > getVisibleRect().width())
		{
			hScrollbar->show(true);
			calcVisible();
			// now if they no longer fit the visible area in y direction
			// we also need to activate the vertical scroll bar
			if (items->size() > getNumVisibleItems())
			{
				vScrollbar->show(true);
				calcVisible();
			}
		}
	}
	else
	{
		// they don't fit, we need the vertical scroll bar for sure
		vScrollbar->show(true);
		calcVisible();
		// if the maximum x width exceeds the number of characters visible
		// we will also need to activate the horizontal scroll bar
		if (maxWidth > getVisibleRect().width())
		{
			hScrollbar->show(true);
			calcVisible();
		}
	}

	// adjust size of vertical scroll bar
	if (vScrollbar->isVisible())
	{
		pp_int32 height = size.height;
		vScrollbar->setSize(height);
	}
	if (hScrollbar->isVisible())
	{
		pp_int32 width = size.width - (vScrollbar->isVisible() ? SCROLLBARWIDTH : 0);
		hScrollbar->setSize(width);
	}
	
	float s = 1.0f;
	
	if (vScrollbar)
	{
		s = (float)(visibleHeight) / (float)(items->size()*(getItemHeight()));

		vScrollbar->setBarSize((pp_int32)(s*65536.0f), false);
	}

	
	if (hScrollbar)
	{
		s = (float)(visibleWidth) / (float)(getMaxWidth()*(font->getCharWidth()));

		hScrollbar->setBarSize((pp_int32)(s*65536.0f), false);
	}
}

void PPListBox::assureCursorVisible()
{
	if (selectionIndex >= 0)
	{
		pp_int32 visiblePixels = (visibleHeight - getItemHeight()) - 1;
		pp_int32 visibleItems = visiblePixels / getItemHeight();
				
		if (!centerSelection)
		{	
		
			if ((startIndex <= selectionIndex) && 
				((selectionIndex - startIndex) * getItemHeight()) < visiblePixels)
			{
			}
			else if (selectionIndex > startIndex &&
				selectionIndex + visibleItems < items->size())
			{
				//startIndex = cursorPositionRow;
				startIndex+=(selectionIndex-(startIndex+visibleItems));
			}
			else if (selectionIndex < startIndex &&
				selectionIndex + visibleItems < items->size())
			{
				//startIndex = cursorPositionRow;
				startIndex+=(selectionIndex-startIndex);
			}
			else
			{
				startIndex = selectionIndex - visibleItems;
				if (startIndex < 0)
					startIndex = 0;
			}
		}
		else
		{
			pp_int32 mid = (visibleHeight/2) / getItemHeight();
			startIndex = selectionIndex - mid;	
			if (startIndex < 0)
				startIndex = 0;
		}
	
	}
	
	if (columnSelectionStart >= 0)
	{
		
		
		pp_int32 visibleChars = (visibleWidth - font->getCharWidth()) / font->getCharWidth();
		
		if ((startPos < columnSelectionStart) && 
			((columnSelectionStart - startPos) * font->getCharWidth()) < (visibleWidth - font->getCharWidth()))
		{
		}
		else if (columnSelectionStart + (signed)visibleChars < (signed)getMaxWidth())
			startPos = columnSelectionStart;				
		else
		{
			startPos = columnSelectionStart - visibleChars;
			if (startPos < 0) 
				startPos = 0;
		}
								
		
	}
	
	adjustScrollbarPositions();
}

void PPListBox::commitChanges()
{
	if (selectionIndex < 0)
		return;

	if (selectionIndex >= items->size())
		return;

	if (isEditing())
	{
		PPString* str = items->get(selectionIndex);
	
		if (editCopy && str->compareTo(*editCopy) != 0)
		{
			PPEvent e(eValueChanged, &str, sizeof(PPString*));				
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
		}
		
		if (!showSelectionAlways)
			selectionIndex = -1;
		
		columnSelectionStart = -1;
		if (editCopy)
		{
			delete editCopy;
			editCopy = NULL;
		}

		startPos = 0;			// Reset horizontal scrollbar
	}
}

void PPListBox::discardChanges()
{
	columnSelectionStart = -1;
	if (editCopy)
	{
		items->get(selectionIndex)->replace(*editCopy);
		delete editCopy;
		editCopy = NULL;
	}
}

// hack
void PPListBox::saveState()
{
	lastStartIndex = startIndex;
	lastStartPos = startPos;
	lastSelectionIndex = selectionIndex;
	hadVScrollbar = (vScrollbar != NULL && vScrollbar->isVisible());
	hadHScrollbar = (hScrollbar != NULL && hScrollbar->isVisible());
}

void PPListBox::restoreState(bool assureCursor/* = true*/)
{
	if (lastStartIndex < items->size())
		startIndex = lastStartIndex;
	
	if (startPos < (signed)getMaxWidth())
		startPos = lastStartPos;
	
	if (lastSelectionIndex < items->size())
		selectionIndex = lastSelectionIndex;
	else
		selectionIndex = items->size()-1;
	
	if (selectionIndex < 0 && showSelectionAlways && items->size())
		selectionIndex = 0;
	
	// adjust scrollbar positions
	if (vScrollbar)
	{
		pp_int32 visibleItems = getNumVisibleItems();
		
		float v = (float)(items->size() - visibleItems);
		
		vScrollbar->setBarPosition((pp_int32)(startIndex*(65536.0f/v)));
		
		// if scrollbar isn't visible, reset vertical position to start
		if (!vScrollbar->isVisible())
			startIndex = 0;
	}
	else if (hadVScrollbar)
	{		
		// if scrollbar isn't visible, reset vertical position to start
		startIndex = 0;
	}
	
	
	if (hScrollbar)
	{
		pp_int32 visibleItems = (visibleWidth) / font->getCharWidth();
		
		float v = (float)(getMaxWidth() - visibleItems);
		
		hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));

		// if scrollbar isn't visible, reset horizontal position to start
		if (!hScrollbar->isVisible())
			startPos = 0;
	}
	else if (hadHScrollbar)
	{		
		// if scrollbar isn't visible, reset horizontal position to start
		startPos = 0;
	}
	
	if (assureCursor)
		assureCursorVisible();
}

PPRect PPListBox::getVisibleRect() const
{
	PPRect rect;
	rect.x1 = rect.x2 = location.x;
	rect.y1 = rect.y2 = location.y;
	rect.x2+=visibleWidth;
	rect.y2+=visibleHeight;
	return rect;
}

pp_int32 PPListBox::getItemHeight() const
{
	return font->getCharHeight();
}

pp_int32 PPListBox::getNumVisibleItems() const
{
	return visibleHeight / getItemHeight();

	//return (visibleHeight - (hScrollbar->isVisible() ? 4 : 0)) / getItemHeight();
}

pp_uint32 PPListBox::getMaxWidth() const
{

	pp_uint32 maxWidth = 0;

	for (pp_int32 i = 0; i <  items->size(); i++)
	{
		pp_uint32 len = items->get(i)->length();
		if (len > maxWidth)
			maxWidth = len;
	}

	return maxWidth+(editable ? 1 : 0);

}
