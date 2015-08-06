/*
 *  fx/TCBSplineTest.cpp
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
#include "TCBSplineTest.h"
#include "TCBSpline.h"
#include "Math3d.h"
#include "Filter.h"

#define RGB2SHORT(r,g,b) ((((r>>3))<<11)+(((g>>2))<<5)+((b>>3))) 

static void setPixel(float* buffer,int width, int height, int x,int y,float r,float g,float b)
{
	if (y>=0&&x>=0&&y<height&&x<width) {
		int offset = (y*width+x);

		r+=buffer[offset*3]; if (r>1.0f) r = 1.0f;
		g+=buffer[offset*3+1]; if (g>1.0f) g = 1.0f;
		b+=buffer[offset*3+2]; if (b>1.0f) b = 1.0f;

		buffer[offset*3] = r;
		buffer[offset*3+1] = g;
		buffer[offset*3+2] = b;
	}
}

static void setBlock(float* buffer,int width, int height, int x,int y,float r,float g,float b)
{
	for (int i = y; i < y+4; i++)
		for (int j = x; j < x+4; j++)
			setPixel(buffer, width, height, j, i, r,g,b);
}

TCBSplineTest::TCBSplineTest(int width, int height)
{
	this->width = width;
	this->height = height;

	buffer = new float[width*height*3];
}

TCBSplineTest::~TCBSplineTest()
{
	delete[] buffer;
}

static void drawSplines(TCBSpline* spline, int maxDots, int maxTime, float* buffer, int width, int height, float r, float g, float b)
{
	for (int j = 0; j < maxDots; j++)
	{	
		
		float curTime = (float)(j / (float)maxDots)*(float)maxTime;		

		VectorFloat pos = spline->getPos(curTime);

		setBlock(buffer, width, height, (int)pos.x, (int)pos.y, r, g, b);
		
	}
}

#define MAX_KEYS 500
#define MAX_TIME 2000

static TCBSpline* spline = new TCBSpline(MAX_KEYS);

void TCBSplineTest::render(unsigned short* vscreen, unsigned int pitch)
{
	int i;
	
	memset(buffer,0, width*height*3*sizeof(float));
	
	for (i = 0; i < MAX_KEYS; i++)
	{
		int x = rand()%width;
		int y = rand()%height;

		VectorFloat v((float)x, (float)y, 0.0f);

		spline->setKey(i, v, (int)(((1.0f/((float)MAX_KEYS-1.0f))*(float)i)*MAX_TIME));
	}

	/*keys[0].v.set(159.0f-40.0f,119.0f-40.0f,0.0f);
	keys[0].ti = 0.0f*MAX_TIME;
	keys[0].t = 0.5f;

	keys[1].v.set(159.0f+40.0f,119.0f-40.0f,0.0f);
	keys[1].ti = 0.25f*MAX_TIME;
	keys[1].t = 0.5f;
	
	keys[2].v.set(159.0f+40.0f,119.0f+40.0f,0.0f);
	keys[2].ti = 0.5f*MAX_TIME;
	keys[2].t = 0.5f;

	keys[3].v.set(159.0f-40.0f,119.0f+40.0f,0.0f);
	keys[3].ti = 0.75f*MAX_TIME;
	keys[3].t = 0.5f;

	keys[4].v.set(159.0f-40.0f,119.0f-40.0f,0.0f);
	keys[4].ti = 1.0f*MAX_TIME;
	keys[4].t = 0.5f;*/

	drawSplines(spline, 100000, MAX_TIME, buffer, width, height, (5.0f/255.0f)*1.0f, (4.0f/255.0f)*1.0f, (2.0f/255.0f)*1.0f);

	for (i = 0; i < MAX_KEYS; i++)
	{
		int x = rand()%width;
		int y = rand()%height;

		VectorFloat v((float)x, (float)y, 0.0f);

		spline->setKey(i, v, (int)(((1.0f/((float)MAX_KEYS-1.0f))*(float)i)*MAX_TIME));
	}

	drawSplines(spline, 100000, MAX_TIME, buffer, width, height, (0.0f/255.0f)*1.0f, (1.0f/255.0f)*1.0f, (4.0f/255.0f)*1.0f);

	int offs = 0;
	for (i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			vscreen[i*pitch+j] = RGB2SHORT((int)(buffer[offs]*127.0f),(int)(buffer[offs+1]*127.0f),(int)(buffer[offs+2]*127.0f));
			offs+=3;
		}


	//Filter::applyRadial(vscreen, 320, 240, 320, 159, 119, 8192, 1);

}

void TCBSplineTest::update(float SyncFrac)
{
}
