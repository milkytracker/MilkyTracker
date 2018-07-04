/*
 *  ppui/sdl/DisplayDevice_SDL.cpp
 *
 *  Copyright 2009 Peter Barth, Christopher O'Neill, Dale Whinham
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

#include "DisplayDevice_SDL.h"
#include "Graphics.h"

SDL_Window* PPDisplayDevice::CreateWindow(pp_int32& w, pp_int32& h, pp_int32& bpp, Uint32 flags)
{
	char rendername[256] = { 0 };
	PFNGLGETSTRINGPROC glGetStringAPI = NULL;

	for (int it = 0; it < SDL_GetNumRenderDrivers(); it++)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(it,&info);

		strncat(rendername, info.name, 9);
		strncat(rendername, " ", 1);

		if (strncmp("opengles2", info.name, 9) == 0)
		{
			drv_index = it;
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
		}
	}

	// Create SDL window
	SDL_Window* theWindow = SDL_CreateWindow("MilkyTracker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | flags);

	if (theWindow == NULL)
	{
		fprintf(stderr, "SDL: SDL_CreateWindow (width: %d, height: %d) failed: %s\n", w, h, SDL_GetError());
		fprintf(stderr, "Retrying with default size...");

		w = getDefaultWidth();
		h = getDefaultHeight();
		
		theWindow = SDL_CreateWindow("MilkyTracker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | flags);
		
		if (theWindow == NULL)
		{
			fprintf(stderr, "SDL: SDL_CreateWindow (width: %d, height: %d) failed: %s\n", w, h, SDL_GetError());
			fprintf(stderr, "Giving up.\n");
			return NULL;
		}
	}

	SDL_GLContext ctx = SDL_GL_CreateContext(theWindow);
	SDL_GL_MakeCurrent(theWindow, ctx);
	
	glGetStringAPI = (PFNGLGETSTRINGPROC)SDL_GL_GetProcAddress("glGetString");

	fprintf(stdout, "Available Renderers: %s\n", rendername);
	if (glGetStringAPI)
	{
		fprintf(stdout, "Vendor     : %s\n", glGetStringAPI(GL_VENDOR));
		fprintf(stdout, "Renderer   : %s\n", glGetStringAPI(GL_RENDERER));
		fprintf(stdout, "Version    : %s\n", glGetStringAPI(GL_VERSION));
#ifdef DEBUG
		fprintf(stdout, "Extensions : %s\n", glGetStringAPI(GL_EXTENSIONS));
#endif
	}
	// Prevent window from being resized below minimum
	SDL_SetWindowMinimumSize(theWindow, w, h);
	fprintf(stderr, "SDL: Minimum window size set to %dx%d.\n", w, h);

	return theWindow;
}

PPDisplayDevice::PPDisplayDevice(pp_int32 width,
								 pp_int32 height, 
								 pp_int32 scaleFactor,
								 pp_int32 bpp,
								 bool fullScreen, 
								 Orientations theOrientation/* = ORIENTATION_NORMAL*/) :
	PPDisplayDeviceBase(width, height, scaleFactor),
	realWidth(width), realHeight(height),
	orientation(theOrientation)
{
	adjust(realWidth, realHeight);

	bFullScreen = fullScreen;

	drv_index = -1;

	initMousePointers();
}

PPDisplayDevice::~PPDisplayDevice()
{	
	delete currentGraphics;
}

void PPDisplayDevice::adjust(pp_int32& x, pp_int32& y)
{
	switch (orientation)
	{
		case ORIENTATION_NORMAL:
			break;
			
		case ORIENTATION_ROTATE90CCW:
		case ORIENTATION_ROTATE90CW:
		{
			pp_int32 h = x;
			x = y;
			y = h;
			break;
		}
	}	
	
	x *= scaleFactor;
	y *= scaleFactor;
}

void PPDisplayDevice::transform(pp_int32& x, pp_int32& y)
{
	pp_int32 h;

	switch (orientation)
	{
		case ORIENTATION_NORMAL:
			break;
			
		case ORIENTATION_ROTATE90CW:
			h = x;
			x = y;
			y = realWidth - 1 - h;
			break;
			
		case ORIENTATION_ROTATE90CCW:
			h = x;
			x = realHeight - 1 - y;
			y = h;
			break;
	}
}

void PPDisplayDevice::transformInverse(pp_int32& x, pp_int32& y)
{
	pp_int32 h;

	switch (orientation)
	{
		case ORIENTATION_NORMAL:
			break;
			
		case ORIENTATION_ROTATE90CW:
			h = x;
			x = realWidth - y;
			y = h;
			break;
			
		case ORIENTATION_ROTATE90CCW:
			h = x;
			x = y;
			y = realHeight - h;
			break;
	}
}

void PPDisplayDevice::transformInverse(PPRect& r)
{
	transformInverse((pp_int32&)r.x1, (pp_int32&)r.y1);	
	transformInverse((pp_int32&)r.x2, (pp_int32&)r.y2);	

	pp_int32 h;
	if (r.x2 < r.x1)
	{
		h = r.x1; r.x1 = r.x2; r.x2 = h;
	}	
	if (r.y2 < r.y1)
	{
		h = r.y1; r.y1 = r.y2; r.y2 = h;
	}	
}

void PPDisplayDevice::setTitle(const PPSystemString& title)
{
	SDL_SetWindowTitle(theWindow, title);
}

bool PPDisplayDevice::goFullScreen(bool b)
{
	// In X11, this will make MilkyTracker go fullscreen at the selected
	// resolution.

	if (!b && (SDL_SetWindowFullscreen(theWindow, SDL_FALSE) == 0))
	{
		bFullScreen = false;
		return true;
	}

	else if (b && (SDL_SetWindowFullscreen(theWindow, SDL_WINDOW_FULLSCREEN_DESKTOP) == 0))
	{
		bFullScreen = true;
		return true;
	}
	
	return false;
}

SDL_Window* PPDisplayDevice::getWindow() {
	return theWindow;
}

// Defined in main.cpp
void exitSDLEventLoop(bool serializedEventInvoked = true);

PPSize PPDisplayDevice::getDisplayResolution() const {
	// Find the monitor MilkyTracker is being displayed on
	int currentDisplay = SDL_GetWindowDisplayIndex(theWindow);

	// Structure to hold the display resolution
	SDL_DisplayMode displayMode;

	// If this fails, return -1 dimensions (makes MilkyTracker display an error)
	if (SDL_GetDesktopDisplayMode(currentDisplay, &displayMode) != 0)
		return PPSize(-1, -1);

	// Return the desktop size
	return PPSize(displayMode.w, displayMode.h);
}

void PPDisplayDevice::shutDown()
{
	exitSDLEventLoop();
}

void PPDisplayDevice::setMouseCursor(MouseCursorTypes type)
{
	currentCursorType = type;
	
	switch (type)
	{
		case MouseCursorTypeStandard:
			SDL_SetCursor(cursorStandard);
			break;
			
		case MouseCursorTypeResizeLeft:
		case MouseCursorTypeResizeRight:
			SDL_SetCursor(cursorResizeHoriz);
			break;
	
		case MouseCursorTypeHand:
			SDL_SetCursor(cursorHand);
			break;

		case MouseCursorTypeWait:
			SDL_SetCursor(cursorEggtimer);
			break;
	}
}

void PPDisplayDevice::signalWaitState(bool b, const PPColor& color)
{
	setMouseCursor(b ? MouseCursorTypeWait : MouseCursorTypeStandard);
}

void PPDisplayDevice::initMousePointers()
{
	cursorStandard = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	cursorResizeHoriz = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursorEggtimer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
	cursorHand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
}
