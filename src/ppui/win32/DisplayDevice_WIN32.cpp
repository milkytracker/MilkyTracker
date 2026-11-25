/*
 *  ppui/win32/DisplayDevice_WIN32.cpp
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

#include "DisplayDevice_WIN32.h"
#include "Graphics.h"
#include "WaitWindow_WIN32.h"
#include "PPSystem.h"

// A 24 bit DIB's rows are padded to 4 bytes
static pp_int32 bitmapPitch(pp_int32 width)
{
	return ((width * 3) + 3) & ~3;
}

PPDisplayDevice::PPDisplayDevice(HWND hWnd, pp_int32 width, pp_int32 height, pp_int32 scaleFactor/* = 1*/) :
	PPDisplayDeviceBase(width, height, scaleFactor),
	m_waitWindowVisible(false),
	m_hThread(NULL),
	m_threadID(0)
{
	m_BitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	m_BitmapInfo.biPlanes = 1;
	m_BitmapInfo.biBitCount = 24;
	m_BitmapInfo.biCompression = BI_RGB;
	m_BitmapInfo.biSizeImage = 0;
	m_BitmapInfo.biXPelsPerMeter = 0;
	m_BitmapInfo.biYPelsPerMeter = 0;
	m_BitmapInfo.biClrUsed = 0;
	m_BitmapInfo.biClrImportant = 0;

	createBitmap();

	m_hWnd = hWnd;

	m_hDC = NULL;

	// initialise graphics instance
	currentGraphics = new PPGraphics_BGR24(getSize().width, getSize().height, bitmapPitch(getSize().width), m_pBits);
	currentGraphics->lock = true;
}

void PPDisplayDevice::createBitmap()
{
	m_BitmapInfo.biWidth = getSize().width;
	m_BitmapInfo.biHeight = -getSize().height;

	HDC hScreenDC = ::GetWindowDC(NULL);

	m_hBitmap = ::CreateDIBSection(hScreenDC,
								   (LPBITMAPINFO)&m_BitmapInfo,
								   DIB_RGB_COLORS,
								   (LPVOID *)&m_pBits,
								   NULL,
								   0);

	::ReleaseDC(NULL,hScreenDC);
}

PPDisplayDevice::~PPDisplayDevice()
{	
	delete currentGraphics;
}

PPGraphicsAbstract* PPDisplayDevice::open()
{
	if (!isEnabled())
		return NULL;		
		
	currentGraphics->lock = false;

	return currentGraphics;
}

void PPDisplayDevice::blit(HWND hWnd, HDC pDC, pp_int32 x, pp_int32 y, pp_int32 width, pp_int32 height)
{
	RECT r;
 
	GetClientRect(hWnd,&r);
  
	HDC hBitmapDC = ::CreateCompatibleDC(NULL);
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hBitmapDC, m_hBitmap);
 
	if (scaleFactor == 1)
		::BitBlt(pDC, x, y, width, height, hBitmapDC, x, y, SRCCOPY);
	else
		::StretchBlt(pDC, x*scaleFactor, y*scaleFactor, width*scaleFactor, height*scaleFactor, hBitmapDC, x, y, width, height, SRCCOPY);

	::SelectObject(hBitmapDC, hOldBitmap);
	::DeleteDC(hBitmapDC);
}

void PPDisplayDevice::close()
{
	currentGraphics->lock = true;
}

void PPDisplayDevice::update()
{
	if (m_hDC != NULL)
		return;
	
	if (!isUpdateAllowed() || !isEnabled())
		return;	

	m_hDC = ::GetDC(m_hWnd);

	blit(m_hWnd, m_hDC, 0, 0, getSize().width, getSize().height);

	// the pixel or so a scaled screen leaves of the window to its right and below
	RECT client, strip;
	::GetClientRect(m_hWnd, &client);
	::SetRect(&strip, getSize().width * scaleFactor, 0, client.right, client.bottom);
	::FillRect(m_hDC, &strip, (HBRUSH)::GetStockObject(BLACK_BRUSH));
	::SetRect(&strip, 0, getSize().height * scaleFactor, client.right, client.bottom);
	::FillRect(m_hDC, &strip, (HBRUSH)::GetStockObject(BLACK_BRUSH));

	::ReleaseDC(m_hWnd, m_hDC);

	m_hDC = NULL;
}

void PPDisplayDevice::update(const PPRect& r)
{
	if (m_hDC != NULL)
		return;
	
	if (!isUpdateAllowed() || !isEnabled())
		return;	
	
	m_hDC = ::GetDC(m_hWnd);

	blit(m_hWnd, m_hDC, r.x1, r.y1, r.width(), r.height());

	::ReleaseDC(m_hWnd, m_hDC);

	m_hDC = NULL;
}

extern HCURSOR g_cursorStandard;
extern HCURSOR g_cursorResizeWE;
extern HCURSOR g_cursorHand;

