/*
 *  ppui/Menu.cpp
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
 *  PPMenu.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Fri Mar 04 2005.
 *
 */

#include "Menu.h"
#include "Font.h"
#include "GraphicsAbstract.h"

// ------------- menu --------------------------------------------
PPMenu::PPMenu(PPFont* aFont, const PPColor& selectionColor, const PPColor& bgColor, bool bSubMenu/* = false*/) :
	font(aFont),
	backColor(&bgColor),
	borderColor(&selectionColor),
	selectionColor(&selectionColor),
	textBrightColor(NULL),
	textDarkColor(NULL),
	subMenu(bSubMenu),
	parentMenu(NULL),
  striped(false)
//borderColor(128, 128, 128)
{
}

pp_uint32 PPMenu::getMaxWidth() const
{
	pp_uint32 maxWidth = 0;

	for (pp_int32 i = 0; i <  items.size(); i++)
	{
		pp_uint32 len = items.get(i)->name.length();
		if (len > maxWidth)
			maxWidth = len;
	}

	return maxWidth;
}

pp_uint32 PPMenu::getEntryHeight() const
{
#ifndef __LOWRES__
	return font->getCharHeight()+2;
#else
	return font->getCharHeight();
#endif
}

PPRect PPMenu::getBoundingRect() const
{
	PPRect r;
	r.x1 = 0;
	r.y1 = 0;
	r.y2 = items.size()*getEntryHeight()+4;
	r.x2 = getMaxWidth()*font->getCharWidth()+5;
	return r;
}

bool PPMenu::setState(pp_int32 theId, pp_uint32 newState)
{
	bool b = false;
	for (pp_int32 i = 0; i < items.size(); i++)
	{
		if (items.get(i)->identifier == theId)
		{
			items.get(i)->state = newState;
			b = true;
		}
	}
	
	return b;
}

void PPMenu::paint(PPGraphicsAbstract* g, pp_int32 px, pp_int32 py, pp_int32 menuSelection)
{
	pp_int32 i;
	
	PPRect r = getBoundingRect();

#ifndef __LOWRES__
	const pp_int32 yOffset = 1;
#else
	const pp_int32 yOffset = 0;
#endif

	{
		PPColor color = *backColor;
	
		g->setRect(px, py, px + r.x2 + 2, py + r.y2 + 2);
		
		g->setColor(color);
	
		static pp_uint8 pattern[] = {224, 210, 198, 210, 224};
					
		for (pp_int32 i = py; i < py + r.y2; i++)
		{
			pp_int32 c = (pattern[(i-py)%5]-64);
			if( striped ) g->setColor(c,c,c);
			g->drawHLine(px, px + r.x2, i);
		}
		
		// border
		
		color = *selectionColor;
		PPColor bColor = color;
		// adjust bright color
		bColor.scaleFixed(87163);
		PPPoint location(px,py);
		PPSize size(r.x2, r.y2);
		
		g->setColor(color);
		
		g->drawHLine(location.x, location.x + size.width, location.y);
		g->drawVLine(location.y, location.y + size.height, location.x);

		// adjust dark color
		bColor = color;
		bColor.scaleFixed(20000);
		
		g->setColor(bColor);
		
		g->drawHLine(location.x, location.x + size.width, location.y + size.height - 1);
		g->drawVLine(location.y, location.y + size.height, location.x + size.width - 1);

		g->setColor(0,0,0);
		g->drawHLine(location.x+1, location.x + size.width+1, location.y + size.height);
		g->drawVLine(location.y+1, location.y + size.height+1, location.x + size.width);
		g->drawHLine(location.x+1, location.x + size.width+2, location.y + size.height+1);
		g->drawVLine(location.y+1, location.y + size.height+2, location.x + size.width+1);

		
	}

	g->setFont(font);
	
	pp_int32 they = 0;
	for (i = 0; i < items.size(); i++)
	{
		if (i == menuSelection)
		{
			PPColor color = *selectionColor;
			PPColor nsdColor = color, nsbColor = color;
			
			nsdColor.scaleFixed(45000);
			
			// adjust bright color
			nsbColor.scaleFixed(100000);
			
			pp_int32 y = py + 2 + i*(getEntryHeight())  - 1;
			pp_int32 h = (getEntryHeight()+1);
			PPRect rect(px+1, y, px + 1 + r.x2 - 3, y + h/2);
			g->fillVerticalShaded(rect, nsbColor, nsdColor, true);
			rect.y1 += h/2;
			rect.y2 += h/2 + 1;
			g->fillVerticalShaded(rect, nsbColor, nsdColor, false);
						
			g->setColor(*textBrightColor);
		}
		else
			g->setColor(*textDarkColor);

		if (items.get(i)->state & 1)
		{
			g->setColor(*textDarkColor);
			g->drawString(items.get(i)->name, px + 2, py + 2 + they+yOffset);
		}
		else
		{
			PPColor curCol = g->getColor();
			g->setColor((textBrightColor->r-curCol.r)>>1, (textBrightColor->g-curCol.g)>>1, (textBrightColor->b-curCol.b)>>1);
			g->drawString(items.get(i)->name, px + 3, py + 3 + they+yOffset);
			g->setColor(curCol);
			g->drawString(items.get(i)->name, px + 2, py + 2 + they+yOffset);
		}
		
		they+=getEntryHeight();		
	}
}
