/*
 *  ppui/sdl/DisplayDevice_SDL.h
 *
 *  Copyright 2008 Peter Barth
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

#include <SDL/SDL.h>

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
		ORIENTATION_ROTATE180,		// unsupported
		ORIENTATION_ROTATE90CW,
		ORIENTATION_UNKNOWN
	};

private:
	pp_int32 realWidth, realHeight;
	SDL_Surface* theSurface;
	Orientations orientation;
	
	pp_uint8* temporaryBuffer;
	pp_uint32 temporaryBufferPitch, temporaryBufferBPP;
	
	SDL_Surface* CreateScreen(Uint16 w, Uint16 h, Uint8 bpp, Uint32 flags);

	// used for rotating coordinates etc.
	void adjust(pp_int32& x, pp_int32& y);
	void swap(const PPRect& r);

	// Mouse pointers
	SDL_Cursor *cursorStandard, *cursorResizeLeft, *cursorResizeRight, *cursorEggtimer, *cursorHand;
	void initMousePointers();
	static Uint8 resizeLeft_data[], resizeLeft_mask[];
	static Uint8 resizeRight_data[], resizeRight_mask[];
	static Uint8 eggtimer_data[], eggtimer_mask[];
	static Uint8 hand_data[], hand_mask[];
	
public:
	PPDisplayDevice(SDL_Surface*& screen, 
				  pp_int32 width, 
				  pp_int32 height, 
				  pp_int32 bpp, 
				  bool fullScreen,
				  Orientations theOrientation = ORIENTATION_NORMAL, 
				  bool swapRedBlue = false);
				  
	virtual ~PPDisplayDevice();

	virtual PPGraphicsAbstract* open();
	virtual void close();

	void update();
	void update(const PPRect& r);
	
	void transform(pp_int32& x, pp_int32& y);
	void transformInverse(pp_int32& x, pp_int32& y);
	void transformInverse(PPRect& r);
	
	Orientations getOrientation() { return orientation; }
	
	// ----------------------------- ex. PPWindow ----------------------------
	virtual void setTitle(const PPSystemString& title);	
	virtual void setSize(const PPSize& size);
	virtual bool goFullScreen(bool b);
	virtual void shutDown();
	virtual void signalWaitState(bool b, const PPColor& color);
	virtual void setMouseCursor(MouseCursorTypes type);
};

#endif
