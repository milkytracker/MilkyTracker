/*
 *  fx/TexturedGrid.cpp
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

#include "TexturedGrid.h"
#include "Math3d.h"
#include "Filter.h"
#include "Texture.h"

#define PRGB2SHORT(r,g,b) ((((r))<<11)+(((g))<<5)+((b))) 
#define RGB2SHORT(r,g,b) ((((r>>3))<<11)+(((g>>2))<<5)+((b>>3))) 

#define TEXTURESIZE 256

#define REDMASK (0x1F<<11)
#define GREENMASK (0x3F<<5)
#define BLUEMASK 0x1F

TexturedGrid::TexturedGrid(int width, int height, int gridshift)
{
	this->width = width;
	this->height = height;

	this->gridshift = gridshift;
	this->gridsize = 1 << gridshift;

	this->gridWidth = (width>>gridshift)+1;
	this->gridHeight = (height>>gridshift)+1;

	grid = new VectorFP[gridWidth*gridHeight];

	this->texture = NULL;	// we're not rendering anything until texture is set

	this->buffer = new unsigned short[width*height];

	// for debugging, load texture
	int i;
	
	/*unsigned char *texture = new unsigned char[TEXTURESIZE*TEXTURESIZE*3];

    //FILE* f = fopen("/demo/t2new.tga","rb");
    //fseek(f,18,SEEK_SET);
    //fread(texture,TEXTURESIZE*TEXTURESIZE,3,f);
    //fclose(f);
	Texture::createSplineTexture(texture);


	this->texture = new unsigned short[TEXTURESIZE*TEXTURESIZE];

	for (i = 0; i < TEXTURESIZE*TEXTURESIZE; i++) {
		this->texture[i] = RGB2SHORT(texture[i*3]>>0,texture[i*3+1]>>0,texture[i*3+2]>>0);
	}

	delete texture;*/

	unsigned char* rgbTex = new unsigned char[256*256*3];
	unsigned char* rgbTex2 = new unsigned char[256*256*3];

	texture = new unsigned short[256*256];

	Texture::createSplineTexture(rgbTex);
	
	/*Texture::createPlasmaTexture(rgbTex2, 256, 3, 255, 255, 255);

	for (i = 0; i < 256*256; i++)
	{
		int r = (rgbTex2[i*3]>>0) - (rgbTex[i*3]>>0) + 63*2; 
		int g = (rgbTex2[i*3+1]>>0) - (rgbTex[i*3+1]>>0) + 31*2;
		int b = (rgbTex2[i*3+2]>>0) - (rgbTex[i*3+2]>>0) + 9*2;

		if (r < 0) r = 0; if (r > 255) r = 255;
		if (g < 0) g = 0; if (g > 255) g = 255;
		if (b < 0) b = 0; if (b > 255) b = 255;

		rgbTex[i*3] = r;
		rgbTex[i*3+1] = g;
		rgbTex[i*3+2] = b;
	}*/
	
	Texture::convert24to16(texture, rgbTex, 256*256, 1);

	delete[] rgbTex;
	delete[] rgbTex2;

	for (i = 0; i < 1024; i++)
		sintab[i] = (int)(sin((float)i/512.0f*M_PI)*65536.0f);

}

TexturedGrid::~TexturedGrid()
{
	if (grid)
		delete[] grid;

	if (buffer)
		delete[] buffer;

	if (texture)
		delete[] texture;
}

struct Blob
{
	int x,y,z;
	int strength;
};

static Blob blobs[] = {{128,128,0,65536/16},{100,100,0,65536/16},{100,100,0,65536/16}};

#define MAXCLAMP 65536

