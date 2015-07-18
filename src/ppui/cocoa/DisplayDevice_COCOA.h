/*
 *  ppui/cocoa/DisplayDevice_COCOA.h
 *
 *  Copyright 2014 Dale Whinham
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
#include "Graphics.h"
#include "PPSystem.h"

#include <Cocoa/Cocoa.h>
#include "AppDelegate.h"
#include "MTTrackerView.h"

class PPDisplayDevice : public PPDisplayDeviceBase
{
public:
	PPDisplayDevice(NSWindow* window, MTTrackerView* trackerView, pp_int32 width, pp_int32 height, pp_int32 scaleFactor = 1, pp_uint32 bpp = 32);
	virtual ~PPDisplayDevice();

	virtual PPGraphicsAbstract* open();
	virtual void close();

	virtual void update();

	virtual void update(const PPRect& r);
	
	// Window handling
	virtual void setTitle(const PPSystemString& title);	
	virtual void setSize(const PPSize& size);
	virtual bool supportsScaling() const { return false; }
	
	virtual bool goFullScreen(bool b);
	virtual PPSize getDisplayResolution() const;
	virtual void shutDown();
	virtual void signalWaitState(bool b, const PPColor& color);
	virtual void setMouseCursor(MouseCursorTypes type);
	void setImmediateUpdates(BOOL b) { immediateUpdates = b; };

private:
	NSWindow* theWindow;
	MTTrackerView* theTrackerView;
	uint8_t* pixelBuffer;
	BOOL immediateUpdates;
};

#endif

