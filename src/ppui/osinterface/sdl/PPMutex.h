/*
 *  PPMutex.h
 *  PPUI SDL
 *
 *  Created by Peter Barth on 18.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef PPMUTEX__H
#define PPMUTEX__H

#include <SDL/SDL.h>

class PPMutex
{
private:
	SDL_mutex* mutex;
	
public:
	PPMutex();
	~PPMutex();
	
	void lock();
	void unlock();
};

#endif

