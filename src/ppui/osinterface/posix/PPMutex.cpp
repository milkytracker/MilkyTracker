/*
 *  ppui/osinterface/posix/PPMutex.cpp
 *
 *  Copyright 2009 Peter Barth
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *  PPMutex.cpp
 *  PPUI SDL
 *
 *  Created by Peter Barth on 18.11.05.
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
