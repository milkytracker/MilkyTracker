/*
 *  tracker/wince/WaitStateThread.cpp
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

#include "WaitStateThread.h"
#include "PPMutex.h"

extern	unsigned short* vScreen;
extern	PPMutex* globalMutex;
void	UpdateScreen(unsigned short* vScreen);
void	drawString(const char* textBuffer,unsigned short* drawBuffer,unsigned int pitch, unsigned int posx,unsigned int posy, unsigned short col);

WaitStateThread* WaitStateThread::instance = NULL;

WaitStateThread::WaitStateThread()
{
	bActivated		= FALSE;
	nSleepTime		= 250;
	hThread		= NULL;
	threadID		= 0;
	
	saveBuffer		= NULL;
	
	xres = 320;
	yres = 240;
	pitch = 320;
	
	UpperLeftX = 160-70;
	UpperLeftY = 120-30;
	LowerRightX = 160+70;
	LowerRightY = 120+30;
	
	Width		= (LowerRightX - UpperLeftX);
	Height	= (LowerRightY - UpperLeftY);

	hThread = CreateThread(NULL, 0, &MyThreadProc, (LPVOID)this, 0, &threadID);

	bActivated = FALSE;
	nSleepTime = 250;
}

void WaitStateThread::setDisplayResolution(int width, int height)
{
	xres = pitch = width;
	yres = height;

	UpperLeftX = (width>>1)-70;
	UpperLeftY = (height>>1)-30;
	LowerRightX = (width>>1)+70;
	LowerRightY = (height>>1)+30;

	Width		= (LowerRightX - UpperLeftX);
	Height	= (LowerRightY - UpperLeftY);
}

static void SaveArea(unsigned short* saveBuffer, unsigned short* src, unsigned int pitch, const int fromX, const int fromY, const int toX, const int toY)
{
	const int Width		= (toX - fromX);
	const int Height	= (toY - fromY);

	for (int y = fromY; y < toY; y++)
	{
		for (int x = fromX;  x < toX; x++)
		{
			unsigned short* vPtr = src + y*pitch+x;
			unsigned short* dst = saveBuffer + (y-fromY) * Width + (x-fromX);
			*dst = *vPtr;
		}
	}
}

static void RestoreArea(unsigned short* src, unsigned short* saveBuffer, unsigned int pitch, const int fromX, const int fromY, const int toX, const int toY)
{
	const int Width		= (toX - fromX);
	const int Height	= (toY - fromY);

	for (int y = fromY; y < toY; y++)
	{
		for (int x = fromX;  x < toX; x++)
		{
			unsigned short* vPtr = src + y*pitch+x;
			unsigned short* dst = saveBuffer + (y-fromY) * Width + (x-fromX);
			*vPtr = *dst;
		}
	}
}

static void Darken(unsigned short* buffer, unsigned short pitch, const int fromX, const int fromY, const int toX, const int toY, const int scale)
{
	int x,y;

	for (y = fromY; y < toY; y++)
	{
		buffer[y*pitch+fromX] = 0xFFFF;
		buffer[y*pitch+toX-1] = 0xFFFF;
	}
	for (x = fromX; x < toX; x++)
	{
		buffer[fromY*pitch+x] = 0xFFFF;
		buffer[(toY-1)*pitch+x] = 0xFFFF;
	}
	
	for (y = fromY+1; y < toY-1; y++)
	{
		for (x = fromX+1;  x < toX-1; x++)
		{
			unsigned short* vPtr = buffer + y*pitch+x;
			int r = ((*vPtr >> 11)*scale)>>8;
			int g = (((*vPtr >> 5)&0x3F)*scale)>>8;
			int b = ((*vPtr & 0x1F)*scale)>>8;

			*vPtr = (r << 11) + (g << 5) + b;
		}
	}
}

static void DrawWaitBar(unsigned short* buffer,	const unsigned int width, const unsigned int height)
{
	const unsigned int barWidth			= 80;	 
	const unsigned int BAR_BRIGHT_COLOR = 0xffffff;
	const unsigned int BAR_DARK_COLOR 	= 0x103f67;

	static int offset = 0;

	int hBarPos = (width >> 1) - (barWidth>>1);
	int vBarPos = (height >> 1) + 2;
	//int vBarPos = LowerRightY - 20 - 8;
	
	unsigned short* vPtr = buffer + (vBarPos*width) + hBarPos;

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0;  x < barWidth; x++)
		{
			// rand für arme/faule
			if ((y == 0) || (y == 7) ||
				(x == 0) || (x == barWidth-1))
			{
				*vPtr = 0xFFFF;						
			}
			else
			{
				int c = (x+y+(offset)) & 0xf;
				
				int shade = 0;
				if (c <= 7)
					shade = c;
				else
					shade = 7 - (c - 8);
				
				shade = 255-(shade*32);
				
				DWORD red = ((BAR_BRIGHT_COLOR >> 16) * shade + (BAR_DARK_COLOR >> 16) * (255-shade)) >> 8;
				DWORD green = (((BAR_BRIGHT_COLOR >> 8) & 0xff) * shade + ((BAR_DARK_COLOR >> 8) & 0xff) * (255-shade)) >> 8;
				DWORD blue = ((BAR_BRIGHT_COLOR & 0xff) * shade + (BAR_DARK_COLOR & 0xff) * (255-shade)) >> 8;
				
				*vPtr = (WORD)((blue>>3) + ((green>>2)<<5)+ ((red>>3)<<11));
				
				
			}
			
			vPtr++;
		}
		vPtr+=width-barWidth;
	}

	offset+=4;
}

DWORD WINAPI WaitStateThread::MyThreadProc(LPVOID lpParameter)
{
	WaitStateThread* thisPtr = (WaitStateThread*)lpParameter;

	while (true)
	{
		if (thisPtr->bActivated)
		{
			DrawWaitBar(vScreen, thisPtr->xres, thisPtr->yres);
			UpdateScreen(vScreen);
		}

		Sleep(thisPtr->nSleepTime);
	}

	ExitThread(0);

	return 0;
} 

void WaitStateThread::activate(BOOL bActivate,BOOL bDarken/* = TRUE*/, BOOL bPutText/* = TRUE*/)
{
	if (bActivate)
	{
		globalMutex->lock();

		if (!saveBuffer)
		{
			saveBuffer = new WORD[Width*Height];
			if (saveBuffer)
				SaveArea(saveBuffer, vScreen, xres, UpperLeftX, UpperLeftY, LowerRightX, LowerRightY);
		}

		Darken(vScreen, xres, UpperLeftX, UpperLeftY, LowerRightX, LowerRightY, bDarken ? 128 : 256);
		if (bPutText)
			drawString("please wait", vScreen, xres, (xres>>1)-11*4, (yres>>1)-12, 0xFFFF);

		bActivated = TRUE;
		nSleepTime = 100;
		
		globalMutex->unlock();
	}
	else
	{	
		globalMutex->lock();

		bActivated = FALSE;
		nSleepTime = 250;

		if (saveBuffer)
		{
			RestoreArea(vScreen, saveBuffer, xres, UpperLeftX, UpperLeftY, LowerRightX, LowerRightY);
			delete saveBuffer;
			saveBuffer = NULL;
		}

		globalMutex->unlock();
	}
}
