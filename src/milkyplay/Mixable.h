/*
 *  Mixable.h
 *  cliplayer
 *
 *  Created by Peter Barth on 14.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __MIXABLE_H__
#define __MIXABLE_H__

#include "MilkyPlayTypes.h"

struct Mixable
{
	virtual void mix(mp_sint32* buffer, mp_uint32 numSamples) = 0;			
};

#endif
