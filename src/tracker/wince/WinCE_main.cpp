/*
 *  tracker/wince/WinCE_main.cpp
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

// ------------------ WinCE -------------------------------
#include <windows.h>
#include <aygshell.h>
#include <gx.h>
#include "WinCE_main.h"
#include "VirtualKeyToScanCodeTable.h"
// ------------------ tracker -----------------------------
#include "PPUI.h"
#include "PPMutex.h"
#include "Screen.h"
#include "Tracker.h"
#include "DisplayDevice_GAPI.h"
#include "ButtonMapper.h"
#include "WaitStateThread.h"
#include "LogoSmall.h"
#include "XMFile.h"
#include "GlobalColorConfig.h"

#define WINDOWS_DEFAULT_TIMER
#undef RESOLUTION_CHECK

#define MAX_LOADSTRING			100
#define DBLCLKTIME				1000
#define DRAGSENSITIVITY			2

HINSTANCE				hInst;					// The current instance
WNDCLASS				wc;
HWND					hwndCB;					// The command bar handle
HWND					hWnd;

BOOL					gxInit			= FALSE;
BOOL					taskBar			= TRUE;
BOOL					gxActive    	= FALSE;

PPMutex*				globalMutex 	= NULL;

unsigned short*			vScreen			= NULL;
PPSize					windowSize(DISPLAYDEVICE_WIDTH,DISPLAYDEVICE_HEIGHT);

// Global GAPI variables:
GXDisplayProperties		gx_displayprop;

#ifndef WINDOWS_DEFAULT_TIMER
	#include "WinCE_ThreadTimer.h"
	#define WM_MYTIMER (WM_USER + 10)

	static CThreadTimer			myThreadTimer;

	void MyThreadTimerProc(void* obj, UINT idEvent)
	{
		SendMessage(hWnd, WM_MYTIMER, 0, 0);
	}
#endif

// ------------------ Tracker --------------------------------------------------------------------------
EOrientation				orientation			= eOrientation90CW;
pp_int32					doublePixels		= FALSE;
pp_int32					allowVirtualKeys	= FALSE;
pp_int32					hideTaskBar			= TRUE;
pp_int32					dontTurnOffDevice	= FALSE;

static PPScreen*			myTrackerScreen = NULL;
static Tracker*				myTracker		= NULL;
static PPDisplayDevice*		myDisplayDevice = NULL;

// ------------------- Logger -----------------------------
#ifdef DEBUG
#include "Logger.h"
#include "Simple.h"
#include "PPSystem.h"
static CLogger*				logger			= NULL;
#endif

void				TrackerCreate();
void				TrackerStartUp(bool showSplash);
void				TrackerInitGUI(bool showSplash);

// ------------------ Forward declarations of functions included in this code module: ------------------
void				TaskBar(bool show);
ATOM				MyRegisterClass(HINSTANCE, LPTSTR);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// ------------------ debugging forwards ---------------------------------------------------------------
void				drawString(const char*, unsigned short*, unsigned int, unsigned int, unsigned int, unsigned short);

// ------------------- update screen -------------------------------------------------------------------
void				UpdateScreen(unsigned short*);
void				UpdateScreen();
void 				DrawBackground(bool shade = true);
bool 				NeedsDrawBackground();


//////////////////////////////////////////////////////////////////////////
// Exported functions:
// PPGetTickCount() is used for milisecond timing
// QueryKeyModifiers is used for detecting the state of the modifier keys
// SuspendFullScreen() is used to disable full screen mode
// ResumeFullScreen() is used to disable full screen mode
//////////////////////////////////////////////////////////////////////////
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

void SuspendFullScreen()
{
	GXCloseInput();
	GXCloseDisplay();
	TaskBar(true);
	gxActive = FALSE;
	ShowWindow(hWnd, SW_SHOW);
}

void ResumeFullScreen()
{
	ShowWindow(hWnd, SW_SHOW);
	TaskBar(false);
	SHSipPreference(hWnd, SIP_FORCEDOWN);
	GXOpenDisplay(hWnd, GX_FULLSCREEN);
	GXOpenInput();	
	gxActive = TRUE;

	// clear screen with shade if necessary 
	if (NeedsDrawBackground())
		DrawBackground();

	UpdateScreen();
}


//////////////////////////////////////////////////////////////////////////
// Taskbar handling
//////////////////////////////////////////////////////////////////////////
void TaskBar(bool show)
{
	if (!hideTaskBar)
		return;

	RECT rc;
	GetWindowRect( hWnd, &rc );
	HWND hWndTB = FindWindow(TEXT("HHTaskbar"),NULL);
	if (show && hWndTB)
	{
		if (taskBar) return;
		SHFullScreen( hWnd, SHFS_SHOWTASKBAR | SHFS_SHOWSIPBUTTON + SHFS_SHOWSTARTICON );
		ShowWindow( hWndTB, SW_SHOW );
		MoveWindow( hWnd, rc.left, rc.top + 26, rc.right, rc.bottom - 26, TRUE );
		taskBar = TRUE;
	}
	else if (hWndTB)
	{
		if (!taskBar) return;
		SHFullScreen( hWnd, SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON + SHFS_HIDESTARTICON );
		ShowWindow( hWndTB, SW_HIDE );
		MoveWindow( hWnd, rc.left, rc.top - 26, rc.right, rc.bottom + 26, TRUE );
		taskBar = FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////
// Init GAPI 
// To-do: Should go into DisplayDevice someday
//////////////////////////////////////////////////////////////////////////
BOOL InitGAPI(HWND hWnd)
{
	TaskBar(false);

	// Attempt to take over the screen
	if (GXOpenDisplay( hWnd, GX_FULLSCREEN) == 0)
		return FALSE;

	gxActive = TRUE;

	// Get display properties
	gx_displayprop = GXGetDisplayProperties();

#ifdef RESOLUTION_CHECK
	if ((gx_displayprop.cyHeight!=320)||(gx_displayprop.cxWidth!=240)) 
	{
		// Only dealing with 240x320 resolution in this code
		GXCloseDisplay();
		MessageBox(hWnd,L"Sorry, only supporting 240x320 display devices",L"Sorry!", MB_OK);
		return FALSE;	
	}
#endif

	// Check for 16 bit color display
	if (gx_displayprop.cBPP != 16)
	{
		// Only dealing with 16 bit color in this code
		GXCloseDisplay();
		MessageBox(hWnd,L"Sorry, only supporting 16bit color",L"Sorry!", MB_OK);
		return FALSE;
	}

	// Take over button handling
	GXOpenInput();

	globalMutex = new PPMutex();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Dispose GAPI
//////////////////////////////////////////////////////////////////////////
void ExitGAPI()
{
	if (gxActive)
	{
		// Clean up
		GXSuspend();
		GXCloseInput();
		GXCloseDisplay();
		TaskBar(true);
		gxActive = false;
	}
}

void HandleCommandLine(LPCTSTR lpCmdLine)
{
	// Retrieve input file
	PPSystemString fileInput(lpCmdLine);
	
	// When there is something specified, check if it's an existing file
	if (fileInput.length() && XMFile::exists(fileInput))
	{
		PPSystemString* strPtr = &fileInput;
		
		PPEvent event(eFileDragDropped, &strPtr, sizeof(PPSystemString*));
		myTrackerScreen->raiseEvent(&event);		
	}
}

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
#ifdef DEBUG
	SimpleString path(System::getConfigFileName(_T("milky.log")));
	CLogger _logger(path);
	logger = &_logger;
#endif

	MSG msg;
	HACCEL hAccelTable;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		TaskBar(true);
		return FALSE;
	}

	HandleCommandLine(lpCmdLine);

#ifndef WINDOWS_DEFAULT_TIMER
	HANDLE hThread = GetCurrentThread();
	SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
#endif

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_FIRSTGX);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete myTracker;
	delete myTrackerScreen;

	ExitGAPI();

	delete globalMutex;

	return msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS	wc;

    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) WndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= NULL;
	//wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAPI1));
    wc.hCursor			= 0;
    wc.hbrBackground	= (HBRUSH) GetStockObject(HOLLOW_BRUSH);
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= szWindowClass;

	return RegisterClass(&wc);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	TCHAR	szTitle[MAX_LOADSTRING];			// The title bar text
	TCHAR	szWindowClass[MAX_LOADSTRING];		// The window class name

	hInst = hInstance;		// Store instance handle in our global variable
	// Initialize global strings
	LoadString(hInstance, IDC_FIRSTGX, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

	//If it is already running, then focus on the window
	hWnd = FindWindow(szWindowClass, szTitle);	
	if (hWnd) 
	{
		SetForegroundWindow ((HWND) (((DWORD)hWnd) | 0x01));    
		return 0;
	} 

	MyRegisterClass(hInstance, szWindowClass);
	
	RECT	rect;
	GetClientRect(hWnd, &rect);
	
	hWnd = CreateWindow(szWindowClass, 
						szTitle, 
						WS_VISIBLE,
						CW_USEDEFAULT, 
						CW_USEDEFAULT, 
						CW_USEDEFAULT, 
						CW_USEDEFAULT, 
						NULL, 
						NULL, 
						hInstance, 
						NULL);
	if (!hWnd)
	{	
		return FALSE;
	}

	// This will also load local WinCE config
	InitButtonRemapper();

	if (!InitGAPI(hWnd))
		return FALSE;

	TrackerCreate();
	
	bool showSplash = myTracker->getShowSplashFlagFromDatabase();

	TrackerInitGUI(showSplash);
	TrackerStartUp(showSplash);
	
	UpdateScreen();

	// Create timers
#ifdef WINDOWS_DEFAULT_TIMER
	SetTimer(hWnd, 10, 20, NULL);
#else
	myThreadTimer.SetTimer (NULL, 1, 20, MyThreadTimerProc);
#endif
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Convert final coordinates from current orientation
//////////////////////////////////////////////////////////////////////////
void POINTFROMPARAM(PPPoint& point, int param)
{
	const unsigned int screenWidth = ::windowSize.width;
	const unsigned int screenHeight = ::windowSize.height;

	switch (orientation)
	{
		case eOrientation90CW:
		{
			int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenHeight<<doublePixels) >> 1);
			if (hCenter < 0) hCenter = 0;
			int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenWidth<<doublePixels) >> 1);
			if (vCenter < 0) vCenter = 0;

			point.y = LOWORD(param) - hCenter; 
			point.x = (gx_displayprop.cyHeight - 1 - (HIWORD(param) + vCenter));
			break;
		}

		case eOrientation90CCW:
		{
			int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenHeight<<doublePixels) >> 1);
			if (hCenter < 0) hCenter = 0;
			int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenWidth<<doublePixels) >> 1);
			if (vCenter < 0) vCenter = 0;

			point.y = gx_displayprop.cxWidth - 1 - (LOWORD(param) + hCenter); 
			point.x = HIWORD(param) - vCenter;
			break;
		}

		case eOrientationNormal:
			int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenWidth<<doublePixels) >> 1);
			if (hCenter < 0) hCenter = 0;
			int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenHeight<<doublePixels) >> 1);
			if (vCenter < 0) vCenter = 0;

			point.x = LOWORD(param) - hCenter; 
			point.y = HIWORD(param) - vCenter;
			break;
	}

	point.x >>= doublePixels;
	point.y >>= doublePixels;
}


void UpdateScreen(unsigned short* vScreen)
{
	if (!gxActive)
		return;

	globalMutex->lock();

	// Get the start of the screen memory from the GX function.
	unsigned short * buffer  = (unsigned short *) GXBeginDraw();
	unsigned short * line_buffer  = buffer;


	if (buffer == NULL) 
	{
		globalMutex->unlock();
		return;
	}

	const unsigned int screenWidth = ::windowSize.width;
	const unsigned int screenHeight = ::windowSize.height;

	const unsigned int hcbxpitch = gx_displayprop.cbxPitch >> 1;
	const unsigned int hcbypitch = gx_displayprop.cbyPitch >> 1;

	const unsigned int hRes = gx_displayprop.cxWidth >> doublePixels;
	const unsigned int vRes = gx_displayprop.cyHeight >> doublePixels;

	unsigned short* vscr = vScreen;

	if (gx_displayprop.ffFormat & kfDirect565)
	{

		switch (orientation)
		{
			case eOrientation90CW:
			{
				// PocketPC: gx_displayprop.cxWidth = 240, gx_displayprop.cyHeight = 320
				
				const unsigned int width = hRes > screenHeight ? screenHeight : hRes;
				const unsigned int height = vRes > screenWidth ? screenWidth : vRes;
				
				int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenHeight<<doublePixels) >> 1);
				if (hCenter < 0) hCenter = 0;
				int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenWidth<<doublePixels) >> 1);
				if (vCenter < 0) vCenter = 0;

				if (!doublePixels)
				{
					for (unsigned int x=0; x<width; x++)
					{
						unsigned short* pixel = (buffer+(x+hCenter)*hcbxpitch)+((gx_displayprop.cyHeight-1+vCenter)*hcbypitch);
						
						for (unsigned int y=0; y<height; y++)
						{
							*pixel = *vscr;
							pixel-=hcbypitch; 
							vscr++;
						}
						
						vscr+=screenWidth-height;
					}
				}
				else
				{
					for (unsigned int x=0; x<width; x++)
					{
						unsigned int y;

						unsigned short* pixel = (buffer+(x*2+hCenter)*hcbxpitch)+((gx_displayprop.cyHeight-1+vCenter)*hcbypitch);
						unsigned short* src = vscr;
						for (y=0; y<height; y++)
						{
							*pixel = *vscr;
							pixel-=hcbypitch; 
							*pixel = *vscr;
							pixel-=hcbypitch; 
							vscr++;
						}
						
						pixel = buffer+((x*2+1+hCenter)*hcbxpitch)+((gx_displayprop.cyHeight-1+vCenter)*hcbypitch);
						vscr = src;
						for (y=0; y<height; y++)
						{
							*pixel = *vscr;
							pixel-=hcbypitch; 
							*pixel = *vscr;
							pixel-=hcbypitch; 
							vscr++;
						}
						
						vscr+=screenWidth-height;
					}
				}
				break;
			};

			case eOrientation90CCW:
			{
				// PocketPC: gx_displayprop.cxWidth = 240, gx_displayprop.cyHeight = 320

				const unsigned int width = hRes > screenHeight ? screenHeight : hRes;
				const unsigned int height = vRes > screenWidth ? screenWidth : vRes;

				int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenHeight<<doublePixels) >> 1);
				if (hCenter < 0) hCenter = 0;
				int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenWidth<<doublePixels) >> 1);
				if (vCenter < 0) vCenter = 0;
				
				if (!doublePixels)
				{
					for (unsigned int x=0; x<width; x++)
					{
						unsigned short* pixel = (buffer+(gx_displayprop.cxWidth-1-x-hCenter)*hcbxpitch + (vCenter*hcbypitch));
						
						for (unsigned int y=0; y<height; y++)
						{
							*pixel = *vscr;
							pixel+=hcbypitch; 
							vscr++;
						}
						
						vscr+=screenWidth-height;
					}
				}
				else
				{
					for (unsigned int x=0; x<width; x++)
					{						
						unsigned int y;

						unsigned short* pixel = (buffer+(gx_displayprop.cxWidth-1-x*2-hCenter)*hcbxpitch + (vCenter*hcbypitch));
						unsigned short* src = vscr;
						for (y=0; y<height; y++)
						{
							*pixel = *vscr;
							pixel+=hcbypitch; 
							*pixel = *vscr;
							pixel+=hcbypitch; 
							vscr++;
						}
						
						pixel = (buffer+(gx_displayprop.cxWidth-1-(x*2+1)-hCenter)*hcbxpitch + (vCenter*hcbypitch));
						vscr = src;
						for (y=0; y<height; y++)
						{
							*pixel = *vscr;
							pixel+=hcbypitch; 
							*pixel = *vscr;
							pixel+=hcbypitch; 
							vscr++;
						}
						
						vscr+=screenWidth-height;
					}
				}

				break;
			};

			case eOrientationNormal:
			{
				const unsigned int width = hRes > screenWidth ? screenWidth : hRes;
				const unsigned int height = vRes > screenHeight ? screenHeight : vRes;
				
				int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenWidth<<doublePixels) >> 1);
				if (hCenter < 0) hCenter = 0;
				int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenHeight<<doublePixels) >> 1);
				if (vCenter < 0) vCenter = 0;

				if (!doublePixels)
				{
					for (unsigned int y=0; y<height; y++)
					{
						unsigned short* pixel = buffer + hCenter*hcbxpitch + (vCenter+y)*hcbypitch;
						for (unsigned int x=0; x<width; x++)
						{				
							*pixel=*vscr;
							pixel += hcbxpitch;
							vscr++;
						}
						
						vscr+=screenWidth-width;
					}
				}
				else
				{
					for (unsigned int y=0; y<height; y++)
					{
						unsigned int x;

						unsigned short* pixel = buffer + (y*2 + vCenter)*hcbypitch + hCenter*hcbxpitch;
						unsigned short* src = vscr;			
						for (x=0; x<width; x++)
						{				
							*pixel=*vscr;
							pixel += hcbxpitch;
							*pixel=*vscr;
							pixel += hcbxpitch;
							vscr++;
						}
						
						pixel = buffer + ((y*2+1) + vCenter)*hcbypitch + hCenter*hcbxpitch;
						vscr = src;
						for (x=0; x<width; x++)
						{				
							*pixel=*vscr;
							pixel += hcbxpitch;
							*pixel=*vscr;
							pixel += hcbxpitch;
							vscr++;
						}
						
						vscr+=screenWidth-width;
					}

				}

				break;
			};
		}

	}
	else
	{
		// 15 bit color is not supported
	}

	// End the drawing code
	GXEndDraw();
	
	globalMutex->unlock();
}

void UpdateScreenRegion(unsigned short* vScreen, const PPRect& rect)
{
	if (!gxActive)
		return;

	globalMutex->lock();

	// Get the start of the screen memory from the GX function.
	unsigned short * buffer  = (unsigned short *) GXBeginDraw();
	unsigned short * line_buffer  = buffer;

	if (buffer == NULL) 
	{
		globalMutex->unlock();
		return;
	}

	const unsigned int screenWidth = ::windowSize.width;
	const unsigned int screenHeight = ::windowSize.height;

	const unsigned int hcbxpitch = gx_displayprop.cbxPitch >> 1;
	const unsigned int hcbypitch = gx_displayprop.cbyPitch >> 1;

	const unsigned int hRes = gx_displayprop.cxWidth >> doublePixels;
	const unsigned int vRes = gx_displayprop.cyHeight >> doublePixels;

	unsigned short* vscr = vScreen + rect.y1 * screenWidth + rect.x1;

	if (gx_displayprop.ffFormat & kfDirect565)
	{

		switch (orientation)
		{
			case eOrientation90CW:
			{
				// PocketPC: gx_displayprop.cxWidth = 240, gx_displayprop.cyHeight = 320

				const unsigned int width = hRes > screenHeight ? screenHeight : hRes;
				const unsigned int height = vRes > screenWidth ? screenWidth : vRes;
			
				if (rect.x1 >= (signed)height || rect.y1 >= (signed)width)
					break;

				int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenHeight<<doublePixels) >> 1);
				if (hCenter < 0) hCenter = 0;
				int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenWidth<<doublePixels) >> 1);
				if (vCenter < 0) vCenter = 0;
				
				int rx2 = rect.x2;
				int ry2 = rect.y2;
				
				if (rx2 > (signed)height) rx2 = height;
				if (ry2 > (signed)width) ry2 = width;

				if (!doublePixels)
				{
					for (int x=rect.y1; x<ry2; x++)
					{
						unsigned short* pixel = (buffer+(x+hCenter)*hcbxpitch)+((gx_displayprop.cyHeight-1+vCenter)*hcbypitch) - (rect.x1*hcbypitch);
						
						for (int y=rect.x1; y<rx2; y++)
						{
							*pixel = *vscr;
							pixel-=hcbypitch; 
							vscr++;
						}
						
						vscr+=screenWidth - (rx2 - rect.x1);
					}
				}
				else
				{
					for (int x=rect.y1; x<ry2; x++)
					{
						int y;

						unsigned short* pixel = (buffer+(x*2+hCenter)*hcbxpitch)+((gx_displayprop.cyHeight-1+vCenter)*hcbypitch) - (rect.x1*2*hcbypitch);
						unsigned short* src = vscr;
						for (y=rect.x1; y<rx2; y++)
						{
							*pixel = *vscr;
							pixel-=hcbypitch; 
							*pixel = *vscr;
							pixel-=hcbypitch; 
							vscr++;
						}

						pixel = (buffer+(x*2+1+hCenter)*hcbxpitch)+((gx_displayprop.cyHeight-1+vCenter)*hcbypitch) - ((rect.x1*2)*hcbypitch);
						vscr = src;
						for (y=rect.x1; y<rx2; y++)
						{
							*pixel = *vscr;
							pixel-=hcbypitch; 
							*pixel = *vscr;
							pixel-=hcbypitch; 
							vscr++;
						}
						
						vscr+=screenWidth - (rx2 - rect.x1);
					}
				}
				break;
			};

			case eOrientation90CCW:
			{
				// PocketPC: gx_displayprop.cxWidth = 240, gx_displayprop.cyHeight = 320

				const unsigned int width = hRes > screenHeight ? screenHeight : hRes;
				const unsigned int height = vRes > screenWidth ? screenWidth : vRes;
			
				if (rect.x1 >= (signed)height || rect.y1 >= (signed)width)
					break;
				
				int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenHeight<<doublePixels) >> 1);
				if (hCenter < 0) hCenter = 0;
				int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenWidth<<doublePixels) >> 1);
				if (vCenter < 0) vCenter = 0;

				int rx2 = rect.x2;
				int ry2 = rect.y2;
				
				if (rx2 > (signed)height) rx2 = height;
				if (ry2 > (signed)width) ry2 = width;

				if (!doublePixels)
				{
					for (int x=rect.y1; x<ry2; x++)
					{
						unsigned short* pixel = (buffer+(gx_displayprop.cxWidth-1-x-hCenter)*hcbxpitch) + ((rect.x1+vCenter)*hcbypitch);
						
						for (int y=rect.x1; y<rx2; y++)
						{
							*pixel = *vscr;
							pixel+=hcbypitch; 
							vscr++;
						}
						
						vscr+=screenWidth - (rx2 - rect.x1);
					}
				}
				else
				{
					for (int x=rect.y1; x<ry2; x++)
					{
						int y;

						unsigned short* pixel = (buffer+(gx_displayprop.cxWidth-1-x*2-hCenter)*hcbxpitch) + (((rect.x1*2)+vCenter)*hcbypitch);
						unsigned short* src = vscr;
						for (y=rect.x1; y<rx2; y++)
						{
							*pixel = *vscr;
							pixel+=hcbypitch; 
							*pixel = *vscr;
							pixel+=hcbypitch; 
							vscr++;
						}

						pixel = (buffer+(gx_displayprop.cxWidth-1-(x*2+1)-hCenter)*hcbxpitch) + (((rect.x1*2)+vCenter)*hcbypitch);
						vscr = src;
						for (y=rect.x1; y<rx2; y++)
						{
							*pixel = *vscr;
							pixel+=hcbypitch; 
							*pixel = *vscr;
							pixel+=hcbypitch; 
							vscr++;
						}
						
						vscr+=screenWidth - (rx2 - rect.x1);
					}
				}
				break;
			};

			case eOrientationNormal:
			{
				int width = hRes > screenWidth ? screenWidth : hRes;
				int height = vRes > screenHeight ? screenHeight : vRes;
			
				if (rect.x1 >= width || rect.y1 >= height)
					break;
				
				int hCenter = (gx_displayprop.cxWidth >> 1) - ((screenWidth<<doublePixels) >> 1);
				if (hCenter < 0) hCenter = 0;
				int vCenter = (gx_displayprop.cyHeight >> 1) - ((screenHeight<<doublePixels) >> 1);
				if (vCenter < 0) vCenter = 0;

				int rx2 = rect.x2;
				int ry2 = rect.y2;
				
				if (rx2 > width) rx2 = width;
				if (ry2 > height) ry2 = height;
				
				if (!doublePixels)
				{
					for (int y=rect.y1; y<ry2; y++)
					{
						unsigned short* pixel = buffer+((rect.x1+hCenter)*hcbxpitch) + ((y+vCenter)*hcbypitch);
						for (int x=rect.x1; x<rx2; x++)
							
						{
							*pixel = *vscr;
							pixel+=hcbxpitch; 
							vscr++;
						}
						
						vscr+=screenWidth - (rx2 - rect.x1);
					}
				}
				else
				{
					for (int y=rect.y1; y<ry2; y++)
					{
						int x;

						unsigned short* pixel = buffer+((rect.x1*2+hCenter)*hcbxpitch) + ((y*2+vCenter)*hcbypitch);
						unsigned short* src = vscr;
						for (x=rect.x1; x<rx2; x++)							
						{
							*pixel = *vscr;
							pixel+=hcbxpitch; 
							*pixel = *vscr;
							pixel+=hcbxpitch; 
							vscr++;
						}
						
						pixel = buffer+((rect.x1*2+hCenter)*hcbxpitch) + ((y*2+1+vCenter)*hcbypitch);
						vscr = src;
						for (x=rect.x1; x<rx2; x++)							
						{
							*pixel = *vscr;
							pixel+=hcbxpitch; 
							*pixel = *vscr;
							pixel+=hcbxpitch; 
							vscr++;
						}

						vscr+=screenWidth - (rx2 - rect.x1);
					}
				}

				break;
			};
		}

	}
	else
	{
		// 15 bit color is not supported
	}

	// End the drawing code
	GXEndDraw();

	globalMutex->unlock();
}

void UpdateScreen()
{
	if (myTrackerScreen)
	{
		myTrackerScreen->paint();
	}
	UpdateScreen(vScreen);
}

void DrawBackground(bool shade)
{
	if (!gxActive)
		return;

	globalMutex->lock();

	// Get the start of the screen memory from the GX function.
	unsigned short * buffer  = (unsigned short *) GXBeginDraw();

	if (buffer == NULL) 
	{
		globalMutex->unlock();
		return;
	}

	const unsigned int screenWidth = gx_displayprop.cxWidth;
	const unsigned int screenHeight = gx_displayprop.cyHeight;

	const unsigned int hcbxpitch = gx_displayprop.cbxPitch >> 1;
	const unsigned int hcbypitch = gx_displayprop.cbyPitch >> 1;

	if (shade)
	{
		PPColor col = GlobalColorConfig::getInstance()->getColor(GlobalColorConfig::ColorTheme);
	
		PPColor srcCol = col, dstCol = col;
		srcCol.scaleFixed(32768);
	
		for (unsigned int y = 0; y < screenHeight; y++)
			for (unsigned int x = 0;  x < screenWidth; x++)
			{
				unsigned short* pixel = buffer+x*hcbxpitch+y*hcbypitch;
	
				int r,g,b;
				
				switch (orientation)
				{				
					case eOrientation90CW:
					{
						int s = (x*65536) / screenWidth;
						
						r = ((srcCol.r) * s + dstCol.r * (65535-s)) >> (16 + 3);
						g = ((srcCol.g) * s + dstCol.g * (65535-s)) >> (16 + 2);
						b = ((srcCol.b) * s + dstCol.b * (65535-s)) >> (16 + 3);
						break;
					}
					
					case eOrientation90CCW:
					{
						int s = (x*65536) / screenWidth;
						
						r = ((srcCol.r) * (65535-s) + dstCol.r * s) >> (16 + 3);
						g = ((srcCol.g) * (65535-s) + dstCol.g * s) >> (16 + 2);
						b = ((srcCol.b) * (65535-s) + dstCol.b * s) >> (16 + 3);
						break;
					}
	
					case eOrientationNormal:
					{
						int s = (y*65536) / screenHeight;
						
						r = ((srcCol.r) * s + dstCol.r * (65535-s)) >> (16 + 3);
						g = ((srcCol.g) * s + dstCol.g * (65535-s)) >> (16 + 2);
						b = ((srcCol.b) * s + dstCol.b * (65535-s)) >> (16 + 3);
						break;
					}
				}
				
				*pixel = (r << 11) + (g << 5) + b;
			}
	}
	else
	{
		for (unsigned int y = 0; y < screenHeight; y++)
			for (unsigned int x = 0;  x < screenWidth; x++)
			{
				unsigned short* pixel = buffer+x*hcbxpitch+y*hcbypitch;
				*pixel = 0;
			}	
	}

	// End the drawing code
	GXEndDraw();

	globalMutex->unlock();
}

bool NeedsDrawBackground()
{
	switch (orientation)
	{				
		case eOrientation90CCW:
		case eOrientation90CW:
		{
			return (gx_displayprop.cxWidth != (unsigned)windowSize.height) || (gx_displayprop.cyHeight != (unsigned)windowSize.width);
		}
					
		case eOrientationNormal:
		{
			return (gx_displayprop.cxWidth != (unsigned)windowSize.width) || (gx_displayprop.cyHeight != (unsigned)windowSize.height);
		}
	}
			
	return false;
}

void TrackerCreate()
{
	myTracker = new Tracker();

	windowSize = myTracker->getWindowSizeFromDatabase();
#ifdef __LOWRES__
	windowSize.width = DISPLAYDEVICE_WIDTH;
	windowSize.height = DISPLAYDEVICE_HEIGHT;
#endif

	// Allocate virtual screen
	vScreen = new unsigned short[windowSize.width*windowSize.height];

	// Clear virtual screen
	memset(vScreen, 0, windowSize.width*windowSize.height*sizeof(unsigned short));
	
	myDisplayDevice = new PPDisplayDevice(hWnd, windowSize.width, windowSize.height);
	
	myDisplayDevice->setSize(windowSize);
	
	myTrackerScreen = new PPScreen(myDisplayDevice, myTracker);
	myTracker->setScreen(myTrackerScreen);
}


void TrackerStartUp(bool showSplash)
{
	// Startup procedure
	myTracker->startUp(true);

	WaitStateThread::getInstance()->activate(FALSE);

	myTrackerScreen->enableDisplay(true);	

	if (showSplash)
		myTracker->hideSplash();

	// clear screen with shade if necessary 
	if (NeedsDrawBackground())
		DrawBackground();

	myTrackerScreen->paint(false);
}

void TrackerInitGUI(bool showSplash)
{
	// clear screen with black if necessary 
	if (NeedsDrawBackground())
		DrawBackground(false);

	if (showSplash)
		myTracker->showSplash();
	
	myTrackerScreen->enableDisplay(false);	

	// Put init message on screen
	drawString("initializing", vScreen, windowSize.width, (windowSize.width>>1)-12*4, (windowSize.height>>1)-12, 0xFFFF);

	WaitStateThread::getInstance()->setDisplayResolution(windowSize.width, windowSize.height);

	WaitStateThread::getInstance()->activate(TRUE, TRUE, FALSE);
}

#ifdef DEBUG
void LogWinMsg(CLogger& logger, LPCTSTR msg, LPARAM lParam)
{
	TCHAR dummy[1024];

	wsprintf(dummy, _T("%x"), lParam);

	SimpleString logstr(msg);
	logstr.append(_T(": "));
	logstr.append(dummy);

	logger.Log(logstr);
}


void LogMouseDown(CLogger& logger, LPARAM lParam, const PPPoint& point)
{
	TCHAR dummy[1024];

	wsprintf(dummy, _T("Before: %i, %i"), LOWORD(lParam), HIWORD(lParam));

	SimpleString logstr(dummy);
	logstr.append(_T(" "));
	wsprintf(dummy, _T("After: %i, %i"), point.x, point.y);
	logstr.append(dummy);

	logger.Log(logstr);
}
#endif

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool     LMouseDrag  = false;
	static PPPoint	LMouseDragStartPoint;
	static bool     RMouseDrag  = false;
	static PPPoint	RMouseDragStartPoint;

	static DWORD	ltime;
	static PPPoint	llastClickPosition = PPPoint(0,0);
	static WORD		lClickCount = 0;
	
	static BOOL		lMouseDown = FALSE;
	static DWORD	lButtonDownStartTime;
	
	static BOOL		rMouseDown = FALSE;
	static DWORD	rtime;
	static PPPoint	rlastClickPosition = PPPoint(0,0);
	static WORD		rClickCount = 0;
	
	static DWORD	rButtonDownStartTime;
	
	static DWORD	timerTicker = 0;
	static DWORD	idleResetCounter = 1;

	static PPPoint	p;

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
#ifdef WINDOWS_DEFAULT_TIMER
		case WM_PAINT:
			DefWindowProc(hWnd, msg, wParam, lParam);
			return 0;
#else
		case WM_PAINT:
			return 0;
#endif

		case WM_SIZE:
		{
#ifdef DEBUG
			LogWinMsg(*logger, _T("WM_SIZE"), lParam);
#endif
			break;
		}

		// ----- left mousebutton -------------------------------
		case WM_LBUTTONDOWN:
		{	
			if (!myTrackerScreen || rMouseDown)
				break;

			POINTFROMPARAM(p, lParam);

#ifdef DEBUG
			LogWinMsg(*logger, _T("WM_LBUTTONDOWN"), lParam);
			LogMouseDown(*logger, lParam, p);
#endif

			PPEvent myEvent(eLMouseDown, &p, sizeof(PPPoint));			
			myTrackerScreen->raiseEvent(&myEvent);
			
			lMouseDown = TRUE;
			//lButtonDownStartTime = timerTicker;
			lButtonDownStartTime = GetTickCount();

			if (!lClickCount)
			{
				ltime = GetTickCount();				
				POINTFROMPARAM(llastClickPosition, lParam);
			}
			else if (lClickCount == 2)
			{
				DWORD deltat = GetTickCount() - ltime;				
				if (deltat > DBLCLKTIME)
				{
					lClickCount = 0;
					ltime = GetTickCount();
					POINTFROMPARAM(llastClickPosition, lParam);
				}
			}
			lClickCount++;	
			break;
		}

		// ----- right mousebutton -------------------------------
		case WM_RBUTTONDOWN:
		{
			if (!myTrackerScreen || lMouseDown)
				break;
			
			POINTFROMPARAM(p, lParam);

			PPEvent myEvent(eRMouseDown, &p, sizeof(PPPoint));			
			myTrackerScreen->raiseEvent(&myEvent);
			
			rMouseDown = TRUE;
			//rButtonDownStartTime = timerTicker;
			rButtonDownStartTime = GetTickCount();

			if (!rClickCount)
			{
				rtime = GetTickCount();
				POINTFROMPARAM(rlastClickPosition, lParam);
			}
			else if (rClickCount == 2)
			{
				DWORD deltat = GetTickCount() - rtime;				
				if (deltat > DBLCLKTIME)
				{
					rClickCount = 0;
					rtime = GetTickCount();
					POINTFROMPARAM(rlastClickPosition, lParam);
				}
			}
			rClickCount++;	
			break;
		}
		
		// ----- left mousebutton -------------------------------
		case WM_LBUTTONUP:
		{
			LMouseDrag = false;

			if (!myTrackerScreen || !lMouseDown)
				break;

			lClickCount++;

			if (lClickCount == 4)
			{
				DWORD deltat = GetTickCount() - ltime;
				
				if (deltat < DBLCLKTIME)
				{
					POINTFROMPARAM(p, lParam);
					
					if (abs(p.x - llastClickPosition.x) < 4 &&
						abs(p.y - llastClickPosition.y) < 4)
					{

						PPEvent myEvent(eLMouseDoubleClick, &p, sizeof(PPPoint));						
						myTrackerScreen->raiseEvent(&myEvent);
					}
				}
				
				lClickCount = 0;

			}
				
			POINTFROMPARAM(p, lParam);
			PPEvent myEvent(eLMouseUp, &p, sizeof(PPPoint));			
			myTrackerScreen->raiseEvent(&myEvent);			
			lMouseDown = FALSE;			
			break;
		}		

		// ----- right mousebutton -------------------------------
		case WM_RBUTTONUP:
		{
			RMouseDrag = false;

			if (!myTrackerScreen || !rMouseDown)
				break;

			rClickCount++;

			if (rClickCount == 4)
			{
				DWORD deltat = GetTickCount() - rtime;				
				if (deltat < DBLCLKTIME)
				{
					POINTFROMPARAM(p, lParam);					
					if (abs(p.x - rlastClickPosition.x) < 4 &&
						abs(p.y - rlastClickPosition.y) < 4)
					{
						PPEvent myEvent(eRMouseDoubleClick, &p, sizeof(PPPoint));						
						myTrackerScreen->raiseEvent(&myEvent);
					}
				}
				
				rClickCount = 0;
			}
				
			POINTFROMPARAM(p, lParam);
			PPEvent myEvent(eRMouseUp, &p, sizeof(PPPoint));			
			myTrackerScreen->raiseEvent(&myEvent);			
			rMouseDown = FALSE;			
			break;
		}		

		case WM_MOUSEMOVE:
		{
			if (!myTrackerScreen)
				break;

			if ((wParam&MK_LBUTTON) && lMouseDown)
			{
				POINTFROMPARAM(p, lParam);
				
				if (!LMouseDrag)
				{
					LMouseDragStartPoint = p;
					LMouseDrag = true;
				}

				if (LMouseDrag && (abs(LMouseDragStartPoint.x - p.x) >= DRAGSENSITIVITY ||
					abs(LMouseDragStartPoint.y - p.y) >= DRAGSENSITIVITY))
				{
					PPEvent myEvent(eLMouseDrag, &p, sizeof(PPPoint));				
					myTrackerScreen->raiseEvent(&myEvent);
				}
			
			}
			else if ((wParam&MK_RBUTTON) && rMouseDown)
			{
				POINTFROMPARAM(p, lParam);
				
				if (!RMouseDrag)
				{
					RMouseDragStartPoint = p;
					RMouseDrag = true;
				}

				if (RMouseDrag && (abs(RMouseDragStartPoint.x - p.x) >= DRAGSENSITIVITY ||
					abs(RMouseDragStartPoint.y - p.y) >= DRAGSENSITIVITY))
				{

					PPEvent myEvent(eRMouseDrag, &p, sizeof(PPPoint));
					myTrackerScreen->raiseEvent(&myEvent);
				}

			}
			else
			{
				POINTFROMPARAM(p, lParam);				
				PPEvent myEvent(eMouseMoved, &p, sizeof(PPPoint));				
				myTrackerScreen->raiseEvent(&myEvent);
			}
			break;
		}

#ifdef WINDOWS_DEFAULT_TIMER
		case WM_TIMER:
#else
		case WM_MYTIMER:
#endif
		{
			if (!myTrackerScreen)
				break;

			if (!(timerTicker % 1))
			{
				PPEvent myEvent(eTimer);			
				myTrackerScreen->raiseEvent(&myEvent);
			}

			// reset idle timer, so the device will not turn off
			// automatically
			// handle with care and use only when forced to do so
			if (!(idleResetCounter & 1023) && dontTurnOffDevice)
			{
				SystemIdleTimerReset();
			}

			idleResetCounter++;
			timerTicker++;
			DWORD currentTime = GetTickCount();
			
			if (lMouseDown &&
				(currentTime - lButtonDownStartTime) > 500)
			{
				PPEvent myEvent(eLMouseRepeat, &p, sizeof(PPPoint));				
				myTrackerScreen->raiseEvent(&myEvent);
			}
			else if (rMouseDown &&
				(currentTime - rButtonDownStartTime) > 500)
			{
				PPEvent myEvent(eRMouseRepeat, &p, sizeof(PPPoint));				
				myTrackerScreen->raiseEvent(&myEvent);
			}

			break;
		}

		case WM_CLOSE:
		{
			if (myTrackerScreen && myTracker)
			{
				PPEvent e(eAppQuit);
				myTrackerScreen->raiseEvent(&e);
				
				bool res = myTracker->shutDown();
				
				if (res)
					DestroyWindow(hWnd);
			}
			else
			{
				DestroyWindow(hWnd);
			}

			break;
		}

		case WM_SETTINGCHANGE: 
			TaskBar(false); 
			break; 

		
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:

#ifdef DEBUG
			::PostMessage(hWnd, WM_CLOSE, 0, 0);
#endif

			if (wParam < 256)
			{
				if (mappings[wParam].keyModifiers != 0xFFFF &&
					mappings[wParam].virtualKeyCode != 0xFFFF)
				{
					pp_int32 i;

					WORD vkModifiers[3] = {VK_ALT, VK_SHIFT, VK_CONTROL};

					for (i = 0; i < 3; i++)
						if (mappings[wParam].keyModifiers & (1 << i))
						{
							// modifier key:
							// scan code and character code remains zero
							pp_uint16 vk[3] = {vkModifiers[i], 0, 0};
							PPEvent eventKeyDown(eKeyDown, &vk, sizeof(vk));
							myTrackerScreen->raiseEvent(&eventKeyDown);
						}

					setForceKeyModifier((KeyModifiers)mappings[wParam].keyModifiers);

					// functionality key:
					// scan code and character code remains zero
					pp_uint16 vk[3] = {mappings[wParam].virtualKeyCode, 0, 0};
					PPEvent eventKeyDown(eKeyDown, &vk, sizeof(vk));			
					myTrackerScreen->raiseEvent(&eventKeyDown);					
				}
				else if (allowVirtualKeys)
				{
					QueryKeyModifiers();
					
					WORD character = MapVirtualKey(wParam & 0xFFFF, 2);

					// Check for right ALT key
					if (::GetAsyncKeyState(VK_RMENU)>>15)
						wParam = VK_RMENU;
					// Check for right CTRL key
					else if (::GetAsyncKeyState(VK_RCONTROL)>>15)
						wParam = VK_RCONTROL;			
					
					if (wParam == VK_MENU)
						wParam = VK_ALT;
					
					// Since scancodes probably differ from desktop keyboards
					// we do some virtual key => scancode transformation here
					WORD chr[3] = {wParam, vkeyToScancode[wParam&255], character}; 
					
					PPEvent myEvent(eKeyDown, &chr, sizeof(chr));
					
					myTrackerScreen->raiseEvent(&myEvent);
				}
			}

			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:			
			if (wParam < 256)
			{
				if (mappings[wParam].keyModifiers != 0xFFFF &&
					mappings[wParam].virtualKeyCode != 0xFFFF)
				{
					pp_int32 i;

					WORD vkModifiers[3] = {VK_ALT, VK_SHIFT, VK_CONTROL};

					// functionality key:
					// scan code and character code remains zero
					pp_uint16 vk[3] = {mappings[wParam].virtualKeyCode, 0, 0};
					PPEvent eventKeyUp(eKeyUp, &vk, sizeof(vk));			
					myTrackerScreen->raiseEvent(&eventKeyUp);
					
					for (i = 0; i < 3; i++)
						if (mappings[wParam].keyModifiers & (1 << (2-i)))
						{
							// modifier key:
							// scan code and character code remains zero
							pp_uint16 vk[3] = {vkModifiers[i], 0, 0};
							PPEvent eventKeyUp(eKeyUp, &vk, sizeof(vk));
							myTrackerScreen->raiseEvent(&eventKeyUp);
						}
					
					clearForceKeyModifier((KeyModifiers)mappings[wParam].keyModifiers);
				}
				else if (allowVirtualKeys)
				{					
					WORD character = MapVirtualKey(wParam & 0xFFFF, 2);

					if (wParam == VK_MENU)
						wParam = VK_ALT;

					// Since scancodes probably differ from desktop keyboards
					// we do some virtual key => scancode transformation here
					WORD chr[3] = {wParam, vkeyToScancode[wParam&255], character}; 
					
					PPEvent myEvent(eKeyUp, &chr, sizeof(chr));
					
					myTrackerScreen->raiseEvent(&myEvent);
				}
			}

			break;

		case WM_CHAR:
		{
			WORD chr = wParam; 

			if (chr < 32 || chr > 127)
				break;
				
			PPEvent myEvent(eKeyChar, &chr, sizeof(chr));
			
			myTrackerScreen->raiseEvent(&myEvent);
			
			break;
		}
		
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_KILLFOCUS:
			GXSuspend();
			break;

		case WM_SETFOCUS:
			GXResume();
			break;

		/*case WM_SETTINGCHANGE:
			SHHandleWMSettingChange(hWnd, wParam,lParam,&s_sai);
     		break;*/
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// -------------------------- debugging --------------------------------------------------------------------- //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void drawChar(unsigned short* buffer, unsigned int pitch, unsigned int chr,unsigned int tx,unsigned int ty,unsigned short col)
{
	PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);

    unsigned int x,y;
 
    for (y=0;y<8;y++)
        for (x=0;x<8;x++) 
			if (font->getPixelBit(chr, x, y)) 
                buffer[((ty+y)*pitch+(tx+x))]=col;
}

void drawString(const char* textBuffer,unsigned short* drawBuffer, unsigned int pitch, unsigned int posx,unsigned int posy, unsigned short col)
{
    unsigned int x = posx;
    while (*textBuffer) 
	{
        drawChar(drawBuffer, pitch, *textBuffer,x,posy,col);
        textBuffer++;
        x+=8;
    }
}
