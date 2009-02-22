/*
 *  ppui/osinterface/win32/PPPath_WIN32.cpp
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
 *  PPPath_WIN32.cpp
 *  PPUI SDL
 *
 *  Created by Peter Barth on 10.03.06.
 *
 */

#include "PPPath_WIN32.h"
#include <tchar.h>

#ifdef UNICODE 
#ifndef _WIN32_WCE
#include <shlobj.h>
#else
#include <Shellapi.h>
#endif
#endif

#define PPMAX_DIR_PATH PATH_MAX

void PPPathEntry_WIN32::create(const PPSystemString& path, const PPSystemString& name)
{
	drive = false;

	this->name = name;
	PPSystemString fullPath = path;
	
	fullPath.append(name);

	if (name.compareTo("..") == 0 || name.compareTo(".") == 0)
	{
		type = Directory;
		size = 0;
		return;
	}

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(fullPath, &fd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		/*DWORD err = GetLastError();

		char* message;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (char*)&message, 0, NULL);

		LocalFree(message);*/
		type = Nonexistent;
		return;
	}
	else
	{
		size = fd.nFileSizeLow;
		
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			type = Directory;
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			type = Hidden;
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			type = File;
	}

	FindClose(hFind);
}

void PPPathEntry_WIN32::createNoCheck(const PPSystemString& path, const PPSystemString& name, Type type, bool drive)
{
	this->name = name;
	PPSystemString fullPath = path;
	
	fullPath.append(name);

	this->type = type;

	this->drive = drive;

	this->size = 0;
}

bool PPPathEntry_WIN32::isHidden() const
{
	return PPPathEntry::isHidden() || (name.compareTo(".") == 0);
}

PPPathEntry* PPPathEntry_WIN32::clone() const
{
	// check if this is the correct type
	PPPathEntry_WIN32* result = new PPPathEntry_WIN32();
	
	result->name = name;	
	result->type = type;
	result->size = size;
	result->drive = drive;
	
	return result;
}

bool PPPath_WIN32::updatePath()
{
#ifndef _WIN32_WCE
	return SetCurrentDirectory(current) == TRUE;
#else
	if (current.length() == 0)
		return true;

	DWORD res = GetFileAttributes(current);
	if (res == 0xFFFFFFFF)
		return false;

	return (res & FILE_ATTRIBUTE_DIRECTORY) != 0;
#endif
}

PPPath_WIN32::PPPath_WIN32() :
	hFind(NULL),
#ifndef _WIN32_WCE
	drives(0),
	driveCount(-1)
#else
	contentCount(0)
#endif
	{
#ifndef _WIN32_WCE
	current = getCurrent();
	updatePath();
#else
	change("");
#endif
}

PPPath_WIN32::PPPath_WIN32(const PPSystemString& path) :
	hFind(NULL),
#ifndef _WIN32_WCE
	drives(0),
	driveCount(-1),
#else
	contentCount(0),
#endif
	current(path)
{
	updatePath();
}

const PPSystemString PPPath_WIN32::getCurrent()
{
#ifndef _WIN32_WCE
	TCHAR szPath[MAX_PATH+1];

	GetCurrentDirectory(MAX_PATH, szPath);

	PPSystemString path(szPath);
	
	path.ensureTrailingCharacter(getPathSeparatorAsASCII());
	return path;
#else
	PPSystemString path(current);
	
	path.ensureTrailingCharacter(getPathSeparatorAsASCII());
	return path;
#endif
}
	
bool PPPath_WIN32::change(const PPSystemString& path)
{
	PPSystemString old = current;
	current = path;
	current.ensureTrailingCharacter(getPathSeparatorAsASCII());
	bool res = updatePath();
	if (res)
		return true;
	current = old;
	updatePath();
	return false;
}

bool PPPath_WIN32::stepInto(const PPSystemString& directory)
{
	PPSystemString old = current;	

#ifndef _WIN32_WCE
	// check for drive
	if (directory.length() == 2)
	{
		const TCHAR* str = directory.getStrBuffer();
		if (str[1] == ':')
		{
			if (change(directory))
				return true;
			else
			{
				current = old;
				return false;
			}
		}
	}
#else
	if (directory.compareTo("..") == 0)
	{
		PPSystemString temp = current;
		temp.ensureTrailingCharacter(getPathSeparatorAsASCII());
		temp.deleteAt(temp.length()-1, 1);
		
		const TCHAR* base = temp.getStrBuffer();
		const TCHAR* ptr = base + temp.length();

		while (*ptr != '\\' && ptr > base)
			ptr--;
		
		// TO-DO: make this safe :(
		TCHAR* tempStr = new TCHAR[ptr - base + 1];
		memcpy(tempStr, base, (ptr - base)*sizeof(TCHAR));
		tempStr[(ptr - base)] = '\0';
		
		current = tempStr;
		
		delete[] tempStr;
		current.append(getPathSeparator());

		if (updatePath())
			return true;
		else
		{
			current = old;
			return false;
		}
	}
#endif
	current.append(directory);
	current.append(getPathSeparator());
	
	if (updatePath())
		return true;
	current = old;
	return false;
}
	
