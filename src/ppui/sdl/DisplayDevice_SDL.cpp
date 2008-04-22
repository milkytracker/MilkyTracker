/*
 *  ppui/sdl/DisplayDevice_SDL.cpp
 *
 *  Copyright 2008 Peter Barth, Christopher O'Neill
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

SDL_Surface* PPDisplayDevice::CreateScreen(Uint16 w, Uint16 h, Uint8 bpp, Uint32 flags)
{
	SDL_Surface *screen;
	int i;

	/* Set the video mode */
	screen = SDL_SetVideoMode(w, h, bpp, flags);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set display mode: %s\n",
							SDL_GetError());
		return(NULL);
	}
// 	fprintf(stderr, "Screen is in %s mode\n",
// 		(screen->flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");

	return(screen);
}

PPDisplayDevice::PPDisplayDevice(SDL_Surface*& screen, 
							 pp_int32 width, 
							 pp_int32 height, 
							 pp_int32 bpp,
							 bool fullScreen, 
							 Orientations theOrientation/* = ORIENTATION_NORMAL*/, 
							 bool swapRedBlue/* = false*/) :
	PPDisplayDeviceBase(width, height),
	realWidth(width), realHeight(height),
	orientation(theOrientation),
	temporaryBuffer(NULL)
	
{
	const SDL_VideoInfo *videoinfo;

	adjust(realWidth, realHeight);

	bFullScreen = fullScreen;

	/* Some SDL to get display format */
	videoinfo = SDL_GetVideoInfo();
	if(bpp == -1) {
		bpp = videoinfo->vfmt->BitsPerPixel > 16 ? videoinfo->vfmt->BitsPerPixel : 16;
	}

	/* Set a video mode */
	theSurface = screen = CreateScreen(realWidth, realHeight, bpp, SDL_SWSURFACE | (bFullScreen==true ? SDL_FULLSCREEN : 0));
	if ( screen == NULL ) 
	{
		fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());	
		exit(2);
	}	

	switch (bpp)
	{
		case 16:
			currentGraphics = new PPGraphics_16BIT(width, height, 0, NULL);
			break;
			
		case 24:
		{
			PPGraphics_24bpp_generic* g = new PPGraphics_24bpp_generic(width, height, 0, NULL);
			if (swapRedBlue)
			{
				g->setComponentBitpositions(videoinfo->vfmt->Bshift, 
											videoinfo->vfmt->Gshift, 
											videoinfo->vfmt->Rshift);
			}
			else
			{
				g->setComponentBitpositions(videoinfo->vfmt->Rshift, 
											videoinfo->vfmt->Gshift, 
											videoinfo->vfmt->Bshift);
			}
			currentGraphics = static_cast<PPGraphicsAbstract*>(g);
			break;
		}
			
		case 32:
		{
			PPGraphics_32bpp_generic* g = new PPGraphics_32bpp_generic(width, height, 0, NULL);
			if (swapRedBlue)
			{
				g->setComponentBitpositions(videoinfo->vfmt->Bshift, 
											videoinfo->vfmt->Gshift, 
											videoinfo->vfmt->Rshift);
			}
			else
			{
				g->setComponentBitpositions(videoinfo->vfmt->Rshift, 
											videoinfo->vfmt->Gshift, 
											videoinfo->vfmt->Bshift);
			}
			currentGraphics = static_cast<PPGraphicsAbstract*>(g);
			break;
		}
			
		default:
			fprintf(stderr, "Unsupported color depth (%i), try either 16, 24 or 32", bpp);	
			exit(2);
	}
	
	if (orientation != ORIENTATION_NORMAL)
	{
		temporaryBufferPitch = (width*bpp)/8;
		temporaryBufferBPP = bpp;
		temporaryBuffer = new pp_uint8[this->width*this->height*(bpp/8)];
	}
	
	currentGraphics->lock = true;
	initMousePointers();
}

PPDisplayDevice::~PPDisplayDevice()
{	
	delete temporaryBuffer;
	delete currentGraphics;
}

