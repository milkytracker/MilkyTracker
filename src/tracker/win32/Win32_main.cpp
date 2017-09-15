/*
 *  tracker/win32/Win32_main.cpp
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

#include <Windows.h>
#include <Windowsx.h>
#include <tchar.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <ctype.h>
#include "Win32_resource.h"
#include "ThreadTimer.h"
#include "PreferencesDialog.h"
//------------------------------------------------------------------------------
#include "PPUI.h"
#include "Screen.h"
#include "Tracker.h"
#include "DisplayDevice_WIN32.h"
#include "PPSystem.h"
#include "PPMutex.h"
#include "XMFile.h"

#include "MIDIInDevice.h"
#include "MidiReceiver_win32.h"

using midi::CMIDIInDevice;
using midi::CMIDIReceiver;

#undef	FULLSCREEN

#define IDC_HAND			MAKEINTRESOURCE(32649)
#define IDM_FULLSCREEN		0x10
#define IDM_PREFERENCES		0x11

#define WINDOWTITLE			_T("Loading MilkyTracker...")

#define	FS_FREQUENCY		0x3C
#define	FS_BPS				0x20
#ifdef DEBUG
#define SPLASH_WAIT_TIME	2000
#else
#define SPLASH_WAIT_TIME	0
#endif
#define WM_MYTIMER			WM_USER+10

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL		0x020A
#endif

/****************************************************************************
 *
 *      Global variables
 *
 ****************************************************************************/

TCHAR						c_szClassName[]		= _T("MILKYTRACKERMAINCLASS");

HINSTANCE					g_hinst				= NULL;       /* My instance handle */
BOOL						g_fPaused			= TRUE;       /* Should I be paused? */
HWND						hWnd				= NULL;
BOOL						g_mouseDragging		= FALSE;

PPMutex*					g_globalMutex		= NULL;

HCURSOR						g_cursorStandard	= NULL;
HCURSOR						g_cursorResizeWE	= NULL;
HCURSOR						g_cursorHand		= NULL;

static PPScreen*			myTrackerScreen		= NULL;
static Tracker*				myTracker			= NULL;
static PPDisplayDevice*		myDisplayDevice		= NULL;

static CThreadTimer			myThreadTimer;
static CPreferencesDialog*	myPreferenceDialog	= NULL;
static CMIDIInDevice*		myMidiInDevice		= NULL;
static MidiReceiver*		myMidiReceiver		= NULL;

#ifdef MOUSETRACKING
#if(WIN32_WINNT < 0x0400)
   #define WM_MOUSELEAVE   WM_USER+2
   #define TME_LEAVE               1

typedef struct tagTRACKMOUSEEVENT {
	DWORD cbSize;
	DWORD dwFlags;
	HWND  hwndTrack;
} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;

VOID CALLBACK
TrackMouseTimerProc(HWND hWnd,UINT uMsg,UINT idEvent,DWORD dwTime) {
	RECT rect;
	POINT pt;
	
	GetClientRect(hWnd,&rect);
	MapWindowPoints(hWnd,NULL,(LPPOINT)&rect,2);
	GetCursorPos(&pt);
	if (!PtInRect(&rect,pt) || (WindowFromPoint(pt) != hWnd)) {
		if (!KillTimer(hWnd,idEvent)) {
			// Error killing the timer!
		}
		
		PostMessage(hWnd,WM_MOUSELEAVE,0,0);
	}
}

BOOL
TrackMouseEvent(LPTRACKMOUSEEVENT ptme) {
	OutputDebugString(_T("TrackMouseEvent\n"));
	
	if (!ptme || ptme->cbSize < sizeof(TRACKMOUSEEVENT)) {
		OutputDebugString(_T("TrackMouseEvent: invalid TRACKMOUSEEVENT structure\n"));
		return FALSE;
	}
	
	if (!IsWindow(ptme->hwndTrack)) {
		OutputDebugString(
						  _T("TrackMouseEvent: invalid hwndTrack\n"));
		return FALSE;
	}
	
	if (!(ptme->dwFlags & TME_LEAVE)) {
		OutputDebugString(_T("TrackMouseEvent: invalid dwFlags\n"));
		return FALSE;
	}
	
	return SetTimer(ptme->hwndTrack, ptme->dwFlags,
					100,(TIMERPROC)TrackMouseTimerProc);
}
#endif // WIN32_WINNT

#endif // MOUSETRACKING

static void MyThreadTimerProc(void* obj, UINT idEvent)
{
	SendMessage(hWnd, WM_MYTIMER, 0, 0);
}

