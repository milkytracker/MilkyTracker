/*
 *  PPMutex.cpp
 *  PPUI SDL
 *
 *  Created by Peter Barth on 18.11.05.
 *  Copyright 2005 milkytracker.net. All rights reserved.
 *
 */

#include "PPMutex.h"
#include <errno.h>

PPMutex::PPMutex() :
	mutexpth_p(NULL)
{
	mutexpth_p = new pthread_mutex_t;
	if (pthread_mutex_init(mutexpth_p, NULL)) 
	{
		delete mutexpth_p;
		mutexpth_p = NULL;
	}
}

PPMutex::~PPMutex()
{
	delete mutexpth_p;
}

void PPMutex::lock()
{
	if (mutexpth_p)
	{
		pthread_mutex_lock(mutexpth_p);
	}
}

bool PPMutex::tryLock()
{
	if (mutexpth_p)
	{
		if (pthread_mutex_trylock(mutexpth_p) == EBUSY)
			return false;	
	}
	
	return true;
}

void PPMutex::unlock()
{
	if (mutexpth_p)
	{
		pthread_mutex_unlock(mutexpth_p);
	}
}
