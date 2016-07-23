/*
 *  ppui/Button.cpp
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

#include "Button.h"
#include "GraphicsAbstract.h"
#include "Event.h"
#include "Screen.h"
#include "Font.h"
#include "PPUIConfig.h"

PPButton::PPButton(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
				   const PPPoint& location, const PPSize& size, 
				   bool border/*= true*/, 
				   bool clickable/*= true*/, 
				   bool update/*=true*/) :
	PPControl(id, parentScreen, eventListener, location, size),
	color(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorDefaultButton)),
	textColor(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorDefaultButtonText)),
	border(border),
	clickable(clickable),
	update(update),
	verticalText(false),
	flat(false),
	autoSizeFont(true),
	offset(0,0),
	invertShading(false),
	lMouseDown(false), rMouseDown(false)
{
	// default colors
	pressed = false;
	
	font = PPFont::getFont(PPFont::FONT_SYSTEM);
}

PPButton::~PPButton()
{
}

void PPButton::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	PPPoint location = this->location;

	g->setRect(location.x, location.y, location.x + size.width, location.y + size.height);

	g->setColor(*color);

	//g->fill();
	{
		PPColor nsdColor = *color, nsbColor = *color;

		if (!pressed)
		{
			// adjust not so dark color
			nsdColor.scaleFixed(flat ? 65536 : 40000);
			
			// adjust bright color
			nsbColor.scaleFixed(flat ? 65536 : 80000);
		}
		else
		{
			// adjust not so dark color
			nsdColor.scaleFixed(flat ? 65536 : 30000);
			
			// adjust bright color
			nsbColor.scaleFixed(flat ? 65536 : 60000);
		}
		
		g->fillVerticalShaded(nsbColor, nsdColor, invertShading);

	}

	PPColor bColor = *color;

	g->setFont(font);

	if (!pressed)
	{			
		// adjust bright color
		bColor.scaleFixed(87163);
		
		g->setColor(bColor);
		
		g->drawHLine(location.x, location.x + size.width, location.y);
		g->drawVLine(location.y, location.y + size.height, location.x);

		// adjust dark color
		bColor = *color;
		bColor.scaleFixed(20000);

		g->setColor(bColor);
		
		g->drawHLine(location.x, location.x + size.width, location.y + size.height - 1);
		g->drawVLine(location.y, location.y + size.height, location.x + size.width - 1);
		
		if (text)
		{
			PPColor tColor = *textColor;
			if (!clickable || !enabled)
			{
				tColor.r = (tColor.r + color->r)>>1;
				tColor.g = (tColor.g + color->g)>>1;
				tColor.b = (tColor.b + color->b)>>1;
			}
			
			g->setColor(tColor);

			if (!verticalText)
			{
				pp_int32 cx = (size.width>>1)-(font->getStrWidth(text)>>1) + location.x;
				pp_int32 cy = (size.height>>1)-(font->getCharHeight()>>1) + location.y + 1;
				g->drawString(text, cx+offset.x, cy+offset.y);
			}
			else
			{
				pp_int32 cx = (size.width>>1)-(font->getCharWidth()>>1) + location.x;
				pp_int32 cy = (size.height>>1)-((font->getCharHeight()*text.length())>>1) + location.y + 1;
				g->drawStringVertical(text, cx+offset.x, cy+offset.y);
			}
		}
	}
	else
	{
		// adjust dark color
		bColor = *color;
		bColor.scaleFixed(32768);
		
		g->setColor(bColor);
		
		g->drawHLine(location.x, location.x + size.width, location.y);
		g->drawVLine(location.y, location.y + size.height, location.x);

		g->setColor(bColor.r>>1,bColor.g>>1,bColor.b>>1);
		g->drawHLine(location.x, location.x + size.width, location.y + size.height - 1);
		g->drawVLine(location.y, location.y + size.height, location.x + size.width - 1);
		
		//g->drawHLine(location.x, location.x + size.width, location.y + size.height - 1);
		//g->drawVLine(location.y, location.y + size.height, location.x + size.width - 1);
		
		if (text)
		{
			g->setColor(*textColor);

			if (!verticalText)
			{
				pp_int32 cx = (size.width>>1)-(font->getStrWidth(text)>>1) + location.x + 1;
				pp_int32 cy = (size.height>>1)-(font->getCharHeight()>>1) + location.y + 2;
				g->drawString(text, cx+offset.x, cy+offset.y);
			}
			else
			{
				pp_int32 cx = (size.width>>1)-(font->getCharWidth()>>1) + location.x + 1;
				pp_int32 cy = (size.height>>1)-((font->getCharHeight()*text.length())>>1) + location.y + 2;
				g->drawStringVertical(text, cx+offset.x, cy+offset.y);
			}
		}
	}

	if (border && enabled)
	{
		bColor.r = bColor.g = bColor.b = 0;
		
		g->setColor(bColor);
	
		g->setRect(location.x-1, location.y-1, location.x + size.width+1, location.y + size.height+1);
		
		g->drawHLine(location.x, location.x + size.width, location.y-1);
		g->drawVLine(location.y, location.y + size.height, location.x-1);
		g->drawHLine(location.x, location.x + size.width, location.y + size.height);
		g->drawVLine(location.y, location.y + size.height, location.x + size.width);
	}

}

