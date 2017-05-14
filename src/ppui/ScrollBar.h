/*
 *  ppui/ScrollBar.h
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

/////////////////////////////////////////////////////////////////
//
//	PPScrollbar control class
//
/////////////////////////////////////////////////////////////////
#ifndef SCROLLBAR__H
#define SCROLLBAR__H

//#ifndef __LOWRES__
	#define SCROLLBUTTONSIZE 10
//#else
//	#define SCROLLBUTTONSIZE 8
//#endif

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

// Forwards
class PPGraphicsAbstract;
class PPButton;

class PPScrollbar : public PPControl, public EventListenerInterface
{
private:
	const PPColor* backgroundColor;

	pp_uint32 oneDimSize;
	bool horizontal;

	PPButton* backgroundButton;
	PPButton* buttonUp;
	PPButton* buttonDown;
	PPButton* buttonBar;

	PPControl* caughtControl;
	bool controlCaughtByLMouseButton, controlCaughtByRMouseButton;
	PPPoint caughtMouseLocation, caughtControlLocation;

	pp_int32 currentBarSize, currentBarPosition;
	
	//bool pressed;
	void initButtons();

public:
	PPScrollbar(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
				const PPPoint& location, pp_int32 size, 
				bool horizontal = false);

	virtual ~PPScrollbar();	
	
	void setBackgroundColor(const PPColor& color) { backgroundColor = &color; }

	// set bar size [none:0 - full:65536]
	void setBarSize(pp_int32 size, bool repaint = false);
	// set bar position [0 - 65536]
	void setBarPosition(pp_int32 pos, bool repaint = false);

	pp_uint32 getBarSize() const { return currentBarSize; }
	pp_uint32 getBarPosition() const { return currentBarPosition; }

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 dispatchEvent(PPEvent* event);
	
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual void setLocation(const PPPoint& location);

	virtual void setSize(pp_uint32 size);
};

#endif
