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

PPDisplayDeviceOGL::PPDisplayDeviceOGL(
#if !SDL_VERSION_ATLEAST(2, 0, 0)
									   SDL_Surface*& screen, 
#endif
									   pp_int32 width, 
									   pp_int32 height, 
									   pp_int32 scaleFactor,
									   pp_int32 bpp,
									   bool fullScreen, 
									   Orientations theOrientation/* = ORIENTATION_NORMAL*/, 
									   bool swapRedBlue/* = false*/) :
	PPDisplayDevice(
#if !SDL_VERSION_ATLEAST(2, 0, 0)
screen, 
#endif
width, height, bpp, scaleFactor, fullScreen, theOrientation)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
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
#else
	const SDL_VideoInfo* videoinfo;

	/* Some SDL to get display format */
	videoinfo = SDL_GetVideoInfo();
	if (bpp == -1) 
	{
		bpp = videoinfo->vfmt->BitsPerPixel > 16 ? videoinfo->vfmt->BitsPerPixel : 16;
	}

    Uint32 videoFlags = SDL_OPENGL;    /* Enable OpenGL in SDL          */
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering       */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */


    /* This checks to see if surfaces can be stored in memory */
    if (videoinfo->hw_available)
	videoFlags |= videoinfo->hw_available ? SDL_HWSURFACE : SDL_SWSURFACE;

    /* This checks if hardware blits can be done */
    if (videoinfo->blit_hw)
		videoFlags |= SDL_HWACCEL;
#endif

	/* Sets up OpenGL double buffering */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	/* Use immediate updates; don't wait for Vblank */
	SDL_GL_SetSwapInterval(0);
#else
	/* Set a video mode */
	theSurface = screen = CreateScreen(realWidth, realHeight, 
									   bpp, videoFlags | (bFullScreen==true ? SDL_FULLSCREEN : 0));
	if (NULL == screen) 
	{
		fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());	
		exit(2);
	}
#endif

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
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GL_DeleteContext(glContext);
#endif
	// base class is responsible for deleting currentGraphics
}

PPGraphicsAbstract* PPDisplayDeviceOGL::open()
{
	if (!isEnabled())
		return NULL;

	if (currentGraphics->lock)
	{
#if !SDL_VERSION_ATLEAST(2, 0, 0)
		if (SDL_LockSurface(theSurface) < 0)
			return NULL;
#endif

		currentGraphics->lock = false;

		return currentGraphics;
	}
	
	return NULL;
}

void PPDisplayDeviceOGL::close()
{
#if !SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_UnlockSurface(theSurface);
#endif
	currentGraphics->lock = true;
}

void PPDisplayDeviceOGL::update()
{
	if (!isUpdateAllowed() || !isEnabled())
		return;

#if SDL_VERSION_ATLEAST(2, 0, 0)	
	if (!currentGraphics->lock)
		return;
	
	SDL_GL_SwapWindow(theWindow);
#else
	if (theSurface->locked)
	{
		return;
	}
	
	SDL_GL_SwapBuffers();
#endif
}

void PPDisplayDeviceOGL::update(const PPRect& r)
{
	update();
}

#endif
