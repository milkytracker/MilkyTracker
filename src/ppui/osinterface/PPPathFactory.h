/*
 *  ppui/osinterface/PPPathFactory.h
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
 *  PPPathFactory.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.11.06.
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
