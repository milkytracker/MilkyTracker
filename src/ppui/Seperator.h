/*
 *  Seperator.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.03.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */
#ifndef SEPERATOR__H
#define SEPERATOR__H

#include "BasicTypes.h"
#include "Control.h"

class PPSeperator : public PPControl
{
private:
	bool horizontal;
	
	const PPColor* color;	
	
public:
	PPSeperator(pp_int32 id, PPScreen* parentScreen, PPPoint location, pp_uint32 size, const PPColor& theColor, bool horizontal = true);

	void setColor(const PPColor& color) { this->color = &color; }

	virtual void paint(PPGraphicsAbstract* graphics);

	virtual bool gainsFocus() { return false; };

	virtual bool isActive() { return true; }
};

#endif
