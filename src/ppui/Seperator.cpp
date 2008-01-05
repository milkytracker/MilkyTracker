/*
 *  PPSeperator.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.03.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "Seperator.h"
#include "GraphicsAbstract.h"

PPSeperator::PPSeperator(pp_int32 id, PPScreen* parentScreen, PPPoint location, pp_uint32 size, const PPColor& theColor, bool horizontal/* = true*/) :
	PPControl(id, parentScreen, NULL, location, PPSize(0,0)),
	horizontal(horizontal),
	color(&theColor)
{
	if (horizontal)
	{
		this->size.width = size;
		this->size.height = 2;
	}
	else
	{
		this->size.height = size;
		this->size.width = 2;
	}
}

void PPSeperator::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	g->setRect(location.x, location.y, location.x + size.width+1, location.y + size.height+1);

	//g->setColor(color);	
	
	PPColor bColor = *color, dColor = *color;

	// adjust dark color
	dColor.scaleFixed(20000);
	
	// adjust bright color
	bColor.scaleFixed(87163);
	
	if (horizontal)
	{
		g->setColor(dColor);
		g->drawHLine(location.x, location.x + size.width, location.y);
	}
	else
	{
		g->setColor(dColor);
		g->drawVLine(location.y, location.y + size.height, location.x);
	}
	
	if (horizontal)
	{
		g->setColor(bColor);
		g->drawHLine(location.x+1, location.x +1+ size.width, location.y+1);
	}
	else
	{
		g->setColor(bColor);
		g->drawVLine(location.y+1, location.y +1+ size.height, location.x+1);
	}
}
