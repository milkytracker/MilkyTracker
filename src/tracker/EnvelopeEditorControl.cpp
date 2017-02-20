/*
 *  tracker/EnvelopeEditorControl.cpp
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

#include "EnvelopeEditorControl.h"
#include "EnvelopeEditor.h"
#include "Screen.h"
#include "GraphicsAbstract.h"
#include "Font.h"
#include "ScrollBar.h"
#include "ContextMenu.h"
#include "XModule.h"
#include "TrackerConfig.h"
#include "DialogWithValues.h"
#include "Tools.h"
#include "FilterParameters.h"

#define SCROLLBARWIDTH SCROLLBUTTONSIZE
#define YMAX 256
#define XMAX 96

#ifdef __LOWRES__
	#define POINTCATCHBOUNDS 7
#else
	#define POINTCATCHBOUNDS 5
#endif

EnvelopeEditorControl::EnvelopeEditorControl(pp_int32 id, 
											 PPScreen* parentScreen, 
											 EventListenerInterface* eventListener, 
											 const PPPoint& location, 
											 const PPSize& size, 
											 bool border/*= true*/) :
	PPControl(id, parentScreen, eventListener, location, size),
	borderColor(&ourOwnBorderColor),
	envelopeEditor(NULL),
	envelope(NULL),
	showMarks(NULL)
{
	this->border = border;

	ourOwnBorderColor.set(192, 192, 192);

	// default color
	backgroundColor.set(0, 0, 0);

	hScrollbar = new PPScrollbar(0, parentScreen, this, PPPoint(location.x, location.y + size.height - SCROLLBARWIDTH - 1), size.width - 1, true);

	xMax = XMAX;
	yMax = YMAX;
	currentPosition.x = currentPosition.y = -1;
	xScale = 256;

	showVCenter = false;	

	startPos = 0;
	visibleWidth = size.width - 5;
	visibleHeight = size.height - SCROLLBARWIDTH - 5;

	adjustScrollbars();
	
	caughtControl = NULL;	
	controlCaughtByLMouseButton = controlCaughtByRMouseButton = false;

	showMarks = new ShowMark[TrackerConfig::maximumPlayerChannels];
	clearShowMarks();
	
	// build submenu
	subMenuAdvanced = new PPContextMenu(5, parentScreen, this, PPPoint(0,0), TrackerConfig::colorThemeMain);
	subMenuAdvanced->setSubMenu(true);
	subMenuAdvanced->addEntry("Scale X", MenuCommandIDXScale);
	subMenuAdvanced->addEntry("Scale Y", MenuCommandIDYScale);
	
	// Context PPMenu
	editMenuControl = new PPContextMenu(4, parentScreen, this, PPPoint(0,0), TrackerConfig::colorThemeMain, true);
	
	editMenuControl->addEntry("Undo", MenuCommandIDUndo);
	editMenuControl->addEntry("Redo", MenuCommandIDRedo);
	editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
	editMenuControl->addEntry("Copy", MenuCommandIDCopy);
	editMenuControl->addEntry("Paste", MenuCommandIDPaste);
	editMenuControl->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", -1);
	editMenuControl->addEntry("Advanced   \x10", 0xFFFF, subMenuAdvanced);

	// Create tool handler responder
	toolHandlerResponder = new ToolHandlerResponder(*this);
	dialog = NULL;	
	
	resetLastValues();
}

EnvelopeEditorControl::~EnvelopeEditorControl()
{
	if (envelopeEditor)
		envelopeEditor->removeNotificationListener(this);

	delete toolHandlerResponder;
	delete dialog;	

	delete hScrollbar;
	
	delete editMenuControl;
	delete subMenuAdvanced;
	
	delete[] showMarks;
}

float EnvelopeEditorControl::calcXScale()
{
	return 1.0f / ((float)(xMax*xScale/256) / (float)visibleWidth);
}

float EnvelopeEditorControl::calcYScale()
{
	return 1.0f / ((float)yMax / (float)visibleHeight);
}

