/*
 *  ppui/Graphics_BGR24_SLOW.cpp
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

#include "Graphics.h"
#include "Font.h"

#define BPP 3

PPGraphics_BGR24_SLOW::PPGraphics_BGR24_SLOW(pp_int32 w, pp_int32 h, pp_int32 p, void* buff) :
	PPGraphicsFrameBuffer(w, h, p, buff)
{
}

void PPGraphics_BGR24_SLOW::setPixel(pp_int32 x, pp_int32 y)
{
	if (y >= currentClipRect.y1 && y < currentClipRect.y2 &&
		x >= currentClipRect.x1 && x < currentClipRect.x2)
	{

		pp_uint8* buff = (pp_uint8*)buffer+pitch*y+x*BPP;
		
		buff[0] = (pp_uint8)currentColor.b;
		buff[1] = (pp_uint8)currentColor.g;
		buff[2] = (pp_uint8)currentColor.r;		

	}	
}

void PPGraphics_BGR24_SLOW::setPixel(pp_int32 x, pp_int32 y, const PPColor& color)
{
	if (y >= currentClipRect.y1 && y < currentClipRect.y2 &&
		x >= currentClipRect.x1 && x < currentClipRect.x2)
	{

		pp_uint8* buff = (pp_uint8*)buffer+pitch*y+x*BPP;
		
		buff[0] = (pp_uint8)color.b;
		buff[1] = (pp_uint8)color.g;
		buff[2] = (pp_uint8)color.r;		

	}	
}

void PPGraphics_BGR24_SLOW::fill(PPRect rect)
{
	
	if (rect.y1 < currentClipRect.y1)
		rect.y1 = currentClipRect.y1;

	if (rect.x1 < currentClipRect.x1)
		rect.x1 = currentClipRect.x1;

	if (rect.y2 > currentClipRect.y2)
		rect.y2 = currentClipRect.y2;

	if (rect.x2 > currentClipRect.x2)
		rect.x2 = currentClipRect.x2;

	pp_uint8 r = (pp_uint8)currentColor.r;
	pp_uint8 g = (pp_uint8)currentColor.g;
	pp_uint8 b = (pp_uint8)currentColor.b;

	pp_int32 len = (rect.x2-rect.x1); 

	pp_uint8* buff = (pp_uint8*)buffer+(pitch*rect.y1+rect.x1*BPP); 
	
	for (pp_int32 y = rect.y1; y < rect.y2; y++)
	{				
		pp_uint8* ptr = buff;
		
		pp_int32 i;
		
		for (i = 0; i < len; i++)
		{
			*ptr = b;
			*(ptr+1) = g;
			*(ptr+2) = r;
			ptr+=3;
		}
		
		buff+=pitch;
		
	}	

}

void PPGraphics_BGR24_SLOW::fill()
{

	fill(currentClipRect);

}

void PPGraphics_BGR24_SLOW::drawHLine(pp_int32 x1, pp_int32 x2, pp_int32 y)
{

	pp_int32 lx = x1;
	pp_int32 rx = x2;

	if (x1 > x2)
	{
		pp_int32 h = x2;
		x2 = x1;
		x1 = h;
	}

	if (lx < currentClipRect.x1)
		lx = currentClipRect.x1;

	if (rx > currentClipRect.x2)
		rx = currentClipRect.x2;

	if (y < currentClipRect.y1)
		return;

	if (y >= currentClipRect.y2)
		return;

	pp_uint8 r = (pp_uint8)currentColor.r;
	pp_uint8 g = (pp_uint8)currentColor.g;
	pp_uint8 b = (pp_uint8)currentColor.b;

	pp_int32 len = (rx-lx); 
	pp_uint8* buff = (pp_uint8*)buffer+pitch*y+lx*BPP;

	pp_int32 i;		
	for (i = 0; i < len; i++)
	{
		*buff = b;
		*(buff+1) = g;
		*(buff+2) = r;
		buff+=3;
	}
	
}

void PPGraphics_BGR24_SLOW::drawVLine(pp_int32 y1, pp_int32 y2, pp_int32 x)
{

	pp_int32 ly = y1;
	pp_int32 ry = y2;

	if (y1 > y2)
	{
		pp_int32 h = y2;
		y2 = y1;
		y1 = h;
	}

	if (ly < currentClipRect.y1)
		ly = currentClipRect.y1;

	if (ry > currentClipRect.y2)
		ry = currentClipRect.y2;

	if (x < currentClipRect.x1)
		return;

	if (x >= currentClipRect.x2)
		return;

	pp_uint8* buff = (pp_uint8*)buffer+pitch*ly+x*BPP;

	pp_uint8 r = (pp_uint8)currentColor.r;
	pp_uint8 g = (pp_uint8)currentColor.g;
	pp_uint8 b = (pp_uint8)currentColor.b;

	for (pp_int32 y = ly; y < ry; y++)
	{	
		*buff = b;
		*(buff+1) = g;
		*(buff+2) = r;
		buff+=pitch;
	}

}

void PPGraphics_BGR24_SLOW::drawLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{
	__PPGRAPHICSLINETEMPLATE
}

void PPGraphics_BGR24_SLOW::drawAntialiasedLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{
	__PPGRAPHICSAALINETEMPLATE
}

void PPGraphics_BGR24_SLOW::blit(const pp_uint8* src, const PPPoint& p, const PPSize& size, pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity/* = 256*/)
{
	pp_int32 w = size.width;
	pp_int32 h = size.height;
	
	const pp_int32 bytesPerPixel = bpp;
	
	pp_uint8* dst = (pp_uint8*)buffer+(this->pitch*p.y+p.x*BPP); 
	
	if (intensity == 256)
	{
		for (pp_int32 y = 0; y < h; y++)
		{
			for (pp_int32 x = 0; x < w; x++)
			{		
				dst[2] = src[0];
				dst[1] = src[1];
				dst[0] = src[2];
				dst+=BPP;
				src+=bytesPerPixel;
			}
			dst+=this->pitch - w*BPP;
			src+=pitch - w*bytesPerPixel;
		}
	}
	else
	{
		for (pp_int32 y = 0; y < h; y++)
		{
			for (pp_int32 x = 0; x < w; x++)
			{		
				dst[2] = (src[0]*intensity)>>8;
				dst[1] = (src[1]*intensity)>>8;
				dst[0] = (src[2]*intensity)>>8;
				dst+=BPP;
				src+=bytesPerPixel;
			}
			dst+=this->pitch - w*BPP;
			src+=pitch - w*bytesPerPixel;
		}
	}
}