pp_uint32 PPGetTickCount()
{
	return ::GetTickCount();
}

void QueryKeyModifiers()
{
	if (::GetAsyncKeyState(VK_SHIFT)>>15)
		setKeyModifier(KeyModifierSHIFT);
	else
		clearKeyModifier(KeyModifierSHIFT);

	if (::GetAsyncKeyState(VK_MENU)>>15)
		setKeyModifier(KeyModifierALT);
	else
		clearKeyModifier(KeyModifierALT);

	if (::GetAsyncKeyState(VK_CONTROL)>>15)
		setKeyModifier(KeyModifierCTRL);
	else
		clearKeyModifier(KeyModifierCTRL);
}

static void EnableNumPad(pp_uint16* chr, LPARAM lParam)
{
	// Always enable NUMPAD VKs (check scancode)
	switch (chr[1])
	{
		// VK_DIVIDE
		case 0x35:
			if (lParam & (1 << 24))
				chr[0] = VK_DIVIDE;
			break;
		// VK_MULTIPLY
		case 0x37:
			if (lParam & (1 << 24))
				chr[0] = VK_MULTIPLY;
			break;
		// VK_MULTIPLY
		case 0x4A:
			if (lParam & (1 << 24))
				chr[0] = VK_SUBTRACT;
			break;
		// VK_MULTIPLY
		case 0x4E:
			if (lParam & (1 << 24))
				chr[0] = VK_ADD;
			break;
		// VK_MULTIPLY
		case 0x1C:
			// Check for extended key enter
			if (lParam & (1 << 24))
				chr[0] = VK_SEPARATOR;
			break;
		// VK_DECIMAL
		case 0x53:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_DECIMAL;
			break;
		// VK_NUMPAD0
		case 0x52:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD0;
			break;
		// VK_NUMPAD1
		case 0x4F:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD1;
			break;
		// VK_NUMPAD2
		case 0x50:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD2;
			break;
		// VK_NUMPAD3
		case 0x51:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD3;
			break;
		// VK_NUMPAD4
		case 0x4B:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD4;
			break;
		// VK_NUMPAD5
		case 0x4C:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD5;
			break;
		// VK_NUMPAD6
		case 0x4D:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD6;
			break;
		// VK_NUMPAD7
		case 0x47:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD7;
			break;
		// VK_NUMPAD8
		case 0x48:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD8;
			break;
		// VK_NUMPAD9
		case 0x49:
			if (!(lParam & (1 << 24)))
				chr[0] = VK_NUMPAD9;
			break;
	}
}

static void SendFile(LPCTSTR file)
{
	PPSystemString finalFile(file);
	PPSystemString* strPtr = &finalFile;
		
	PPEvent event(eFileDragDropped, &strPtr, sizeof(PPSystemString*));
	myTrackerScreen->raiseEvent(&event);		
}

static void OnDropFiles(HDROP hDropInfo)
{
	TCHAR buffer[MAX_PATH+1];

	unsigned int count = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, 0, 0);

	if (count)
	{
		::DragQueryFile(hDropInfo, 0, buffer, MAX_PATH);
		
		::SendFile(buffer);
	}
}

static void StopMidiRecording()
{
	// clean up midi device & listener
	if (myMidiInDevice)
	{
		delete myMidiInDevice;
		myMidiInDevice = NULL;
	}
	if (myMidiReceiver)
	{
		delete myMidiReceiver;
		myMidiReceiver = NULL;
	}
}

static void StartMidiRecording(UINT devID, 
							   bool recordVelocity,
							   UINT velocityAmplify,
							   UINT threadPriority = CMIDIInDevice::MIDI_THREAD_PRIORITY_NORMAL)
{
	if (!CMIDIInDevice::GetNumDevs() || devID == (unsigned)-1)
		return;

	StopMidiRecording();

	myMidiReceiver = new MidiReceiver(*myTracker, *g_globalMutex);
	myMidiReceiver->setRecordVelocity(recordVelocity);
	myMidiReceiver->setVelocityAmplify(velocityAmplify);
	myMidiInDevice = new CMIDIInDevice(*myMidiReceiver);
	// ---------------- Initialize MIDI -------------------
	// We'll use the first device - we're not picky here
	try
	{
		myMidiInDevice->Open(devID);
	
		// Start recording
		myMidiInDevice->StartRecording((CMIDIInDevice::ThreadPriority)threadPriority);
	}
	catch (midi::CMIDIInException e)
	{
		StopMidiRecording();
		
		PPSystemString str("Error while trying to setup the MIDI device. The error message is:\r\n");		
		str.append(e.what());
		str.append("\r\nMIDI will be disabled.");

		::MessageBox(hWnd, str, NULL, MB_OK | MB_ICONERROR);
	}
}