void EnvelopeEditorControl::updateCurrentPosition(const PPPoint& cp)
{
	currentPosition.x = (pp_int32)((cp.x + startPos)*(1.0f / calcXScale()));
	currentPosition.y = (((pp_int32)((visibleHeight-cp.y)*(1.0f / calcYScale()))) * /*16645*/65536) >> 16;	
			
	if (currentPosition.x < 0)
		currentPosition.x = 0;
	if (currentPosition.x > 65535)
		currentPosition.x = 65535;
	
	if (currentPosition.y < 0)
		currentPosition.y = 0;
	if (currentPosition.y > 256)
		currentPosition.y = 256;
}

void EnvelopeEditorControl::setScale(pp_int32 scale)
{
	if (scale < 16 || scale > 1024)
		return;

	xScale = scale;

	adjustScrollbars();
}

void EnvelopeEditorControl::paintGrid(PPGraphicsAbstract* g, pp_int32 xOffset, pp_int32 yOffset)
{
	pp_int32 i;
	
	PPColor lineColor = TrackerConfig::colorThemeMain;
	lineColor.r>>=1;
	lineColor.g>>=1;
	lineColor.b>>=1;

	// Draw grid lines first
	float scaley = calcYScale();
	float scalex = calcXScale();
	
	// horizontal
	g->setColor(lineColor);
	
	/*for (i = 0; i <= yMax; i++)
	{
		if (!(i & 15))
		{
			pp_int32 y1 = (pp_int32)(i*scaley);
			y1 = location.y + yOffset + visibleHeight - y1;

			if (!(i&31))
			{
				g->setColor(lineColor);
				g->drawHLine(location.x + xOffset+1, location.x + xOffset + visibleWidth,y1);
			}
		}
	}*/
	
	float sy = 0;
	while (sy < visibleHeight+(pp_int32)(31.0f*scaley))
	{
		pp_int32 y1 = (pp_int32)sy;
		pp_int32 y = location.y + yOffset + visibleHeight - y1;
		g->drawHLine(location.x + xOffset+1, location.x + xOffset + visibleWidth,y);
		sy+=32.0f*scaley;
	}		
	
	// vertical
	float sx = -(float)startPos;
	
	if (sx < 0)
	{
		pp_int32 ink = (pp_int32)((-sx)/(16.0f*scalex)) - 1;
		sx += ink*(16.0f*scalex);
	}
	
	while (sx < visibleWidth)
	{
		pp_int32 x1 = (pp_int32)sx;
		pp_int32 x = x1 + location.x + xOffset;
		g->drawVLine(location.y+3, location.y + yOffset + visibleHeight,x);
		sx+=16.0f*scalex;
	}	

	g->setColor(255, 255, 255);
	sx = -(float)startPos;

	if (sx < 0)
	{
		pp_int32 ink = (pp_int32)((-sx)/(16.0f*scalex)) - 1;
		sx += ink*(16.0f*scalex);
	}

	i = 0;
	while (sx < visibleWidth)
	{
		pp_int32 x1 = (pp_int32)sx;
		pp_int32 x = x1 + location.x + xOffset;
		g->setPixel(x, location.y + yOffset + visibleHeight);
		if (!(i&1))
			g->setPixel(x, location.y + yOffset + visibleHeight+1);
		sx+=2.0f*scalex;
		i++;
	}	
}

