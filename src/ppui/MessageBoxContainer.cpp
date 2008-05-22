/*
 *  ppui/MessageBoxContainer.cpp
 *
 *  Copyright 2008 Peter Barth
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

#include "GraphicsAbstract.h"
#include "Button.h"
#include "MessageBoxContainer.h"
#include "Font.h"
#include "BasicTypes.h"
#include "PPUIConfig.h"
#include "Screen.h"

PPMessageBoxContainer::PPMessageBoxContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size, const PPString& caption) :
	PPContainer(id, parentScreen, eventListener, location, size),
	caption(caption),
	captured(false)	
{
	PPContainer::setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorMessageBoxContainer));	

	PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);
	captionSize = 2 + 2*2 + font->getCharHeight() + 1;

	buttonLocation = location;
	buttonLocation.y+=captionSize;
	buttonSize = size;
	buttonSize.height-=captionSize;

	button = new PPButton(-1, NULL, NULL, buttonLocation, buttonSize, false, false, false);
	button->setColor(*PPContainer::color);
}

PPMessageBoxContainer::~PPMessageBoxContainer()
{
	delete button;
}

void PPMessageBoxContainer::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;
	
	PPColor bColor = *color, dColor = *color, vbColor = *color;
	// adjust dark color
	
	dColor.scaleFixed(32768);
	bColor.scaleFixed(87163);
	vbColor.scaleFixed(131072);

	g->setColor(*color);
	
	g->setRect(location.x, location.y, location.x + size.width, location.y + captionSize);
	g->fill();

	//g->fill();
	button->paint(g);

	g->setRect(location.x, location.y, location.x + size.width, location.y + size.height);

	g->setColor(bColor);

	g->drawHLine(location.x, location.x + size.width, location.y);
	g->drawVLine(location.y, location.y + size.height, location.x);

	g->drawHLine(location.x + 2, location.x + size.width - 2, location.y + size.height - 3);
	g->drawVLine(location.y + 2, location.y + size.height - 2, location.x + size.width - 3);

	g->setColor(dColor);

	g->drawHLine(location.x + 2, location.x + size.width - 2, location.y + 2);
	g->drawVLine(location.y + 2, location.y + size.height - 2, location.x + 2);

	g->drawHLine(location.x, location.x + size.width, location.y + size.height - 1);
	g->drawVLine(location.y, location.y + size.height, location.x + size.width - 1);

	// seperator
	g->setColor(dColor);

	PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);

	g->drawHLine(location.x + 2, location.x + size.width - 3, location.y + captionSize-1);

	g->setColor(bColor);

	g->drawHLine(location.x + 3, location.x + size.width - 2, location.y + captionSize);

	pp_int32 cx = size.width / 2 - font->getStrWidth(caption) / 2;

	g->setColor(0,0,0);

	g->drawString(caption, location.x + cx + 1, location.y + 5);

	g->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText));

	g->drawString(caption, location.x + cx , location.y + 4);

	paintControls(g);
}

void PPMessageBoxContainer::setSize(PPSize size)
{
	PPContainer::setSize(size);

	buttonSize = size;
	buttonSize.height-=captionSize;
	
	button->setSize(buttonSize);
}

void PPMessageBoxContainer::setLocation(PPPoint location)
{
	PPContainer::setLocation(location);

	buttonLocation = location;
	buttonLocation.y+=captionSize;

	button->setLocation(buttonLocation);
}

pp_int32 PPMessageBoxContainer::dispatchEvent(PPEvent* event)
{
	switch (event->getID())
	{
		case eLMouseDown:
			lastCapturePoint = *reinterpret_cast<PPPoint*>(event->getDataPtr());
			captured = isPointInCaption(lastCapturePoint);
			if (captured)
				return 0;
			break;

		case eLMouseDrag:
			if (handleMove(*reinterpret_cast<PPPoint*>(event->getDataPtr())))
				return 0;
			break;

		case eLMouseUp:
			if (captured)
			{
				captured = false;
				return 0;
			}
			break;			
	}
	
	return PPContainer::dispatchEvent(event);
}

bool PPMessageBoxContainer::isPointInCaption(const PPPoint& point) const
{
	PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);
	return (point.x >= location.x && point.x <= location.x + size.width &&
			point.y >= location.y && point.y <= location.y + captionSize);
}

bool PPMessageBoxContainer::handleMove(const PPPoint& point)
{
	if (!captured)
		return false;

	if (point.x < 0 || point.y < 0)
		return false;

	if (point.x >= parentScreen->getWidth() || point.y >= parentScreen->getHeight())
		return false;
		
	PPPoint delta(point.x - lastCapturePoint.x, point.y - lastCapturePoint.y);
	
	lastCapturePoint = point;
	
	if (location.x + delta.x < 0)
	{
		delta.x -= (location.x + delta.x);
	}

	if (location.y + delta.y < 0)
	{
		delta.y -= (location.y + delta.y);
	}

	if (location.x + delta.x + size.width >= parentScreen->getWidth())
	{
		delta.x -= (location.x + delta.x + size.width) - parentScreen->getWidth();
	}
	
	if (location.y + delta.y + size.height >= parentScreen->getHeight())	
	{
		delta.y -= (location.y + delta.y + size.height) - parentScreen->getHeight();
	}
	
	move(delta);
	
	PPPoint buttonPos = button->getLocation();
	buttonPos.x+=delta.x;
	buttonPos.y+=delta.y;
	button->setLocation(buttonPos);
	
	parentScreen->paint();
	
	return true;
}

