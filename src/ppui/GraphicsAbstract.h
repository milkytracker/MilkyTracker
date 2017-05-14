/*
 *  ppui/GraphicsAbstract.h
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

#ifndef __GRAPHICSABSTRACT_H__
#define __GRAPHICSABSTRACT_H__

#include "BasicTypes.h"

class PPFont;

#define __PPGRAPHICSLINETEMPLATE \
	if (x1 > currentClipRect.x2 || \
		x2 < currentClipRect.x1 || \
		y1 > currentClipRect.y2 || \
		y2 < currentClipRect.y1) \
		return; \
	clipLine(x1, y1, x2, y2); \
	if (x2 < x1) \
		return; \
	pp_int32 i, deltax, deltay, numpixels; \
    pp_int32 d, dinc1, dinc2; \
    pp_int32 x, xinc1, xinc2; \
    pp_int32 y, yinc1, yinc2; \
	/* Calculate deltax and deltay for initialisation */ \
	deltax = abs(x2 - x1); \
	deltay = abs(y2 - y1); \
	/* Initialize all vars based on which is the independent variable */ \
	if (deltax >= deltay)  \
	{ \
		/* x is independent variable */ \
		numpixels = deltax + 1; \
		d = (2 * deltay) - deltax; \
		dinc1 = deltay << 1; \
		dinc2 = (deltay - deltax) << 1; \
		xinc1 = 1; \
		xinc2 = 1; \
		yinc1 = 0; \
		yinc2 = 1; \
	} \
	else  \
	{ \
		/* y is independent variable */ \
		numpixels = deltay + 1; \
		d = (2 * deltax) - deltay; \
		dinc1 = deltax << 1; \
		dinc2 = (deltax - deltay) << 1; \
		xinc1 = 0; \
		xinc2 = 1; \
		yinc1 = 1; \
		yinc2 = 1; \
    } \
	/* Make sure x and y move in the right directions */ \
	if (x1 > x2) \
	{ \
		xinc1 = - xinc1; \
		xinc2 = - xinc2; \
	} \
	if (y1 > y2) \
	{ \
		yinc1 = - yinc1; \
		yinc2 = - yinc2; \
	} \
	/* Start drawing at */ \
	x = x1; \
	y = y1; \
	/* Draw the pixels */ \
	for (i = 1; i<=numpixels; i++) \
	{ \
		setPixel(x,y); \
		if (d < 0)  \
		{ \
			d = d + dinc1; \
			x = x + xinc1; \
			y = y + yinc1; \
		} \
		else \
		{ \
			d = d + dinc2; \
			x = x + xinc2; \
			y = y + yinc2; \
		} \
	}

#define __PPGRAPHICSAALINETEMPLATE \
	if (x1 > currentClipRect.x2 || \
		x2 < currentClipRect.x1 || \
		y1 > currentClipRect.y2 || \
		y2 < currentClipRect.y1) \
	return; \
	clipLine(x1, y1, x2, y2); \
	if (x2 < x1) \
		return; \
	PPColor* col = &currentColor; \
    pp_int32 x, y, inc; /* these must be >=32 bits */ \
    pp_int32 dx, dy; \
    dx = (x2 - x1); \
    dy = (y2 - y1); \
    if (abs(dx) > abs(dy)) { \
        if (dx < 0) { \
            dx = -dx; \
            dy = -dy; \
            swap(&x1, &x2); \
            swap(&y1, &y2); \
        } \
        x = x1 << 16; \
        y = y1 << 16; \
        inc = (dx == 0 ? 0 : (dy * 65536) / dx); \
        while ((x >> 16) < x2) { \
            set_pixel_transp(this, x >> 16, y >> 16, \
                col, (y >> 8) & 0xFF); \
            set_pixel_transp(this, x >> 16, (y >> 16) + 1, \
                col, (~y >> 8) & 0xFF); \
            x += (1 << 16); \
            y += inc; \
        } \
    } else { \
        if (dy < 0) { \
            dx = -dx; \
            dy = -dy; \
            swap(&x1, &x2); \
            swap(&y1, &y2); \
        } \
        x = x1 << 16; \
        y = y1 << 16; \
        inc = (dy == 0 ? 0 : (dx * 65536) / dy); \
        while ((y >> 16) < y2) { \
            set_pixel_transp(this, x >> 16, y >> 16, \
                col, (x >> 8) & 0xFF); \
            set_pixel_transp(this, (x >> 16) + 1, (y >> 16), \
                col, (~x >> 8) & 0xFF); \
            x += inc; \
            y += (1 << 16); \
        } \
    }

class PPGraphicsAbstract
{
protected:
	static void swap(pp_int32* a, pp_int32* b)
	{
		pp_int32 h = *a;
		*a = *b;
		*b = h;
	}

	pp_int32 width, height;

public:
	bool lock;

protected:		
	PPColor currentColor;
	pp_uint16 color16;
	PPRect currentClipRect, origRect;

	PPFont* currentFont;

