/*
 *  ppui/sdl/DisplayDeviceOGL_SDL.h
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

/* Using the OpenGL API for drawing/font operations turned out to be very CPU
 * intensive.. This code left here for historic reasons (edit the relevant
 * parts of src/tracker/Makefile.am if you want to play around).
 */

#ifndef __DISPLAYDEVICEOGL_H__
#define __DISPLAYDEVICEOGL_H__

#include "DisplayDevice_SDL.h"

class PPDisplayDeviceOGL : public PPDisplayDevice
{
public:
	PPDisplayDeviceOGL(pp_int32 width,
					   pp_int32 height, 
					   pp_int32 scaleFactor,
					   pp_int32 bpp, 
					   bool fullScreen,
					   Orientations theOrientation = ORIENTATION_NORMAL, 
					   bool swapRedBlue = false);
				  
	virtual ~PPDisplayDeviceOGL();

	virtual PPGraphicsAbstract* open();
	virtual void close();

	void update();
	void update(const PPRect& r);
protected:
	SDL_GLContext glContext;
};

#endif