PPGraphicsAbstract* PPDisplayDevice::open()
{
	if (!isEnabled())
		return NULL;

	if (currentGraphics->lock)
	{
		if (SDL_LockSurface(theSurface) < 0)
			return NULL;

		currentGraphics->lock = false;

		if (orientation != ORIENTATION_NORMAL)
			currentGraphics->setBufferProperties(temporaryBufferPitch, (pp_uint8*)temporaryBuffer);		
		else
			currentGraphics->setBufferProperties(theSurface->pitch, (pp_uint8*)theSurface->pixels);		
		
		return currentGraphics;
	}
	
	return NULL;
}

void PPDisplayDevice::close()
{
	SDL_UnlockSurface(theSurface);

	currentGraphics->lock = true;
}

void PPDisplayDevice::update()
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	if (theSurface->locked)
	{
		return;
	}
	
	PPRect r(0, 0, getWidth(), getHeight());
	swap(r);
	
	SDL_UpdateRect(theSurface, 0, 0, 0, 0);
}

void PPDisplayDevice::update(const PPRect& r)
{
	if (!isUpdateAllowed() || !isEnabled())
		return;
	
	if (theSurface->locked)
	{
		return;
	}

	PPRect r2(r.x1, r.y1, r.x2, r.y2);
	swap(r2);

	PPRect r3 = r;
	transformInverse(r3);

	SDL_UpdateRect(theSurface, r3.x1, r3.y1, r3.x2-r3.x1, r3.y2-r3.y1);
}

