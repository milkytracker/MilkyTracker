/*
 *  fx/Texture.cpp
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
#include "Math3d.h"
#include "Texture.h"
#include "TCBSpline.h"

#define RGB2SHORT(r,g,b) ((((r>>3))<<11)+(((g>>2))<<5)+((b>>3))) 

#define MAX_KEYS 50
#define MAX_TIME 200

static void setPixel(int* buffer,int x,int y,int r,int g,int b)
{
	int offset = (y&255)*256+(x&255);

	r+=buffer[offset*3]; if (r>65536) r = 65536;
	g+=buffer[offset*3+1]; if (g>65536) g = 65536;
	b+=buffer[offset*3+2]; if (b>65536) b = 65536;
	
	buffer[offset*3] = r;
	buffer[offset*3+1] = g;
	buffer[offset*3+2] = b;
}

static void setBlock(int* buffer,int x,int y,int blockSize, int r,int g,int b)
{
	for (int i = y; i < y+blockSize; i++)
		for (int j = x; j < x+blockSize; j++)
			setPixel(buffer, j, i, r,g,b);
}

static void drawSplines(TCBSpline* spline, int maxDots, int maxTime, int* buffer, int blockSize, int r, int g, int b)
{
	
	float rMaxdots = (1.0f / (float)maxDots) * maxTime;
	
	for (int j = 0; j < maxDots; j++)
	{	
		
		float curTime = (float)j * rMaxdots;		

		VectorFloat pos = spline->getPos(curTime);

		//int shade = pos.z*65536.0f;

		setBlock(buffer, (int)pos.x, (int)pos.y, blockSize, r, g, b);
		
	}
}


void Texture::createSplineTexture(unsigned char* tex, int numBlocks, int blockSize)
{

	TCBSpline* spline = new TCBSpline(MAX_KEYS);

	int* buffer = new int[256*256*3];

	memset(buffer, 0, 256*256*3*sizeof(int));

	int i;
	
	for (i = 0; i < MAX_KEYS; i++)
	{
		int x = rand()%256;
		int y = rand()%256;

		int z = rand()%256;

		VectorFloat v((float)x, (float)y, (float)z/255.0f);

		spline->setKey(i, v, (int)(((1.0f/((float)MAX_KEYS-1.0f))*(float)i)*MAX_TIME));
	}

	drawSplines(spline, numBlocks, MAX_TIME, buffer, blockSize, (5*65536/255)>>1, (4*65536/255)>>1, (65536*2/255)>>1);

	for (i = 0; i < MAX_KEYS; i++)
	{
		int x = rand()%256;
		int y = rand()%256;

		int z = rand()%256;

		VectorFloat v((float)x, (float)y, (float)z/255.0f);

		spline->setKey(i, v, (int)(((1.0f/((float)MAX_KEYS-1.0f))*(float)i)*MAX_TIME));
	}

	drawSplines(spline, numBlocks, MAX_TIME, buffer, blockSize, (0*65536/255)>>1, (65536/255)>>1, (4*65536/255)>>1);

	for (i = 0; i < 256*256; i++)
	{
		tex[i*3] = (int)(buffer[i*3]*255)>>16;
		tex[i*3+1] = (int)(buffer[i*3+1]*255)>>16;
		tex[i*3+2] = (int)(buffer[i*3+2]*255)>>16;
	}

	delete[] buffer;

	delete spline;
}

void Texture::createFlareTexture(unsigned char* tex,
								 int r,int g,int b,
								 float pw /*= 4.0f*/,
								 unsigned int size /*= 256*/)
{

	int hx = size>>1;
	int hy = size>>1;

	float maxlen = (float)sqrt(pow(size*0.5,2));

	float fac = 1.0f/maxlen;

	for (unsigned int y = 0; y < size; y++)
		for (unsigned int x = 0; x < size; x++)
		{
			
			int offset = (y*size+x)*3;

			int px = x - hx;
			int py = y - hy;

			float v = (float)(px*px+py*py);
			float len = (float)sqrt(v);

			if (len > maxlen)
				len = maxlen;

			v = (float)(sin((1.0f-(len*fac))*(M_PI/2.0f)));
			float fc = (float)pow(v,pw);
			
			//float fc = (float)pow(1.0f-(len*fac),pw);

			tex[offset] = (unsigned char)(r*fc);
			tex[offset+1] = (unsigned char)(g*fc);
			tex[offset+2] = (unsigned char)(b*fc);

		}

}

static int CalcLight(unsigned char* heightmap, int texmapsize, int x1,int y1,int x2,int y2,int depth, int smooth)
{
    int l,ran;
    ran = (texmapsize<<smooth)>>depth;
    l = (((int)heightmap[y1*texmapsize+x1]+(int)heightmap[y2*texmapsize+x2]) >> 1) + (rand()%ran)-(ran>>1);
    if (l<1) 
        l = 1;
    else if (l>255) 
        l = 255;
    
    return l;
}

