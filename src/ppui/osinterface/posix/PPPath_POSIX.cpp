/*
 *  ppui/osinterface/posix/PPPath_POSIX.cpp
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
 *  PPPath_POSIX.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.03.06.
 *
 */

#include "PPPath_POSIX.h"
#include <sys/stat.h>
#include <limits.h>

#ifdef __PSP__
// Needed for PATH_MAX
#include <sys/syslimits.h>
#endif

#ifdef __AMIGA__
#define PATH_MAX 1024
#endif

#define PPMAX_DIR_PATH PATH_MAX

void PPPathEntry_POSIX::create(const PPSystemString& path, const PPSystemString& name)
{
	this->name = name;
	PPSystemString fullPath = path;
	
	fullPath.append(name);

	struct stat file_status;
	
	if (::stat(fullPath, &file_status) == 0) 
	{
		size = file_status.st_size;
		
		if (S_ISDIR(file_status.st_mode))
			type = Directory;
		//if (S_ISLNK(file_status.st_mode))
		//	printf("foo.txt is a symbolic link\n");
		if (S_ISCHR(file_status.st_mode))
			type = Hidden;
		if (S_ISBLK(file_status.st_mode))
			type = Hidden;
		if (S_ISFIFO(file_status.st_mode))
			type = Hidden;
		if (S_ISSOCK(file_status.st_mode))
			type = Hidden;
		if (S_ISREG(file_status.st_mode))
			type = File;
	}
	else 
	{ /* stat() call failed and returned '-1'. */
		type = Nonexistent;
	}		
}

bool PPPathEntry_POSIX::isHidden() const
{
	return PPPathEntry::isHidden() || (name.startsWith(".") && name.compareTo("..") != 0);
}

bool PPPath_POSIX::updatePath()
{
	return chdir(current) == 0;
}

PPPath_POSIX::PPPath_POSIX() 
{
	current = getCurrent();
	updatePath();
}

PPPath_POSIX::PPPath_POSIX(const PPSystemString& path) :
	current(path)
{
	updatePath();
}

const PPSystemString PPPath_POSIX::getCurrent()
{
	char cwd[PPMAX_DIR_PATH+1];
	memset(cwd, 0, sizeof(cwd));
	
	getcwd(cwd, PPMAX_DIR_PATH+1);

	PPSystemString path(cwd);
	
	path.ensureTrailingCharacter(getPathSeparatorAsASCII());
	return path;
}
	
bool PPPath_POSIX::change(const PPSystemString& path)
{
	PPSystemString old = current;
	current = path;
	current.ensureTrailingCharacter(getPathSeparatorAsASCII());
	bool res = updatePath();
	if (res)
		return true;
	current = old;
	return false;
}

bool PPPath_POSIX::stepInto(const PPSystemString& directory)
{
	PPSystemString old = current;
	current.append(directory);
	current.append(getPathSeparator());
	bool res = updatePath();
	if (res)
		return true;
	current = old;
	return false;
}
	
const PPPathEntry* PPPath_POSIX::getFirstEntry()
{
	dir = ::opendir(current);
	if (!dir) 
	{
		return NULL;
	}
	
	return getNextEntry();
}

const PPPathEntry* PPPath_POSIX::getNextEntry()
{
	struct dirent* entry;
	if ((entry = ::readdir(dir)) != NULL)
	{
		PPSystemString file(entry->d_name);
		this->entry.create(current, file);
		
		return &this->entry;
	}
	
	::closedir(dir);
	return NULL;
}

bool PPPath_POSIX::canGotoHome() const
{
	return getenv("HOME") ? true : false;
}

void PPPath_POSIX::gotoHome()
{
	char* home = getenv("HOME");
	if (home)
	{
		change(home);
		updatePath();
	}
}

bool PPPath_POSIX::canGotoRoot() const
{
	return true;
}

void PPPath_POSIX::gotoRoot()
{
	change("/");
	updatePath();
}

bool PPPath_POSIX::canGotoParent() const
{
	return true;
}

void PPPath_POSIX::gotoParent()
{
	stepInto("..");
}

char PPPath_POSIX::getPathSeparatorAsASCII() const
{
	return '/';
}

const PPSystemString PPPath_POSIX::getPathSeparator() const
{
	return PPSystemString(getPathSeparatorAsASCII());
}

bool PPPath_POSIX::fileExists(const PPSystemString& fileName) const
{
	struct stat file_status;
	
	if (::stat(fileName, &file_status) == 0) 
	{
		return (S_ISREG(file_status.st_mode)) != 0;
	}

	return false;
}


