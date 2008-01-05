/*
 *  PPPath_POSIX.h
 *  PPUI SDL
 *
 *  Created by Peter Barth on 10.03.06.
 *  Copyright (c) 2006 milkytracker.net, All rights reserved.
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

