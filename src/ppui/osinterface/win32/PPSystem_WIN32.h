/*
 *  System_WIN32.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Thu Mar 10 2005.
 *  Copyright (c) 2005 milkytracker.net All rights reserved.
 *
 */

#ifndef SYSTEM_WIN32_H
#define SYSTEM_WIN32_H

#include "XModule.h"

class System
{
private:
	static SYSCHAR buffer[];

public:
	static const SYSCHAR* getTempFileName();

	static const SYSCHAR* getConfigFileName(SYSCHAR* fileName = NULL);

	static void msleep(int msecs);
};

#endif
