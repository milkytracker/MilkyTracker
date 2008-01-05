/*
 *  PPMutex.cpp
 *  PPUI SDL
 *
 *  Created by Peter Barth on 18.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "PPMutex.h"

PPMutex::PPMutex()
{
	mutex = SDL_CreateMutex();
}

PPMutex::~PPMutex()
{
	SDL_DestroyMutex(mutex);
}

void PPMutex::lock()
{
	SDL_LockMutex(mutex);
}

void PPMutex::unlock()
{
	SDL_UnlockMutex(mutex);
}
