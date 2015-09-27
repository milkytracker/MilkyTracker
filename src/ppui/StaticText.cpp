/*
 *  ppui/StaticText.cpp
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

#include "StaticText.h"
#include "Event.h"
#include "GraphicsAbstract.h"
#include "Font.h"
#include "Tools.h"
#include "PPUIConfig.h"

PPStaticText::PPStaticText(pp_int32 id,	PPScreen* parentScreen, EventListenerInterface* eventListener, 
						   const PPPoint& location, 
						   const PPString& text, 
						   bool drawShadow /*= false*/, 
						   bool drawUnderlined /*= false*/,
						   bool autoShrink /*= false*/) :
	PPControl(id, parentScreen, eventListener, location, PPSize(0,0)),
	color(&PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText)),
	drawShadow(drawShadow),
	underlined(drawUnderlined),
	autoShrink(autoShrink),
	shadowColor(0,0,0),
	text(text),
	extent(-1, -1)
{
	font = PPFont::getFont(PPFont::FONT_SYSTEM);		

	calcExtent();
}

PPStaticText::~PPStaticText()
{
}

void PPStaticText::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	g->setFont(font);

	if (extent.width != -1 && extent.height != -1)
		g->setRect(location.x, location.y, location.x + extent.width, location.y + extent.height+1);
	else
		g->setRect(location.x, location.y, location.x + size.width, location.y + size.height+1);

	const char* text = this->text;
	
	char* temp = NULL;
	if (autoShrink && (signed)font->getStrWidth(text) > extent.width)
	{
		pp_uint32 numchars = extent.width / font->getCharWidth();
		
		temp = new char[numchars+1];
		
		pp_uint32 i;
		for (i = 0; i < (numchars / 2); i++)
			temp[i] = text[i];
			
		temp[i] = '\xef';
	
		pp_uint32 j = i+1;
		for (i = strlen(text)-(numchars / 2); i < strlen(text); i++, j++)
			temp[j] = text[i];
			
		temp[j] = '\0';
		
		text = temp;
	}

	if (drawShadow)
	{
		if (enabled)
			g->setColor(shadowColor);
		else
		{
			PPColor col(shadowColor);
			col.r+=color->r;
			col.g+=color->g;
			col.b+=color->b;
			col.scaleFixed(8192);
			g->setColor(col);
		}

		g->drawString(text, location.x+1, location.y+1, underlined);
	}
	
	if (enabled)
		g->setColor(*color);
	else
	{
		PPColor col(*color);
		col.scaleFixed(40000);
		g->setColor(col);
	}

	g->drawString(text, location.x, location.y, underlined);
	
	if (temp)
		delete[] temp;
}

pp_int32 PPStaticText::dispatchEvent(PPEvent* event)
{ 
	if (!eventListener)
		return -1;

	switch (event->getID())
	{
		case eLMouseDown:
			if (hit(*reinterpret_cast<const PPPoint*>(event->getDataPtr())))
			{
				PPEvent e(eCommand);
				return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e); 
			}
			return 0;
        default:
            return -1;
	}
}

void PPStaticText::setText(const PPString& text)
{
	this->text = text;

	calcExtent();
}

void PPStaticText::setIntValue(pp_int32 value, pp_uint32 numDecDigits/* = 0*/, bool negative/*= false*/)
{
	
	/*char text[15];	

	_itoa(value, text, 10);
	
	setText(text);*/

	if (numDecDigits == 0)
	{
		if (negative)
		{
			numDecDigits = PPTools::getDecNumDigits(abs(value));
			numDecDigits++;
		}
		else
		{
			numDecDigits = PPTools::getDecNumDigits(value);
		}
	}

	// enough memory to hold 32bit Dec value
	char Dec[15];

	if (negative)
	{
		PPTools::convertToDec(Dec+1, abs(value), numDecDigits-1);
		if (value < 0)
			Dec[0] = '-';
		else	
			Dec[0] = '+';
	}
	else
	{
		PPTools::convertToDec(Dec, value, numDecDigits);
	}

	setText(Dec);
}

void PPStaticText::setValue(pp_int32 value, bool hex, pp_uint32 numDigits/* = 0*/, bool negative/* = false*/)
{
	if (hex)
		setHexValue(value, numDigits);
	else
		setIntValue(value, numDigits, negative);
}

void PPStaticText::setHexValue(pp_int32 value, pp_uint32 numHexDigits/*= 0*/)
{
	if (numHexDigits == 0)
		numHexDigits = PPTools::getHexNumDigits(value);

	// enough memory to hold 32bit hex value
	char hex[10];

	PPTools::convertToHex(hex, value, numHexDigits);

	setText(hex);
}

void PPStaticText::calcExtent()
{	
	size.height = font->getCharHeight()*text.countLines() + (drawShadow?1:0);
	size.width = font->getStrWidth(text) + (drawShadow?1:0);	
}
