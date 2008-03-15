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
