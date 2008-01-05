/*
 *  PPPathFactory.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.11.06.
 *  Copyright 2006 milkytracker.net. All rights reserved.
 *
 */

#include "PPPathFactory.h"
#include "BasicTypes.h"

#if !defined(__PPUI_WINDOWS__)

#include "PPPath_POSIX.h"

PPPathEntry* PPPathFactory::createPathEntry()
{
	return new PPPathEntry_POSIX();
}

PPPath* PPPathFactory::createPath()
{
	return new PPPath_POSIX();
}

PPPath* PPPathFactory::createPathFromString(const PPSystemString& path)
{
	return new PPPath_POSIX(path);
}

#else

#include "PPPath_WIN32.h"

PPPathEntry* PPPathFactory::createPathEntry()
{
	return new PPPathEntry_WIN32();
}

PPPath* PPPathFactory::createPath()
{
	return new PPPath_WIN32();
}

PPPath* PPPathFactory::createPathFromString(const PPSystemString& path)
{
	return new PPPath_WIN32(path);
}

#endif
