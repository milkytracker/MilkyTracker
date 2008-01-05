/*
 *  PPPath_WIN32.h
 *  PPUI SDL
 *
 *  Created by Peter Barth on 10.03.06.
 *  Copyright (c) 2006 milkytracker.net, All rights reserved.
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
	
	virtual bool canGotoHome();
	virtual void gotoHome();
	virtual bool canGotoRoot();
	virtual void gotoRoot();
	virtual bool canGotoParent();
	virtual void gotoParent();
	
	virtual char getPathSeparatorAsASCII();
	virtual const PPSystemString getPathSeparator();

	virtual bool fileExists(const PPSystemString& fileName);
};

#endif

