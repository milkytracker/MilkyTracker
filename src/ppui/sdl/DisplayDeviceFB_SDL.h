/*
 *  ppui/sdl/DisplayDeviceFB_SDL.h
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
 *  12/5/14 - Dale Whinham
 *    - Port to SDL2
 *
 */

/////////////////////////////////////////////////////////////////
//
//	Our display device
//
/////////////////////////////////////////////////////////////////
#ifndef __DISPLAYDEVICEFB_H__
#define __DISPLAYDEVICEFB_H__

#include "DisplayDevice_SDL.h"

class PPDisplayDeviceFB : public PPDisplayDevice
{
private:
	bool needsTemporaryBuffer;
	pp_uint8* temporaryBuffer;
	pp_uint32 temporaryBufferPitch, temporaryBufferBPP;
	
	// used for rotating coordinates etc.
	void swap(const PPRect& r);

public:
	PPDisplayDeviceFB(pp_int32 width,
					  pp_int32 height, 
					  pp_int32 scaleFactor,
					  pp_int32 bpp, 
					  bool fullScreen,
					  Orientations theOrientation = ORIENTATION_NORMAL, 
					  bool swapRedBlue = false);
				  
	virtual ~PPDisplayDeviceFB();

	virtual bool supportsScaling() const { return true; }
	virtual void setSize(const PPSize& size);

	virtual PPGraphicsAbstract* open();
	virtual void close();

	void update();
	void update(const PPRect& r);
protected:
	SDL_Surface* theSurface;
	SDL_Texture* theTexture;
	SDL_Renderer* theRenderer;
};

#endif
