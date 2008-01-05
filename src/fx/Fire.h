/*
 *  Fire.h
 *  MyFirstCarbonProject
 *
 *  Created by Peter Barth on Wed Feb 20 2002.
 *  Copyright (c) 2001 milkytracker.net, All rights reserved.
 *
 */

#ifndef FIRE__H
#define FIRE__H

#include "BasicTypes.h"
#include "FXAbstract.h"

class Fire : public FXAbstract
{
private:
	pp_uint16* workbuffer;
	pp_uint8 colorLUT[256][3];

	void buildColorLUT();

public:
	Fire(pp_int32 w, pp_int32 h);
	virtual ~Fire();

	virtual void render(pp_uint8* buffer);
};

#endif