static void HandleMidiRecording()
{
	if (!myPreferenceDialog)
		return;

	if (myPreferenceDialog->getUseMidiDeviceFlag())
		StartMidiRecording(myPreferenceDialog->getSelectedMidiDeviceID(),
						   myPreferenceDialog->getRecordVelocityFlag(),
						   myPreferenceDialog->getVelocityAmplify(),
						   myPreferenceDialog->getMidiRecordThreadPriority());
	else
		StopMidiRecording();
}

static void HandlePreferencesDialog()
{
	if (myPreferenceDialog)
	{
		if (myPreferenceDialog->runModal() == IDOK)
		{
			HandleMidiRecording();
		}
	}
}

static void RaiseEventSynchronized(PPEvent* event)
{
	//if (globalMutex->tryLock())
	//{
		g_globalMutex->lock();

		if (myTrackerScreen)
			myTrackerScreen->raiseEvent(event);
			
		g_globalMutex->unlock();
	//}
}

static LONG WINAPI CrashHandler(EXCEPTION_POINTERS*)
{
	// get path of our executable
	static TCHAR szPath[MAX_PATH+1];
	static TCHAR buffer[MAX_PATH+1];
	static TCHAR fileName[MAX_PATH+1];

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

	_tcscpy(buffer, szPath);
	_tcscat(buffer, _T("BACKUP00.XM"));
	int num = 1;
	while(XMFile::exists(buffer) && num <= 100)
	{
		_tcscpy(buffer, szPath);
#ifdef _UNICODE
		wsprintf(fileName, L"BACKUP%02i.XM", num);
#else
		sprintf(fileName, "BACKUP%02i.XM", num);
#endif
		_tcscat(buffer, fileName);
		num++;
	}

	if (num != 100) 
		myTracker->saveModule(buffer);

	MessageBox(NULL, _T("MilkyTracker has crashed, sorry for the inconvenience.\n\n")\
		_T("An attempt was made to save the current module in the application folder.\n")\
		_T("Please report this error back to the MilkyTracker development team.\n"), NULL, MB_OK);
	
	return EXCEPTION_CONTINUE_SEARCH;
}

/****************************************************************************
 *
 *      Ex_WndProc
 *
 *      Window procedure for simple sample.
 *
 ****************************************************************************/

LRESULT CALLBACK Ex_WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static BOOL		lMouseDown			= FALSE;
	static DWORD	ltime;	
	static PPPoint	llastClickPosition	= PPPoint(0,0);
	static WORD		lClickCount			= 0;
	static DWORD	lButtonDownStartTime;
	
	static BOOL		rMouseDown			= FALSE;
	static DWORD	rtime;
	static PPPoint	rlastClickPosition	= PPPoint(0,0);
	static WORD		rClickCount			= 0;
	static DWORD	rButtonDownStartTime;
	
	static DWORD	timerTicker			= 0;

	static PPPoint	p;

	static bool		wasFullScreen		= false;

	// ------------------------------------------ stupid fucking shit --------
#ifdef MOUSETRACKING
	TRACKMOUSEEVENT tme;
