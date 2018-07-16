/*
 *  ppui/sdl/DisplayDeviceFB_SDL.cpp
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

#include "DisplayDeviceFB_SDL.h"
#include "Graphics.h"

PPDisplayDeviceFB::PPDisplayDeviceFB(SDL_Surface*& screen, 
									 pp_int32 width, 
									 pp_int32 height, 
									 pp_int32 scaleFactor,
									 pp_int32 bpp,
									 bool fullScreen, 
									 Orientations theOrientation/* = ORIENTATION_NORMAL*/, 
									 bool swapRedBlue/* = false*/) :
	PPDisplayDevice(screen, width, height, scaleFactor, bpp, fullScreen, theOrientation),
	needsTemporaryBuffer((orientation != ORIENTATION_NORMAL) || (scaleFactor != 1)),
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
	
	if (needsTemporaryBuffer)
	{
		temporaryBufferPitch = (width*bpp)/8;
		temporaryBufferBPP = bpp;
		temporaryBuffer = new pp_uint8[getSize().width*getSize().height*(bpp/8)];
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

		if (needsTemporaryBuffer)
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
	
	PPRect r(0, 0, getSize().width, getSize().height);
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

	PPRect r2(r);
	swap(r2);

	PPRect r3(r);
	r3.scale(scaleFactor);
	
	transformInverse(r3);

	SDL_UpdateRect(theSurface, r3.x1, r3.y1, (r3.x2-r3.x1), (r3.y2-r3.y1));
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
		{
			if (!needsTemporaryBuffer)
				return;
			
			if (SDL_LockSurface(theSurface) < 0)
				return;
						
			const pp_uint32 srcBPP = temporaryBufferBPP/8;
			const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;

			PPRect destRect(r);		
			destRect.scale(scaleFactor);

			const pp_uint32 stepU = (r.x2 - r.x1) * 65536 / (destRect.x2 - destRect.x1);
			const pp_uint32 stepV = (r.y2 - r.y1) * 65536 / (destRect.y2 - destRect.y1);
			
			switch (temporaryBufferBPP)
			{
				case 16:
				{
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;

					pp_uint32 v = r.y1 * 65536;
					for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
					{
						pp_uint32 u = r.x1 * 65536;
						pp_uint16* dstPtr = (pp_uint16*)(dst + y*dstPitch + destRect.x1*dstBPP);
						pp_uint8* srcPtr = src + (v>>16)*srcPitch;
						for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
						{
							*dstPtr++ = *(pp_uint16*)(srcPtr + (u>>16) * srcBPP);
							u += stepU;
						}
						v += stepV;
					}

					
					break;
				}

				case 24:
				{
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;

					pp_uint32 v = r.y1 * 65536;
					for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
					{
						pp_uint32 u = r.x1 * 65536;
						pp_uint8* dstPtr = (pp_uint8*)(dst + y*dstPitch + destRect.x1*dstBPP);
						pp_uint8* srcPtr = src + (v>>16)*srcPitch;
						for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
						{
							*dstPtr = *(pp_uint8*)(srcPtr + (u>>16) * srcBPP);
							*(dstPtr+1) = *(pp_uint8*)(srcPtr + (u>>16) * srcBPP + 1);
							*(dstPtr+2) = *(pp_uint8*)(srcPtr + (u>>16) * srcBPP + 2);
							dstPtr+=3;
							u += stepU;
						}
						v += stepV;
					}
																
					break;
				}
				
				case 32:
				{
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;

					pp_uint32 v = r.y1 * 65536;
					for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
					{
						pp_uint32 u = r.x1 * 65536;
						pp_uint32* dstPtr = (pp_uint32*)(dst + y*dstPitch + destRect.x1*dstBPP);
						pp_uint8* srcPtr = src + (v>>16)*srcPitch;
						for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
						{
							*dstPtr++ = *(pp_uint32*)(srcPtr + (u>>16) * srcBPP);
							u += stepU;
						}
						v += stepV;
					}

					break;
				}
				
				default:
					fprintf(stderr, "Unsupported color depth for requested orientation");	
					exit(2);
			}
			
			SDL_UnlockSurface(theSurface);				

			break;
		}
	
		case ORIENTATION_ROTATE90CCW:
		{
			if (SDL_LockSurface(theSurface) < 0)
				return;
						
			switch (temporaryBufferBPP)
			{
				case 16:
				{
					pp_uint32 srcPitch = temporaryBufferPitch >> 1;
					pp_uint32 dstPitch = theSurface->pitch >> 1;
					
					pp_uint16* src = (pp_uint16*)temporaryBuffer; 
					pp_uint16* dst = (pp_uint16*)theSurface->pixels;
					
					if (scaleFactor != 1)
					{
						PPRect destRect(r);		
						destRect.scale(scaleFactor);
						
						const pp_uint32 stepU = (r.x2 - r.x1) * 65536 / (destRect.x2 - destRect.x1);
						const pp_uint32 stepV = (r.y2 - r.y1) * 65536 / (destRect.y2 - destRect.y1);
						
						pp_uint32 v = r.y1 * 65536;
						for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
						{
							pp_uint32 u = r.x1 * 65536;
							pp_uint16* srcPtr = src + (v>>16)*srcPitch;
							pp_uint16* dstPtr = dst + y + (realHeight-destRect.x1)*dstPitch;
							for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
							{
								*(dstPtr-=dstPitch) = *(srcPtr+(u>>16));
								
								u += stepU;
							}
							v += stepV;
						}
					}
					else
					{
						for (pp_uint32 y = r.y1; y < r.y2; y++)
						{
							pp_uint16* srcPtr = src + y*srcPitch + r.x1;
							pp_uint16* dstPtr = dst + y + (realHeight-r.x1)*dstPitch;
							for (pp_uint32 x = r.x1; x < r.x2; x++)
								*(dstPtr-=dstPitch) = *srcPtr++;
						}
					}
					
					break;
				}

				case 24:
				{
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;
					
					if (scaleFactor != 1)
					{
						PPRect destRect(r);		
						destRect.scale(scaleFactor);
						
						const pp_uint32 stepU = (r.x2 - r.x1) * 65536 / (destRect.x2 - destRect.x1);
						const pp_uint32 stepV = (r.y2 - r.y1) * 65536 / (destRect.y2 - destRect.y1);
						
						pp_uint32 v = r.y1 * 65536;
						for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
						{
							pp_uint32 u = r.x1 * 65536;
							pp_uint8* srcPtr = src + (v>>16)*srcPitch;
							pp_uint8* dstPtr = dst + y*dstBPP + dstPitch*(realHeight-1-destRect.x1);
							for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
							{
								dstPtr[0] = *(srcPtr+(u>>16) * srcBPP);
								dstPtr[1] = *(srcPtr+(u>>16) * srcBPP + 1);
								dstPtr[2] = *(srcPtr+(u>>16) * srcBPP + 2);
								dstPtr-=dstPitch;
								
								u += stepU;
							}
							v += stepV;
						}
					}
					else
					{
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
					}
					
					break;
				}
				
				case 32:
				{
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;
					
					if (scaleFactor != 1)
					{
						PPRect destRect(r);		
						destRect.scale(scaleFactor);
						
						const pp_uint32 stepU = (r.x2 - r.x1) * 65536 / (destRect.x2 - destRect.x1);
						const pp_uint32 stepV = (r.y2 - r.y1) * 65536 / (destRect.y2 - destRect.y1);

						pp_uint32 v = r.y1 * 65536;
						for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
						{
							pp_uint32 u = r.x1 * 65536;
							pp_uint8* srcPtr = src + (v>>16)*srcPitch;
							pp_uint32* dstPtr = (pp_uint32*)(dst + y*dstBPP + dstPitch*(realHeight-1-destRect.x1));
							for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
							{
								*(dstPtr-=(dstPitch>>2)) = *(pp_uint32*)(srcPtr + (u>>16) * srcBPP);
								u += stepU;
							}
							v += stepV;
						}
					}
					else
					{
						for (pp_uint32 y = r.y1; y < r.y2; y++)
						{
							pp_uint32* srcPtr = (pp_uint32*)(src + y*srcPitch + r.x1*srcBPP);
							pp_uint32* dstPtr = (pp_uint32*)(dst + y*dstBPP + dstPitch*(realHeight-1-r.x1));
							for (pp_uint32 x = r.x1; x < r.x2; x++)
								*(dstPtr-=(dstPitch>>2)) = *srcPtr++;
						}
					}
					
					break;
				}
				
				default:
					fprintf(stderr, "Unsupported color depth for requested orientation");	
					exit(2);
			}
		
			SDL_UnlockSurface(theSurface);				
			break;
		}

		case ORIENTATION_ROTATE90CW:
		{
			if (SDL_LockSurface(theSurface) < 0)
				return;
						
			switch (temporaryBufferBPP)
			{
				case 16:
				{					
					pp_uint32 srcPitch = temporaryBufferPitch >> 1;
					pp_uint32 dstPitch = theSurface->pitch >> 1;
					
					pp_uint16* src = (pp_uint16*)temporaryBuffer; 
					pp_uint16* dst = (pp_uint16*)theSurface->pixels;
					
					if (scaleFactor != 1)
					{
						PPRect destRect(r);		
						destRect.scale(scaleFactor);
						
						const pp_uint32 stepU = (r.x2 - r.x1) * 65536 / (destRect.x2 - destRect.x1);
						const pp_uint32 stepV = (r.y2 - r.y1) * 65536 / (destRect.y2 - destRect.y1);
						
						pp_uint32 v = r.y1 * 65536;
						for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
						{
							pp_uint32 u = r.x1 * 65536;
							pp_uint16* srcPtr = src + (v>>16)*srcPitch;
							pp_uint16* dstPtr = dst + (realWidth-1-y) + (dstPitch*(destRect.x1-1));
							for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
							{
								*(dstPtr+=dstPitch) = *(srcPtr+(u>>16));
								
								u += stepU;
							}
							v += stepV;
						}
					}
					else
					{
						for (pp_uint32 y = r.y1; y < r.y2; y++)
						{
							pp_uint16* srcPtr = src + y*srcPitch + r.x1;
							pp_uint16* dstPtr = dst + (realWidth-1-y) + (dstPitch*(r.x1-1));
							for (pp_uint32 x = r.x1; x < r.x2; x++)
								*(dstPtr+=dstPitch) = *srcPtr++;
						}
					}
					
					break;
				}
				
				case 24:
				{
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;
					
					if (scaleFactor != 1)
					{
						PPRect destRect(r);		
						destRect.scale(scaleFactor);
						
						const pp_uint32 stepU = (r.x2 - r.x1) * 65536 / (destRect.x2 - destRect.x1);
						const pp_uint32 stepV = (r.y2 - r.y1) * 65536 / (destRect.y2 - destRect.y1);

						pp_uint32 v = r.y1 * 65536;
						for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
						{
							pp_uint32 u = r.x1 * 65536;
							pp_uint8* srcPtr = src + (v>>16)*srcPitch;
							pp_uint8* dstPtr = dst + (realWidth-1-y)*dstBPP + (dstPitch*(destRect.x1-1));
							for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
							{
								dstPtr[0] = *(srcPtr+(u>>16) * srcBPP);
								dstPtr[1] = *(srcPtr+(u>>16) * srcBPP + 1);
								dstPtr[2] = *(srcPtr+(u>>16) * srcBPP + 2);
								dstPtr+=dstPitch;

								u += stepU;
							}
							v += stepV;
						}
					}
					else
					{
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
					}
					
					break;
				}
				
				case 32:
				{
					pp_uint32 srcPitch = temporaryBufferPitch;
					pp_uint32 dstPitch = theSurface->pitch;
					
					pp_uint8* src = (pp_uint8*)temporaryBuffer; 
					pp_uint8* dst = (pp_uint8*)theSurface->pixels;
					
					const pp_uint32 srcBPP = temporaryBufferBPP/8;
					const pp_uint32 dstBPP = theSurface->format->BytesPerPixel;
					
					if (scaleFactor != 1)
					{
						PPRect destRect(r);		
						destRect.scale(scaleFactor);
						
						const pp_uint32 stepU = (r.x2 - r.x1) * 65536 / (destRect.x2 - destRect.x1);
						const pp_uint32 stepV = (r.y2 - r.y1) * 65536 / (destRect.y2 - destRect.y1);

						pp_uint32 v = r.y1 * 65536;
						for (pp_uint32 y = destRect.y1; y < destRect.y2; y++)
						{
							pp_uint32 u = r.x1 * 65536;
							pp_uint8* srcPtr = src + (v>>16)*srcPitch;
							pp_uint32* dstPtr = (pp_uint32*)(dst + (realWidth-1-y)*dstBPP + (dstPitch*(destRect.x1-1)));
							for (pp_uint32 x = destRect.x1; x < destRect.x2; x++)
							{
								*(dstPtr+=(dstPitch>>2)) = *(pp_uint32*)(srcPtr + (u>>16) * srcBPP);
								u += stepU;
							}
							v += stepV;
						}
					}
					else
					{
						for (pp_uint32 y = r.y1; y < r.y2; y++)
						{
							pp_uint32* srcPtr = (pp_uint32*)(src + y*srcPitch + r.x1*srcBPP);
							pp_uint32* dstPtr = (pp_uint32*)(dst + (realWidth-1-y)*dstBPP + (dstPitch*(r.x1-1)));
							for (pp_uint32 x = r.x1; x < r.x2; x++)
								*(dstPtr+=(dstPitch>>2)) = *srcPtr++;
						}
					}
					
					break;
				}
				
				default:
					fprintf(stderr, "Unsupported color depth for requested orientation");	
					exit(2);
			}

			SDL_UnlockSurface(theSurface);				
		
			break;
		}

	}
}
