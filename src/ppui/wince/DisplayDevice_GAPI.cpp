/*
 *  ppui/wince/DisplayDevice_GAPI.cpp
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

#include "DisplayDevice_GAPI.h"
#include "Graphics.h"
#include "gx.h"
#include "WaitStateThread.h"

// virtual screen comes from outside
extern unsigned short *vScreen;

// Also the Update functions
void UpdateScreen(unsigned short* vScreen);
void UpdateScreenRegion(unsigned short* vScreen, const PPRect& r);

PPDisplayDevice::PPDisplayDevice(HWND hWnd, pp_uint32 width, pp_uint32 height) :
	PPDisplayDeviceBase(width, height)  
{
	currentGraphics = new PPGraphics_16BIT(width, height, (width * 16) / 8, (pp_uint8*)vScreen);

	this->hWnd = hWnd;
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

void PPDisplayDevice::close()
{
	currentGraphics->lock = true;
}

void PPDisplayDevice::update()
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	::UpdateScreen(vScreen);
}

void PPDisplayDevice::update(const PPRect& r)
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	::UpdateScreenRegion(vScreen, r);
}

void PPDisplayDevice::setTitle(const PPSystemString& title)
{
	::SetWindowText(hWnd, title);
}

void PPDisplayDevice::shutDown()
{
	::PostMessage(hWnd, WM_CLOSE, 0, 0);
}

void PPDisplayDevice::signalWaitState(bool b, const PPColor& color)
{
	WaitStateThread::getInstance()->activate(b);
}
