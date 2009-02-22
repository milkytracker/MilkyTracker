/*
 *  tracker/AnimatedFXControl.h
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
 *  AnimatedFXControl.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.10.05.
 *
 */

#ifndef ANIMATEDFXCONTROL__H
#define ANIMATEDFXCONTROL__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

class AnimatedFXControl : public PPControl
{
private:
	static pp_int32 counter;

	PPColor color;

	bool border;
	PPColor ourOwnBorderColor;
	const PPColor* borderColor;

	// extent
	pp_int32 visibleWidth;
	pp_int32 visibleHeight;

	class FXAbstract* fx;
	pp_uint8* vscreen;
	pp_int32 fxTicker;
	
	class PPFont* font;
	pp_int32 xPos, currentSpeed;
	pp_int32 currentCharIndex;
	pp_uint32 textBufferMaxChars;
	pp_uint32 lastTime;
	char* textBuffer;

	char milkyVersionString[100];

	void createFX();

public:
	AnimatedFXControl(pp_int32 id, 
					  PPScreen* parentScreen, 
					  EventListenerInterface* eventListener, 
					  const PPPoint& location, 
					  const PPSize& size, 
					  bool border = true);
	
	virtual ~AnimatedFXControl();

	void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { color.r = r; color.g = g; color.b = b; }
	void setColor(PPColor color) { this->color = color; }

	void setBorderColor(const PPColor& color) { this->borderColor = &color; }

	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);

	virtual pp_int32 dispatchEvent(PPEvent* event);
	
	virtual bool receiveTimerEvent() const { return true; }	
	
	virtual void show(bool bShow);
};


#endif