#endif
    static BOOL fInWindow;
    static BOOL fInMenu;
	// -----------------------------------------------------------------------

	if (lClickCount > 4)
	{
		lClickCount = 0;
	}
	if (rClickCount > 4)
	{
		rClickCount = 0;
	}

	switch (msg) 
	{
		case WM_CREATE:
			myThreadTimer.SetTimer (NULL, 1, 20, MyThreadTimerProc);
#ifndef _DEBUG
			SetUnhandledExceptionFilter(CrashHandler); 
#endif
			fInWindow = FALSE;
			fInMenu = FALSE;
			break;

		case WM_PAINT:
			if (!myTrackerScreen)
				break;
			myTrackerScreen->paint();
			break;

		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		{
			RECT rc;
			GetWindowRect(hwnd, &rc);
		
			TMouseWheelEventParams mouseWheelParams;
			// Absolute screen coordinates into client coordinates?
			//mouseWheelParams.pos.x = LOWORD(lParam)-(rc.left+GetSystemMetrics(SM_CXEDGE)+1);
			//mouseWheelParams.pos.y = HIWORD(lParam)-(rc.top+(GetSystemMetrics(SM_CXEDGE)+GetSystemMetrics(SM_CYCAPTION)+1));
			
			if (myDisplayDevice->isFullScreen())
			{
				mouseWheelParams.pos.x = LOWORD(lParam)-(rc.left);
				mouseWheelParams.pos.y = HIWORD(lParam)-(rc.top);
			}
			else
			{
				mouseWheelParams.pos.x = LOWORD(lParam)-(rc.left+GetSystemMetrics(SM_CXEDGE)+1);
				mouseWheelParams.pos.y = HIWORD(lParam)-(rc.top+(GetSystemMetrics(SM_CXEDGE)+GetSystemMetrics(SM_CYCAPTION)+1));
			}
			
			mouseWheelParams.deltaX = msg == WM_MOUSEHWHEEL ? ((signed short)HIWORD(wParam)) / 60 : 0;
			mouseWheelParams.deltaY = msg == WM_MOUSEWHEEL ? ((signed short)HIWORD(wParam)) / 60 : 0;
			
			PPEvent myEvent(eMouseWheelMoved, &mouseWheelParams, sizeof(mouseWheelParams));	
			
			RaiseEventSynchronized(&myEvent);				

			break;
		}

		// ----- left mousebutton -------------------------------
		case WM_LBUTTONDOWN:
		{
			if (!myTrackerScreen)
				break;
			
			if (lMouseDown)
			{
				p.x = -1;
				p.y = -1;
				PPEvent myEvent(eLMouseUp, &p, sizeof(PPPoint));				
				RaiseEventSynchronized(&myEvent);
				lMouseDown = false;
			}

			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			PPEvent myEvent(eLMouseDown, &p, sizeof(PPPoint));
			
			RaiseEventSynchronized(&myEvent);
			
			lMouseDown = TRUE;
			//lButtonDownStartTime = timerTicker;
			lButtonDownStartTime = GetTickCount();

			if (!lClickCount)
			{
				ltime = GetTickCount();
				llastClickPosition.x = LOWORD(lParam);
				llastClickPosition.y = HIWORD(lParam);
			}
			else if (lClickCount == 2)
			{
				DWORD deltat = GetTickCount() - ltime;
				
				if (deltat > 500)
				{
					lClickCount = 0;
					ltime = GetTickCount();
					llastClickPosition.x = LOWORD(lParam);
					llastClickPosition.y = HIWORD(lParam);
				}
			}

			lClickCount++;	

			break;
		}

		// ----- middle mousebutton -------------------------------
		case WM_MBUTTONDOWN:
		{
			if (!myTrackerScreen)
				break;

			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			PPEvent myEvent(eMMouseDown, &p, sizeof(PPPoint));

			RaiseEventSynchronized(&myEvent);

			break;
		}

		// ----- right mousebutton -------------------------------
		case WM_RBUTTONDOWN:
		{
			if (!myTrackerScreen)
				break;
			
			if (rMouseDown)
			{
				p.x = -1;
				p.y = -1;
				PPEvent myEvent(eRMouseUp, &p, sizeof(PPPoint));				
				RaiseEventSynchronized(&myEvent);
				rMouseDown = false;
			}

			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			PPEvent myEvent(eRMouseDown, &p, sizeof(PPPoint));
			
			RaiseEventSynchronized(&myEvent);
			
			rMouseDown = TRUE;
			//rButtonDownStartTime = timerTicker;
			rButtonDownStartTime = GetTickCount();

			if (!rClickCount)
			{
				rtime = GetTickCount();
				rlastClickPosition.x = LOWORD(lParam);
				rlastClickPosition.y = HIWORD(lParam);
			}
			else if (rClickCount == 2)
			{
				DWORD deltat = GetTickCount() - rtime;
				
				if (deltat > 500)
				{
					rClickCount = 0;
					rtime = GetTickCount();
					rlastClickPosition.x = LOWORD(lParam);
					rlastClickPosition.y = HIWORD(lParam);
				}
			}

			rClickCount++;	

			break;
		}
		
		// ----- left mousebutton -------------------------------
		case WM_LBUTTONUP:
		{
			if (!myTrackerScreen || !lMouseDown)
				break;

			if (g_mouseDragging)
			{
				ReleaseCapture();
				g_mouseDragging = FALSE;
			}
			lClickCount++;

			if (lClickCount == 4)
			{
				DWORD deltat = GetTickCount() - ltime;
				
				if (deltat < 500)
				{
					p.x = LOWORD(lParam);
					p.y = HIWORD(lParam);
					
					if (abs(p.x - llastClickPosition.x) < 4 &&
						abs(p.y - llastClickPosition.y) < 4)
					{

						PPEvent myEvent(eLMouseDoubleClick, &p, sizeof(PPPoint));
						
						RaiseEventSynchronized(&myEvent);
					}
				}
				
				lClickCount = 0;

			}
				
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			PPEvent myEvent(eLMouseUp, &p, sizeof(PPPoint));
			
			RaiseEventSynchronized(&myEvent);
			
			lMouseDown = FALSE;
			
			break;
		}		

		// ----- middle mousebutton -------------------------------
		case WM_MBUTTONUP:
		{
			if (!myTrackerScreen)
				break;

			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			PPEvent myEvent(eMMouseUp, &p, sizeof(PPPoint));

			RaiseEventSynchronized(&myEvent);


			break;
		}

		// ----- right mousebutton -------------------------------
		case WM_RBUTTONUP:
		{
			if (!myTrackerScreen || !rMouseDown)
				break;

			rClickCount++;

			if (rClickCount == 4)
			{
				DWORD deltat = GetTickCount() - rtime;
				
				if (deltat < 500)
				{
					p.x = LOWORD(lParam);
					p.y = HIWORD(lParam);
					
					if (abs(p.x - rlastClickPosition.x) < 4 &&
						abs(p.y - rlastClickPosition.y) < 4)
					{

						PPEvent myEvent(eRMouseDoubleClick, &p, sizeof(PPPoint));
						
						RaiseEventSynchronized(&myEvent);
					}
				}
				
				rClickCount = 0;

			}
				
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			PPEvent myEvent(eRMouseUp, &p, sizeof(PPPoint));
			
			RaiseEventSynchronized(&myEvent);
			
			rMouseDown = FALSE;
			
			break;
		}		
		
#ifdef MOUSETRACKING
		case WM_MOUSELEAVE:
			fInWindow = FALSE;
			if (!fInMenu)
			{
				PPPoint p(-1000, -1000);
				if (lMouseDown)
				{
					PPEvent myEvent(eLMouseUp, &p, sizeof(PPPoint));					
					RaiseEventSynchronized(&myEvent);
					lMouseDown = TRUE;
				}
				if (rMouseDown)
				{
					PPEvent myEvent(eRMouseDown, &p, sizeof(PPPoint));					
					RaiseEventSynchronized(&myEvent);
					rMouseDown = TRUE;
				}
			}
			break;

		case WM_ENTERMENULOOP:
			fInMenu = TRUE;
			break;

		case WM_EXITMENULOOP:
			fInMenu = FALSE;
			break;
#endif

		case WM_MOUSEMOVE:
		{
#ifdef MOUSETRACKING
			if (!fInWindow) 
			{
				fInWindow = TRUE;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				if (!TrackMouseEvent(&tme)) {
					MessageBox(hwnd,
						TEXT("TrackMouseEvent Failed"),
						TEXT("Mouse Leave"),MB_OK);
				}
			}
#endif			
			if (!myTrackerScreen)
				break;

			if ((wParam&MK_LBUTTON) && lMouseDown)
			{
				p.x = GET_X_LPARAM(lParam);
				p.y = GET_Y_LPARAM(lParam);
				if (!g_mouseDragging)
				{
					SetCapture(hWnd);
					g_mouseDragging = TRUE;
				}
				
				PPEvent myEvent(eLMouseDrag, &p, sizeof(PPPoint));
				
				RaiseEventSynchronized(&myEvent);
			}
			else if ((wParam&MK_RBUTTON) && rMouseDown)
			{
				p.x = LOWORD(lParam);
				p.y = HIWORD(lParam);
				
				PPEvent myEvent(eRMouseDrag, &p, sizeof(PPPoint));
				
				RaiseEventSynchronized(&myEvent);
			}
			else
			{
				p.x = LOWORD(lParam);
				p.y = HIWORD(lParam);
				
				PPEvent myEvent(eMouseMoved, &p, sizeof(PPPoint));
				
				RaiseEventSynchronized(&myEvent);
			}
			break;
		}

		case WM_CHAR:
		{
			WORD chr = (WORD)wParam; 

			if (chr < 32 || chr > 127)
				break;
				
			PPEvent myEvent(eKeyChar, &chr, sizeof(chr));
			
			RaiseEventSynchronized(&myEvent);
			
			break;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (msg == WM_SYSKEYDOWN &&
				wParam == VK_RETURN && 
				myTrackerScreen)
			{
				PPEvent e(eFullScreen);
				RaiseEventSynchronized(&e);
				break;
			}

			WORD keyBuf[2] = { 0,0 };
			if (ToAscii(wParam, (lParam>>16)&255, 0, keyBuf, 0) != 1)
				keyBuf[0] = keyBuf[1] = 0;

			QueryKeyModifiers();

			// Check for right ALT key
			if (GetAsyncKeyState(VK_RMENU)>>15)
				wParam = VK_RMENU;
			// Check for right CTRL key
			else if (GetAsyncKeyState(VK_RCONTROL)>>15)
				wParam = VK_RCONTROL;			

			WORD chr[3] = {(WORD)wParam, (WORD)(lParam>>16)&255, keyBuf[0]}; 

			EnableNumPad(chr, lParam);

			if (wParam == VK_CAPITAL)
			{
				chr[1] = 0x100;
				BYTE keyState[256];
				GetKeyboardState(keyState);
				// Toggle caps lock
				keyState[wParam] = !(BOOL)GetKeyState(VK_CAPITAL);
				SetKeyboardState(keyState);
			}

			PPEvent myEvent(eKeyDown, &chr, sizeof(chr));
			
			RaiseEventSynchronized(&myEvent);

			if (wParam == VK_SPACE && (GetAsyncKeyState(VK_MENU)>>15))
				return 0;

			break;
		}

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			WORD keyBuf[2] = {0,0};
			if (ToAscii(wParam, (lParam>>16)&255+256, 0, keyBuf, 0) != 1)
				keyBuf[0] = keyBuf[1] = 0;

			WORD chr[3] = {(WORD)wParam, (WORD)(lParam>>16)&255, keyBuf[0]}; 

			EnableNumPad(chr, lParam);

			PPEvent myEvent(eKeyUp, &chr, sizeof(chr));
			
			RaiseEventSynchronized(&myEvent);
			
			// Disable F10 system menu pop-up
			// --------------------------------------------------------
			// From MSDN:
			// If the F10 key is pressed, the DefWindowProc function 
			// sets an internal flag. When DefWindowProc receives the 
			// WM_KEYUP message, the function checks whether the 
			// internal flag is set and, if so, sends a WM_SYSCOMMAND 
			// message to the top-level window. The wParam parameter of 
			// the message is set to SC_KEYMENU.
			// --------------------------------------------------------
			// Solution is to skip DefWindowProc when F10 is pressed

			if (wParam == VK_F10)
				return 0;

			break;
		}

		case WM_MYTIMER:
		{
			if (!myTrackerScreen)
				break;

			PPEvent myEvent(eTimer);			
			RaiseEventSynchronized(&myEvent);

			timerTicker++;
			DWORD currentTime = GetTickCount();
			
			if (lMouseDown &&
				(currentTime - lButtonDownStartTime) > 500 &&
				!(timerTicker%3))
			{
				PPEvent myEvent(eLMouseRepeat, &p, sizeof(PPPoint));				
				RaiseEventSynchronized(&myEvent);
			}
			else if (rMouseDown &&
				(currentTime - rButtonDownStartTime) > 500)
			{
				PPEvent myEvent(eRMouseRepeat, &p, sizeof(PPPoint));				
				RaiseEventSynchronized(&myEvent);
			}

			break;
		}

		case WM_SYSCOMMAND:
		{
			if (wParam == IDM_FULLSCREEN && myTrackerScreen)
			{
				PPEvent e(eFullScreen);
				RaiseEventSynchronized(&e);
			}
			else if (wParam == IDM_PREFERENCES)
			{
				HandlePreferencesDialog();
			}
			else if (wParam == SC_MINIMIZE)
			{
				// activating the preferences dialog in minimized mode
				// seems to freeze, so better disable it
				EnableMenuItem(GetSystemMenu(hWnd, FALSE), IDM_PREFERENCES, (MF_GRAYED | MF_DISABLED));
			}
			else if (wParam == SC_RESTORE)
			{
				// enable preference item in the system menu
				EnableMenuItem(GetSystemMenu(hWnd, FALSE), IDM_PREFERENCES, MF_ENABLED);
			}
			else if (wParam == SC_KEYMENU && lParam == 0x20)
			{
				// disable ALT+SPACE system menu popup)
				return 0;
			}

			break;
		}

		case WM_DROPFILES:
			OnDropFiles((HDROP)wParam);
			break;

		case WM_ACTIVATE:	
			g_fPaused = (wParam == WA_INACTIVE); 
			break;
		
		case WM_CLOSE:
		{
			if (GetAsyncKeyState(VK_F4) && GetAsyncKeyState(VK_MENU))
			{
				return 0;
			}

			bool res = myTracker->shutDown();
			if (res)
			{
				myThreadTimer.KillTimer();

				PPEvent e(eAppQuit);
				RaiseEventSynchronized(&e);

				DestroyWindow(hwnd);
			}

			return 0;
		}

		case WM_SIZE:
			// Ignore WM_SIZE events sent during window creation, minimize and restore
			return 0;

		case WM_DESTROY:	
			PostQuitMessage(0);
			break;

