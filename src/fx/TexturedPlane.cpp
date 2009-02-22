/*
 *  fx/TexturedPlane.cpp
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

#include "BasicTypes.h"
#include "TexturedPlane.h"
#include "Texture.h"
#include "Math3d.h"
#include "Filter.h"

#define AMBIENT_LIGHT 96*65536

TexturedPlane::TexturedPlane(int width, int height, int gridshift) : 
	TexturedGrid(width, height, gridshift)
{

	/*unsigned char* texture = new unsigned char[256*256*3];

	Texture::createSplineTexture(texture);

	Texture::convert24to16(this->texture, texture, 256*256, 1);

	delete texture;*/

}

void TexturedPlane::update(float syncFrac)
{
	int gridSizeX = gridWidth;
	int gridSizeY = gridHeight;

	int x,y;

	VectorFP* gridPt = grid;

	MatrixFP m = rotMatrix;

	VectorFP d, d2, p = cop, light = this->light;

	int hx = width>>1;
	int hy = height>>1;

	int gridshift = this->gridshift;

	int FPP = 12;

	int textureshift = 6;

	int scalex = (65536*320/width)<<gridshift;
	int scaley = (65536*240/height)<<gridshift;

	for (y = 0; y < gridSizeY; y++)
		for ( x = 0; x < gridSizeX; x++)
		{
			d2.x = fpmul(x<<FPP,scalex)-(160<<FPP);
			d2.y = fpmul(y<<FPP,scaley)-(120<<FPP);
			
			d2.z = (192<<FPP);

			d = m*d2;

			int t,t3 = 0;

			int ix,iy,iz;
			bool intersection = false;

			// two "horizon" planes
			if (d.y>0)
			{
				t = fpdiv(65536*50-p.y,d.y);
				iz = (p.z+fpmul(t,d.z));
				ix = (p.x+fpmul(t,d.x));
				iy = (p.y+fpmul(t,d.y));
				
				intersection = true;
				//gridPt->x = (ix+65536)<<textureshift;
				//gridPt->y = (iz)<<textureshift;
				
				gridPt->x = ((ix>>4)+65536)<<textureshift;
				gridPt->y = ((iz>>4))<<textureshift;
			}
			else if (d.y<0)
			{
				t = fpdiv(-65536*50-p.y,d.y);
				iz = (p.z+fpmul(t,d.z));
				ix = (p.x+fpmul(t,d.x));
				iy = (p.y+fpmul(t,d.y));
				
				intersection = true;
				//gridPt->x = (-ix+65536)<<textureshift;
				//gridPt->y = (iz)<<textureshift;
				gridPt->x = ((-ix>>4)+65536)<<textureshift;
				gridPt->y = ((iz>>4))<<textureshift;
			}
			
/*again:
			gridPt->x = -12345678;
			gridPt->y = -12345678;*/

			/*if (d.y>0)
			{
				t = fpdiv(65536*64-p.y,d.y);

				if (t > 0 && t < 0x01000000)
				{
					iz = (p.z+fpmul(t,d.z));
					ix = (p.x+fpmul(t,d.x));
					iy = (p.y+fpmul(t,d.y));
					if ((ix<=65536*64)&&(ix>=-65536*64))
					{
						intersection = true;
						gridPt->x = ((ix>>6)+65536)<<textureshift;
						gridPt->y = ((iz>>6))<<textureshift;
						
					}	
				}
			}

			if (!intersection && d.y<0)
			{
				t = fpdiv(-65536*64-p.y,d.y);
				
				if (t > 0 && t < 0x01000000)
				{
					iz = (p.z+fpmul(t,d.z));
					ix = (p.x+fpmul(t,d.x));
					iy = (p.y+fpmul(t,d.y));
					if ((ix<=65536*64)&&(ix>=-65536*64))
					{
						intersection = true;
						gridPt->x = (-(ix>>6)+65536)<<textureshift;
						gridPt->y = ((iz>>6))<<textureshift;
					}
				}
			}
			
			if (!intersection&&(d.x>0))
			{
				t = fpdiv(65536*64-p.x,d.x);

				if (t > 0 && t < 0x01000000)
				{
					iz = (p.z+fpmul(t,d.z));
					ix = (p.x+fpmul(t,d.x));
					iy = (p.y+fpmul(t,d.y));
					if ((iy<=65536*65)&&(iy>=-65536*65))
					{
						intersection = true;
						gridPt->x = ((iy>>6)+65536)<<textureshift;
						gridPt->y = ((iz>>6))<<textureshift;
					}
				}
			}
			
			if (!intersection&&(d.x!=0))
			{
				t = fpdiv(-65536*64-p.x,d.x);

				if (t > 0 && t < 0x01000000)
				{
					iz = (p.z+fpmul(t,d.z));
					ix = (p.x+fpmul(t,d.x));
					iy = (p.y+fpmul(t,d.y));
					//if ((iy<=65536)&&(iy>=-65536))
					//{
					intersection = true;
					gridPt->x = (-(iy>>6)+65536)<<textureshift;
					gridPt->y = ((iz>>6))<<textureshift;
					//}
				}
			}*/

			/*if (! intersection)
			{
				
				intersection = false;

			}*/

			/*VectorFloat f(ix/65536.0f,iy/65536.0f,iz/65536.0f);

			VectorFloat f2 = f;
			f2.x = f2.y = 0.0f;

			f2 = f-f2;

			f2.normalize();

			float phi = atan2(f2.x, f2.y);

			int s = (int)((1.0f+(sin(phi*4.0f)+1.0f))*65536.0f);

			gridPt->x = (int)((fabs(phi)/M_PI)*128.0f*65536.0f);

			gridPt->x = fpmul(gridPt->x, s);
			gridPt->y = fpmul(gridPt->y, s);*/

			if (intersection)
			{
				int h, diz = t>>2;

				if (diz<0) 
					h = AMBIENT_LIGHT;
				else if (diz<(1<<(31-6)))
					h = (diz<<6)-200*65536;
				else 
					h = AMBIENT_LIGHT;

				if (h>AMBIENT_LIGHT) h = AMBIENT_LIGHT;
				if (h<0) 
					h = 0;

				gridPt->z = AMBIENT_LIGHT-h;
				//grid[y][x].z = fpmul((255*65536-h)>>1,l);

			
				int dx = abs(ix - light.x)>>8;
				int dy = abs(iy - light.y)>>8;
				int dz = abs(iz - light.z)>>8;
				
				if (dx <= 65536 && dy <= 65536 && dz <= 65536)
				{
					
					int sdist = (fpmul(dx,dx)+fpmul(dy,dy)+fpmul(dz,dz))<<6;
					
					if (sdist == 0)
						sdist=65536;
					
					int dist = fpdiv(65536,sdist);
					
					if (dist > 65536)
						dist = 65536;
					
					dist = fpmul(dist, dist) << 1;
					
					//if (dist < 4096) dist = 4096;
					
					//gridPt->z = fpmul(gridPt->z, dist);
					
					gridPt->z += dist*256;
					
					if (gridPt->z > 256*65536)
						gridPt->z = 256*65536;
				}
			
			
			}
			else gridPt->z = 0;		
			//else
			//	gridPt->z = fpmul(gridPt->z, /*4096*/0);

			gridPt++;
		}

}
