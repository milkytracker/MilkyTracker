/*
 *  ppui/sdl/DisplayDevice_SDL.h
 *
 *  Copyright 2009 Peter Barth, Dale Whinham
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
#ifndef __DISPLAYDEVICE_H__
#define __DISPLAYDEVICE_H__

#include "BasicTypes.h"
#include "DisplayDeviceBase.h"

#include <SDL.h>
#include <SDL_opengl.h>

typedef const GLubyte *(APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);

// Forwards
class PPGraphicsAbstract;

class PPDisplayDevice : public PPDisplayDeviceBase
{
public:
	enum Orientations
	{
		// Orientations
		ORIENTATION_NORMAL,
		ORIENTATION_ROTATE90CCW,
		ORIENTATION_ROTATE90CW
	};

protected:
	pp_int32 realWidth, realHeight;
	SDL_Window* theWindow;
	Orientations orientation;
	int drv_index;
	
	SDL_Window* CreateWindow(pp_int32& w, pp_int32& h, pp_int32& bpp, Uint32 flags);

	// used for rotating coordinates etc.
	void adjust(pp_int32& x, pp_int32& y);

	// Mouse pointers
	SDL_Cursor *cursorStandard, *cursorResizeHoriz, *cursorEggtimer, *cursorHand;
	void initMousePointers();

public:
	PPDisplayDevice(pp_int32 width,
					pp_int32 height, 
					pp_int32 scaleFactor,
					pp_int32 bpp, 
					bool fullScreen,
					Orientations theOrientation = ORIENTATION_NORMAL);
				  
	virtual ~PPDisplayDevice();
	
	void transform(pp_int32& x, pp_int32& y);
	void transformInverse(pp_int32& x, pp_int32& y);
	void transformInverse(PPRect& r);

	Orientations getOrientation() { return orientation; }
	
	SDL_Window* getWindow();

	// ----------------------------- ex. PPWindow ----------------------------
	virtual void setTitle(const PPSystemString& title);	
	virtual bool goFullScreen(bool b);
	virtual PPSize getDisplayResolution() const;
	virtual void shutDown();
	virtual void signalWaitState(bool b, const PPColor& color);
	virtual void setMouseCursor(MouseCursorTypes type);
};

#endif
