/*
 *  fx/Fire.cpp
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

/*
 *  Fire.cpp
 *  MyFirstCarbonProject
 *
 *  Created by Peter Barth on Wed Feb 20 2002.
 *
 */

#include "Fire.h"
#include "Math3d.h"

Fire::Fire(pp_int32 w, pp_int32 h) :	
	FXAbstract(w, h)
{
	w>>=1;
	h>>=1;

	workbuffer = new pp_uint16[w*(h+2)];

	memset(workbuffer, 0, w*(h+2)*sizeof(pp_uint16));

	buildColorLUT();
}

Fire::~Fire()
{
	delete workbuffer;
}

///////////////////////////////////////////////
//         Schicke Farbtabelle bauen
///////////////////////////////////////////////
void Fire::buildColorLUT()
{
	struct TColor
	{
		pp_uint8 r,g,b;
	};
	
	struct TColorKey
	{
		TColor color;
		pp_uint32 t;
	};
	
	const pp_uint32 NUMKEYS = 5;
	
	TColorKey colorKeys[NUMKEYS];

	colorKeys[4].color.r = 255;
	colorKeys[4].color.g = 255;
	colorKeys[4].color.b = 255;
	colorKeys[4].t = 192;
	
	colorKeys[3].color.r = 255;
	colorKeys[3].color.g = 255;
	colorKeys[3].color.b = 0;
	colorKeys[3].t = 160;

	colorKeys[2].color.r = 255;
	colorKeys[2].color.g = 0;
	colorKeys[2].color.b = 0;
	colorKeys[2].t = 96;

	colorKeys[1].color.r = 0;
	colorKeys[1].color.g = 0;
	colorKeys[1].color.b = 32;
	colorKeys[1].t = 32;

	colorKeys[0].color.r = 0;
	colorKeys[0].color.g = 0;
	colorKeys[0].color.b = 0;
	colorKeys[0].t = 0;


	for (pp_uint32 i = 0; i < NUMKEYS-1; i++)
	{
		
		for (pp_uint32 j = colorKeys[i].t; j < colorKeys[i+1].t; j++)
		{
			float t = (float)(j - colorKeys[i].t) / (float)(colorKeys[i+1].t - colorKeys[i].t);
			
			pp_uint32 r = (pp_uint32)((1.0f - t) * colorKeys[i].color.r + t * colorKeys[i+1].color.r);
			pp_uint32 g = (pp_uint32)((1.0f - t) * colorKeys[i].color.g + t * colorKeys[i+1].color.g);
			pp_uint32 b = (pp_uint32)((1.0f - t) * colorKeys[i].color.b + t * colorKeys[i+1].color.b);
		
			colorLUT[j][0] = (pp_uint8)r;
			colorLUT[j][1] = (pp_uint8)g;
			colorLUT[j][2] = (pp_uint8)b;
		}
		
	}

	for (pp_uint32 j = colorKeys[NUMKEYS-1].t; j < 256; j++)
	{
		colorLUT[j][0] = (pp_uint8)colorKeys[2].color.r;
		colorLUT[j][1] = (pp_uint8)colorKeys[2].color.g;
		colorLUT[j][2] = (pp_uint8)colorKeys[2].color.b;
	}
	
}

///////////////////////////////////////////////
//				 BURN, BURN
///////////////////////////////////////////////
void Fire::render(pp_uint8* buffer)
{
	pp_int32 x,y,i;

	pp_int32 h = height>>1;
	pp_int32 w = width>>1;	

	for (i = 0; i < w/8; i++)
	{
		x = rand()%w;
		workbuffer[(h+1)*w+x] = (rand()&255) << 8;
	}
		
	unsigned short* vptr = workbuffer;
	
	for (y = 0; y < h+1; y++)
	{
		vptr++;
		for (x = 1; x < w; x++)
		{
			pp_uint32 c = (vptr[w] + vptr[-1] + vptr[+1] + vptr[0]) >> 2;
			
			if (c>=64) c-=64;
			else c = 0;
			
			*vptr++ = (pp_uint16)c;
		}
	}
			

	vptr = workbuffer;
	
	pp_uint8* buff2 = buffer + width*3;
	pp_uint8* buff = buffer;
	
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			pp_int16 c = *vptr>>8;
			*buff++ = colorLUT[c][0]; // rot
			*buff++ = colorLUT[c][1]; // gruen
			*buff++ = colorLUT[c][2]; // blau

			*buff++ = colorLUT[c][0]; // rot
			*buff++ = colorLUT[c][1]; // gruen
			*buff++ = colorLUT[c][2]; // blau

			*buff2++ = colorLUT[c][0]; // rot
			*buff2++ = colorLUT[c][1]; // gruen
			*buff2++ = colorLUT[c][2]; // blau

			*buff2++ = colorLUT[c][0]; // rot
			*buff2++ = colorLUT[c][1]; // gruen
			*buff2++ = colorLUT[c][2]; // blau*/
			vptr++;
		}
		buff+=w*2*3;
		buff2+=w*2*3;
	}
}

