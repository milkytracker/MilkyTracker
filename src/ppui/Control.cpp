/*
 *  ppui/Control.cpp
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

#include "Control.h"
#include "Event.h"
#include "Screen.h"
#include "GraphicsAbstract.h"

//////////////////////////////////////////////////////////////////////////////
// PPControl construction
// ------------------------------------------------------------------------
// In: Parent screen object
// ------------------------------------------------------------------------
// Note: All controls are placed within a valid screen object.
//		 The screen is responsible for drawing the controls, if a control
//       needs to repaint itself it must tell the parent screen to draw it
//////////////////////////////////////////////////////////////////////////////
PPControl::PPControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener /* = NULL */) :
	id(id),
	parentScreen(parentScreen),
	eventListener(eventListener),
	ownerControl(NULL),
	location(0,0),
	size(0,0),
	visible(true),
	enabled(true),
	hasFocus(false)
{
}

PPControl::PPControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, const PPPoint& location, const PPSize& size) :
	id(id),
	parentScreen(parentScreen),
	eventListener(eventListener),
	ownerControl(NULL),
	location(location),
	size(size),
	visible(true),
	enabled(true),
	hasFocus(false)
{
}

pp_int32 PPControl::dispatchEvent(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), event); 
}

bool PPControl::hit(const PPPoint& p) const
{
	if (!visible)
		return false;

	return ((p.x >= location.x && p.x < location.x + size.width) &&
			(p.y >= location.y && p.y < location.y + size.height));
}

void PPControl::notifyChanges()
{
	PPEvent e(eUpdateChanged);
	eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
}

void PPControl::drawBorder(PPGraphicsAbstract* g, const PPColor& borderColor)
{
	PPColor bColor = borderColor, dColor = borderColor;
	// adjust dark color
	dColor.scaleFixed(32768);

	// adjust bright color
	bColor.scaleFixed(87163);

	g->setColor(dColor);
	
	g->drawHLine(location.x, location.x + size.width, location.y);
	g->drawVLine(location.y, location.y + size.height, location.x);
	
	g->setColor(bColor);
	
	g->drawHLine(location.x, location.x + size.width, location.y + size.height - 1);
	g->drawVLine(location.y, location.y + size.height, location.x + size.width - 1);
}

void PPControl::drawThickBorder(PPGraphicsAbstract* g, const PPColor& borderColor)
{
	PPColor bColor = borderColor, dColor = borderColor;
	// adjust dark color
	dColor.scaleFixed(32768);

	// adjust bright color
	bColor.scaleFixed(87163);

	g->setColor(borderColor);
	g->drawHLine(location.x, location.x + size.width, location.y);
	g->drawVLine(location.y, location.y + size.height, location.x);
	g->drawHLine(location.x, location.x + size.width, location.y + size.height - 1);
	g->drawVLine(location.y, location.y + size.height, location.x + size.width - 1);
	
	g->setColor(dColor);
	
	g->drawHLine(location.x+1, location.x + size.width-2, location.y+1);
	g->drawVLine(location.y+1, location.y + size.height-2, location.x+1);
	
	g->setColor(bColor);
	
	g->drawHLine(location.x+1, location.x + size.width-1, location.y + size.height - 2);
	g->drawVLine(location.y+1, location.y + size.height-2, location.x + size.width - 2);
}

bool PPControl::isVisible() const
{
	bool visible = this->visible;

	if (ownerControl)
		visible = visible && ownerControl->isVisible();
	
	return visible;
}

