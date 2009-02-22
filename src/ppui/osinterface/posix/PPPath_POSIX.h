/*
 *  ppui/osinterface/posix/PPPath_POSIX.h
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
 *  PPPath_POSIX.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.03.06.
 *
 */

#ifndef __PPPATH_POSIX_H__
#define __PPPATH_POSIX_H__

#include "PPPath.h"
#include <unistd.h>
#include <dirent.h>

class PPPathEntry_POSIX : public PPPathEntry
{
public:
	PPPathEntry_POSIX() { }

	virtual void create(const PPSystemString& path, const PPSystemString& name);

	virtual bool isHidden() const;	
};

class PPPath_POSIX : public PPPath
{
protected:
	// POSIX
	DIR* dir;

	PPSystemString current;
	PPPathEntry_POSIX entry;

	virtual bool updatePath();

public:
	PPPath_POSIX();
	PPPath_POSIX(const PPSystemString& path);
	virtual ~PPPath_POSIX() {}

	virtual const PPSystemString getCurrent();
	
	virtual bool change(const PPSystemString& path);
	virtual bool stepInto(const PPSystemString& directory);
	
	virtual const PPPathEntry* getFirstEntry();
	virtual const PPPathEntry* getNextEntry();	
	
	virtual bool canGotoHome() const;
	virtual void gotoHome();
	virtual bool canGotoRoot() const;
	virtual void gotoRoot();
	virtual bool canGotoParent() const;
	virtual void gotoParent();
	
	virtual char getPathSeparatorAsASCII() const;
	virtual const PPSystemString getPathSeparator() const;
	
	virtual bool fileExists(const PPSystemString& fileName) const;
};

#endif

