/*
 *  PPSystem_POSIX.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Thu Mar 10 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SYSTEM_POSIX_H
#define SYSTEM_POSIX_H

#include "../../../milkyplay/MilkyPlayCommon.h"

class System
{
private:
	static SYSCHAR buffer[];

public:
	static const SYSCHAR* getTempFileName();

	static const SYSCHAR* getConfigFileName();

	static void msleep(int msecs);
};

#endif