void TexturedGrid::update(float syncFrac)
{
	int gridSizeX = gridWidth;
	int gridSizeY = gridHeight;

	int x,y;

	VectorFP* gridPt = grid;

	float phi = ::PPGetTickCount()*0.001f*0.5f;;

	blobs[0].x = (int)(sin(phi*4.0f)*8.0f*65536.0f);
	blobs[0].y = (int)(cos(phi*4.0f)*8.0f*65536.0f);
	blobs[0].z = (int)(sin(phi*4.0f)*8.0f*65536.0f);

	blobs[1].x = (int)(cos(-phi*2.0f)*8.0f*65536.0f);
	blobs[1].y = (int)(sin(phi*2.0f)*8.0f*65536.0f);
	blobs[1].z = (int)(cos(phi*2.0f)*8.0f*65536.0f);

	blobs[2].x = (int)(cos(-phi*3.0f)*12.0f*65536.0f);
	blobs[2].y = (int)(cos(phi*3.0f)*6.0f*65536.0f);
	blobs[2].z = (int)(sin(phi*3.0f)*10.0f*65536.0f);

	int numBlobs = sizeof(blobs)/sizeof(Blob);

	int pulse = (int)((sin(phi*16.0f)+1.0f)*32768.0f);

	int phiFP = (int)(phi*1024.0f);

	MatrixFP m;
	m.setRotZ(phi);

	for (y = 0; y < gridSizeY; y++)
		for ( x = 0; x < gridSizeX; x++)
		{
			VectorFP p;
			p.x = ((x-(gridSizeX>>1))<<16);
			p.y = ((y-(gridSizeY>>1))<<16);
			p.z = 0;

			p = m*p;
			
			int xOffset = /*0*/sintab[((p.x>>11)+phiFP)&1023]*32;
			int yOffset = /*0*/sintab[((p.y>>11)+phiFP)&1023]*32;

			//int xOffset = 0;
			//int yOffset = 0;

			gridPt->x = (((p.x>>1)-(gridSizeX<<15))<<4)+yOffset;
			gridPt->y = (((p.y>>1)-(gridSizeY<<15))<<4)+xOffset;
			gridPt->z = 256*65536;

			//////////////////////////////////////////////////////////////////////
			// blob stuff
			//////////////////////////////////////////////////////////////////////

			/*VectorFP p;
			p.x = ((x-(gridSizeX>>1))<<16);
			p.y = ((y-(gridSizeY>>1))<<16);
			p.z = 0.0f;

			int scale = 32768;
			for (int j = 0; j < numBlobs; j++) {
				int dx = fpmul((blobs[j].x-p.x),blobs[j].strength);
				int dy = fpmul((blobs[j].y-p.y),blobs[j].strength);
				int dz = fpmul((blobs[j].z-p.z),blobs[j].strength);
				
				int sd = fpmul(dx,dx)+fpmul(dy,dy)+fpmul(dz,dz);
				
				if (sd<(0.707f*0.707f*65536))
					scale += (fpmul(sd,sd)-sd+16384)*4;
			}
			
			if (scale>=MAXCLAMP) scale = MAXCLAMP;
			scale = fpmul(scale,scale);
			scale = fpmul(scale,scale);

			gridPt->x = fpdiv(p.x,scale+32768)<<4;
			gridPt->y = fpdiv(p.y,scale+32768)<<4;
			gridPt->z = 256*65536;*/

			//////////////////////////////////////////////////////////////////////
			// airbag
			//////////////////////////////////////////////////////////////////////
			/*int len = fpsqrt(fpmul(p.x-blobs[0].x,p.x-blobs[0].x)+fpmul(p.y-blobs[0].y,p.y-blobs[0].y));

			if (len>16*65536) len = 16*65536;

			int scale = len>>4;

			scale = fpmul(scale,scale);
			scale = fpmul(scale,scale);
			scale = fpmul(scale,scale);

			scale = 65536-fpmul((65536-scale),pulse);

			gridPt->x = fpmul(p.x,(scale<<2)+165536);
			gridPt->y = fpmul(p.y,(scale<<2)+165536);
			gridPt->z = 256*65536;*/

			//////////////////////////////////////////////////////////////////////
			// scale waves
			//////////////////////////////////////////////////////////////////////
			/*int len = fpsqrt(fpmul(p.x-blobs[0].x,p.x-blobs[0].x)+fpmul(p.y-blobs[0].y,p.y-blobs[0].y)+fpmul(p.z,p.z));

			int scale = 65536-((sintab[((len>>10)+phiFP)&1023]+65536)>>1);

			scale = fpmul(scale,scale);
			//scale = fpmul(scale,scale);
			//scale = fpmul(scale,scale);

			//scale = 65536-fpmul((65536-scale),pulse);

			gridPt->x = fpmul(p.x,(scale<<2)+165536);
			gridPt->y = fpmul(p.y,(scale<<2)+165536);
			gridPt->z = 256*65536;*/

			//////////////////////////////////////////////////////////////////////
			// displacement waves
			//////////////////////////////////////////////////////////////////////
			/*int cx = 0;//blobs[0].x;
			int cy = 0;//blobs[0].y;
			
			int dx = cx - p.x;
			int dy = cy - p.y;
			
			int len = fpsqrt(fpmul(dx,dx)+fpmul(dy,dy));
			
			if (len)
			{
				dx = fpdiv(dx, len);
				dy = fpdiv(dy, len);
			}

			int scale = sintab[((len>>10)-(phiFP<<1))&1023];

			//scale = fpmul(scale,scale);
			//scale = fpmul(scale,scale);
			//scale = fpmul(scale,scale);

			//scale = 65536-fpmul((65536-scale),pulse);

			gridPt->x = ((p.x+(phiFP<<4))<<3)+(fpmul(dx,scale<<5));
			gridPt->y = (p.y<<3)+(fpmul(dy,scale<<5));
			gridPt->z = 128*(scale+65536+8192);*/

			gridPt++;
		}

	phi+=0.1f;

}

