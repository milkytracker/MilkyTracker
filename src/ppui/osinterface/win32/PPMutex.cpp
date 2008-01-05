/*
 *  PPMutex.cpp
 *  PPUI SDL
 *
 *  Created by Peter Barth on 18.11.05.
 *  Copyright 2005 milkytracker.net All rights reserved.
 *
 */

#include "PPMutex.h"

PPMutex::PPMutex()
{
	InitializeCriticalSection( &m_CS );
}

PPMutex::~PPMutex()
{
	DeleteCriticalSection( &m_CS );
}

void PPMutex::lock()
{
	EnterCriticalSection( &m_CS );
}

void PPMutex::unlock()
{
	LeaveCriticalSection( &m_CS );
}
