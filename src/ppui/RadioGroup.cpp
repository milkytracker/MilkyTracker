/*
 *  ppui/RadioGroup.cpp
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

#include "RadioGroup.h"
#include "Event.h"
#include "GraphicsAbstract.h"
#include "Screen.h"
#include "Font.h"
#include "PPUIConfig.h"

PPRadioGroup::PPRadioGroup(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
	const PPPoint& location, const PPSize& size, pp_uint32 spacerHeight /* = 4 */) :
	PPControl(id, parentScreen, eventListener, location, size),
	radioButtonColor(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorRadioGroupButton)),
	textColor(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText)),
	spacerHeight(spacerHeight),
	choice(0),
	horizontal(false),
	maxWidth(0)
{
	font = PPFont::getFont(PPFont::FONT_SYSTEM);
}

PPRadioGroup::~PPRadioGroup()
{
}

void PPRadioGroup::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	pp_int32 i,j,k;

	g->setRect(PPRect(location.x, location.y, location.x+size.width, location.y+size.height));

	g->setFont(font);

	PPColor dark, ldark, bright, normal;
	
	normal = bright = ldark = dark = *radioButtonColor;
	bright.scale(1.25f);
	ldark.scale(0.75f);
	dark.scale(0.125f);

	if (!enabled)
	{
		normal.scaleFixed(40000);
		bright.scaleFixed(40000);
		ldark.scaleFixed(40000);
		dark.scaleFixed(40000);
	}

	PPColor textColor = *(this->textColor);		
	if (!enabled)
		textColor.scaleFixed(40000);		

	pp_uint32 spacerHeight = (font->getCharHeight()) + this->spacerHeight;

	pp_uint32 spacerWidth = maxWidth + DefaultRadioWidth;

	for (i = 0; i < items.size(); i++)
	{

		pp_int32 px = location.x + 2 + (horizontal ? i*spacerWidth : 0);
		pp_int32 py = location.y + 2 + (!horizontal ? (i*spacerHeight) : 0) + (spacerHeight>>1);

		if (i == (signed)choice)
		{

			for (j = 0; j < 5; j++)
			{
				
				g->setColor(ldark);
				for (k = px+j; k <= px+8-j; k++)
					g->setPixel(k, py-j);
				
				g->setColor(dark);
				g->setPixel(px+j, py-j-1);
				g->setPixel(px+8-j, py-j-1);
				
				g->setPixel(px+j, py-j);
				g->setPixel(px+8-j, py-j);
			}
			
			for (j = 0; j < 5; j++)
			{
				
				g->setColor(ldark);
				for (k = px+j; k <= px+8-j; k++)
					g->setPixel(k, py+j);
				
				g->setColor(normal);
				g->setPixel(px+j, py+j);
				g->setPixel(px+j-1, py+j);
				
				g->setPixel(px+8-j, py+j+1);
				g->setPixel(px+8-j+1, py+j+1);
			}

			
			for (j = 0; j < 3; j++)
			{
				
				g->setColor(textColor);
				for (k = px+j; k <= px+4-j; k++)
					g->setPixel(k+2, py-j+1);
			}
			
			for (j = 0; j < 3; j++)
			{
				
				g->setColor(textColor);
				for (k = px+j; k <= px+4-j; k++)
					g->setPixel(k+2, py+j+1);
				
			}

		}
		else
		{
			for (j = 0; j < 5; j++)
			{
				
				g->setColor(normal);
				for (k = px+j; k <= px+8-j; k++)
					g->setPixel(k, py-j);
				
				g->setColor(bright);
				g->setPixel(px+j, py-j-1);
				g->setPixel(px+8-j, py-j-1);
			}
			
			for (j = 0; j < 5; j++)
			{
				
				g->setColor(normal);
				for (k = px+j; k <= px+8-j; k++)
					g->setPixel(k, py+j);
				
				g->setColor(dark);
				g->setPixel(px+j, py+j+1);
				g->setPixel(px+j-1, py+j+1);
				
				g->setPixel(px+8-j, py+j+1);
				g->setPixel(px+8-j+1, py+j+1);
			}

		}
		
		g->setColor(0, 0, 0);
		g->drawString(*items.get(i), px + 12 + 1, py - (font->getCharHeight()>>1)+1 + 1);

		g->setColor(textColor);

		g->drawString(*items.get(i), px + 12, py - (font->getCharHeight()>>1)+1);

	}

}

pp_int32 PPRadioGroup::dispatchEvent(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	//if (!visible)
	//	return 0;

	switch (event->getID())
	{
		case eLMouseDown:
		{
			PPPoint* p = (PPPoint*)event->getDataPtr();
		
			p->y -= location.y + 2;
			p->x -= location.x + 2;

			if (p->y < 0 || p->x < 0)
				break;

			pp_uint32 spacerHeight = (font->getCharHeight()) + this->spacerHeight;
			pp_uint32 spacerWidth = maxWidth + DefaultRadioWidth;

			pp_int32 index = -1;
			if (!horizontal)
				index = p->y / spacerHeight;
			else
				index = p->x / spacerWidth;

			if (index >= items.size() || index < 0)
				break;

			pp_int32 size = font->getStrWidth(*items.get(index)) + 12;

			if ((p->x % maxWidth) >= size)
				break;

			choice = index;
			
			PPEvent e(eSelection, &choice, sizeof(choice));
			eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);

			parentScreen->paintControl(this);
			break;
		}
		case eLMouseUp:
			parentScreen->paintControl(this);
			break;
		default:
			break;
	}	

	return 0;
	//return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), event); 
}

void PPRadioGroup::addItem(const PPString& item)
{
	items.add(new PPString(item));

	pp_int32 width = font->getStrWidth(item);

	if (width > maxWidth)
		maxWidth = width;
}

const PPString& PPRadioGroup::getItem(pp_int32 index) const
{
	return *items.get((index >= 0 && index < items.size()) ? index : 0);
}

void PPRadioGroup::fitSize()
{
	size.height = items.size()*((font->getCharHeight()) + this->spacerHeight + 1);
	size.width = maxWidth + DefaultRadioWidth;
}
