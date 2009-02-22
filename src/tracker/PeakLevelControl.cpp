/*
 *  tracker/PeakLevelControl.cpp
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
 *  PeakLevelControl.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.10.05.
 *
 */

#include "PeakLevelControl.h"
#include "Screen.h"
#include "GraphicsAbstract.h"
#include "TrackerConfig.h"

PeakLevelControl::PeakLevelControl(pp_int32 id, 
								   PPScreen* parentScreen, 
								   EventListenerInterface* eventListener, 
								   const PPPoint& location, const PPSize& size, 
								   bool border/*= true*/) :
	PPControl(id, parentScreen, eventListener, location, size),
	borderColor(&ourOwnBorderColor)
{
	this->border = border;

	// default color
	color.set(0, 0, 0);

	ourOwnBorderColor.set(192, 192, 192);

	visibleWidth = size.width - 2;
	visibleHeight = size.height - 2;
	
	peak[0] = peak[1] = 0;
	
	buildColorLUT();
}

PeakLevelControl::~PeakLevelControl()
{
}

void PeakLevelControl::paint(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;
	
	pp_int32 xOffset = 2;

	pp_int32 yOffset = 2;

	g->setRect(location.x, location.y, location.x + size.width, location.y + size.height);

	g->setColor(color);

	g->fill();

	if (border)
	{
		drawBorder(g, *borderColor);
	}

	g->setRect(location.x + 1, location.y + 1, location.x + size.width - 2, location.y + size.height - 2);

	pp_int32 centerx = location.x + xOffset + (visibleWidth >> 1)-1;

	pp_int32 i;

	pp_int32 pixelPeak = ((visibleWidth >> 1)*peak[0]) >> 16;

	pp_int32 maxPeak = (visibleWidth >> 1);

	for (i = 0; i < pixelPeak; i+=2)
	{
		pp_int32 c = i*256/maxPeak; 
		g->setColor(peakColorLUT[c][0], peakColorLUT[c][1], peakColorLUT[c][2]);
		g->drawVLine(location.y + yOffset, location.y + yOffset + visibleHeight, centerx - i);
	}

	pixelPeak = ((visibleWidth >> 1)*peak[1]) >> 16;
	for (i = 0; i < pixelPeak; i+=2)
	{
		pp_int32 c = i*256/maxPeak; 
		g->setColor(peakColorLUT[c][0], peakColorLUT[c][1], peakColorLUT[c][2]);
		g->drawVLine(location.y + yOffset, location.y + yOffset + visibleHeight, centerx + i);
	}
}

void PeakLevelControl::buildColorLUT()
{
	struct TColorKey
	{
		PPColor color;
		pp_uint32 t;
	};
	
	TColorKey colorKeys[3];
	
	colorKeys[2].color.r = 255;
	colorKeys[2].color.g = 0;
	colorKeys[2].color.b = 0;
	colorKeys[2].t = 224;

	colorKeys[1].color.r = 255;
	colorKeys[1].color.g = 255;
	colorKeys[1].color.b = 0;
	colorKeys[1].t = 192;

	colorKeys[0].color.r = 0;
	colorKeys[0].color.g = 255;
	colorKeys[0].color.b = 0;
	colorKeys[0].t = 0;


	for (pp_int32 i = 0; i < 2; i++)
	{
		
		for (pp_uint32 j = colorKeys[i].t; j < colorKeys[i+1].t; j++)
		{
			float t = (float)(j - colorKeys[i].t) / (float)(colorKeys[i+1].t - colorKeys[i].t);
			
			pp_int32 r = (pp_int32)((1.0f - t) * colorKeys[i].color.r + t * colorKeys[i+1].color.r);
			pp_int32 g = (pp_int32)((1.0f - t) * colorKeys[i].color.g + t * colorKeys[i+1].color.g);
			pp_int32 b = (pp_int32)((1.0f - t) * colorKeys[i].color.b + t * colorKeys[i+1].color.b);
		
			peakColorLUT[j][0] = (pp_uint8)r;
			peakColorLUT[j][1] = (pp_uint8)g;
			peakColorLUT[j][2] = (pp_uint8)b;
		}
		
	}

	for (pp_int32 j = colorKeys[2].t; j < 256; j++)
	{
		peakColorLUT[j][0] = (pp_uint8)colorKeys[2].color.r;
		peakColorLUT[j][1] = (pp_uint8)colorKeys[2].color.g;
		peakColorLUT[j][2] = (pp_uint8)colorKeys[2].color.b;
	}
}
