/*
 *  PPPathFactory.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.11.06.
 *  Copyright 2006 milkytracker.net. All rights reserved.
 *
 */

#ifndef __PPPATHFACTORY_POSIX_H__
#define __PPPATHFACTORY_POSIX_H__

#include "PPPath.h"

class PPPathFactory
{
private:
	PPPathFactory() { }

public:
	static PPPathEntry* createPathEntry();

	static PPPath* createPath();
	static PPPath* createPathFromString(const PPSystemString& path);
};

#endif	// __PPPATH_POSIX_H__
