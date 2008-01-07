/*
 *  milkyplay/drivers/generic/rtaudio/asio/asiodrivers.h
 *
 *  Copyright 2008 Peter Barth
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

#ifndef __AsioDrivers__
#define __AsioDrivers__

#include "ginclude.h"

#if MAC
#include "CodeFragments.hpp"

class AsioDrivers : public CodeFragments

#elif WINDOWS
#include <windows.h>
#include "asiolist.h"

class AsioDrivers : public AsioDriverList

#elif SGI || BEOS
#include "asiolist.h"

class AsioDrivers : public AsioDriverList

#else
#error implement me
#endif

{
public:
	AsioDrivers();
	~AsioDrivers();
	
	bool getCurrentDriverName(char *name);
	long getDriverNames(char **names, long maxDrivers);
	bool loadDriver(char *name);
	void removeCurrentDriver();
	long getCurrentDriverIndex() {return curIndex;}
protected:
	unsigned long connID;
	long curIndex;
};

#endif
