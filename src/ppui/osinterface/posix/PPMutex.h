/*
 *  PPMutex.h
 *  PPUI SDL
 *
 *  Created by Peter Barth on 18.11.05.
 *  Copyright 2005 milkytracker.net. All rights reserved.
 *
 */

#ifndef PPMUTEX__H
#define PPMUTEX__H

#include <pthread.h>

class PPMutex
{
private:
	pthread_mutex_t* mutexpth_p;

public:
	PPMutex();
	~PPMutex();
	
	void lock();
	bool tryLock();
	void unlock();
};

#endif