/*		case WM_KILLFOCUS:
			if (myTrackerScreen && myTrackerScreen->isFullScreen())
			{
				wasFullScreen = true;
				PPEvent e(eFullScreen);
				RaiseEventSynchronized(&e);
			}
			else 
				wasFullScreen = false;

			break;
		
		case WM_SETFOCUS:
			if (myTrackerScreen && wasFullScreen)
			{
				PPEvent e(eFullScreen);
				RaiseEventSynchronized(&e);
				wasFullScreen = false;
			}
			else
				wasFullScreen = false;

			break;*/

	}

	return DefWindowProc(hwnd, msg, wParam, lParam);

}

/****************************************************************************
 *
 *      AppInit
 *
 *      Set up everything the application needs to get started.
 *
 ****************************************************************************/

static BOOL AppInit(HINSTANCE hinst,int nCmdShow)
{
	/*
	 *  Save instance handle for future reference.
	 */
	g_hinst = hinst;

	g_globalMutex = new PPMutex();

	g_cursorStandard = LoadCursor(NULL, IDC_ARROW);
	g_cursorResizeWE = LoadCursor(NULL, IDC_SIZEWE);
	g_cursorHand = LoadCursor(NULL, IDC_HAND);

	/*
	 *  Set up the window class.
	 */
	WNDCLASS wc;

	wc.hCursor        = g_cursorStandard;
	wc.hIcon          = LoadIcon(hinst, MAKEINTRESOURCE(IDI_APPLICATIONICON));
	wc.lpszMenuName   = NULL;
	wc.lpszClassName  = c_szClassName;
	wc.hbrBackground  = 0;
	wc.hInstance      = hinst;
	wc.style          = 0;
	wc.lpfnWndProc    = Ex_WndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;

	if (!RegisterClass(&wc)) return FALSE;

	myTracker = new Tracker();

	PPSize windowSize = myTracker->getWindowSizeFromDatabase();
	pp_int32 scaleFactor = myTracker->getScreenScaleFactorFromDatabase();
	bool fullScreen = myTracker->getFullScreenFlagFromDatabase();
#ifdef __LOWRES__
	windowSize.width = 320;
	windowSize.height = 240;
#endif

	RECT rect;
	rect.left = rect.top = 0;
	rect.right = windowSize.width * scaleFactor;
	rect.bottom = windowSize.height * scaleFactor;
 
#ifdef FULLSCREEN
	AdjustWindowRect(&rect, WS_POPUP, false);
	hWnd = CreateWindow(c_szClassName,
							 WINDOWTITLE,
							 WS_POPUP/*|WS_SYSMENU/*|WS_MAXIMIZEBOX|WS_MINIMIZEBOX*/,CW_USEDEFAULT,CW_USEDEFAULT,
							 rect.right - rect.left, rect.bottom - rect.top,
							 NULL,
							 NULL,
							 g_hinst,
							 0);
#else
	AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false);
	hWnd = CreateWindow(c_szClassName,
							 WINDOWTITLE,
							 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
							 CW_USEDEFAULT,CW_USEDEFAULT,
							 rect.right - rect.left, rect.bottom - rect.top,
							 NULL,
							 NULL,
							 g_hinst,
							 0);
