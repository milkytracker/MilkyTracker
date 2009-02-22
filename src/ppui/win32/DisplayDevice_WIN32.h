/*
 *  ppui/win32/DisplayDevice_WIN32.h
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

/////////////////////////////////////////////////////////////////
//
//	Our display device
//
/////////////////////////////////////////////////////////////////
#ifndef DISPLAYDEVICE__H
#define DISPLAYDEVICE__H

#include "BasicTypes.h"
#include "DisplayDeviceBase.h"

#include <windows.h>

// Forwards
class PPGraphicsAbstract;

class PPDisplayDevice : public PPDisplayDeviceBase
{
private:
	BITMAPINFOHEADER	m_BitmapInfo; 
	HBITMAP				m_hBitmap;
	LPBYTE				m_pBits;
	HWND				m_hWnd;
	RECT				m_lastRect;
	LONG				m_lastWindowStyle, m_lastWindowExStyle;
	HDC					m_hDC;

	bool				m_waitWindowVisible;
	HANDLE				m_hThread;
	DWORD				m_threadID;

	void blit(HWND hWnd, HDC pDC, pp_int32 x, pp_int32 y, pp_int32 width, pp_int32 height);

public:
	PPDisplayDevice(HWND hWnd, pp_int32 width, pp_int32 height, pp_int32 scaleFactor = 1);
	virtual ~PPDisplayDevice();

	virtual PPGraphicsAbstract* open();
	virtual void close();

	void update();
	void update(const PPRect& r);

	void adjustWindowSize();

	virtual void setSize(const PPSize& size);

	virtual bool supportsScaling() const { return true; }

	// ----------------------------- ex. PPWindow ----------------------------
public:
	virtual void setTitle(const PPSystemString& title);	
	virtual bool goFullScreen(bool b);
	virtual void shutDown();
	virtual void signalWaitState(bool b, const PPColor& color);
	virtual void setMouseCursor(MouseCursorTypes type);

	static DWORD WINAPI UpdateWindowThreadProc(LPVOID lpParameter);
};

#endif