	virtual void validateRect()
	{
		if ((currentClipRect.x1 < 0 && currentClipRect.x2 < 0) ||
			(currentClipRect.y1 < 0 && currentClipRect.y2 < 0) ||
			(currentClipRect.x1 > width && currentClipRect.x2 > width) ||
			(currentClipRect.y1 > height && currentClipRect.y2 > height))
		{
			currentClipRect.x1 = 0;
			currentClipRect.x2 = 0;
			currentClipRect.y1 = 0;
			currentClipRect.y2 = 0;
			return;
		}
	
		if (currentClipRect.x1 < 0)
			currentClipRect.x1 = 0;
		if (currentClipRect.y1 < 0)
			currentClipRect.y1 = 0;

		if (currentClipRect.x2 > width)
			currentClipRect.x2 = width;
		if (currentClipRect.y2 > height)
			currentClipRect.y2 = height;
	}

	void convertColorTo16()
	{
		color16 = (((pp_uint16)((currentColor.r)>>3)<<11)+((pp_uint16)((currentColor.g)>>2)<<5)+(pp_uint16)((currentColor.b)>>3));
	}

	PPGraphicsAbstract(pp_int32 w, pp_int32 h) :
		width(w), height(h),
		lock(false),
		currentFont(NULL)
	{
	}

public:
	virtual ~PPGraphicsAbstract() {}
	pp_int32 getWidth() { return width; }
	pp_int32 getHeight() { return height; }

	void setRect(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2) 
	{ 
		currentClipRect.x1 = x1; currentClipRect.y1 = y1; currentClipRect.x2 = x2; currentClipRect.y2 = y2; 
		origRect = currentClipRect;
		validateRect();
	}
	
	void setRect(PPRect r) 
	{ 
		currentClipRect = origRect = r; 
		validateRect();
	}

	PPRect getRect() { return origRect; }

	void setFullRect()
	{
		currentClipRect.x1 = 0; 
		currentClipRect.y1 = 0; 
		currentClipRect.x2 = width-1; 
		currentClipRect.y2 = height-1; 	
		origRect = currentClipRect;
		validateRect();
	}

	void clipLine(pp_int32& x1, pp_int32& y1, pp_int32& x2, pp_int32& y2)
	{
		if (x1 < currentClipRect.x1)
		{
			float stepy = (float)(y2 - y1) / (float)(x2 - x1);
		
			y1 = (pp_int32)((float)y1 + stepy * (currentClipRect.x1-x1));
			x1 = currentClipRect.x1;
		}

		if (x2 > currentClipRect.x2)
		{
			float stepy = (float)(y2 - y1) / (float)(x2 - x1);
		
			x2 = currentClipRect.x2;
			y2 = (pp_int32)((float)y1 + stepy * (x2-x1));
		}
	}

	void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { currentColor.r = r; currentColor.g = g; currentColor.b = b; convertColorTo16(); }
	void setColor(const PPColor& color) { currentColor = color;  convertColorTo16(); }
	void setSafeColor(pp_int32 r,pp_int32 g,pp_int32 b) 
	{ 
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;
		currentColor.r = r; currentColor.g = g; currentColor.b = b; convertColorTo16(); 
	}

	PPColor getColor() { return currentColor; }

	virtual void setFont(PPFont* font) { currentFont = font; }
	PPFont* getCurrentFont() { return currentFont; }

	// Interfaces to be implemented by sub class
	virtual void setPixel(pp_int32 x, pp_int32 y) = 0;
	virtual void setPixel(pp_int32 x, pp_int32 y, const PPColor& color) = 0;

	virtual void fill(PPRect r) = 0;
	virtual void fill() = 0;

	virtual void drawHLine(pp_int32 x1, pp_int32 x2, pp_int32 y) = 0;
	virtual void drawVLine(pp_int32 y1, pp_int32 y2, pp_int32 x) = 0;

	virtual void drawLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2) = 0;
	virtual void drawAntialiasedLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2) = 0;

	virtual void blit(const pp_uint8* src, const PPPoint& p, const PPSize& size, 
					  pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity = 256) = 0;

	virtual void drawChar(pp_uint8 chr, pp_int32 x, pp_int32 y, bool underlined = false) = 0;
	virtual void drawString(const char* str, pp_int32 x, pp_int32 y, bool underlined = false) = 0;
	virtual void drawStringVertical(const char* str, pp_int32 x, pp_int32 y, bool underlined = false) = 0;

	virtual void fillVerticalShaded(PPRect r, const PPColor& colSrc, const PPColor& colDst, bool invertShading)
	{
		const pp_int32 height = (r.y2 - r.y1);

		if (!height)
			return;

		pp_int32 addr = (colDst.r - colSrc.r)*65536 / height;
		pp_int32 addg = (colDst.g - colSrc.g)*65536 / height;
		pp_int32 addb = (colDst.b - colSrc.b)*65536 / height;
		
		if (invertShading)
		{
			addr = -addr;
			addg = -addg;
			addb = -addb;
		}
					
		pp_int32 rd = (invertShading ? colDst.r : colSrc.r) * 65536;
		pp_int32 gr = (invertShading ? colDst.g : colSrc.g) * 65536;
		pp_int32 bl = (invertShading ? colDst.b : colSrc.b) * 65536;

		const pp_int32 y1 = r.y1;
		const pp_int32 y2 = r.y2;
		for (pp_int32 i = y1; i < y2; i++)
		{
			setColor(rd>>16,gr>>16,bl>>16);
			drawHLine(r.x1, r.x2, i);
			rd+=addr; gr+=addg; bl+=addb;
		}
	}

	virtual void fillVerticalShaded(const PPColor& colSrc, const PPColor& colDst, bool invertShading)
	{
		fillVerticalShaded(currentClipRect, colSrc, colDst, invertShading);
	}
};

#endif