#endif

	DragAcceptFiles(hWnd, TRUE);

	HMENU hMenu = GetSystemMenu(hWnd, FALSE);

	AppendMenu(hMenu, MF_SEPARATOR, 0xFFFFFFFF, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_FULLSCREEN, _T("Fullscreen	ALT+RETURN"));
	AppendMenu(hMenu, MF_SEPARATOR, 0xFFFFFFFF, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_PREFERENCES, _T("Preferences..."));
	myPreferenceDialog = new CPreferencesDialog(hWnd, g_hinst);

	DWORD style = GetWindowLong(hWnd, GWL_EXSTYLE);

	ShowWindow(hWnd, nCmdShow);

	// Tracker init
	myDisplayDevice = new PPDisplayDevice(hWnd, windowSize.width, windowSize.height, scaleFactor);

	if (fullScreen)
		myDisplayDevice->goFullScreen(fullScreen);

	myTrackerScreen = new PPScreen(myDisplayDevice, myTracker);
	myTracker->setScreen(myTrackerScreen);

	// Startup procedure
	myTracker->startUp();
	HandleMidiRecording();

	return TRUE;
}

/*
 * This code is almost from:
 * http://www.codeguru.com/cpp/w-p/win32/article.php/c1427/
 *
 * The original writer of the code did derive this class from public std::vector<TCHAR*>
 * which is pretty bad ;)
 */
