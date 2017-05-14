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

PPDisplayDevice::PPDisplayDevice(HWND hWnd, pp_int32 width, pp_int32 height, pp_int32 scaleFactor/* = 1*/) :
	PPDisplayDeviceBase(width, height, scaleFactor),
	m_waitWindowVisible(false),
	m_hThread(NULL),
	m_threadID(0)
{
	m_BitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	m_BitmapInfo.biWidth = getSize().width;
	m_BitmapInfo.biHeight = -getSize().height;
	m_BitmapInfo.biPlanes = 1;
	m_BitmapInfo.biBitCount = 24;
	m_BitmapInfo.biCompression = BI_RGB;
	m_BitmapInfo.biSizeImage = 0;
	m_BitmapInfo.biXPelsPerMeter = 0;
	m_BitmapInfo.biYPelsPerMeter = 0;
	m_BitmapInfo.biClrUsed = 0;
	m_BitmapInfo.biClrImportant = 0;
	
	HDC hScreenDC = ::GetWindowDC(NULL);
	
	m_hBitmap = ::CreateDIBSection(hScreenDC, 
								   (LPBITMAPINFO)&m_BitmapInfo, 
								   DIB_RGB_COLORS,
								   (LPVOID *)&m_pBits, 
								   NULL, 
								   0); 
	
	::ReleaseDC(NULL,hScreenDC);				    

	m_hWnd = hWnd;

	m_hDC = NULL;

	// initialise graphics instance
	currentGraphics = new PPGraphics_BGR24(getSize().width, getSize().height, (getSize().width * 24) / 8, m_pBits);
	currentGraphics->lock = true;
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

void PPDisplayDevice::setSize(const PPSize& size)
{
	// Important note: Save this size before the window 
	// is actually resized using SetWindowPos (below)
	this->size = size;

	RECT rc;

	::GetWindowRect(m_hWnd, &rc);

	rc.right = rc.left + size.width * scaleFactor;
	rc.bottom = rc.top + size.height * scaleFactor;
	::AdjustWindowRect(&rc, GetWindowLong(m_hWnd, GWL_STYLE), false);

	::SetWindowPos(m_hWnd, 
				   NULL, 
				   rc.left, 
				   rc.top, 
				   rc.right - rc.left,
				   rc.bottom - rc.top,
				   SWP_SHOWWINDOW | SWP_NOZORDER);

}

void PPDisplayDevice::adjustWindowSize()
{
	PPSize size = this->size;
	setSize(size);
}

static BOOL SetFullScreen (HWND hWnd, int w, int h)
{
	if (!::IsWindow(hWnd))
		return FALSE;

	DEVMODE dmScreen;

	int frequency = -1;
	if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreen))
	{
		frequency = dmScreen.dmDisplayFrequency;
	}

	dmScreen.dmSize			= sizeof ( DEVMODE );
	dmScreen.dmPelsWidth	= w;
	dmScreen.dmPelsHeight	= h;
	if (frequency != -1)
		dmScreen.dmDisplayFrequency	= frequency;

	dmScreen.dmFields		= DM_PELSWIDTH | DM_PELSHEIGHT | (frequency == -1 ? 0 : DM_DISPLAYFREQUENCY);

	// Try with current frequency first
    if ( ::ChangeDisplaySettings( &dmScreen, CDS_FULLSCREEN | CDS_RESET) == DISP_CHANGE_SUCCESSFUL )
		return TRUE;

	// Try without setting frequency
	dmScreen.dmFields		= DM_PELSWIDTH | DM_PELSHEIGHT;
    if ( ::ChangeDisplaySettings( &dmScreen, CDS_FULLSCREEN | CDS_RESET) == DISP_CHANGE_SUCCESSFUL )
		return TRUE;

	return FALSE;
}

bool PPDisplayDevice::goFullScreen(bool b)
{
	if (b)
	{
		::GetWindowRect(m_hWnd, &m_lastRect);

		if (::SetFullScreen(m_hWnd, size.width * scaleFactor, size.height * scaleFactor))
		{
			m_lastWindowStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
			m_lastWindowExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

			::SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS);
			::SetWindowLong(m_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

			::ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
		    
            ::SetWindowPos(m_hWnd, HWND_TOPMOST, 
                           0, 
					       0, 
					       0, 0, SWP_NOSIZE);
			
			bFullScreen = true;
		}
		else 
		{
			bFullScreen = false;
			return false;
		}
	}
	else
	{
		::ChangeDisplaySettings(NULL, 0);
		
		::SetWindowLong(m_hWnd, GWL_STYLE, m_lastWindowStyle);
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, m_lastWindowExStyle);

		::SetWindowPos(m_hWnd, 
					   HWND_NOTOPMOST, 
					   m_lastRect.left, 
					   m_lastRect.top, 
					   m_lastRect.right - m_lastRect.left, 
					   m_lastRect.bottom - m_lastRect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

		bFullScreen = false;
	}

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
