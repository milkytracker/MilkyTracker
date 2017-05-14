/*
 *  ppui/osinterface/win32/PPSystem_WIN32.cpp
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
 *  System_WIN32.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Thu Mar 10 2005.
 *
 */

#include "PPSystem_WIN32.h"
#include <windows.h>
#include <tchar.h>

SYSCHAR System::buffer[MAX_PATH+1];

const SYSCHAR* System::getTempFileName()
{
	TCHAR szPath[MAX_PATH+1];
	UINT result = 0;

	DWORD dwRetVal = GetTempPath(MAX_PATH, szPath);
	if (dwRetVal != 0 || dwRetVal <= MAX_PATH)
		result = GetTempFileName(szPath, _T("mt"), ::GetTickCount(), buffer);
	if (result == 0)
		return getConfigFileName(_T("milkytracker_temp"));

	return buffer;
}

const SYSCHAR* System::getConfigFileName(SYSCHAR *fileName/* = NULL*/)
{
	TCHAR szPath[MAX_PATH + 1];

	DWORD size = GetEnvironmentVariable(_T("APPDATA"), szPath, MAX_PATH);

	if (size)
	{
		_tcscat_s(szPath, MAX_PATH, _T("\\MilkyTracker"));
		CreateDirectory(szPath, NULL);
		_tcscat_s(szPath, MAX_PATH, _T("\\"));
	}
	if (fileName)
		_tcscat_s(szPath, MAX_PATH, fileName);
	else
		_tcscat_s(szPath, MAX_PATH, _T("milkytracker.cfg"));
	_tcscpy(buffer, szPath);

	return buffer;
}

void System::msleep(int msecs)
{
	if (msecs < 0)
		return;
	::Sleep(msecs);
}
