/*
 *  ppui/sdl/DisplayDeviceFB_SDL.cpp
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

#include "DisplayDeviceFB_SDL.h"
#include "Graphics.h"

PPDisplayDeviceFB::PPDisplayDeviceFB(SDL_Surface*& screen, 
									 pp_int32 width, 
									 pp_int32 height, 
									 pp_int32 bpp,
									 bool fullScreen, 
									 Orientations theOrientation/* = ORIENTATION_NORMAL*/, 
									 bool swapRedBlue/* = false*/) :
	PPDisplayDevice(screen, width, height, bpp, fullScreen, theOrientation),
	temporaryBuffer(NULL)	
{
	const SDL_VideoInfo* videoinfo;

	/* Some SDL to get display format */
	videoinfo = SDL_GetVideoInfo();
	if (bpp == -1) 
	{
		bpp = videoinfo->vfmt->BitsPerPixel > 16 ? videoinfo->vfmt->BitsPerPixel : 16;
	}

	/* Set a video mode */
	theSurface = screen = CreateScreen(realWidth, realHeight, 
									   bpp, SDL_SWSURFACE | (bFullScreen==true ? SDL_FULLSCREEN : 0));
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
}

PPDisplayDeviceFB::~PPDisplayDeviceFB()
{	
	delete[] temporaryBuffer;
	// base class is responsible for deleting currentGraphics
}

PPGraphicsAbstract* PPDisplayDeviceFB::open()
{
	if (!isEnabled())
		return NULL;

	if (currentGraphics->lock)
	{
		if (SDL_LockSurface(theSurface) < 0)
			return NULL;

		currentGraphics->lock = false;

		if (orientation != ORIENTATION_NORMAL)
			static_cast<PPGraphicsFrameBuffer*>(currentGraphics)->setBufferProperties(temporaryBufferPitch, (pp_uint8*)temporaryBuffer);		
		else
			static_cast<PPGraphicsFrameBuffer*>(currentGraphics)->setBufferProperties(theSurface->pitch, (pp_uint8*)theSurface->pixels);		
		
		return currentGraphics;
	}
	
	return NULL;
}

void PPDisplayDeviceFB::close()
{
	SDL_UnlockSurface(theSurface);

	currentGraphics->lock = true;
}

void PPDisplayDeviceFB::update()
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

void PPDisplayDeviceFB::update(const PPRect& r)
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

void PPDisplayDeviceFB::swap(const PPRect& r2)
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