void EnvelopeEditorControl::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;
	
	pp_int32 xOffset = 2;

	pp_int32 yOffset = 2;

	g->setRect(location.x, location.y, location.x + size.width, location.y + size.height);

	g->setColor(backgroundColor);

	g->fill();

	if (border)
	{
		drawBorder(g, *borderColor);
	}

	g->setRect(location.x+1, location.y+1, location.x+1 + size.width-2, location.y+1 + size.height-2);

	g->setColor(255, 255, 255);

	if (envelope == NULL)
	{
		PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);
		g->setFont(font);
		PPString s("None selected");

		g->drawString(s, location.x + size.width / 2 - font->getStrWidth(s) / 2, location.y + size.height / 2 - font->getCharHeight() / 2); 

		return;
	}

	float scaley = calcYScale();
	float scalex = calcXScale();

	pp_int32 i;

	paintGrid(g, xOffset, yOffset);

	g->setColor(255, 255, 255);
	// vertical
	for (i = 0; i <= yMax; i++)
	{	
		if (!(i & 15))
		{
			pp_int32 y1 = (pp_int32)(i*scaley);
			y1 = location.y + yOffset + visibleHeight - y1;

			g->setPixel(location.x + xOffset, y1);
			if (!(i&31))
			{
				g->setPixel(location.x + xOffset-1, y1);
			}
		}
	}
	
	/*// horizontal
	for (i = 0; i <= visibleWidth; i++)
	{
		pp_int32 i2 = i + startPos;
	
		pp_int32 x1 = i;
		pp_int32 x = x1 + location.x + xOffset;

		pp_int32 y1 = location.y + yOffset + visibleHeight - (pp_int32)(128*scaley);

		if (!(i2 & 3))
		{		
			g->setPixel(x, location.y + yOffset + visibleHeight);
		
			if (!(i2&31))
			{
				g->setPixel(x, location.y + yOffset + visibleHeight + 1);
			}
		
		}
	}*/

	// centered line
	for (i = 0; i <= visibleWidth; i++)
	{
		pp_int32 x1 = i;
		pp_int32 x = x1 + location.x + xOffset;

		pp_int32 y1 = location.y + yOffset + visibleHeight - (pp_int32)(128*scaley);

		if (showVCenter)
		{
			if (!(i&7))
				g->setColor(128, 128, 128);
			else
				g->setColor(64, 64, 64);
		
			g->setPixel(x, y1);
		}
	}

	g->setColor(255, 255, 128);
	if (!envelope->num)
	{
		PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);
		g->setFont(font);
		PPString s("No envelope");

		g->drawString(s, location.x + size.width / 2 - font->getStrWidth(s) / 2, location.y + size.height / 2 - font->getCharHeight()); 
	}

	for (i = 0; i < envelope->num - 1; i++)
	{
		
		pp_int32 x1 = (pp_int32)(envelope->env[i][0]*scalex);
		pp_int32 y1 = (pp_int32)(envelope->env[i][1]*scaley);
		
		pp_int32 x2 = (pp_int32)(envelope->env[i+1][0]*scalex);
		pp_int32 y2 = (pp_int32)(envelope->env[i+1][1]*scaley);

		x1+=location.x + xOffset - startPos;
		x2+=location.x + xOffset - startPos;

		y1 = location.y + yOffset + visibleHeight - y1;
		y2 = location.y + yOffset + visibleHeight - y2;

		g->drawAntialiasedLine(x1,y1,x2,y2);
		//g->drawLine(x1,y1,x2,y2);

		//g->setPixel(location.x + xOffset + x, location.y + yOffset + size.height - 4 - y);

	}

	// Showmarks
	g->setColor(255, 0, 255);
	
	for (pp_int32 sm = 0; sm < TrackerConfig::maximumPlayerChannels; sm++)
	{
		pp_int32 showMark = showMarks[sm].pos;
		if (!showMarks[sm].intensity || showMark == -1)
			continue;

		pp_int32 x = (pp_int32)(showMark*scalex) - startPos;
		g->drawVLine(location.y + yOffset, location.y + yOffset + visibleHeight + 2, location.x + xOffset + x); 
	}
	
	for (i = 0; i < envelope->num; i++)
	{
		pp_int32 x1 = (pp_int32)(envelope->env[i][0]*scalex);
		pp_int32 y1 = (pp_int32)(envelope->env[i][1]*scaley);

		x1+=location.x + xOffset - startPos;
		y1 = location.y + yOffset + visibleHeight - y1;

		if ((envelope->type & 4) && (i == envelope->loops || i == envelope->loope))
		{
			g->setColor(TrackerConfig::colorHighLight_1);

			for (pp_int32 j = 0; j < visibleHeight; j+=2)
				g->setPixel(x1, j + location.y + yOffset);		
		
			if (i == envelope->loops)
			{
				g->setPixel(x1, location.y + yOffset);

				g->setPixel(x1, location.y + yOffset + 1);
				g->setPixel(x1+1, location.y + yOffset + 1);
				g->setPixel(x1-1, location.y + yOffset + 1);
				
				g->setPixel(x1, location.y + yOffset + 2);
				g->setPixel(x1+1, location.y + yOffset + 2);
				g->setPixel(x1-1, location.y + yOffset + 2);
				g->setPixel(x1+2, location.y + yOffset + 2);
				g->setPixel(x1-2, location.y + yOffset + 2);
			}
			if (i == envelope->loope)
			{
				g->setPixel(x1, location.y + visibleHeight + yOffset);

				g->setPixel(x1, location.y + visibleHeight + yOffset - 1);
				g->setPixel(x1+1, location.y + visibleHeight + yOffset - 1);
				g->setPixel(x1-1, location.y + visibleHeight + yOffset - 1);
				
				g->setPixel(x1, location.y + visibleHeight + yOffset - 2);
				g->setPixel(x1+1, location.y + visibleHeight + yOffset - 2);
				g->setPixel(x1-1, location.y + visibleHeight + yOffset - 2);
				g->setPixel(x1+2, location.y + visibleHeight + yOffset - 2);
				g->setPixel(x1-2, location.y + visibleHeight + yOffset - 2);
			}
		
		}
		if ((envelope->type & 2) && (i == envelope->sustain))
		{
			g->setColor(255, 255, 255);

			for (pp_int32 j = 0; j < visibleHeight; j+=2)
				g->setPixel(x1, j + location.y + yOffset + 1);		
		}

		/*pp_int32 extent = 1;

		if (i == selectionIndex)
		{
			g->setColor(255, 0, 0);
			extent = 2;
		}
		else*/
		
		if (i != envelopeEditor->getSelectionIndex())
		{
			const pp_int32 extent = 1;
			
			g->setColor(255, 255, 255);
			
			for (pp_int32 y = -extent; y <= extent; y++)
				for (pp_int32 x = -extent; x <= extent; x++)
					g->setPixel(x1+x, y1+y);
		}
		
	}
	
	// draw selected point always above the other points
	if (envelopeEditor->getSelectionIndex() != -1)
	{
		i = envelopeEditor->getSelectionIndex();
		pp_int32 x1 = (pp_int32)(envelope->env[i][0]*scalex);
		pp_int32 y1 = (pp_int32)(envelope->env[i][1]*scaley);

		x1+=location.x + xOffset - startPos;
		y1 = location.y + yOffset + visibleHeight - y1;

		const pp_int32 extent = 2;
			
		g->setColor(255, 0, 0);
			
		for (pp_int32 y = -extent; y <= extent; y++)
			for (pp_int32 x = -extent; x <= extent; x++)
				g->setPixel(x1+x, y1+y);
	}

	if (currentPosition.x >= 0 && currentPosition.y >= 0)
	{
		PPFont* font = PPFont::getFont(PPFont::FONT_TINY);
		g->setFont(font);

		// loop markers above range text
		char buffer[32];
		PPTools::convertToHex(buffer, (unsigned)currentPosition.x, 4);

		g->setColor(0, 0, 0);
		g->drawString(buffer, location.x + 3 + visibleWidth - font->getStrWidth(buffer), location.y + 4);
		g->setColor(255, 0, 255);
		g->drawString(buffer, location.x + 2 + visibleWidth - font->getStrWidth(buffer), location.y + 3);

		PPTools::convertToHex(buffer, (unsigned)currentPosition.y, 2);

		g->setColor(0, 0, 0);
		g->drawString(buffer, location.x + 3 + visibleWidth - font->getStrWidth(buffer), location.y + 4 + font->getCharHeight()+2);
		g->setColor(0, 255, 0);
		g->drawString(buffer, location.x + 2 + visibleWidth - font->getStrWidth(buffer), location.y + 3 + font->getCharHeight()+2);
	}

	hScrollbar->paint(g);

}

