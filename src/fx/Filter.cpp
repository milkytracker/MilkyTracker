/*
 *  fx/Filter.cpp
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
#include "fpmath.h"
#include "Filter.h"

#define PRGB2Short(r,g,b) ((((r))<<11)+(((g))<<5)+((b))) 

void Filter::applyRadialToSector(unsigned short *buffer,
								 int width, int height,int pitch,
								 int xCenter,int yCenter,int xBound,int yBound,
								 int radius, int style) 
{

    int stx=-1,sty=-1;
    
	if (yBound>yCenter)
        sty=1;
    if (xBound>xCenter)
        stx=1;

	int i = yCenter;

	switch (style)
	{
		case 0:
			while (i!=yBound)
			{
				int j = xCenter;
				
				unsigned short *line = buffer+i*pitch;
				
				int vs = (((yCenter-i)*radius))+(i<<16);
				int v0=(vs>>16)*pitch;
				int b=(vs>>11)&31;
				int v1=v0+pitch;
				
				while (j!=xBound)
				{
					int h0,h1,h2,h3;
					int at;//,c,col;
					int u0,u1;
					int us = (((xCenter-j)*radius))+(j<<16);
					
					unsigned int mask=0x07e0f81f;
					
					u0=(us>>16);
					at=(us>>11)&31;
					u1=u0+1;
					h0=buffer[(u0+v0)]; h2=buffer[(u0+v1)];
					h1=buffer[(u1+v0)]; h3=buffer[(u1+v1)];
					
					h1 = ((h1<<16) | h1)&mask;
					h2 = ((h2<<16) | h2)&mask;
					h3 = ((h3<<16) | h3)&mask;
					h0 = ((h0<<16) | h0)&mask;
					
					int blend_top =((((h1-h0)*(at))>>5)+h0)&mask;
					
					int blend_bot =((((h3-h2)*(at))>>5)+h2)&mask;
					
					int total = ((((blend_bot - blend_top)*(b))>>5)+blend_top)&mask;
					
					int source = line[j];
					
					source = ((source<<16) | source)&mask;
					
					int mix =((((source-total)*(15))>>5)+total)&mask;
					line[j]	= (mix | (mix>>16))&0xffff;//( (total & 0xF7DE) >> 1) + ( (source & 0xF7DE) >> 1);
					
					j+=stx;
				}
				
				i+=sty;
				
			}
			break;

		case 1:
			while (i!=yBound)
			{
				int j = xCenter;
				
				unsigned short *line = buffer+i*pitch;
				
				int vs = (((yCenter-i)*radius))+(i<<16);
				int v0=(vs>>16)*pitch;
				int b=(vs>>11)&31;
				int v1=v0+pitch;
				
				while (j!=xBound)
				{
					int h0,h1,h2,h3;
					int at;//,c,col;
					int u0,u1;
					int us = (((xCenter-j)*radius))+(j<<16);
					
					unsigned int mask=0x07e0f81f;
					
					u0=(us>>16);
					at=(us>>11)&31;
					u1=u0+1;
					h0=buffer[(u0+v0)]; h2=buffer[(u0+v1)];
					h1=buffer[(u1+v0)]; h3=buffer[(u1+v1)];
					
					h1 = ((h1<<16) | h1)&mask;
					h2 = ((h2<<16) | h2)&mask;
					h3 = ((h3<<16) | h3)&mask;
					h0 = ((h0<<16) | h0)&mask;
					
					int blend_top =((((h1-h0)*(at))>>5)+h0)&mask;
					
					int blend_bot =((((h3-h2)*(at))>>5)+h2)&mask;
					
					int total = ((((blend_bot - blend_top)*(b))>>5)+blend_top)&mask;
					
					int source = line[j];
					
					total = (total | (total>>16))&0xffff;
					int r = ((source>>11)+((total>>11))); r-=4; if (r<0) r = 0; if (r>0x1f) r = 0x1f;
					int g = (((source>>5)&0x3f)+(((total>>5)&0x3f)));  g-=8; if (g<0) g = 0; if (g>0x3f) g = 0x3f;
					int bc = ((source&0x1f)+((total&0x1f)));  bc-=4; if (bc<0) bc = 0; if (bc>0x1f) bc = 0x1f;
					
					line[j] = (r<<11)+(g<<5)+bc;
					
					j+=stx;
				}
				
				i+=sty;
				
			}
			break;
	
	}
	
}

void Filter::applyRadial(unsigned short *buffer,
						 int width, int height,int pitch,
						 int xCenter,int yCenter,
						 int radius, int style) 
{
	
	applyRadialToSector(buffer,width, height, pitch, xCenter-1, yCenter-1, 0,     0,      radius, style);
	applyRadialToSector(buffer,width, height, pitch, xCenter,   yCenter-1, width, 0,      radius, style);
	applyRadialToSector(buffer,width, height, pitch, xCenter-1, yCenter,   0,     height, radius, style);
	applyRadialToSector(buffer,width, height, pitch, xCenter,   yCenter,   width, height, radius, style);

}

void Filter::applyHorizontal(unsigned short *src, 
							 unsigned short *dst, 
							 int width, int height, int pitch, 
							 int boxw)
{
	int x,y;

	if (boxw<0)
	{
		memcpy(dst,src,pitch*height*2); // deal with degenerate kernel sizes
		return;
	}
	if (boxw>=width) boxw=width-1;
	int mul=65536/(boxw*2+1);
	for (y=0;y<height;y++)
	{
		int totr=0;
		int totg=0;
		int totb=0;
		for (x=0;x<boxw;x++) {
			totr+=src[x]>>11;
			totg+=(src[x]>>5)&0x3f;
			totb+=src[x]&0x1f;
		}
		
		for (x=0;x<width;x++)
		{
			if (x>boxw) {
				totr-=src[(-boxw-1)]>>11;
				totg-=(src[(-boxw-1)]>>5)&0x3f;
				totb-=src[(-boxw-1)]&0x1f;
			}
			
			if (x+boxw<width) {
				totr+=src[boxw]>>11;
				totg+=(src[boxw]>>5)&0x3f;
				totb+=src[boxw]&0x1f;
			}
			int finalr = (totr*mul)>>16;
			int finalg = (totg*mul)>>16;
			int finalb = (totb*mul)>>16;
			*dst = (finalr<<11)+(finalg<<5)+finalb;
			dst++;
			src++;
		}
	}	
}

void Filter::applyVertical(unsigned short *src, 
						   unsigned short *dst,
						   int width, int height, int pitch, 
						   int boxw)
{
	int x,y;

	if (boxw<0)
	{
		memcpy(dst,src,pitch*height*2); // deal with degenerate kernel sizes
		return;
	}
	if (boxw>=width) boxw=width-1;
	int mul=65536/(boxw*2+1);
	for (x=0;x<width;x++)
	{
		int totr=0;
		int totg=0;
		int totb=0;
		
		unsigned short* srcPtr = src+x;

		for (y=0;y<boxw;y++) {
			totr+=*srcPtr>>11;
			totg+=(*srcPtr>>5)&0x3f;
			totb+=*srcPtr&0x1f;
			srcPtr+=width;
		}
		
		srcPtr = src+x;
		unsigned short* dstPtr = dst+x;

		int offsm = (-boxw-1)*pitch;
		int offsp = boxw*pitch;

		for (y=0;y<height;y++)
		{
			if (y>boxw) {
				totr-=srcPtr[offsm]>>11;
				totg-=(srcPtr[offsm]>>5)&0x3f;
				totb-=srcPtr[offsm]&0x1f;
			}
			
			if (y+boxw<height) {
				totr+=srcPtr[offsp]>>11;
				totg+=(srcPtr[offsp]>>5)&0x3f;
				totb+=srcPtr[offsp]&0x1f;
			}
			int finalr = (totr*mul)>>16;
			int finalg = (totg*mul)>>16;
			int finalb = (totb*mul)>>16;
			*dstPtr = (finalr<<11)+(finalg<<5)+finalb;
			dstPtr+=width;
			srcPtr+=width;
		}
	}	
}

void Filter::applyBoxed(unsigned short *srcImage, 
						unsigned short *dstImage, 
						int width, int height, int pitch,
						int boxw)
{
	if (boxw<0)
	{
		return;
	}


	//int pitch = width;
	int x,y;
	int mul = 65536/(boxw*2+1);
	
	{
		unsigned short* src = srcImage;
		
		if (boxw>=width) boxw=width-1;
		for (y=0;y<height;y++)
		{
			int totr=0;
			int totg=0;
			int totb=0;
			for (x=0;x<boxw;x++) {
				totr+=src[x]>>11;
				totg+=(src[x]>>5)&0x3f;
				totb+=src[x]&0x1f;
			}
			
			unsigned short* dst = dstImage+y;
			
			for (x=0;x<width;x++)
			{
				if (x>boxw) {
					totr-=src[(-boxw-1)]>>11;
					totg-=(src[(-boxw-1)]>>5)&0x3f;
					totb-=src[(-boxw-1)]&0x1f;
				}
				
				if (x+boxw<width) {
					totr+=src[boxw]>>11;
					totg+=(src[boxw]>>5)&0x3f;
					totb+=src[boxw]&0x1f;
				}
				int finalr = (totr*mul)>>16;
				int finalg = (totg*mul)>>16;
				int finalb = (totb*mul)>>16;
				*dst = (finalr<<11)+(finalg<<5)+finalb;
			
				dst+=height;
				src++;
			}
		}	
		
	}

	{
		
		if (boxw>=height) boxw=height-1;

		unsigned short* src = dstImage;
		
		for (x=0;x<width;x++)
		{
			int totr=0;
			int totg=0;
			int totb=0;
			for (y=0;y<boxw;y++) {
				totr+=src[y]>>11;
				totg+=(src[y]>>5)&0x3f;
				totb+=src[y]&0x1f;
			}
			
			unsigned short* dst = srcImage+x;
			
			for (y=0;y<height;y++)
			{
				if (y>boxw) {
					totr-=src[(-boxw-1)]>>11;
					totg-=(src[(-boxw-1)]>>5)&0x3f;
					totb-=src[(-boxw-1)]&0x1f;
				}
				
				if (y+boxw<height) {
					totr+=src[boxw]>>11;
					totg+=(src[boxw]>>5)&0x3f;
					totb+=src[boxw]&0x1f;
				}
				int finalr = (totr*mul)>>16;
				int finalg = (totg*mul)>>16;
				int finalb = (totb*mul)>>16;
				*dst = (finalr<<11)+(finalg<<5)+finalb;
				//*dst = *src;

				src++;
				dst+=width;
			}
		}	
		
	}


}

void Filter::stylize(unsigned short* srcImage, 
							unsigned short* tmpImage,
							int width, int height, int srcPitch, int tmpPitch,
							int minr, int ming, int minb, 
							int maxr, int maxg, int maxb)
{
	int x,y;
	
	minb>>=3;
	ming>>=2; ming<<=5;
	minr>>=3; minr<<=11;

	maxb>>=3;
	maxg>>=2; maxg<<=5;
	maxr>>=3; maxr<<=11;

	unsigned short* dst = srcImage;
	unsigned short* src = tmpImage;

	unsigned int* dstDW = (unsigned int*)dst;
	unsigned int* srcDW = (unsigned int*)src;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width>>1; x++)
		{
			*srcDW = *dstDW;
			srcDW++; dstDW++;
		}
		srcDW+=(tmpPitch-width)>>1;
		dstDW+=(srcPitch-width)>>1;
	}

	dst = srcImage;
	src = tmpImage;

	//memcpy(tmpImage, srcImage, pitch*height*2);

	int y2;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
		
			//if (*src > RGB2SHORT(50,50,150) &&
			//	*src < RGB2SHORT(150,120,200))
			
			if ((*src&0x1f) > (minb) &&
				(*src&0x1f) < (maxb) &&
				(*src&(0x3f<<5)) > (ming) &&
				(*src&(0x3f<<5)) < (maxg) &&
				(*src&(0x1f<<11)) > (minr) &&
				(*src&(0x1f<<11)) < (maxr))
			{
			
				int b = (*src&0x1f)>>2;
				int g = ((*src>>5)&0x3f)>>2;
				int r = ((*src>>11)&0x1f)>>2;
			
				int from = y-64;
				int to = y+64;
				int adder = 256/64;
				int shade = 0;
			
				if (from < 0)
				{
					shade+=-from*adder;
					from = 0;
				}
				if (to>height)
					to = height;
			
				unsigned short* ptr = srcImage+(from*srcPitch+x);
				for (y2 = from; y2 < to; y2++)
				{
					if (y2 == y)
						adder = -adder;
					int r2 = ((*ptr>>11)&0x1f) + ((r*shade)>>8);
					int g2 = ((*ptr>>5)&0x3f) + ((g*shade)>>8);
					int b2 = (*ptr&0x1f) + ((b*shade)>>8);
					if (r2 > 0x1f) r2 = 0x1f;
					if (g2 > 0x3f) g2 = 0x3f;
					if (b2 > 0x1f) b2 = 0x1f;
					
					*ptr = PRGB2SHORT(r2,g2,b2);
					ptr+=width;
					shade+=adder;
				}
				
				from = x-64;
				to = x+64;
				shade = 0;
				adder = 256/64;
				
				if (from < 0)
				{
					shade+=-from*adder;
					from = 0;
				}
				if (to>width)
					to = width;
			
				ptr = srcImage+(y*srcPitch+from);
				for (y2 = from; y2 < to; y2++)
				{
					if (y2 == x)
						adder = -adder;
					int r2 = ((*ptr>>11)&0x1f) + ((r*shade)>>8);
					int g2 = ((*ptr>>5)&0x3f) + ((g*shade)>>8);
					int b2 = (*ptr&0x1f) + ((b*shade)>>8);
					if (r2 > 0x1f) r2 = 0x1f;
					if (g2 > 0x3f) g2 = 0x3f;
					if (b2 > 0x1f) b2 = 0x1f;
					
					*ptr = PRGB2SHORT(r2,g2,b2);
					ptr++;
					shade+=adder;
				}
			
			}
		
			src++;
		}
		
		src+=tmpPitch-width;

	}
}

void Filter::glow(unsigned short* srcImage,
				  int width, int height, int pitch, 
				  unsigned short* glowBuffer1,
				  unsigned short* glowBuffer2,
				  unsigned int cellSizeShift,
				  unsigned int scale, unsigned int boxw)
{

	if (scale < 512) 
		scale = 512;

	int gWidth = width>>cellSizeShift;
	int gHeight = height>>cellSizeShift;

	int x,y,x2,y2;

	unsigned short* ptr = glowBuffer1;
	for (y = 0; y < gHeight; y++)
		for (x = 0; x < gWidth; x++)
			*ptr++ = srcImage[(y<<cellSizeShift)*pitch+(x<<cellSizeShift)];

	applyBoxed(glowBuffer1, glowBuffer2, gWidth, gHeight, gWidth, boxw);	

	//int scale = 65536*5;

	for (y = 0; y < gHeight; y++)
		for (x = 0; x < gWidth; x++)
		{

			int offset = y*gWidth+x;
			int hOffs = 1;
			int vOffs = gWidth;
			if (x == gWidth-1)
				hOffs = 0;
			if (y == gHeight-1)
				vOffs = 0;

			unsigned int rgb1, rgb2, rgb3, rgb4;

			rgb1 = glowBuffer1[offset];			
			rgb2 = glowBuffer1[offset+hOffs];
			rgb3 = glowBuffer1[offset+vOffs+hOffs];
			rgb4 = glowBuffer1[offset+vOffs];				
			
			int r1 = fpmul((rgb1>>11)<<16,scale), g1 = fpmul((rgb1&0x7E0)<<11,scale), b1 = fpmul((rgb1&31)<<16,scale);
			int r2 = fpmul((rgb2>>11)<<16,scale), g2 = fpmul((rgb2&0x7E0)<<11,scale), b2 = fpmul((rgb2&31)<<16,scale);
			int r3 = fpmul((rgb3>>11)<<16,scale), g3 = fpmul((rgb3&0x7E0)<<11,scale), b3 = fpmul((rgb3&31)<<16,scale);
			int r4 = fpmul((rgb4>>11)<<16,scale), g4 = fpmul((rgb4&0x7E0)<<11,scale), b4 = fpmul((rgb4&31)<<16,scale);
			
			int dr1 = (r4-r1)>>cellSizeShift;
			int dr2 = (r3-r2)>>cellSizeShift;
			
			int dg1 = (g4-g1)>>cellSizeShift;
			int dg2 = (g3-g2)>>cellSizeShift;
			
			int db1 = (b4-b1)>>cellSizeShift;
			int db2 = (b3-b2)>>cellSizeShift;
			
			ptr = srcImage+(y<<cellSizeShift)*pitch+(x<<cellSizeShift);
			
			for (y2=0;y2<(1<<cellSizeShift);y2++) 
			{
				int fr = (r2-r1)>>cellSizeShift;
				int sr = r1;
				
				int fg = (g2-g1)>>cellSizeShift;
				int sg = g1;
				
				int fb = (b2-b1)>>cellSizeShift;
				int sb = b1;
				
				for (x2=0;x2<(1<<cellSizeShift);x2++) 
				{
					int r = (sr>>16) + ((*ptr>>11)&31); if (r>31) r = 31;
					int g = (sg>>16) + ((*ptr>>5)&63); if (g>63) g = 63;
					int b = (sb>>16) + ((*ptr>>0)&31); if (b>31) b = 31;
					
					*ptr++ = PRGB2Short(r,g,b);
					sr+=fr;
					sg+=fg;
					sb+=fb;
				}
				
				ptr+=(pitch-(1<<cellSizeShift));
				
				r1+=dr1;
				r2+=dr2;
				
				g1+=dg1;
				g2+=dg2;
				
				b1+=db1;
				b2+=db2;
			}
			
		}

}