void TexturedGrid::render(unsigned short* vscreen, unsigned int pitch)
{
	if (texture == NULL)
		return;

	int x,y;

	unsigned short* tex = texture;			

	int GRIDSIZE = gridsize;
	int GRIDSHIFT = gridshift;
	int PITCH = pitch;

	int gridSizeX = gridWidth-1;
	int gridSizeY = gridHeight-1;

	for (y=0;y<gridSizeY;y++) 
	{
        for (x=0;x<gridSizeX;x++) 
		{

			VectorFP *gridul = grid+(y*(gridSizeX+1)+x);
			VectorFP *gridur = gridul+1;
			VectorFP *gridll = gridul+(gridSizeX+1);
			VectorFP *gridlr = gridul+(gridSizeX+2);
			
			unsigned short *vscreenPtr = vscreen+((y*GRIDSIZE)*PITCH+(x*GRIDSIZE));                    

			int su1 = gridul->x;
			int sv1 = gridul->y;
			int si1 = gridul->z;
			
			int su2 = gridur->x;
			int sv2 = gridur->y;
			int si2 = gridur->z;
			
			int du1 = (gridll->x-gridul->x)>>GRIDSHIFT;
			int dv1 = (gridll->y-gridul->y)>>GRIDSHIFT;
			int di1 = (gridll->z-gridul->z)>>GRIDSHIFT;
			
			int du2 = (gridlr->x-gridur->x)>>GRIDSHIFT;
			int dv2 = (gridlr->y-gridur->y)>>GRIDSHIFT;
			int di2 = (gridlr->z-gridur->z)>>GRIDSHIFT;
			
			int y2,x2;
			for (y2=0;y2<GRIDSIZE;y2++) 
			{
				int fu = (su2-su1)>>GRIDSHIFT;
				int su = su1;
				int fv = (sv2-sv1)>>GRIDSHIFT;
				int sv = sv1;
				int fi = (si2-si1)>>GRIDSHIFT;
				int si = si1;
				
				for (x2=0;x2<GRIDSIZE;x2++) 
				{
					//int ofs = ((((sv>>8)&0xff00)+((su>>16)&0xff)));
					//unsigned int rgb = tex[ofs];
					
					//int r = ((rgb>>11)*si)>>24;
					//int g = (((rgb>>5)&0x3f)*si)>>24;
					//int b = ((rgb&0x1f)*si)>>24;
					
					//*vscreenPtr = PRGB2Short(r,g,b);
					
					//if (si <= 256*65536)
					//{
						unsigned int wRGB = tex[((((sv>>8)&0xff00)+((su>>16)&0xff)))];
						unsigned int lTemp = (((wRGB&(REDMASK|BLUEMASK))<<11)|((wRGB & GREENMASK) >> 5))*(si>>19);	  
						*vscreenPtr = ((lTemp >> (11 + 5)) & (REDMASK | BLUEMASK))|(lTemp & GREENMASK);
					/*}
					else
					{
						int shade = (si-256*65536)>>16;

						unsigned int rgb16_1 = tex[((((sv>>8)&0xff00)+((su>>16)&0xff)))];
						unsigned int rgb16_2 = RGB2SHORT(shade,shade,shade);
						
						unsigned int result = (rgb16_1&0xF800)+(rgb16_2&0xF800);
						if (result>0xF800) result = 0xF800;
						unsigned int t = (rgb16_1&0x7E0)+(rgb16_2&0x7E0); if (t>0x7E0) t = 0x7E0; result|=t;
						t = (rgb16_1&0x1F)+(rgb16_2&0x1F); if (t>0x1F) t = 0x1F; result|=t;
						
						*vscreenPtr = result;
					}*/

					//*vscreenPtr = tex[((((sv>>8)&0xff00)+((su>>16)&0xff)))];
					
					// enable for motion blur
					/*unsigned int mask=0x07e0f81f;
					
					int texel = tex[((((sv>>8)&0xff00)+((su>>16)&0xff)))];
					int pixel = *vscreenPtr;

					texel = ((texel<<16) | texel)&mask;
					pixel = ((pixel<<16) | pixel)&mask;
					
					int mix =((((texel-pixel)*(10))>>5)+pixel)&mask;
					*vscreenPtr = (mix | (mix>>16))&0xffff;*/

					su+=fu;
					sv+=fv;
					si+=fi;
					vscreenPtr++;
				}
				
				su1+=du1;
				su2+=du2;
				sv1+=dv1;
				sv2+=dv2;				
				si1+=di1;
				si2+=di2;
				vscreenPtr+=(PITCH-GRIDSIZE);
				
			}
			
		}
			
	}

	/*double phi = GetTickCount()*0.01f;
	int scale = (sin(phi)+1.0f) * 65536.0f * 2.0f;

	Filter::glow(vscreen, width, height, pitch, glowBuffer1, glowBuffer2, 2, scale);*/

	double phi = ::PPGetTickCount()*0.001f;

	//phi = 0.0f;

	int cx = (int)(sin(-phi)*(width*(1.0/3.0f))+(width*(1.0/2.0f)));
	int cy = (int)(cos(-phi)*(height*(1.0/3.0f))+(height*(1.0/2.0f)));
	
	//int radius = sin(phi)*2048 + 2048;
	int radius = 2048;

	Filter::applyRadial(vscreen, width, height, pitch, cx, cy, radius, 1);

	//Filter::applyBoxed(vscreen, buffer, width, height, pitch, 2);

	//Filter<160,120>::applyBoxed(vscreen, buffer, 4);

	//memcpy(vscreen, buffer, width*height*2);

}


