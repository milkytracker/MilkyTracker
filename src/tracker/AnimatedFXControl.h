/*
 *  AnimatedFXControl.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.10.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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
					 PPPoint location, 
					 PPSize size, 
					 bool border = true);

	~AnimatedFXControl();

	void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { color.r = r; color.g = g; color.b = b; }
	void setColor(PPColor color) { this->color = color; }

	void setBorderColor(const PPColor& color) { this->borderColor = &color; }

	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);

	virtual pp_int32 callEventListener(PPEvent* event);
	
	virtual bool receiveTimerEvent() { return true; }	
	
	virtual void show(bool bShow);
};


#endif
