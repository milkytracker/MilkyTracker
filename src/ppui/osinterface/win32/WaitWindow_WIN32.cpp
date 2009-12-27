/*
 *  ppui/osinterface/win32/WaitWindow_WIN32.cpp
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

#include "WaitWindow_WIN32.h"
#include "DisplayDeviceBase.h"

extern HINSTANCE g_hinst;                /* My instance handle */

WaitWindow* WaitWindow::instance = NULL;

TCHAR WaitWindow::szClassName[] = _T("MILKYTRACKERWAITWNDCLASS");

LRESULT CALLBACK WaitWindow::WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
		case WM_PAINT:
			WaitWindow::getInstance()->render();
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void WaitWindow::init()
{
	m_BitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	m_BitmapInfo.biWidth = getWidth();
	m_BitmapInfo.biHeight = -getHeight();
	m_BitmapInfo.biPlanes = 1;
	m_BitmapInfo.biBitCount = 24;
	m_BitmapInfo.biCompression = BI_RGB;
	m_BitmapInfo.biSizeImage = 0;
	m_BitmapInfo.biXPelsPerMeter = 0;
	m_BitmapInfo.biYPelsPerMeter = 0;
	m_BitmapInfo.biClrUsed = 0;
	m_BitmapInfo.biClrImportant = 0;
	
	HDC hScreenDC = ::GetWindowDC(NULL);
	
	m_hBitmap = ::CreateDIBSection(hScreenDC, (LPBITMAPINFO)&m_BitmapInfo, DIB_RGB_COLORS,(LPVOID *)&m_pBits, NULL, 0); 
	
	::ReleaseDC(NULL, hScreenDC);				    

	m_hDC = NULL;
}

void DrawPixel24(void* buffer, pp_int32 x, pp_int32 y, pp_int32 pitch, const PPColor& color)
{
	unsigned char* buff = (unsigned char*)buffer;
	buff[y*pitch+x*3] = (unsigned char)color.b;
	buff[y*pitch+x*3+1] = (unsigned char)color.g;
	buff[y*pitch+x*3+2] = (unsigned char)color.r;
}

void WaitWindow::render()
{
	::DrawWaitBar(getWidth(), getHeight(), 
				  160, 16,
				  0xFFFFFF, (color.r << 16) + (color.g << 8) + (color.b),
				  m_pBits, getWidth()*3, &DrawPixel24);	

	m_hDC = ::GetDC(hWnd);

	blit(hWnd, m_hDC, 0, 0, getWidth(), getHeight());

	::ReleaseDC(hWnd, m_hDC);

	m_hDC = NULL;
}

void WaitWindow::blit(HWND hWnd, HDC pDC, pp_int32 x, pp_int32 y, pp_int32 width, pp_int32 height)
{
	RECT r;

	::GetClientRect(hWnd,&r);
	
	HDC hBitmapDC = ::CreateCompatibleDC(NULL);
	
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hBitmapDC, m_hBitmap);
	
	::BitBlt(pDC, x, y, width, height, hBitmapDC, x, y, SRCCOPY);
	
	::SelectObject(hBitmapDC, hOldBitmap);
	
	::DeleteDC(hBitmapDC);
}

WaitWindow::WaitWindow()
{
	WNDCLASS wc;

	wc.hCursor        = NULL;
	wc.hIcon          = (HICON)::LoadIcon(NULL, IDI_APPLICATION);
	wc.lpszMenuName   = NULL;
	wc.lpszClassName  = szClassName;
	wc.hbrBackground  = 0;
	wc.hInstance      = g_hinst;
	wc.style          = 0;
	wc.lpfnWndProc    = WndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;

	::RegisterClass(&wc);
 
	hWnd = ::CreateWindow(szClassName,
						  _T("Working..."),
						  0/*|WS_MAXIMIZEBOX|WS_MINIMIZEBOX*/,CW_USEDEFAULT,CW_USEDEFAULT,
					  	  getWidth()+::GetSystemMetrics(SM_CXEDGE)*2+2,
						  getHeight()+::GetSystemMetrics(SM_CYCAPTION)+2+::GetSystemMetrics(SM_CYEDGE)*2,
					   	  NULL,
						  NULL,
						  g_hinst,
						  0);

	init();
}

WaitWindow* WaitWindow::getInstance()
{
	if (instance == NULL)
		instance = new WaitWindow();

	return instance;
}

WaitWindow::~WaitWindow()
{
	::SendMessage(hWnd, WM_CLOSE, 0, 0);
}

void WaitWindow::show()
{
	::ShowWindow(hWnd, SW_SHOW);
}

void WaitWindow::hide()
{
	::ShowWindow(hWnd, SW_HIDE);
}

void WaitWindow::move(pp_int32 x, pp_int32 y)
{
	::SetWindowPos(hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
}
