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

PPMessageBoxContainer::PPMessageBoxContainer(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, PPPoint location, PPSize size, const PPString& caption) :
	PPContainer(id, parentScreen, eventListener, location, size),
	caption(caption)		
{
	PPContainer::setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorMessageBoxContainer));	

	button = new PPButton(-1, NULL, NULL, location, size, false, false, false);
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
	
	g->setRect(location.x, location.y, location.x + size.width, location.y + size.height);

	//g->fill();
	button->paint(g);

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

	g->drawHLine(location.x + 2, location.x + size.width - 3, location.y + 2 + 2*2 + font->getCharHeight());

	g->setColor(bColor);

	g->drawHLine(location.x + 3, location.x + size.width - 2, location.y + 2 + 2*2 + font->getCharHeight() + 1);

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
	
	button->setSize(size);
}

void PPMessageBoxContainer::setLocation(PPPoint location)
{
	PPContainer::setLocation(location);

	button->setLocation(location);
}
