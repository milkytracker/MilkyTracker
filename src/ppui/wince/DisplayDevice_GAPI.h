/*
 *  ppui/wince/DisplayDevice_GAPI.h
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
	HWND hWnd;

public:
	PPDisplayDevice(HWND hWnd, pp_uint32 width, pp_uint32 height);
	virtual ~PPDisplayDevice();

	virtual PPGraphicsAbstract* open();
	virtual void close();

	void update();
	void update(const PPRect& r);

	// ----------------------------- ex. PPWindow ----------------------------
	virtual void setTitle(const PPSystemString& title);	
	virtual void shutDown();
	virtual void signalWaitState(bool b, const PPColor& color);
};

#endif