void PPGraphics_BGR24_SLOW::drawChar(pp_uint8 chr, pp_int32 x, pp_int32 y, bool underlined)
{

	if (currentFont == NULL)
		return;


	pp_int32 charWidth = (signed)currentFont->getCharWidth();
	pp_int32 charHeight = (signed)currentFont->getCharHeight(); 
	pp_int32 charDim = currentFont->charDim;

	if (x + (signed)charWidth < currentClipRect.x1 ||
		x > currentClipRect.x2 ||
		y + (signed)charHeight < currentClipRect.y1 ||
		y > currentClipRect.y2)
		return;
	
	/*pp_uint8 r = (pp_uint8)currentColor.r;
	pp_uint8 g = (pp_uint8)currentColor.g;
	pp_uint8 b = (pp_uint8)currentColor.b;

	if (x>= currentClipRect.x1 && x + (signed)currentFont->charWidth < currentClipRect.x2 &&
		y>= currentClipRect.y1 && y + (signed)currentFont->charHeight < currentClipRect.y2)
	{
		for (pp_uint32 i = 0; i < currentFont->charHeight; i++)
		{
			pp_uint8* buff = (pp_uint8*)buffer + (y+i)*pitch + x*3;
			for (pp_uint32 j = 0; j < currentFont->charWidth; j++)
			{
				
				if (currentFont->getPixelBit(chr, j,i))
				{
					buff[0] = b;
					buff[1] = g;
					buff[2] = r;					
				}
				buff+=3;
			}
		}
	}
	else
	{
		for (pp_uint32 i = 0; i < currentFont->charHeight; i++)
			for (pp_uint32 j = 0; j < currentFont->charWidth; j++)
			{
				
				if (y+(signed)i >= currentClipRect.y1 && y+(signed)i < currentClipRect.y2 &&
					x+(signed)j >= currentClipRect.x1 && x+(signed)j < currentClipRect.x2 &&
					currentFont->getPixelBit(chr, j,i))
				{
					
					pp_uint8* buff = (pp_uint8*)buffer+((y+i)*pitch+(x+j)*3);
					
					buff[0] = b;
					buff[1] = g;
					buff[2] = r;
					
				}
			}

	}*/
	pp_uint8 r = (pp_uint8)currentColor.r;
	pp_uint8 g = (pp_uint8)currentColor.g;
	pp_uint8 b = (pp_uint8)currentColor.b;

	Bitstream* bitstream = currentFont->bitstream;
	pp_uint8* fontbuffer = bitstream->buffer;

	pp_uint8* buff = (pp_uint8*)buffer + y*pitch + x*BPP;
	pp_uint32 cchrDim = chr*charDim;

	if (x>= currentClipRect.x1 && x + charWidth < currentClipRect.x2 &&
		y>= currentClipRect.y1 && y + charHeight < currentClipRect.y2)
	{
		
		pp_uint32 yChr = cchrDim;
		for (pp_uint32 i = 0; i < (unsigned)charHeight; i++)
		{
			pp_uint32 xChr = yChr;
			for (pp_uint32 j = 0; j < (unsigned)charWidth; j++)
			{
				if ((fontbuffer[xChr>>3]>>(xChr&7)&1))
				{
					buff[0] = b;
					buff[1] = g;
					buff[2] = r;
				}
				buff+=BPP;
				xChr++;
			}
			buff+=pitch-(charWidth*BPP);
			yChr+=charWidth;
		}
	}
	else
	{
		pp_uint32 yChr = cchrDim;
		for (pp_uint32 i = 0; i < (unsigned)charHeight; i++)
		{
			pp_uint32 xChr = yChr;
			for (pp_uint32 j = 0; j < (unsigned)charWidth; j++)
			{
				
				if (y+(signed)i >= currentClipRect.y1 && y+(signed)i < currentClipRect.y2 &&
					x+(signed)j >= currentClipRect.x1 && x+(signed)j < currentClipRect.x2 &&
					(fontbuffer[xChr>>3]>>(xChr&7)&1))
				{
					
					pp_uint8* buff = (pp_uint8*)buffer+((y+i)*pitch+(x+j)*BPP);
					
					buff[0] = b;
					buff[1] = g;
					buff[2] = r;
				}
				xChr++;

			}
			yChr+=charWidth;
		}

	}

	if (underlined)
		drawHLine(x, x+charWidth, y+charHeight);

}

void PPGraphics_BGR24_SLOW::drawString(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
{
	if (currentFont == NULL)
		return;

	pp_int32 charWidth = (signed)currentFont->getCharWidth();
	pp_int32 charHeight = (signed)currentFont->getCharHeight();

	pp_int32 sx = x;

    while (*str) 
	{
		switch (*str)
		{
			case '\xf4':
				setPixel(x+(charWidth>>1), y+(charHeight>>1));
				break;
			case '\n':
				y+=charHeight;
				x=sx-charWidth;
				break;
			default:
				drawChar(*str, x, y, underlined);
		}
        x += charWidth;
        str++;
    }
}

void PPGraphics_BGR24_SLOW::drawStringVertical(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
{
	if (currentFont == NULL)
		return;

	pp_int32 charWidth = (signed)currentFont->getCharWidth();
	pp_int32 charHeight = (signed)currentFont->getCharHeight();

    while (*str) 
	{
		switch (*str)
		{
			case '\xf4':
				setPixel(x+(charWidth>>1), y+(charHeight>>1));
				break;
			default:
				drawChar(*str, x, y, underlined);
		}
        y += charHeight;
        str++;
    }
}
