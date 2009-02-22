/*
 *  fx/ParticleFX.cpp
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
#include "ParticleFX.h"
#include "Math3d.h"
#include "Filter.h"

#define PRGB2SHORT(r,g,b) ((((r))<<11)+(((g))<<5)+((b))) 
#define RGB2SHORT(r,g,b) ((((r>>3))<<11)+(((g>>2))<<5)+((b>>3))) 

// draw glenzed/scaled sprite into virtual screen
// (subpixel/texel accurate)
void ParticleFX::drawSprite(unsigned short *buffer, int XMAX, int YMAX, int PITCH, Sprite* spr)
{
	int	xmax	 = XMAX<<16;
	int	ymax	 = YMAX<<16;
	int	xmaxdec  = xmax-(1<<16);
	int	ymaxdec  = ymax-(1<<16);

	int	x,y,fx,sy,adder,ady,adx;
	int	stepx,stepy,ky,zy,kx,zx;
	unsigned short	*vidink;
	 
	int xsize = spr->xsize;
	int ysize = spr->ysize;

	int tx = spr->tx;
	int ty = spr->ty;

	if (xsize == 0) return;
	if (ysize == 0) return;

	stepx = fpdiv(65536*spr->xres,xsize);
	stepy = fpdiv(65536*spr->yres,ysize);
 
	ty-=(ysize>>1);
	tx-=(xsize>>1);

	ky = zx = zy = kx = 0;

	if ((spr->flags&1))
	{
		zx = spr->xres<<16;
		stepx=-stepx;
	}

	if (ty<0) 
	{
	    ky+=(-ty);
		zy+=fpmul(-ty,stepy);
	}
	if (tx<0) 
	{
		kx+=(-tx);
		zx+=fpmul(-tx,stepx);
	}
	if ((ty+ysize)>ymaxdec) 
		ysize-=(ty+ysize)-ymax;
	if ((tx+xsize)>xmaxdec) 
		xsize-=(tx+xsize)-xmax;

	if (ky>=ysize) return;
	if (kx>=xsize) return;
	if ((ysize+ty)<0) return;
	if ((xsize+tx)<0) return;

	adx = stepx;
	ady = stepy;

	int y1 = ky+ty;
	int y2 = ty+ysize;

	int x1 = kx+tx;
	int x2 = tx+xsize;

	int ry1 = fpceil(y1);
	int ry2 = fpceil(y2);

	int rx1 = fpceil(x1);
	int rx2 = fpceil(x2);

	if (rx2>(xmax>>16)) rx2=xmax>>16;
	if (ry2>(ymax>>16)) ry2=ymax>>16;

	int width = rx2-rx1;
	int height = ry2-ry1;

	if (!width) return;
	if (!height) return;

	int prestep = (fpceil(y1)<<16)-y1;

	zy+=fpmul(prestep,stepy);

	sy=zy;

	prestep = (rx1<<16)-x1;

	fx = (zx+fpmul(prestep,stepx));

	vidink = (ry1*PITCH+rx1)+buffer;

	adder = ((PITCH-rx2)+rx1);

	for (y=0;y<height;y++) 
	{
		unsigned short *sprpos = ((((sy>>16)+spr->yoffset)*spr->texturexres))+spr->texture;
		int ofs = fx+(spr->xoffset<<16);
		for (x=0;x<width;x++)
		{
			unsigned int rgb16_1 = *vidink;
			unsigned int rgb16_2 = sprpos[ofs>>16];

			unsigned int result = (rgb16_1&0xF800)+(rgb16_2&0xF800);
			if (result>0xF800) result = 0xF800;
			unsigned int t = (rgb16_1&0x7E0)+(rgb16_2&0x7E0); if (t>0x7E0) t = 0x7E0; result|=t;
			t = (rgb16_1&0x1F)+(rgb16_2&0x1F); if (t>0x1F) t = 0x1F; result|=t;

			*vidink++ = result;
			ofs+=adx;
		}
		vidink+=adder;
		sy+=ady;
	}
	
}

ParticleFX::ParticleFX(int width, int height, int numParticles, int FOV)
{
	this->width = width;
	this->height = height;

	if (FOV != -1)
		this->FOV = FOV;
	else
		this->FOV = (this->width * 256) / 320;

	zOffset = 0;

	particles = new Particle[numParticles];

	this->numParticles = numParticles;
}

ParticleFX::~ParticleFX()
{
	if (particles)
		delete particles;
}

void ParticleFX::render(unsigned short* vscreen, unsigned int pitch)
{
	int XMAX = width;
	int YMAX = height;
	int PITCH = pitch;

	MatrixFP rotMatrix = this->rotMatrix;
	
	int zOffset = this->zOffset;
	
	int FOV = this->FOV;

	for (unsigned int i = 0; i < numParticles; i++) {
		VectorFP v = rotMatrix*particles[i].pos;
	
		int z = v.z+zOffset;

		if (z>65536)
		{
			if (particles[i].size > 0)
			{
			
				int rz = fpdiv(65536,z);
				int x = (fpmul((v.x),rz)*FOV)+(XMAX>>1)*65536;
				int y = (fpmul((v.y),rz)*FOV)+(YMAX>>1)*65536;
				
				int size = fpmul((particles[i].size*4)*FOV,rz);
				
				Sprite spr;
				spr.tx = x;
				spr.ty = y;
				spr.xsize = size;
				spr.ysize = size;
				spr.xres = particles[i].textureWidth-1;
				spr.yres = particles[i].textureHeight-1;
				spr.xoffset = 0;
				spr.yoffset = 0;
				spr.texturexres = particles[i].textureWidth;
				spr.flags = 0;
				spr.texture = particles[i].texture;
				drawSprite(vscreen,XMAX, YMAX, PITCH, &spr);
				
			}
		}
	}		
}

void ParticleFX::update(float SyncFrac)
{
}