void PPDisplayDevice::setTitle(const PPSystemString& title)
{
	::SetWindowText(m_hWnd, title);
}

// The window is this size already (WM_SIZE, Tracker::setScreenSize): the bitmap the screen is drawn into follows it
void PPDisplayDevice::setSize(const PPSize& size)
{
	if (size == this->size)
		return;

	this->size = size;

	::DeleteObject(m_hBitmap);
	createBitmap();

	static_cast<PPGraphicsFrameBuffer*>(currentGraphics)->setBufferProperties(size.width, size.height, bitmapPitch(size.width), m_pBits);
}

// Borderless over the whole monitor, without changing its display mode: the tracker is laid out again for the monitor's size (WM_SIZE)
bool PPDisplayDevice::goFullScreen(bool b)
{
	if (b)
	{
		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (!::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &monitorInfo))
		{
			bFullScreen = false;
			return false;
		}

		::GetWindowRect(m_hWnd, &m_lastRect);
		m_lastWindowStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
		m_lastWindowExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

		bFullScreen = true;

		::SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS);
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

		const RECT& r = monitorInfo.rcMonitor;
		::SetWindowPos(m_hWnd, HWND_TOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
	}
	else
	{
		bFullScreen = false;

		::SetWindowLong(m_hWnd, GWL_STYLE, m_lastWindowStyle);
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, m_lastWindowExStyle);

		::SetWindowPos(m_hWnd, 
					   HWND_NOTOPMOST, 
					   m_lastRect.left, 
					   m_lastRect.top, 
					   m_lastRect.right - m_lastRect.left, 
					   m_lastRect.bottom - m_lastRect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
	}

	return true;
}

// The window made the size of a screen of size at scaleFactor; WM_SIZE lays the tracker out for it. Fullscreen keeps the monitor's size, and the window it goes back to takes the new one
bool PPDisplayDevice::resizeWindow(const PPSize& size, pp_int32 scaleFactor)
{
	this->scaleFactor = scaleFactor;

	RECT rect = { 0, 0, size.width * scaleFactor, size.height * scaleFactor };
	if (bFullScreen)
	{
		::AdjustWindowRect(&rect, m_lastWindowStyle, FALSE);
		m_lastRect.right = m_lastRect.left + rect.right - rect.left;
		m_lastRect.bottom = m_lastRect.top + rect.bottom - rect.top;
	}
	else
	{
		if (::IsZoomed(m_hWnd))
			::ShowWindow(m_hWnd, SW_RESTORE);
		::AdjustWindowRect(&rect, ::GetWindowLong(m_hWnd, GWL_STYLE), FALSE);
		::SetWindowPos(m_hWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	// laid out at the new scale even when the window kept its size, and drawn again all over
	::SendMessage(m_hWnd, WM_SIZE, SIZE_RESTORED, 0);
	::InvalidateRect(m_hWnd, NULL, FALSE);
	return true;
}

void PPDisplayDevice::shutDown()
{
	::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

DWORD WINAPI PPDisplayDevice::UpdateWindowThreadProc(LPVOID lpParameter)
{
	PPDisplayDevice* thisDisplayDevice = (PPDisplayDevice*)lpParameter;

	while (true)
	{
		if (thisDisplayDevice->m_waitWindowVisible)
			WaitWindow::getInstance()->update(); 
	
		System::msleep(100);
	}

	::ExitThread(0);
	return 0;
}


void PPDisplayDevice::signalWaitState(bool b, const PPColor& color)
{
	waitBarColor = color;
	
	if (b)
	{
		RECT bounds;
		::GetWindowRect(m_hWnd, &bounds);

		WaitWindow::getInstance()->move(bounds.left + ((bounds.right-bounds.left) >> 1) - (WaitWindow::getInstance()->getWidth()>>1),
										bounds.top + ((bounds.bottom-bounds.top) >> 1) - (WaitWindow::getInstance()->getHeight()>>1));
		
		WaitWindow::getInstance()->setColor(color);
		
		WaitWindow::getInstance()->show();

		m_waitWindowVisible = true;

		if (!m_hThread)
		{
			m_hThread = CreateThread(NULL, 
								     0, 
									 &UpdateWindowThreadProc, 
								     (void*)this, 
								     0,
								     &m_threadID);
		}
	}
	else
	{
		m_waitWindowVisible = false;
		WaitWindow::getInstance()->hide();
	}
}

void PPDisplayDevice::setMouseCursor(MouseCursorTypes type)
{
	currentCursorType = type;
	
	switch (type)
	{
		case MouseCursorTypeStandard:
			::SetCursor(g_cursorStandard);
			break;
			
		case MouseCursorTypeResizeLeft:
			::SetCursor(g_cursorResizeWE);
			break;

		case MouseCursorTypeResizeRight:
			::SetCursor(g_cursorResizeWE);
			break;

		case MouseCursorTypeHand:
			::SetCursor(g_cursorHand);
			break;
	}
}
