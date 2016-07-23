/*
 *  ppui/osinterface/posix/PPSystem_POSIX.cpp
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
 *  PPSystem_POSIX.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Thu Mar 10 2005.
 *
 */

#include "PPSystem_POSIX.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __PSP__
#include <pspkernel.h>
#include <sys/syslimits.h>
#endif

#ifdef __AMIGA__
#include <sys/syslimits.h>
#endif

#ifdef __HAIKU__
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <Path.h>
#endif

SYSCHAR System::buffer[PATH_MAX+1];

const SYSCHAR* System::getTempFileName()
{
	// Suppressed warning: "'tmpnam' is deprecated: This function is provided for
	// compatibility reasons only. Due to security concerns inherent in the
	// design of tmpnam(3), it is highly recommended that you use mkstemp(3)
	// instead."

	// Note: Replacing tmpnam() with mkstemp() requires modifying the module
	// load, export and decompressor functions to accept a file handle (XMFILE)
	// instead of a file name.
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	if ((tmpnam(buffer) == NULL))
#pragma clang diagnostic pop
	{
		// should not be the case, if it is the case, create something that
		// "might" work out
		char *home = getenv("HOME");
		if(home)
		{
			strcpy(buffer, home);
			strcat(buffer, "/.milkytracker_temp");
		}
		else
			strcpy(buffer, "milkytracker_temp");
	}
	return buffer;
}

const SYSCHAR* System::getConfigFileName()
{
#ifdef __HAIKU__
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append("MilkyTracker");
	
	BEntry dirEntry(path.Path());
	if (!dirEntry.Exists()) {
		// MilkyTracker settings dir doesn't exist, create it
		BDirectory temp;
		temp.CreateDirectory(path.Path(), NULL);
	}
	
	path.Append("milkytracker_config");
	strcpy(buffer, path.Path());	
	return buffer;
#endif
	char *home = getenv("HOME");
	if(!home)
	{
		// If $HOME isn't set, save in the current dir
		strncpy(buffer, "milkytracker_config", PATH_MAX);
		return buffer;
	}
	// Old location was in the home directory
	char oldLoc[PATH_MAX];
	strncpy(oldLoc, home, PATH_MAX);
	strncat(oldLoc, "/.milkytracker_config", PATH_MAX);
	// New location based on xdg basedir spec
	char *xdg_config_home = getenv("XDG_CONFIG_HOME");
	if(xdg_config_home)
		strncpy(buffer, xdg_config_home, PATH_MAX);
	else
	{
		strncpy(buffer, home, PATH_MAX);
		strncat(buffer, "/.config", PATH_MAX);
	}
	mkdir(buffer, S_IRWXU);
	strncat(buffer, "/milkytracker", PATH_MAX);
	mkdir(buffer, S_IRWXU);
	strncat(buffer, "/config", PATH_MAX);
	// Move possible existing config into new location if not already present
	if(home && access(oldLoc, F_OK) == 0 && access(buffer, F_OK) != 0)
		rename(oldLoc, buffer);
	return buffer;
}

void System::msleep(int msecs)
{
	if (msecs < 0)
		return;
#ifdef __PSP__
	sceKernelDelayThreadCB(msecs*1000);
#elif defined(__AROS__)
	// usleep is not implemented on AROS
	sleep(msecs < 1000 ? 1 : msecs/1000);
#else
	usleep(msecs*1000);
#endif
}
