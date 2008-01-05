/*
 *  Fire.h
 *  MyFirstCarbonProject
 *
 *  Created by Peter Barth on Wed Feb 20 2002.
 *  Copyright (c) 2001 milkytracker.net, All rights reserved.
 *
 */

#ifndef STARFIELD__H
#define STARFIELD__H

#include "BasicTypes.h"
#include "FXAbstract.h"
#include "Math3d.h"

class Starfield : public FXAbstract
{
private:
	VectorFP* stars;

	const pp_int32 numStars;

public:
	Starfield(pp_int32 w, pp_int32 h);
	virtual ~Starfield();

	virtual void render(pp_uint8* buffer);

	virtual void update(pp_uint32 syncFrac);
	
	void setPixel(pp_int32 x, pp_int32 y, const PPColor& color, pp_uint8* buffer);
};

#endif