pp_int32 EnvelopeEditorControl::dispatchEvent(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	switch (event->getID())
	{
		case eRMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();

			if (!hScrollbar->hit(*p))
				invokeContextMenu(*p);
			else
			{
				if (controlCaughtByLMouseButton)
					break;
				controlCaughtByRMouseButton = true;
				caughtControl = hScrollbar;
				caughtControl->dispatchEvent(event);
			}
			break;
		}
		
		case eLMouseDown:
		{
			hasDragged = false;
			selectionTicker = 0;

			PPPoint* p = (PPPoint*)event->getDataPtr();

			// Clicked on horizontal scrollbar -> route event to scrollbar and catch scrollbar control
			if (hScrollbar->hit(*p))
			{
				if (controlCaughtByRMouseButton)
					break;
				controlCaughtByLMouseButton = true;
				caughtControl = hScrollbar;
				caughtControl->dispatchEvent(event);
			}
			// Clicked in client area
			else
			{
				
				PPPoint cp = *p;
				
				translateCoordinates(cp);

				if (cp.x > visibleWidth)
					break;

				if (cp.y > visibleHeight)
					break;

				pp_int32 rx = (pp_int32)(cp.x + startPos);
				pp_int32 ry = (pp_int32)((visibleHeight - cp.y));

				pp_int32 i = selectEnvelopePoint(rx, ry);				

				if (i != -1)
				{
					envelopeEditor->startSelectionDragging(i);
				}
				else
				{
					updateCurrentPosition(cp);
				}

				parentScreen->paintControl(this);
			}

			break;
		}

		case eMouseLeft:
		{
			currentPosition.x = currentPosition.y = -1;
			parentScreen->paintControl(this);
			break;
		}

		case eMouseMoved:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();
			PPPoint cp = *p;

			translateCoordinates(cp);

			if (cp.y < 0 || cp.x > visibleWidth ||
				cp.y < 0 || cp.y > visibleHeight)
				break;

			updateCurrentPosition(cp);

			parentScreen->paintControl(this);
			break;
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
			break;
		}

		case eLMouseUp:
			if (envelopeEditor->isSelectionDragging())
				envelopeEditor->endSelectionDragging();
			
			controlCaughtByLMouseButton = false;

			if (caughtControl == NULL)
				break;

			if (!controlCaughtByLMouseButton && !controlCaughtByRMouseButton)
			{
				caughtControl->dispatchEvent(event);
				caughtControl = NULL;
			}
			break;

		case eLMouseDrag:
		{
			if (caughtControl && controlCaughtByLMouseButton)
			{
				caughtControl->dispatchEvent(event);
				break;
			}

			hasDragged = true;

			if (!envelopeEditor->isSelectionDragging())
				break;

			PPPoint* p = (PPPoint*)event->getDataPtr();
			
			PPPoint cp = *p;
			translateCoordinates(cp);

			setEnvelopePoint(envelopeEditor->getSelectionIndex(), cp.x + startPos, visibleHeight - cp.y);
			
			updateCurrentPosition(cp);
			
			adjustScrollbars();

			parentScreen->paintControl(this);

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

			// no selection has been made or mouse hasn't been dragged
			if (!envelopeEditor->isSelectionDragging() || !hasDragged)
			{
				// mouse hasn't been dragged and selection ticker has reached threshold value
				if (!hasDragged && selectionTicker > 0)
				{
					PPPoint* p = (PPPoint*)event->getDataPtr();

					if (!hScrollbar->hit(*p))
						invokeContextMenu(*p);
				}
			}
			break;
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

		// mouse wheel
		case eMouseWheelMoved:
		{
			TMouseWheelEventParams* params = (TMouseWheelEventParams*)event->getDataPtr();
			
			// Horizontal scrolling takes priority over vertical scrolling (zooming) and is
			// mutually exclusive so that we are less likely to accidentally zoom while scrolling
			// For compatibility for mice without horizontal scroll, SHIFT + vertical scroll is
			// treated as a synonym for horizontal scroll.
			bool shiftHeld = (::getKeyModifier() & KeyModifierSHIFT);
			if (params->deltaX || (params->deltaY && shiftHeld))
			{
				pp_int32 delta = shiftHeld? params->deltaY : params->deltaX;
				// Deltas greater than 1 generate multiple events for scroll acceleration
				PPEvent e = delta > 0 ? PPEvent(eBarScrollDown) : PPEvent(eBarScrollUp);
				
				delta = abs(delta);
				delta = delta > 20 ? 20 : delta;
				
				while (delta)
				{
					handleEvent(reinterpret_cast<PPObject*>(hScrollbar), &e);
					delta--;
				}
			}
			
			else if (params->deltaY)
			{
				if (invertMWheelZoom)
				{
					params->deltaY = -params->deltaY;
				}
				setScale(params->deltaY > 0 ? xScale << 1 : xScale >> 1);
				parentScreen->paintControl(this);
			}
			
			event->cancel();			
			break;
		}
		
		default:
			if (caughtControl == NULL)
				break;

			caughtControl->dispatchEvent(event);
			break;

	}

	return 0;
}

