#include "ThreadTimer.h"

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
    //SetThreadPriority(threadHandle,THREAD_PRIORITY_TIME_CRITICAL); // this is optional
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
