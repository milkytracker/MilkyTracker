/*
 *  FXAbstract.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.11.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef FXABSTRACT__H
#define FXABSTRACT__H

#include "BasicTypes.h"

class FXAbstract
{
protected:
	pp_uint32 width, height;
	
public:
	FXAbstract(pp_uint32 w, pp_uint32 h) :
		width(w), height(h)
	{
	}
	virtual ~FXAbstract() {}
	
	virtual void update(pp_uint32 syncFrac) { }
	
	virtual void render(pp_uint8* buffer) = 0;
};

#endif