const PPPathEntry* PPPath_WIN32::getFirstEntry()
{
	// if we're not on Windows CE we're going to 
	// enumerate drives first before actually 
	// iterating folder contents
#ifndef _WIN32_WCE
	if (driveCount == -1)
	{
		drives = GetLogicalDrives();
		driveCount = 0;

		while (!((drives >> driveCount) & 1) && driveCount < 26)
			driveCount++;

		if (driveCount < 26)
		{
			char drive = 'A' + driveCount;	

			driveCount++;
		
			PPSystemString driveName(drive);
			driveName.append(":");

			entry.createNoCheck("", driveName, PPPathEntry::Directory, true);
			return &entry;
		}
		else driveCount = -1;
	}
#else
	// if we're on Windows CE we're going to add "." and next time ".."
	if (contentCount == 0 && (current.length() != 0 && current.compareTo("\\") != 0))
	{
		contentCount++;
		entry.createNoCheck("", ".", PPPathEntry::Directory, false);
		return &entry;
	}
#endif

	PPSystemString current = this->current;
	current.append("*.*");

	hFind = FindFirstFile(current, &fd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	PPSystemString file(fd.cFileName);	
	entry.create(this->current, file);

	return &entry;
}

const PPPathEntry* PPPath_WIN32::getNextEntry()
{
	// continue iterating drives if not finished
#ifndef _WIN32_WCE
	if (driveCount != -1)
	{
		while (!((drives >> driveCount) & 1) && driveCount < 26)
			driveCount++;

		if (driveCount < 26)
		{
			char drive = 'A' + driveCount;	

			driveCount++;
		
			PPSystemString driveName(drive);
			driveName.append(":");

			entry.createNoCheck("", driveName, PPPathEntry::Directory, true);
			return &entry;
		}
		else 
		{
			const PPPathEntry* entry = getFirstEntry();
			driveCount = -1;
			return entry;
		}
	}
#else
	// on windows CE add ".." to the folder contents first
	if (contentCount == 1)
	{
		contentCount++;
		entry.createNoCheck("", "..", PPPathEntry::Directory, false);
		return &entry;
	}
	else if (contentCount == 2)
	{
		const PPPathEntry* entry = getFirstEntry();
		contentCount = 0;
		return entry;
	}
#endif

	BOOL res = FindNextFile(hFind, &fd);

	if (res)
	{
		PPSystemString file(fd.cFileName);	
		entry.create(current, file);
		
		return &entry;
	}
	
	FindClose(hFind);
	return NULL;
}

bool PPPath_WIN32::canGotoHome() const
{
	// we're going to assume Unicode is for WinNT and higher
	// means SHGetFolderPath is available
#ifdef UNICODE
	return true;
#endif
	return false;
}

void PPPath_WIN32::gotoHome()
{
	// we're going to assume Unicode is for WinNT and higher
	// means SHGetFolderPath is available
#ifdef UNICODE
	TCHAR* pszPath = new TCHAR[MAX_PATH+1];
	SHGetSpecialFolderPath(NULL, pszPath, CSIDL_PERSONAL, FALSE);
	change(pszPath);
	delete[] pszPath;
#endif
}

bool PPPath_WIN32::canGotoRoot() const
{
	return true;
}

void PPPath_WIN32::gotoRoot()
{
	change("\\");
	updatePath();
}

bool PPPath_WIN32::canGotoParent() const
{
	return true;
}

void PPPath_WIN32::gotoParent()
{
	stepInto("..");
}

char PPPath_WIN32::getPathSeparatorAsASCII() const
{
	return '\\';
}

const PPSystemString PPPath_WIN32::getPathSeparator() const
{
	return PPSystemString(getPathSeparatorAsASCII());
}

bool PPPath_WIN32::fileExists(const PPSystemString& fileName) const
{
	HANDLE handle = CreateFile(fileName,
					    GENERIC_READ,
						FILE_SHARE_READ,
						NULL,
						OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL, 
						NULL);

	bool res = handle != INVALID_HANDLE_VALUE;
	if (res)
		CloseHandle(handle);
	return res;	
}
