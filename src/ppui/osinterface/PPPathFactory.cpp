/*
 *  ppui/osinterface/PPPathFactory.cpp
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
 *  PPPathFactory.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.11.06.
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