void PPDisplayDevice::adjust(pp_int32& x, pp_int32& y)
{
	pp_int32 h;

	switch (orientation)
	{
		case ORIENTATION_NORMAL:
		case ORIENTATION_ROTATE180:
			break;
			
		case ORIENTATION_ROTATE90CCW:
		case ORIENTATION_ROTATE90CW:
			h = x;
			x = y;
			y = h;
			break;
	}	
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
			
		case ORIENTATION_ROTATE180:
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
			
		case ORIENTATION_ROTATE180:
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

void PPDisplayDevice::swap(const PPRect& r2)
{
	PPRect r(r2);
	pp_int32 h;
	if (r.x2 < r.x1)
	{
		h = r.x1; r.x1 = r.x2; r.x2 = h;
	}	
	if (r.y2 < r.y1)
	{
		h = r.y1; r.y1 = r.y2; r.y2 = h;
	}		

	switch (orientation)
	{
		case ORIENTATION_NORMAL:
			break;
	
		case ORIENTATION_ROTATE90CCW:
		{
			switch (temporaryBufferBPP)
			{
				case 16:
				{
					if (SDL_LockSurface(theSurface) < 0)
						return;
						
					pp_uint32 srcPitch = temporaryBufferPitch >> 1;
					pp_uint32 dstPitch = theSurface->pitch >> 1;
					
					pp_uint16* src = (pp_uint16*)temporaryBuffer; 
					pp_uint16* dst = (pp_uint16*)theSurface->pixels;
					
					for (pp_uint32 y = r.y1; y < r.y2; y++)
					{
						pp_uint16* srcPtr = src + y*srcPitch + r.x1;
						pp_uint16* dstPtr = dst + y + (realHeight-r.x1)*dstPitch;
						for (pp_uint32 x = r.x1; x < r.x2; x++)
							*(dstPtr-=dstPitch) = *srcPtr++;
					}
						
					SDL_UnlockSurface(theSurface);				
					break;
				}

				case 24:
				{
					if (SDL_LockSurface(theSurface) < 0)
						return;
						
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;
					
					for (pp_uint32 y = r.y1; y < r.y2; y++)
					{
						pp_uint8* srcPtr = src + y*srcPitch + r.x1*srcBPP;
						pp_uint8* dstPtr = dst + y*dstBPP + dstPitch*(realHeight-1-r.x1);
						for (pp_uint32 x = r.x1; x < r.x2; x++)
						{
							dstPtr[0] = srcPtr[0];
							dstPtr[1] = srcPtr[1];
							dstPtr[2] = srcPtr[2];
							srcPtr+=srcBPP;
							dstPtr-=dstPitch;
						}
					}
						
					SDL_UnlockSurface(theSurface);				
					break;
				}
				
				case 32:
				{
					if (SDL_LockSurface(theSurface) < 0)
						return;
						
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;
					
					for (pp_uint32 y = r.y1; y < r.y2; y++)
					{
						pp_uint32* srcPtr = (pp_uint32*)(src + y*srcPitch + r.x1*srcBPP);
						pp_uint32* dstPtr = (pp_uint32*)(dst + y*dstBPP + dstPitch*(realHeight-1-r.x1));
						for (pp_uint32 x = r.x1; x < r.x2; x++)
							*(dstPtr-=(dstPitch>>2)) = *srcPtr++;
					}
						
					SDL_UnlockSurface(theSurface);				
					break;
				}
				
				default:
					fprintf(stderr, "Unsupported color depth for requested orientation");	
					exit(2);
			}
		
			break;
		}

		case ORIENTATION_ROTATE90CW:
		{
			switch (temporaryBufferBPP)
			{
				case 16:
				{
					if (SDL_LockSurface(theSurface) < 0)
						return;
						
					pp_uint32 srcPitch = temporaryBufferPitch >> 1;
					pp_uint32 dstPitch = theSurface->pitch >> 1;
					
					pp_uint16* src = (pp_uint16*)temporaryBuffer; 
					pp_uint16* dst = (pp_uint16*)theSurface->pixels;
					
					for (pp_uint32 y = r.y1; y < r.y2; y++)
					{
						pp_uint16* srcPtr = src + y*srcPitch + r.x1;
						pp_uint16* dstPtr = dst + (realWidth-1-y) + (dstPitch*(r.x1-1));
						for (pp_uint32 x = r.x1; x < r.x2; x++)
							*(dstPtr+=dstPitch) = *srcPtr++;
					}
						
					SDL_UnlockSurface(theSurface);				
					break;
				}
				
				case 24:
				{
					if (SDL_LockSurface(theSurface) < 0)
						return;
						
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;
					
					for (pp_uint32 y = r.y1; y < r.y2; y++)
					{
						pp_uint8* srcPtr = src + y*srcPitch + r.x1*srcBPP;
						pp_uint8* dstPtr = dst + (realWidth-1-y)*dstBPP + (dstPitch*r.x1);
						for (pp_uint32 x = r.x1; x < r.x2; x++)
						{
							dstPtr[0] = srcPtr[0];
							dstPtr[1] = srcPtr[1];
							dstPtr[2] = srcPtr[2];
							srcPtr+=srcBPP;
							dstPtr+=dstPitch;
						}
					}
						
					SDL_UnlockSurface(theSurface);				
					break;
				}
				
				case 32:
				{
					if (SDL_LockSurface(theSurface) < 0)
						return;
						
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;
					
					for (pp_uint32 y = r.y1; y < r.y2; y++)
					{
						pp_uint32* srcPtr = (pp_uint32*)(src + y*srcPitch + r.x1*srcBPP);
						pp_uint32* dstPtr = (pp_uint32*)(dst + (realWidth-1-y)*dstBPP + (dstPitch*(r.x1-1)));
						for (pp_uint32 x = r.x1; x < r.x2; x++)
							*(dstPtr+=(dstPitch>>2)) = *srcPtr++;
					}
						
					SDL_UnlockSurface(theSurface);				
					break;
				}
				
				default:
					fprintf(stderr, "Unsupported color depth for requested orientation");	
					exit(2);
			}
		
			break;
		}

	}
}

void PPDisplayDevice::setTitle(const PPSystemString& title)
{
	::SDL_WM_SetCaption(title, "MilkyTracker");
}

void PPDisplayDevice::setSize(const PPSize& size)
{
	theSurface = SDL_SetVideoMode(size.width, size.height, theSurface->format->BitsPerPixel, theSurface->flags);
}

bool PPDisplayDevice::goFullScreen(bool b)
{
	// In X11, this will make MilkyTracker go fullscreen at the selected
	// resolution.

	SDL_Surface *screen=SDL_GetVideoSurface();
	if (SDL_WM_ToggleFullScreen(screen)) {
		bFullScreen = !bFullScreen;
		return true;
	}
	
	return false;
}

// Defined in main.cpp
void exitSDLEventLoop(bool serializedEventInvoked = true);

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
			SDL_SetCursor(cursorResizeLeft);
			break;

		case MouseCursorTypeResizeRight:
			SDL_SetCursor(cursorResizeRight);
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

// Mouse pointer data
Uint8 PPDisplayDevice::resizeLeft_data[] = { 0, 0, 96, 0, 97, 128, 99, 0, 102, 0, 108, 0, 120, 0, 127, 254, 120, 0, 124, 0, 102, 0, 99, 0, 97, 128, 96, 0, 0, 0, 0, 0 };
Uint8 PPDisplayDevice::resizeLeft_mask[] = { 240, 0, 241, 128, 243, 192, 247, 128, 255, 0, 254, 0, 255, 255, 255, 255, 255, 255, 254, 0, 255, 0, 247, 128, 243, 192, 241, 128, 240, 0, 0, 0 };
Uint8 PPDisplayDevice::resizeRight_data[] = { 0, 0, 0, 6, 1, 134, 0, 198, 0, 102, 0, 54, 0, 30, 127, 254, 0, 30, 0, 62, 0, 102, 0, 198, 1, 134, 0, 6, 0, 0, 0, 0 };
Uint8 PPDisplayDevice::resizeRight_mask[] = { 0, 15, 1, 143, 3, 207, 1, 239, 0, 255, 0, 127, 255, 255, 255, 255, 255, 255, 0, 127, 0, 255, 1, 239, 3, 207, 1, 143, 0, 15, 0, 0, };
Uint8 PPDisplayDevice::eggtimer_data[] = { 0, 0, 127, 192, 32, 128, 32, 128, 17, 0, 17, 0, 10, 0, 4, 0, 4, 0, 10, 0, 17, 0, 17, 0, 32, 128, 32, 128, 127, 192, 0, 0 };
Uint8 PPDisplayDevice::eggtimer_mask[] = { 255, 224, 255, 224, 127, 192, 127, 192, 63, 128, 63, 128, 31, 0, 14, 0, 14, 0, 31, 0, 63, 128, 63, 128, 127, 192, 127, 192, 255, 224, 255, 224 };
Uint8 PPDisplayDevice::hand_data[] = {54, 192, 91, 64, 146, 64, 146, 112, 146, 104, 146, 104, 128, 40, 128, 40, 128, 8, 128, 8, 128, 16, 64, 16, 64, 32, 32, 32, 31, 192, 0, 0, };
Uint8 PPDisplayDevice::hand_mask[] = {54, 192, 127, 192, 255, 192, 255, 240, 255, 248, 255, 248, 255, 248, 255, 248, 255, 248, 255, 248, 255, 240, 127, 240, 127, 224, 63, 224, 31, 192, 0, 0, };

void PPDisplayDevice::initMousePointers()
{
	cursorResizeLeft = SDL_CreateCursor(resizeLeft_data, resizeLeft_mask, 16, 16, 2, 7);
    cursorResizeRight = SDL_CreateCursor(resizeRight_data, resizeRight_mask, 16, 16, 13, 7);
	cursorEggtimer = SDL_CreateCursor(eggtimer_data, eggtimer_mask, 16, 16, 5, 7);
	cursorHand = SDL_CreateCursor(hand_data, hand_mask, 16, 16, 5, 5);

	// The current cursor is used as the standard cursor;
	// This might cause problems if the system if displaying some other cursor at
	// the time, or it might not. It depends.
	cursorStandard = SDL_GetCursor();
}
