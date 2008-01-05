/*
 *  PPSystem_POSIX.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Thu Mar 10 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "PPSystem_POSIX.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

SYSCHAR System::buffer[PATH_MAX+1];

const SYSCHAR* System::getTempFileName()
{	
	// Although tmpnam(3) generates names that are difficult to guess, it is 
	// nevertheless possible that between the time that tmpnam(3) returns a pathname, 
	// and the time that the program opens it, another program might create that 
	// pathname using open(2), or create it as a symbolic link. This can lead to security holes. 
	// To avoid such possibilities, use the open(2) O_EXCL flag to open the pathname. 
	// Or better yet, use mkstemp(3) or tmpfile(3).
	tmpnam(buffer);
	// should not be the case, if it is the case, 
	// create something that "might" work out
	if (buffer == NULL)
	{
		strcpy(buffer, getenv("HOME"));	
		strcat(buffer, "/.milkytracker_temp");
	}
	return buffer;
}

const SYSCHAR* System::getConfigFileName()
{
	strcpy(buffer, getenv("HOME"));	
	strcat(buffer, "/.milkytracker_config");
	
	return buffer;
}

void System::msleep(int msecs)
{
	if (msecs < 0)
		return;
	usleep(msecs*1000);
}
