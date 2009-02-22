/*
 *  ppui/carbon/DisplayDevice_CARBON.h
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

#include <carbon/carbon.h>
#include <pthread.h>
#include "PPSystem.h"

class PPDisplayDevice : public PPDisplayDeviceBase
{
private:
	struct GWorldWrapper
	{
		Rect		pictSize;
		GWorldPtr	offscreen;
		PixMap*		ppixMap;
		
		RgnHandle 	visibleRgn;
		
		GWorldWrapper(pp_uint32 width, pp_uint32 height, pp_uint32 bpp = 32);
		~GWorldWrapper();
		
		void UnlockGWorldMemory(unsigned char** Buffer, unsigned int* BytesPerRow);
		void LockGWorldMemory();
	};

	WindowPtr	mainWindow, waitWindow;

	GWorldWrapper* mainGWorld;
	GWorldWrapper* waitGWorld;

	CFDictionaryRef oldmode;
	CFDictionaryRef mode;
	Rect oldRc;
	
public:
	PPDisplayDevice(WindowPtr mainWindow, WindowPtr waitWindow, 
					pp_int32 width, pp_int32 height, pp_int32 scaleFactor = 1,
					pp_uint32 bpp = 32);
	virtual ~PPDisplayDevice();

	virtual PPGraphicsAbstract* open();
	virtual void close();

	virtual void update();

	virtual void update(const PPRect& r);
	
	// Window handling
	virtual bool init();
	virtual void setTitle(const PPSystemString& title);	
	virtual void setSize(const PPSize& size);
	virtual bool supportsScaling() const { return true; }
	
	virtual bool goFullScreen(bool b);
	virtual PPSize getDisplayResolution() const;
	virtual void shutDown();
	virtual void signalWaitState(bool b, const PPColor& color);
	virtual void setMouseCursor(MouseCursorTypes type);
	
private:
	pp_int32		waitWindowWidth;
	pp_int32		waitWindowHeight;

	pthread_t		drawThread;
	bool			waitWindowVisible;

	void			updateWaitWindow();
	void			renderAndUpdateWaitWindow(pp_uint32 width, pp_uint32 height, const PPColor& waitBarColor);	

	static void*	UpdateWaitWindowThreadProc(void* theID);
};

#endif

