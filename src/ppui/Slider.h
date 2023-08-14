/*
 *  ppui/Slider.h
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
//	PPSlider control class
//
/////////////////////////////////////////////////////////////////
#ifndef SLIDER__H
#define SLIDER__H

#define SLIDERBUTTONSIZE 12
#define SLIDERBUTTONHEIGHT 10
#define SLIDERBUTTONWIDTH 12

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

// Forwards
class PPGraphicsAbstract;
class PPButton;

class PPSlider : public PPControl, public EventListenerInterface
{
private:
	PPColor backgroundColor;

	pp_uint32 oneDimSize;
	bool horizontal;
	bool buttonSwap;

	PPButton* backgroundButton;
	PPButton* buttonUp;
	PPButton* buttonDown;
	PPButton* buttonBar;

	PPControl* caughtControl;
	bool controlCaughtByLMouseButton, controlCaughtByRMouseButton;
	PPPoint caughtMouseLocation, caughtControlLocation;

	pp_int32 currentBarSize, currentBarPosition;

	pp_int32 minValue;
	pp_int32 maxValue;
	pp_int32 currentValue;

	//bool pressed;

	void initButtons();

public:
	PPSlider(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
			 const PPPoint& location, pp_int32 size, 
			 bool horizontal = false, bool buttonSwap = false);

	virtual ~PPSlider();	

	virtual void paint(PPGraphicsAbstract* graphics);
	
	virtual pp_int32 dispatchEvent(PPEvent* event);
	
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual void setLocation(const PPPoint& location);
	
	virtual void setSize(pp_uint32 size);

	void setMinValue(pp_int32 min) { minValue = min; }
	void setMaxValue(pp_int32 max) { maxValue = max; }
	void setCurrentValue(pp_int32 newValue)
	{
		currentValue = newValue;
		float f = (float)(currentValue - minValue)/(maxValue - minValue);

		setBarPosition((pp_int32)(f*65536.0f));
	}
	pp_int32 getCurrentValue() { return currentValue; }

	void setBackgroundColor(const PPColor& color) { backgroundColor = color; }

	// set bar size [none:0 - full:65536]
	void setBarSize(pp_int32 size, bool repaint = false);
	// set bar position [0 - 65536]
	void setBarPosition(pp_int32 pos, bool repaint = false);

	pp_uint32 getBarSize() const { return currentBarSize; }
	pp_uint32 getBarPosition() const { return currentBarPosition; }
};

#endif
