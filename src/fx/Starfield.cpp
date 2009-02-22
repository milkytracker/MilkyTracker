/*
 *  fx/Starfield.cpp
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
 *  Starfield.cpp
 *  MyFirstCarbonProject
 *
 *  Created by Peter Barth on Wed Feb 20 2002.
 *
 */

#include "Starfield.h"
#include "Math3d.h"

Starfield::Starfield(pp_int32 w, pp_int32 h) :	
	FXAbstract(w, h),
	numStars(2000)
{
	stars = new VectorFP[numStars];
	
	for (pp_int32 i = 0; i < numStars; i++)
	{
		stars[i].x = (((rand()&255)<<8) - 32768)>>2;
		stars[i].y = (((rand()&255)<<8) - 32768)>>2;
		stars[i].z = (((rand()&255)<<8))>>2;
	}
}

Starfield::~Starfield()
{
	delete[] stars;
}

///////////////////////////////////////////////
//				 BURN, BURN
///////////////////////////////////////////////
void Starfield::render(pp_uint8* buffer)
{
	memset(buffer, 0, width*height*3);

	pp_int32 centerx = width*65536/2;
	pp_int32 centery = height*65536/2;

	MatrixFP m;
	
	for (pp_int32 i = 0; i < numStars; i++)
	{
		VectorFP f = stars[i];
		
		if (f.z > 0)
		{
			pp_int32 px = (fpdiv(f.x*200, f.z) + centerx)>>16;
			pp_int32 py = (fpdiv(f.y*200, f.z) + centery)>>16;
			
			pp_int32 c = 255-(f.z>>6);
			if (c < 0) c = 0;
			if (c > 255) c = 255;
			
			PPColor col(c,c,c);
			
			setPixel(px, py, col, buffer);
		}
	}

	//phi+=0.01f;

}

void Starfield::update(pp_uint32 syncFrac)
{
	for (pp_int32 i = 0; i < numStars; i++)
	{
		stars[i].z-=syncFrac>>9;
		if (stars[i].z < 0)
		{
			stars[i].z = ((65535>>2) + 2048) + stars[i].z;
		}
	}
}

void Starfield::setPixel(pp_int32 x, pp_int32 y, const PPColor& color, pp_uint8* buffer)
{
	if (x < 0 || x >= (signed)width-1 || y < 0 || y >= (signed)height-1)
		return;
		
	pp_int32 offset = y*width+x;
		
	buffer[offset*3] = (pp_uint8)color.r;
	buffer[offset*3+1] = (pp_uint8)color.g;
	buffer[offset*3+2] = (pp_uint8)color.b;

	/*buffer[(offset+1)*3] = color.r;
	buffer[(offset+1)*3+1] = color.g;
	buffer[(offset+1)*3+2] = color.b;

	buffer[(offset+width)*3] = color.r;
	buffer[(offset+width)*3+1] = color.g;
	buffer[(offset+width)*3+2] = color.b;

	buffer[(offset+width+1)*3] = color.r;
	buffer[(offset+width+1)*3+1] = color.g;
	buffer[(offset+width+1)*3+2] = color.b;*/
}


