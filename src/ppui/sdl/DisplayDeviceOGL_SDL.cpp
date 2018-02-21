/*
 *  ppui/sdl/DisplayDeviceOGL_SDL.cpp
 *
 *  Copyright 2009 Peter Barth, Christopher O'Neill
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

#include "DisplayDeviceOGL_SDL.h"
#include "Graphics.h"

#ifdef __OPENGL__

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

PPDisplayDeviceOGL::PPDisplayDeviceOGL(pp_int32 width, 
									   pp_int32 height, 
									   pp_int32 scaleFactor,
									   pp_int32 bpp,
									   bool fullScreen, 
									   Orientations theOrientation/* = ORIENTATION_NORMAL*/, 
									   bool swapRedBlue/* = false*/) :
	PPDisplayDevice(width, height, bpp, scaleFactor, fullScreen, theOrientation)
{
	/* Set a video mode */
	theWindow = CreateWindow(width, height, bpp, SDL_WINDOW_OPENGL | (bFullScreen==true ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
	if (theWindow == NULL)
	{
		fprintf(stderr, "SDL: Could not create window: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	
	/* Get SDL OpenGL context */
	glContext = SDL_GL_CreateContext(theWindow);
	if (glContext == NULL)
	{
		fprintf(stderr, "SDL: Could not create OpenGL context: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

    /* Sets up OpenGL double buffering */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	/* Use immediate updates; don't wait for Vblank */
	SDL_GL_SetSwapInterval(0);

#if 0
	CGLError err;
	CGLContextObj ctx = CGLGetCurrentContext();
	
	// Enable the multi-threading
	err =  CGLEnable( ctx, kCGLCEMPEngine);
	
	if (err != kCGLNoError )
	{
		exit(EXIT_FAILURE);
	}    	
#endif
	
	currentGraphics = new PPGraphics_OGL(width, height);	
	currentGraphics->lock = true;
}

PPDisplayDeviceOGL::~PPDisplayDeviceOGL()
{
	SDL_GL_DeleteContext(glContext);
	// base class is responsible for deleting currentGraphics
}

PPGraphicsAbstract* PPDisplayDeviceOGL::open()
{
	if (!isEnabled())
		return NULL;

	if (currentGraphics->lock)
	{
		currentGraphics->lock = false;

		return currentGraphics;
	}
	
	return NULL;
}

void PPDisplayDeviceOGL::close()
{
	currentGraphics->lock = true;
}

void PPDisplayDeviceOGL::update()
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	if (!currentGraphics->lock)
		return;
	
	SDL_GL_SwapWindow(theWindow);
}

void PPDisplayDeviceOGL::update(const PPRect& r)
{
	update();
}

#endif