static void GenRandom(unsigned char* heightmap, int texmapsize, int x1,int y1,int x2,int y2,int depth, int smooth)
{
    int xh,yh;
    int light;

    xh = (x1+x2) >> 1;
    yh = (y1+y2) >> 1;
    if (heightmap[y1*texmapsize+xh] == 0) heightmap[y1*texmapsize+xh] = CalcLight(heightmap, texmapsize,x1,y1,x2,y1,depth, smooth);
    if (heightmap[y2*texmapsize+xh] == 0) heightmap[y2*texmapsize+xh] = CalcLight(heightmap, texmapsize,x1,y2,x2,y2,depth, smooth);
    if (heightmap[yh*texmapsize+x1] == 0) heightmap[yh*texmapsize+x1] = CalcLight(heightmap, texmapsize,x1,y1,x1,y2,depth, smooth);
    if (heightmap[yh*texmapsize+x2] == 0) heightmap[yh*texmapsize+x2] = CalcLight(heightmap, texmapsize,x2,y1,x2,y2,depth, smooth);
    
	light = CalcLight(heightmap, texmapsize,x1,y1,x2,y2,depth, smooth) + 
			CalcLight(heightmap, texmapsize,x1,y2,x2,y1,depth, smooth);
    
	if (light>511)
        light = 511;
    
	heightmap[yh*texmapsize+xh] = light >> 1;

    if ((1<<depth) < texmapsize)
    {
        GenRandom(heightmap, texmapsize,x1,y1,xh,yh,depth+1, smooth);
        GenRandom(heightmap, texmapsize,xh,y1,x2,yh,depth+1, smooth);
        GenRandom(heightmap, texmapsize,x1,yh,xh,y2,depth+1, smooth);
        GenRandom(heightmap, texmapsize,xh,yh,x2,y2,depth+1, smooth);
    }
}

static void GenPlasma(unsigned char* heightmap, int texmapsize, int smooth)
{
    int i,x,y;
    
    memset(heightmap,0,texmapsize*texmapsize);
    i = rand()&255;
    heightmap[0*texmapsize+0]=i;
    heightmap[0*texmapsize+(texmapsize-1)]=i;
    heightmap[(texmapsize-1)*texmapsize+0]=i;
    heightmap[(texmapsize-1)*texmapsize+(texmapsize-1)] = i;
  
    GenRandom(heightmap, texmapsize, 0,0,texmapsize-1,texmapsize-1,1, smooth);
    for (y=1;y<=texmapsize-2;y++)
        for (x=1;x<=texmapsize-2;x++)
            heightmap[y*texmapsize+x] = 0;
  
    for (x = 0; x<=texmapsize-1; x++) heightmap[(texmapsize-1)*texmapsize+x] = heightmap[0*texmapsize+x];
    for (y = 0; y<=texmapsize-1; y++) heightmap[y*texmapsize+(texmapsize-1)] = heightmap[y*texmapsize+0];
    GenRandom(heightmap, texmapsize, 0,0,texmapsize-1,texmapsize-1,1, smooth);
}

void Texture::createPlasmaTexture(unsigned char* tex, unsigned int size, int smooth, int r, int g, int b)
{
	unsigned char* map = new unsigned char[size*size];

	GenPlasma(map, size, smooth);

	for (unsigned int i = 0; i < size*size; i++)
	{
		tex[i*3] = (map[i]*r)/255;
		tex[i*3+1] = (map[i]*g)/255;
		tex[i*3+2] = (map[i]*b)/255;
	}

	delete[] map;
}

void Texture::convert24to16(unsigned short* dstImage, unsigned char* srcImage, int size, unsigned int shifter)
{
	for (int i = 0; i < size; i++) 
		dstImage[i] = RGB2SHORT(srcImage[i*3]>>shifter,srcImage[i*3+1]>>shifter,srcImage[i*3+2]>>shifter);
}

void Texture::blur24(unsigned char* tex, unsigned int width, unsigned int height, unsigned int passes /*= 1*/)
{
	unsigned char* dst = new unsigned char[width*height*3];

	memcpy(dst, tex, width*height*3);

	for (unsigned int j = 0; j < passes; j++)
	{

		for (unsigned int y = 1; y < height-1; y++)
			for (unsigned int x = 1; x < width-1; x++)
			{
				
				int y1p = (y+1);
				int y1m = (y-1);
				
				int x1p = (x+1);
				int x1m = (x-1);
				
				for (int i = 0; i < 3; i++)
				{
					
					int c = (tex[(y*width+x)*3+i] + 
						tex[(y1m*width+x)*3+i] +
						tex[(y1p*width+x)*3+i] +
						tex[(y*width+x1m)*3+i] +
						tex[(y*width+x1p)*3+i])/5;
					
					dst[(y*width+x)*3+i] = c; 
				}
				
			}
			
		
		memcpy(tex, dst, width*height*3);
	}

	delete[] dst;

}
