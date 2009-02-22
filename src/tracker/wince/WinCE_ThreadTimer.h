/*
 *  tracker/wince/WinCE_ThreadTimer.h
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

#ifndef THREADTIMER__H
#define THREADTIMER__H

#include <windows.h>

typedef void (*ThreadTimerProc)(void* obj, UINT idEvent);

class CThreadTimer  
{  
    void* object;			// pointer to the "parent" object (like CTimersDlg)
    UINT idEvent;			// timer ID
    UINT elapse;			// "Sleep time" in milliseconds
    ThreadTimerProc proc;	// Callback function, supplied by the user

    BOOL isActive;			// Set to FALSE after the call to KillTimer
    CRITICAL_SECTION lock;  // thread synchronization

    static DWORD WINAPI ThreadFunction (LPVOID pParam); // thread entry point
public:
	CThreadTimer();
	virtual ~CThreadTimer();
    
    UINT SetTimer (void* obj, UINT nIDEvent, UINT uElapse, ThreadTimerProc lpTimerProc);
    BOOL KillTimer();
};

#endif
