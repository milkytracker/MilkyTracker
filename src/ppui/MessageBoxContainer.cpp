/*
 *  ppui/MessageBoxContainer.cpp
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

#include "GraphicsAbstract.h"
#include "Button.h"
#include "MessageBoxContainer.h"
#include "Font.h"
#include "BasicTypes.h"
#include "PPUIConfig.h"
#include "Screen.h"
#include "SimpleVector.h"

PPMessageBoxContainer::PPMessageBoxContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
											 const PPPoint& location, const PPSize& size, const PPString& caption) :
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

void PPMessageBoxContainer::setSize(const PPSize& size)
{
	PPContainer::setSize(size);

	buttonSize = size;
	buttonSize.height-=captionSize;
	
	button->setSize(buttonSize);
}

void PPMessageBoxContainer::setLocation(const PPPoint& location)
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
			lastCapturePoint = *reinterpret_cast<const PPPoint*>(event->getDataPtr());
			captured = isPointInCaption(lastCapturePoint);
			if (captured)
				return 0;
			break;

		case eLMouseDrag:
			if (handleMove(*reinterpret_cast<const PPPoint*>(event->getDataPtr())))
				return 0;
			break;

		case eLMouseUp:
			if (captured)
			{
				captured = false;
				return 0;
			}
			break;
		default:
			break;
	}
	
	return PPContainer::dispatchEvent(event);
}

bool PPMessageBoxContainer::isPointInCaption(const PPPoint& point)
{
	PPSimpleVector<PPControl>& controls = getControls();

	for (pp_int32 i = 0; i < controls.size(); i++)
		if (controls.get(i)->hit(point))
			return false;

	return hit(point);

	//return (point.x >= location.x && point.x <= location.x + size.width &&
	//		point.y >= location.y && point.y <= location.y + captionSize);
}

bool PPMessageBoxContainer::handleMove(PPPoint point)
{
	if (!captured)
		return false;

	const pp_int32 catchBound = 4;
	const pp_int32 dcatchBound = catchBound*2;
	
	const pp_int32 width = size.width;
	const pp_int32 height = size.height;

	bool ignorex = (point.x < -(width-dcatchBound)) || (point.x >= parentScreen->getWidth() + (width - dcatchBound));
	bool ignorey = (point.y < -(height-dcatchBound)) || (point.y >= parentScreen->getHeight() + (height - dcatchBound));;

	PPPoint delta(ignorex ? 0 : point.x - lastCapturePoint.x, ignorey ? 0 : point.y - lastCapturePoint.y);
	
	if (!ignorex)
		lastCapturePoint.x = point.x;
	if (!ignorey)
		lastCapturePoint.y = point.y;
	
	if (location.x + delta.x < -(width-dcatchBound))
	{
		delta.x += -(width-dcatchBound) - (location.x + delta.x);
	}

	if (location.y + delta.y < -(height-dcatchBound))
	{
		delta.y += -(height-dcatchBound) - (location.y + delta.y);
	}

	if (location.x + delta.x >= parentScreen->getWidth() - dcatchBound)
	{
		delta.x -= (location.x + delta.x) - (parentScreen->getWidth() - dcatchBound);
	}
	
	if (location.y + delta.y >= parentScreen->getHeight() - dcatchBound)
	{
		delta.y -= (location.y + delta.y) - (parentScreen->getHeight() - dcatchBound);
	}
	
	move(delta);
	
	PPPoint buttonPos = button->getLocation();
	buttonPos.x+=delta.x;
	buttonPos.y+=delta.y;
	button->setLocation(buttonPos);
	
	parentScreen->paint();
	
	return true;
}

