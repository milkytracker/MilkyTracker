/*
 *  ppui/osinterface/win32/PPPath_WIN32.h
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
 *  PPPath_WIN32.h
 *  PPUI SDL
 *
 *  Created by Peter Barth on 10.03.06.
 *
 */

#ifndef __PPPATH_WIN32_H__
#define __PPPATH_WIN32_H__

#include "PPPath.h"

class PPPathEntry_WIN32 : public PPPathEntry
{
private:
	bool drive;

public:
	PPPathEntry_WIN32() :
		drive(false)
	{ 
	}

	virtual void create(const PPSystemString& path, const PPSystemString& name);

	virtual bool isHidden() const;	

	virtual bool isDrive() const { return drive; }

	void createNoCheck(const PPSystemString& path, const PPSystemString& name, Type type, bool drive);

	PPPathEntry* clone() const;
};

class PPPath_WIN32 : public PPPath
{
protected:
	// WIN32
	WIN32_FIND_DATA fd;
	HANDLE hFind;
#ifndef _WIN32_WCE
	pp_uint32 drives;
	pp_int32 driveCount;
#else
	pp_uint32 contentCount;
#endif

	PPSystemString current;
	PPPathEntry_WIN32 entry;

	virtual bool updatePath();

public:
	PPPath_WIN32();
	PPPath_WIN32(const PPSystemString& path);

	virtual const PPSystemString getCurrent();
	
	bool change(const PPSystemString& path);
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

