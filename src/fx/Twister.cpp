/*
 *  fx/Twister.cpp
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
#include "Twister.h"
#include "Math3d.h"
#include "Texture.h"

#define PRGB2SHORT(r,g,b) (((unsigned short)((r))<<11)+((unsigned short)((g))<<5)+(unsigned short)((b)))

static float	phi = 0.0f;
static int		heightFieldOffset = 0; 

Twister::Twister(int width, int height, int center)
{
	int i;

	this->width = width;
	this->height = height;

	if (center == -1)
		this->center = height>>1;
	else
		this->center = center;

	unsigned char* rgbTex = new unsigned char[256*256*3];
	unsigned char* rgbTex2 = new unsigned char[256*256*3];

	texture = new unsigned short[256*256];

	Texture::createSplineTexture(rgbTex);
	
	Texture::createPlasmaTexture(rgbTex2, 256, 5, 192, 140, 63);

	for (i = 0; i < 256*256; i++)
	{
		int r = (rgbTex2[i*3]>>0) - (rgbTex[i*3]*0) + 31*2; 
		int g = (rgbTex2[i*3+1]>>0) - (rgbTex[i*3+1]*0) + 31*2;
		int b = (rgbTex2[i*3+2]>>0) - (rgbTex[i*3+2]*0) + 64*2;

		if (r < 0) r = 0; if (r > 255) r = 255;
		if (g < 0) g = 0; if (g > 255) g = 255;
		if (b < 0) b = 0; if (b > 255) b = 255;

		rgbTex[i*3] = r;
		rgbTex[i*3+1] = g;
		rgbTex[i*3+2] = b;
	}

	Texture::convert24to16(texture, rgbTex, 256*256, 1);

	for (i = 0; i < 1024; i++)
	{
		float phi = (i/512.0f)*(float)M_PI;
		sinTable[i] = (int)((float)sin(phi)*65536.0f);
		cosTable[i] = (int)((float)cos(phi)*65536.0f);
	}

	delete[] rgbTex;
	delete[] rgbTex2;

	zbuffer = new int[height];
}

Twister::~Twister()
{
	delete[] zbuffer;

	delete[] texture;
}

void Twister::update(float syncFrac)
{
	phi+=(syncFrac*0.01f);

	heightFieldOffset+=(int)(65536.0f*syncFrac);
}

void Twister::render(unsigned short* vscreen, unsigned int pitch)
{
	int XMAX = width;
	int YMAX = height;

	//unsigned int bgColor = RGB2SHORT(31*4,7*4,15*4);
	unsigned int bgColor = 0;

	unsigned int fill = (bgColor<<16) | bgColor;

	int twistStep = (int)((float)sin(phi*4.0f)*256.0f);

	for (int j = 0; j < YMAX; j++)
	{
		unsigned int* vPtr = (unsigned int*)(vscreen+j*pitch);
		for (int i = 0; i < XMAX>>1; i++)
			*vPtr++ = fill;
	}

	int beta = (int)((sin(phi)/M_PI)*512.0f*256.0f);
	//int beta = 0;

	int theta = beta>>6;

	int* zBuffer = zbuffer;

	int scale = (YMAX*65536)/200;

	int center = this->center;

	for (int scanline = 0; scanline < XMAX; scanline++)
	{
		
		//VectorFP sourcePoints[4] = {{-64*65536,0,64*65536},{64*65536,0,64*65536},{-64*65536,0,-64*65536},{64*65536,0,-64*65536}};
		
		for (int j = 0; j < NUMPOINTS; j++)
		{
			sourcePoints[j].u = (j*(256/NUMPOINTS))*2;
		
			//int radius = 64+(heightmap[((scanline+(heightFieldOffset>>16))&255)*256+(sourcePoints[j].u&255)]>>2);
			
			//int radius = 64+(mySin((scanline<<11)+theta)>>11);

			int radius = 64;

			//sourcePoints[j].p.x = (int)(-sin(((float)j/(float)NUMPOINTS)*2*M_PI+beta)*radius*65536.0f);
			//sourcePoints[j].p.y = 0;
			//sourcePoints[j].p.z = (int)(-cos(((float)j/(float)NUMPOINTS)*2*M_PI+beta)*radius*65536.0f);
			
			sourcePoints[j].p.x = -mySin(((1024*256/NUMPOINTS)*j+beta*4))*radius+(sinTable[theta&1023]*0);
			sourcePoints[j].p.y = 0;
			sourcePoints[j].p.z = -myCos(((1024*256/NUMPOINTS)*j+beta*4))*radius;
		}

		theta+=4;

		beta+=-(mySin(theta*256)*twistStep)>>15;

		sourcePoints[NUMPOINTS].p = sourcePoints[0].p;
		sourcePoints[NUMPOINTS].u = 256*2;

		int i;
		for (i = 0; i < YMAX; i++)
			zBuffer[i] = -0x7fffffff;

		for (i = 0; i < NUMPOINTS; i++)
		{
			MyVertex *p1;
			MyVertex *p2;  
			
			p1 = &sourcePoints[i]; 
			p2 = &sourcePoints[i+1];

			if (p2->p.x-p1->p.x>0)
			{
				int x1,x2,z1,z2,u1,u2;
				x1 = (fpmul(p1->p.x, scale)>>16)+center;
				x2 = (fpmul(p2->p.x, scale)>>16)+center;
				z1 = (p1->p.z>>14);
				z2 = (p2->p.z>>14);
				u1 = (p1->u);
				u2 = (p2->u);

				int stepu = 0,stepz = 0;
				if (x2!=x1)
				{
					stepu = (u2-u1)*65536/(x2-x1);
					stepz = (z2-z1)*65536/(x2-x1);			
				}
				int su = u1*65536, sz = z1*65536;
					
				if (x1 < 0)
				{
					su+=-x1*stepu;
					sz+=-x1*stepz;
					x1 = 0;
				}
				if (x2 > YMAX)
				{
					x2 = YMAX;
				}


				unsigned short *texline = texture+((((heightFieldOffset>>16)+scanline)<<1)&255)*256;
				unsigned short *vLine = (unsigned short*)vscreen+x1*pitch+scanline;

				for (int px = x1; px < x2; px++)
				{
					if (sz>zBuffer[px])
					{
						int INTENSITY = ((sz>>16)+(32));
						
						if (INTENSITY<0) INTENSITY = 0;
						if (INTENSITY>256) INTENSITY = 256;

						INTENSITY = (INTENSITY*INTENSITY)>>7;

						int TEXEL = texline[((su>>16)&255)];

						/*int r=TEXEL;											
						r*=INTENSITY;												
						r>>=8;													
						r&=0xf800;												
						int g=TEXEL&0x7e0;										
						g*=INTENSITY;												
						g>>=8;													
						g&=0x7e0;												
						int b=TEXEL&0x1f;										
						b*=INTENSITY;												
						b>>=8;	
						
						*vLine = r+g+b;*/
						
						int rgb16_1 = bgColor;
						int rgb16_2 = TEXEL;
						int b = (((rgb16_1&31)<<8)+INTENSITY*((rgb16_2&31)-(rgb16_1&31)))>>8;
						int g = ((((rgb16_1>>5)&63)<<8)+INTENSITY*(((rgb16_2>>5)&63)-((rgb16_1>>5)&63)))>>8;
						int r = (((rgb16_1>>11)<<8)+INTENSITY*((rgb16_2>>11)-(rgb16_1>>11)))>>8;
						
						if (r>31) r = 31;
						if (g>63) g = 63;
						if (b>31) b = 31;

						*vLine = PRGB2SHORT(r,g,b);

						//*vLine = RGB2SHORT((INTENSITY*255)>>8,(INTENSITY*255)>>8,(INTENSITY*255)>>8);

						//*vLine = TEXEL;


						//buffptr[scanline*320+px] = texture[((heightFieldOffset+scanline)&255)*256+((su>>16)&255)];
						zBuffer[px] = sz;
						vLine+=pitch;
					}

					/*int INTENSITY = sz>>16;
					
					if (INTENSITY<0) INTENSITY = 0;
					if (INTENSITY>255) INTENSITY = 255;
					
					int TEXEL = texture[scanline*256+((su>>16)&255)];
					//vir[vidink+x2] = tex[((((sv>>8)&0xff00)+((su>>16)&0xff)))];										
					int r=TEXEL;											
					r*=INTENSITY;												
					r>>=8;													
					r&=0xf800;												
					int g=TEXEL&0x7e0;										
					g*=INTENSITY;												
					g>>=8;													
					g&=0x7e0;												
					int b=TEXEL&0x1f;										
					b*=INTENSITY;												
					b>>=8;	
					
					buffptr[scanline*320+px] = r+g+b;*/
					
					su+=stepu;
					sz+=stepz;
				}
			
			
			}		
		}
		

	}	
	
}