class CmdLineArgs
{
public:
	CmdLineArgs()
	{
		TCHAR* cmdline = GetCommandLine();
		// Save local copy of the command line string, because
		// ParseCmdLine() modifies this string while parsing it.
		m_cmdline = new TCHAR[_tcslen (cmdline) + 1];
		if (m_cmdline)
		{
			_tcscpy(m_cmdline, cmdline);
			ParseCmdLine();
		}
	}
	~CmdLineArgs()
	{
		delete[] m_cmdline;
	}

	const TCHAR* operator[](size_t index) const
	{
		return m_args[index];
	}

	size_t size() const 
	{
		return m_args.size();
	}

private:
	TCHAR* m_cmdline; // the command line string
	std::vector<TCHAR*> m_args;

	////////////////////////////////////////////////////////////////////////////////
	// Parse m_cmdline into individual tokens, which are delimited by spaces. If a
	// token begins with a quote, then that token is terminated by the next quote
	// followed immediately by a space or terminator.  This allows tokens to contain
	// spaces.
	// This input string:     This "is" a ""test"" "of the parsing" alg"o"rithm.
	// Produces these tokens: This, is, a, "test", of the parsing, alg"o"rithm
	////////////////////////////////////////////////////////////////////////////////
	void ParseCmdLine ()
	{
		enum { TERM  = '\0',
			QUOTE = '\"' };

		bool bInQuotes = false;
		TCHAR* pargs = m_cmdline;

		while (*pargs)
		{
			while (isspace (*pargs))        // skip leading whitespace
				pargs++;

			bInQuotes = (*pargs == QUOTE);  // see if this token is quoted

			if (bInQuotes)                  // skip leading quote
				pargs++;

			m_args.push_back (pargs);              // store position of current token

			// Find next token.
			// NOTE: Args are normally terminated by whitespace, unless the
			// arg is quoted.  That's why we handle the two cases separately,
			// even though they are very similar.
			if (bInQuotes)
			{
				// find next quote followed by a space or terminator
				while (*pargs &&
					!(*pargs == QUOTE && (isspace (pargs[1]) || pargs[1] == TERM)))
					pargs++;
				if (*pargs)
				{
					*pargs = TERM;  // terminate token
					if (pargs[1])   // if quoted token not followed by a terminator
						pargs += 2; // advance to next token
				}
			}
			else
			{
				// skip to next non-whitespace character
				while (*pargs && !isspace (*pargs))
					pargs++;
				if (*pargs && isspace (*pargs)) // end of token
				{
					*pargs = TERM;    // terminate token
					pargs++;         // advance to next token or terminator
				}
			}
		} // while (*pargs)
	} // ParseCmdLine()
}; // class CmdLineArgs

/****************************************************************************
 *
 *      Main
 *
 ****************************************************************************/
int PASCAL WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR szCmdLine, int nCmdShow)
{
	MSG msg;
	msg.wParam = 0;         /* In case something goes horribly wrong */

	if (!AppInit(hinst, nCmdShow))
		return -1;

	// Convert command line parameters
	CmdLineArgs args;
	
	if (args.size() >= 2)
	{
		// Retrieve second parameter (= input file)	
		PPSystemString fileInput = args[1];

		// When there is something specified, check if it's an existing file
		if (fileInput.length() && XMFile::exists(fileInput))
		{
			SendFile(fileInput);
		}
	}
	
	if (hWnd) 
	{

		MSG    Msg;

		while (GetMessage(&Msg, NULL, 0, 0))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	
	}

	delete myMidiInDevice;
	delete myMidiReceiver;

	delete myTracker;
	delete myTrackerScreen;
	delete myPreferenceDialog;

	delete g_globalMutex;

	if (myDisplayDevice->isFullScreen())
		myDisplayDevice->goFullScreen(false);

	delete myDisplayDevice;

	return (int)msg.wParam;

}
