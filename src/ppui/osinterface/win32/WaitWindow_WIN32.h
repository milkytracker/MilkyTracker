/*
 *  ppui/osinterface/win32/WaitWindow_WIN32.h
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

#ifndef WAITWINDOW__H
#define WAITWINDOW__H

#include <windows.h>
#include "tchar.h"
#include "BasicTypes.h"

class WaitWindow
{
private:
	static WaitWindow*		instance;

	// Windows stuff
	static TCHAR			szClassName[];
	
	HWND					hWnd;

	// Window callback
	static LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

	// Render my own window content
	BITMAPINFOHEADER		m_BitmapInfo; // <- this should be saved, eg make it a member variable
	HBITMAP					m_hBitmap;
	LPBYTE					m_pBits;
	HDC						m_hDC;

	void					init();

	// Blit my content into DC
	void					blit(HWND hWnd, HDC pDC, 
								 pp_int32 x, pp_int32 y, 
								 pp_int32 width, pp_int32 height);
	

	void					render();

	PPColor					color;

	WaitWindow();

public:
	static WaitWindow* getInstance();

	~WaitWindow();

	void show();
	void hide();

	void move(pp_int32 x, pp_int32 y);

	void update() { render(); }

	pp_int32 getWidth() { return 260; }
	pp_int32 getHeight() { return 80; }

	void setColor(const PPColor& color) { this->color = color; }
};

#endif
