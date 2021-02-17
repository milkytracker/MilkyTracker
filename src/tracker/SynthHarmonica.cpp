/*
 *  tracker/SynthHarmonica.cpp
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

#include "SynthHarmonica.h"
#include "Screen.h"
#include "GraphicsAbstract.h"
#include "Tools.h"
#include "DialogSynth.h"
#include "ControlIDs.h"

#include <math.h>

#define SH_PI 3.1415926535897932384626433832795f

SynthHarmonica::SynthHarmonica(pp_int32 id, 
									 PPScreen* parentScreen, 
									 EventListenerInterface* eventListener, 
									 const PPPoint& location, 
									 const PPSize& size,
									 DialogSynth* m) :
	PPControl(id, parentScreen, eventListener, location, size),
	borderColor(&ourOwnBorderColor),
	harmonics(64),
	magic(m)
{
	// default color
	color.set(0, 0, 0);
	
	ourOwnBorderColor.set(192, 192, 192);
	
	visibleWidth = size.width - 2;
	visibleHeight = size.height - 2;

	for(int i = 0; i < 128; i++) wave[i] = 0.0f;
}


SynthHarmonica::~SynthHarmonica()
{
}


void SynthHarmonica::paint(PPGraphicsAbstract* g)
{
	// Draw border
	g->setRect(location.x, location.y, location.x + size.width+1, location.y + size.height+1);
	drawThickBorder(g, *borderColor);

	// Draw bg
	PPRect drawingRect(location.x + 2, location.y + 2, location.x + size.width-2, location.y + size.height-2);	
	g->setRect(drawingRect);
	g->setColor(0, 0, 0);
	g->fill();

	// Draw bars
	for(int i = 0; i < harmonics; i++) {
		PPRect barRect = drawingRect;
		int barw = size.width / harmonics;

		barRect.x1+= i * barw;
		barRect.x2 = barRect.x1 + barw - 1;
		
		if(i&1) {
			g->setColor(0x22, 0x22, 0x22);	
		} else {
			g->setColor(0x33, 0x33, 0x33);	
		}
		g->fill(barRect);

		barRect.y1+= (1.0f - wave[i]) * drawingRect.height();
		
		g->setColor(0x00, 0xAA, 0xFF);	
		g->fill(barRect);
	}

}


pp_int32 SynthHarmonica::dispatchEvent(PPEvent* event)
{ 
	if (eventListener == NULL)
		return -1;

	switch (event->getID())
	{	
		case eLMouseDrag:
		case eRMouseDrag:
		case eLMouseDown:
		case eRMouseDown:
		{
			PPPoint *p = (PPPoint*)event->getDataPtr();
			int x = p->x - location.x,
				y = p->y - location.y;

			float i = (float)x / ((float)size.width / (float)harmonics);

			if(event->getID() == eRMouseDown || event->getID() == eRMouseDrag) {
				wave[(int)i] = 0.0f;
				parentScreen->paintControl(this);
			} else if(event->getID() == eLMouseDown || event->getID() == eLMouseDrag) {
				wave[(int)i] = ((float)size.height - (float)y) / (float)size.height;
				parentScreen->paintControl(this);
			}

			PPEvent e(eHarmonicaUpdated);
			magic->handleEvent(this, &e);

			break;
		}
	}

	return 0;
}


void SynthHarmonica::setHarmonics(pp_int32 h)
{
	harmonics = h;
	parentScreen->paintControl(this);
}