pp_int32 PPButton::dispatchEvent(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	//if (!visible)
	//	return 0;

	switch (event->getID())
	{
		case eLMouseDrag:
		case eRMouseDrag:
		{
			PPPoint p = *reinterpret_cast<const PPPoint*>(event->getDataPtr());
			if (!hit(p) && clickable && pressed && update)
			{
				pressed = false;
				parentScreen->paintControl(this);
			}
			else if (hit(p) && clickable && !pressed && update)
			{
				pressed = true;
				parentScreen->paintControl(this);
			}
		}
		break;

		case eLMouseDown:
			handleButtonPress(lMouseDown, rMouseDown);
			break;

		case eLMouseUp:
			handleButtonRelease(lMouseDown, rMouseDown, event, eCommand);
			break;
			
		case eRMouseDown:
			handleButtonPress(rMouseDown, lMouseDown);
			break;

		case eRMouseUp:
			handleButtonRelease(rMouseDown, lMouseDown, event, eCommandRight);
			break;

		case eLMouseRepeat:
		case eRMouseRepeat:
			if (clickable && pressed)
			{
				PPEvent e(eCommandRepeat);
				eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e); 
			}
			break;
		default:
			break;
	}
	return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), event);
}

void PPButton::setText(const PPString& text) 
{ 
	bool lastCharIsPeriod = text.length() ? (text[text.length()-1] == '.') : false;
	
	this->text = text; 
	
	// Fall back to tiny font if string doesn't fit with current font
	if (autoSizeFont &&
		font != PPFont::getFont(PPFont::FONT_TINY) &&
		!verticalText && 
		((signed)font->getStrWidth(text) > size.width - (lastCharIsPeriod ? -6 : 2) ||
		 (signed)font->getCharHeight() > size.height))
	{
		font = PPFont::getFont(PPFont::FONT_TINY);
	}
}

void PPButton::handleButtonPress(bool& lMouseDown, bool& rMouseDown)
{
	if (rMouseDown)
	{
		lMouseDown = false;
		pressed = false;
		if (update)
			parentScreen->paintControl(this);
		return;
	}

	lMouseDown = true;
	if (clickable)
	{
		pressed = true;
		if (update)
			parentScreen->paintControl(this);
	}
}

void PPButton::handleButtonRelease(bool& lMouseDown, bool& rMouseDown, PPEvent* event, EEventDescriptor postEvent)
{
	lMouseDown = false;
	if (clickable && !rMouseDown && pressed)
	{
		pressed = false;
		
		if (update)
			parentScreen->paintControl(this);			
		
		if (hit(*reinterpret_cast<const PPPoint*>(event->getDataPtr())))
		{
			PPEvent e(postEvent);
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e); 
		}
		
	}
}

