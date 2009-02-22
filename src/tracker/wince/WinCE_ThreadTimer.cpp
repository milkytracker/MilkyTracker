/*
 *  tracker/wince/WinCE_ThreadTimer.cpp
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

#include "WinCE_ThreadTimer.h"

CThreadTimer::CThreadTimer():object(0),idEvent(0),elapse(0), isActive(FALSE)
    {		
        InitializeCriticalSection(&lock); 
    }

CThreadTimer::~CThreadTimer()
    {
        DeleteCriticalSection(&lock);
    }   

UINT CThreadTimer::SetTimer (void* obj, UINT nIDEvent, UINT uElapse, ThreadTimerProc lpTimerProc)
    {
    object = obj;
    idEvent = nIDEvent;
    elapse = uElapse;
    proc = lpTimerProc;

    EnterCriticalSection(&lock);
    // is it already active?
    if (isActive)
    {
        LeaveCriticalSection(&lock);
        return 0;
    }

    // Start the thread
    DWORD threadId;    
    HANDLE threadHandle = CreateThread (NULL, 0, CThreadTimer::ThreadFunction, this, 0, &threadId);    
    //SetThreadPriority(threadHandle,THREAD_PRIORITY_HIGHEST); // this is optional
    
	SetThreadPriority(threadHandle,THREAD_PRIORITY_ABOVE_NORMAL); // this is optional
    isActive = TRUE;
    LeaveCriticalSection(&lock);
    return nIDEvent;
    }

BOOL CThreadTimer::KillTimer()
    {
    EnterCriticalSection(&lock); 
    isActive = FALSE;
    LeaveCriticalSection(&lock);
    return TRUE;
    }

DWORD WINAPI CThreadTimer::ThreadFunction (LPVOID pParam)
    {
    // Here is the heart of our little timer
    CThreadTimer* obj = (CThreadTimer*) pParam;
    BOOLEAN isActive = TRUE;
    do
        {
        Sleep(obj->elapse);
        obj->proc (obj->object, obj->idEvent);

        EnterCriticalSection(&obj->lock);
        isActive = obj->isActive;
        LeaveCriticalSection(&obj->lock);
        } while (isActive);
    return 0;
    }
