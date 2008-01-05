/*
 *  System_WIN32.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Thu Mar 10 2005.
 *  Copyright (c) 2005 milkytracker.net All rights reserved.
 *
 */

#include "PPSystem_WIN32.h"
#include <windows.h>
#include <tchar.h>

SYSCHAR System::buffer[MAX_PATH+1];

const SYSCHAR* System::getTempFileName()
{
	TCHAR szPath[MAX_PATH+1];
	_tcscpy(szPath, getConfigFileName(_T("")));

	UINT result = ::GetTempFileName(szPath, _T("mt"), ::GetTickCount(), buffer);
	if (result == 0)
	{
		return getConfigFileName(_T("milkytracker_temp"));
	}

	return buffer;
}

const SYSCHAR* System::getConfigFileName(SYSCHAR* fileName/* = NULL*/)
{
	// get path of our executable
	TCHAR szPath[MAX_PATH+1];

    DWORD dwLen;
    LPTSTR p;
    ::GetModuleFileName(NULL, szPath, MAX_PATH);

	// cut off executable
	dwLen = (DWORD)_tcslen(szPath);

	if (dwLen)
	{
		p = szPath + dwLen;
		while (p != szPath) {
			if (TEXT('\\') == *--p) {
				*(++p) = 0;
				break;
			}
		}
    }

	if (fileName)
		_tcscat(szPath, fileName);
	else
		_tcscat(szPath, _T("milkytracker.cfg"));
	_tcscpy(buffer, szPath);
	return buffer;
}

void System::msleep(int msecs)
{
	if (msecs < 0)
		return;
	::Sleep(msecs);
}