pp_int32 EnvelopeEditorControl::handleEvent(PPObject* sender, PPEvent* event)
{	
	// Horizontal scrollbar, scroll up (=left)
	if (sender == reinterpret_cast<PPObject*>(hScrollbar) &&
			 event->getID() == eBarScrollUp)
	{
		if (startPos)
			startPos--;

		pp_int32 visibleItems = visibleWidth;

		float v = (float)(getMaxWidth() - visibleItems);

		hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
	// Horizontal scrollbar, scroll down (=right)
	else if (sender == reinterpret_cast<PPObject*>(hScrollbar) &&
			 event->getID() == eBarScrollDown)
	{
		pp_int32 visibleItems = visibleWidth;

		if (startPos + visibleItems < (signed)getMaxWidth())
			startPos++;

		float v = (float)(getMaxWidth() - visibleItems);

		hScrollbar->setBarPosition((pp_int32)(startPos*(65536.0f/v)));
	}
	// Horizontal scrollbar, position changed
	else if (sender == reinterpret_cast<PPObject*>(hScrollbar) &&
			 event->getID() == eBarPosChanged)
	{

		float pos = hScrollbar->getBarPosition()/65536.0f;

		pp_int32 visibleItems = visibleWidth;
		
		float v = (float)(getMaxWidth() - visibleItems);

		if (v < 0.0f)
			v = 0.0f;

		startPos = (pp_uint32)(v*pos);
	}
	else if ((sender == reinterpret_cast<PPObject*>(editMenuControl) ||
			 sender == reinterpret_cast<PPObject*>(subMenuAdvanced)) &&
			 event->getID() == eCommand)
	{
		executeMenuCommand(*((pp_int32*)event->getDataPtr()));
		return 0;
	}

	parentScreen->paintControl(this);
	
	return 0;
}

void EnvelopeEditorControl::setSize(const PPSize& size)
{
	PPControl::setSize(size);

	hScrollbar->setSize(size.width - 1);
}

void EnvelopeEditorControl::setLocation(const PPPoint& location)
{
	PPControl::setLocation(location);

	hScrollbar->setLocation(PPPoint(location.x, location.y + size.height - SCROLLBARWIDTH - 1));
 }

void EnvelopeEditorControl::attachEnvelopeEditor(EnvelopeEditor* envelopeEditor)
{
	if (this->envelopeEditor)
		this->envelopeEditor->removeNotificationListener(this);

	this->envelopeEditor = envelopeEditor;
	envelopeEditor->addNotificationListener(this);
	adjustScrollbars();	
}

pp_int32 EnvelopeEditorControl::getMaxWidth()
{
	if (envelopeEditor == NULL)
		return visibleWidth;
		
	pp_int32 max = envelopeEditor->getHorizontalExtent();

	if (max < 0)
		return visibleWidth;

	float scalex = calcXScale();
	return (pp_int32)(scalex * max);
}

void EnvelopeEditorControl::adjustScrollbars()
{
	float s = (float)visibleWidth / (float)getMaxWidth();

	float olds = hScrollbar->getBarSize() / 65536.0f;

	hScrollbar->setBarSize((pp_int32)(s*65536.0f), false);	

	s = hScrollbar->getBarSize() / 65536.0f;

	float scale = s / olds;

	float pos = hScrollbar->getBarPosition()/65536.0f;
	hScrollbar->setBarPosition((pp_int32)(pos*scale*65536.0f));

	pos = hScrollbar->getBarPosition()/65536.0f;

	pp_int32 visibleItems = visibleWidth;
		
	float v = (float)(getMaxWidth() - visibleItems);

	startPos = (pp_uint32)(v*pos);	

	if (startPos < 0)
		startPos = 0;
}

pp_int32 EnvelopeEditorControl::selectEnvelopePoint(pp_int32 x, pp_int32 y)
{
	if (envelopeEditor == NULL || envelope == NULL)
		return -1;

	if (envelope->num == 0)
		return -1;

	float scaley = calcYScale();
	float scalex = calcXScale();

	if (envelopeEditor->getSelectionIndex() >= 0 && 
		envelopeEditor->getSelectionIndex() < envelope->num)
	{
		pp_int32 i = envelopeEditor->getSelectionIndex();
		pp_int32 ex = (pp_int32)(envelope->env[i][0]*scalex);
		pp_int32 ey = (pp_int32)(envelope->env[i][1]*scaley);

		if ((x - POINTCATCHBOUNDS <= ex) && (x + POINTCATCHBOUNDS >= ex) &&
			(y - POINTCATCHBOUNDS <= ey) && (y + POINTCATCHBOUNDS >= ey))
		{
			return i;
		}
	}

	for (pp_int32 i = 0; i < envelope->num; i++)
	{
		pp_int32 ex = (pp_int32)(envelope->env[i][0]*scalex);
		pp_int32 ey = (pp_int32)(envelope->env[i][1]*scaley);

		if ((x - POINTCATCHBOUNDS <= ex) && (x + POINTCATCHBOUNDS >= ex) &&
			(y - POINTCATCHBOUNDS <= ey) && (y + POINTCATCHBOUNDS >= ey))
		{
			return i;
		}	
	}

	return -1;

}

void EnvelopeEditorControl::setEnvelopePoint(pp_int32 index, pp_int32 x, pp_int32 y)
{	
	// inverse transformation
	float scaley = 1.0f / calcYScale();
	float scalex = 1.0f / calcXScale();

	pp_int32 ex = (pp_int32)(x*scalex);
	pp_int32 ey = (pp_int32)(y*scaley);

	envelopeEditor->setEnvelopePoint(index, ex, ey);
}

void EnvelopeEditorControl::reset()
{
	envelopeEditor->reset();
	clearShowMarks();
}

void EnvelopeEditorControl::validate()
{
	if (envelope)
	{
		if (envelopeEditor->getSelectionIndex() >= envelope->num)
			envelopeEditor->setSelectionIndex(envelope->num-1);
	}
}

void EnvelopeEditorControl::clearShowMarks()
{
	for (pp_int32 i = 0; i < TrackerConfig::maximumPlayerChannels; i++)
	{
		showMarks[i].pos = -1;
		showMarks[i].intensity = 0;
		showMarks[i].panning = 128;
	}
}

bool EnvelopeEditorControl::hasShowMarks()
{
	for (pp_int32 i = 0; i < TrackerConfig::maximumPlayerChannels; i++)
	{
		if (showMarks[i].pos != -1)
			return true;
	}
	return false;
}
	
void EnvelopeEditorControl::invokeContextMenu(PPPoint p)
{
	PPPoint cp = p;
	
	translateCoordinates(cp);
	
	if (cp.x > visibleWidth || cp.y > visibleHeight)
		return;
	
	editMenuControl->setLocation(p);
	editMenuControl->setState(MenuCommandIDUndo, !envelopeEditor->canUndo());
	editMenuControl->setState(MenuCommandIDRedo, !envelopeEditor->canRedo());
	editMenuControl->setState(MenuCommandIDPaste, !envelopeEditor->canPaste());
	editMenuControl->setState(MenuCommandIDCopy, !envelopeEditor->canCopy());

	subMenuAdvanced->setState(MenuCommandIDXScale, envelopeEditor->isEmptyEnvelope());
	subMenuAdvanced->setState(MenuCommandIDYScale, envelopeEditor->isEmptyEnvelope());

	parentScreen->setContextMenuControl(editMenuControl);
}

void EnvelopeEditorControl::executeMenuCommand(pp_int32 commandId)
{
	switch (commandId)
	{
		// copy
		case MenuCommandIDCopy:
		{
			envelopeEditor->makeCopy();
			notifyUpdate();
			break;
		}
		
		// paste
		case MenuCommandIDPaste:
			envelopeEditor->pasteCopy();
			break;

		case MenuCommandIDUndo:
			envelopeEditor->undo();
			break;
			
		case MenuCommandIDRedo:
			envelopeEditor->redo();
			break;
			
		case MenuCommandIDXScale:
			invokeToolParameterDialog(EnvelopeToolTypeScaleX);
			break;

		case MenuCommandIDYScale:
			invokeToolParameterDialog(EnvelopeToolTypeScaleY);
			break;
	}
	
}

// ----- some tools for modifying envelopes ----------------------------------
bool EnvelopeEditorControl::invokeToolParameterDialog(EnvelopeEditorControl::EnvelopeToolTypes type)
{
	if (dialog)
	{
		delete dialog;
		dialog = NULL;
	}
	
	toolHandlerResponder->setEnvelopeToolType(type);
	
	switch (type)
	{
		case EnvelopeToolTypeScaleX:
			dialog = new DialogWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Scale envelope along x-axis", DialogWithValues::ValueStyleEnterOneValue);
			static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Enter scale in percent:");
			static_cast<DialogWithValues*>(dialog)->setValueOneRange(0, 10000.0f, 2); 
			static_cast<DialogWithValues*>(dialog)->setValueOne(lastValues.scaleEnvelope != -1.0f ? lastValues.scaleEnvelope : 100.0f);
			break;

		case EnvelopeToolTypeScaleY:
			dialog = new DialogWithValues(parentScreen, toolHandlerResponder, PP_DEFAULT_ID, "Scale envelope along y-axis", DialogWithValues::ValueStyleEnterOneValue);
			static_cast<DialogWithValues*>(dialog)->setValueOneCaption("Enter scale in percent:");
			static_cast<DialogWithValues*>(dialog)->setValueOneRange(0, 10000.0f, 2); 
			static_cast<DialogWithValues*>(dialog)->setValueOne(lastValues.scaleEnvelope != -1.0f ? lastValues.scaleEnvelope : 100.0f);
			break;
	}
	
	dialog->show();
	
	return true;
}

bool EnvelopeEditorControl::invokeTool(EnvelopeToolTypes type)
{
	if (envelope == NULL)
		return false;

	bool res = false;
	switch (type)
	{
		case EnvelopeToolTypeScaleX:
		{
			lastValues.scaleEnvelope = static_cast<DialogWithValues*>(dialog)->getValueOne();
			FilterParameters par(1);
			par.setParameter(0, FilterParameters::Parameter(lastValues.scaleEnvelope / 100.0f));
			envelopeEditor->tool_xScaleEnvelope(&par);
			break;
		}

		case EnvelopeToolTypeScaleY:
		{
			lastValues.scaleEnvelope = static_cast<DialogWithValues*>(dialog)->getValueOne();
			FilterParameters par(1);
			par.setParameter(0, FilterParameters::Parameter(lastValues.scaleEnvelope / 100.0f));
			envelopeEditor->tool_yScaleEnvelope(&par);
			break;
		}
	}
	return res;
}

void EnvelopeEditorControl::editorNotification(EditorBase* sender, EditorBase::EditorNotifications notification)
{
	switch (notification)
	{
		case EnvelopeEditor::EditorDestruction:
		{
			envelopeEditor = NULL;
			break;
		}

		case EnvelopeEditor::NotificationReload:
		{
			startPos = 0;
			clearShowMarks();		
			this->envelope = envelopeEditor->getEnvelope();
			adjustScrollbars();
			break;
		}
		
		case EnvelopeEditor::NotificationChanges:
		{
			notifyUpdate();
			notifyChanges();
			break;
		}
		
		case EnvelopeEditor::NotificationUndoRedo:
		{
			adjustScrollbars();	
			validate();
			notifyUpdate();
			notifyChanges();
			break;
		}
		default:
			break;
	}
}

EnvelopeEditorControl::ToolHandlerResponder::ToolHandlerResponder(EnvelopeEditorControl& theEnvelopeEditorControl) :
	envelopeEditorControl(theEnvelopeEditorControl)
{
}

pp_int32 EnvelopeEditorControl::ToolHandlerResponder::ActionOkay(PPObject* sender)
{
	envelopeEditorControl.invokeTool(envelopeToolType);
	return 0;
}

pp_int32 EnvelopeEditorControl::ToolHandlerResponder::ActionCancel(PPObject* sender)
{
	return 0;
}


