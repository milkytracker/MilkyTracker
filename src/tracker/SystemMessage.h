/*
 *  SystemMessage.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 27.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __SYSTEMMESSAGE_H__
#define __SYSTEMMESSAGE_H__

#include "BasicTypes.h"

class SystemMessage
{
private:
	class PPScreen& screen;
	pp_uint32 message;

public:
	enum Messages
	{
		MessageSoundDriverInitFailed,
		MessageFullScreenFailed,
		MessageResChangeRestart,
		MessageLimitedInput
	};
	
	SystemMessage(PPScreen& screen, Messages message);
	
	void show();
};

#endif
