/*
 *  ppui/DisplayDeviceBase.h
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

#ifndef __DISPLAYDEVICEBASE_H__
#define __DISPLAYDEVICEBASE_H__

// Some default values
#ifndef __LOWRES__
	#define DISPLAYDEVICE_WIDTH 640
	#define DISPLAYDEVICE_HEIGHT 480
#else
	#define DISPLAYDEVICE_WIDTH 320
	#define DISPLAYDEVICE_HEIGHT 240
#endif

#include "BasicTypes.h"
#include "Font.h"

class PPGraphicsAbstract;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------ really shitty stuff ----------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*DrawPixelFunc)(void* buffer, pp_int32 x, pp_int32 y, pp_int32 pitch, const PPColor& color);

static void	DrawString(const char* textBuffer,
					   pp_int32 posx,
					   pp_int32 posy, 
					   const PPColor& color, 
					   void* buffer, 
					   pp_int32 pitch, 
					   DrawPixelFunc drawPixelFuncPtr)
{
	PPFont* font = PPFont::getFont(PPFont::FONT_SYSTEM);

    while (*textBuffer) 
	{
		pp_uint8 chr = *textBuffer++;
		{
			unsigned int j,i;
			
			for (i=0;i<8;i++)
				for (j=0;j<8;j++) 
					if (font->getPixelBit(chr, j, i)) 
						(*drawPixelFuncPtr)(buffer, posx+j, posy+i, pitch, color);
		}
        posx+=8;
    }
}

static void DrawWaitBar(const pp_int32 width, const pp_int32 height, 
						const pp_int32 barWidth, const pp_int32 barHeight,
						const pp_int32 BAR_BRIGHT_COLOR, const pp_int32 BAR_DARK_COLOR,
						void* buffer, pp_int32 pitch, DrawPixelFunc drawPixelFuncPtr)
{
	pp_int32 x,y;
	
	static int offset = 0;

	// background pattern, must be of type pp_uint8
	static pp_uint8 pattern[] = {224, 210, 198, 210, 224};
	
	for (y = 0; y < height; y++)
	{
		PPColor back;
		back.r = back.g = back.b = pattern[y % sizeof(pattern)];
		for (x = 0;  x < width; x++)
			(*drawPixelFuncPtr)(buffer, x, y, pitch, back);
	}

	PPColor textColor(0,0,0);
	::DrawString("please wait" PPSTR_PERIODS, (width>>1)-(14*4), (height>>1)-16, textColor, buffer, pitch, drawPixelFuncPtr);
	
	int hBarPos = (width >> 1) - (barWidth>>1);
	int vBarPos = (height >> 1) - (barHeight>>1) + (height>>3);
	
	for (y = 0; y < barHeight; y++)
	{
		for (x = 0;  x < barWidth; x++)
		{
			PPColor col;

			// little border for the lazy guys
			if ((y == 0) || (y == barHeight-1) ||
				(x == 0) || (x == barWidth-1))
			{
				col.r = col.g = col.b = 128;						
			}
			else
			{
				int c = (x+y+(offset)) & 0x1f;
				
				int shade = 0;
				if (c <= 31)
					shade = c;
				else
					shade = 31 - (c - 32);
				
				shade = 255-(shade*8);
				
				col.r = ((BAR_BRIGHT_COLOR >> 16) * shade + (BAR_DARK_COLOR >> 16) * (255-shade)) >> 8;
				col.g = (((BAR_BRIGHT_COLOR >> 8) & 0xff) * shade + ((BAR_DARK_COLOR >> 8) & 0xff) * (255-shade)) >> 8;
				col.b = ((BAR_BRIGHT_COLOR & 0xff) * shade + (BAR_DARK_COLOR & 0xff) * (255-shade)) >> 8;
			}

			(*drawPixelFuncPtr)(buffer, hBarPos+x, vBarPos+y, pitch, col);  
			
		}
	}

	offset+=8;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

enum MouseCursorTypes
{
	MouseCursorTypeStandard,
	MouseCursorTypeResizeLeft,
	MouseCursorTypeResizeRight,
	MouseCursorTypeHand,
	MouseCursorTypeWait
};

class PPDisplayDeviceBase
{
protected:
	PPGraphicsAbstract* currentGraphics;
	PPSize size;
	pp_int32 scaleFactor;

	pp_int32 updateStackPtr;
	pp_int32 disabledStackPtr;
	
	// ----------------------------- ex. PPWindow ----------------------------
	bool bFullScreen;
	PPColor waitBarColor;
	MouseCursorTypes currentCursorType;
	
public:
	PPDisplayDeviceBase(pp_int32 theWidth = DISPLAYDEVICE_WIDTH, 
						pp_int32 theHeight = DISPLAYDEVICE_HEIGHT,
						pp_int32 scaleFactor = 1) : 
		currentGraphics(0),
		size(PPSize(theWidth, theHeight)),
		scaleFactor(scaleFactor),
		updateStackPtr(0),
		disabledStackPtr(0),
		bFullScreen(false),
		currentCursorType(MouseCursorTypeStandard)
	{
	}

	virtual ~PPDisplayDeviceBase()
	{
	}

	void enable(bool b) 
	{
		if (!b)
			disabledStackPtr++;
		else if (disabledStackPtr > 0)
			--disabledStackPtr;
	}
	
	void allowForUpdates(bool b) 
	{ 
		if (!b)
			updateStackPtr++;
		else if (updateStackPtr > 0)
			--updateStackPtr;
	}

	bool isEnabled() const
	{
		return disabledStackPtr == 0;
	}

	bool isUpdateAllowed() const
	{
		return updateStackPtr == 0;
	}

	virtual PPGraphicsAbstract* open() = 0;
	virtual void close() = 0;
	
	virtual void update() = 0;

	virtual void update(const PPRect& r) = 0;

	virtual void setSize(const PPSize& size) { this->size = size; }
	virtual const PPSize& getSize() const { return this->size; }

	pp_int32 getScaleFactor() const { return scaleFactor; }
	virtual bool supportsScaling() const { return false; }

	static pp_int32 getDefaultWidth() { return DISPLAYDEVICE_WIDTH; }
	static pp_int32 getDefaultHeight() { return DISPLAYDEVICE_HEIGHT; }
	
	// ----------------------------- ex. PPWindow ----------------------------
public:

	virtual bool init()	{ return true; }

	virtual void setTitle(const PPSystemString& title) { }	

	virtual bool goFullScreen(bool b) { return true; }
	virtual bool isFullScreen() { return bFullScreen; }
	
	virtual PPSize getDisplayResolution() const { return PPSize(-1, -1); }
	
	virtual void shutDown()	= 0;

	virtual void signalWaitState(bool b, const PPColor& color) { }

	virtual void setMouseCursor(MouseCursorTypes type) { currentCursorType = type; }
	virtual MouseCursorTypes getCurrentActiveMouseCursor() { return currentCursorType; }
};

#endif
