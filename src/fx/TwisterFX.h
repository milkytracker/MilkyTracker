/*
 *  TwisterFX.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 26.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef TWISTERFX__H
#define TWISTERFX__H

#include "BasicTypes.h"
#include "FXAbstract.h"

class FXInterface;

class TwisterFX : public FXAbstract
{
private:
	FXInterface* fx;
	pp_uint16* vscreen;

	pp_uint32 realw, realh;

public:
	TwisterFX(pp_int32 w, pp_int32 h);
	~TwisterFX();
	
	virtual void update(pp_uint32 syncFrac);
	
	virtual void render(pp_uint8* buffer);
};

#endif
