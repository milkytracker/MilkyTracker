/*
 *  PPMutex.h
 *  PPUI SDL
 *
 *  Created by Peter Barth on 18.11.05.
 *  Copyright 2005 milkytracker.net All rights reserved.
 *
 */

#ifndef PPMUTEX__H
#define PPMUTEX__H

#include <windows.h>

class PPMutex
{
private:
	CRITICAL_SECTION m_CS;
	
public:
	PPMutex();
	~PPMutex();
	
	void lock();
	void unlock();
};

#endif

