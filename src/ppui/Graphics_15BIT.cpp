/*
 *  ppui/Graphics_15BIT.cpp
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
#include "fastfill.h"

#define BPP 2

#define _16TO15BIT(col) \
	(((col) & 0x1f) + (((col)>>1)&0x3E0) + (((col)>>1)&0x7C00))

PPGraphics_15BIT::PPGraphics_15BIT(pp_int32 w, pp_int32 h, pp_int32 p, void* buff) :
	PPGraphicsFrameBuffer(w, h, p, buff)
{
}

void PPGraphics_15BIT::setPixel(pp_int32 x, pp_int32 y)
{
	if (y >= currentClipRect.y1 && y < currentClipRect.y2 &&
		x >= currentClipRect.x1 && x < currentClipRect.x2)
	{

		pp_uint16* buff = (pp_uint16*)buffer+(pitch>>1)*y+x;
		
		*buff = _16TO15BIT(color16);
	}	
}

void PPGraphics_15BIT::setPixel(pp_int32 x, pp_int32 y, const PPColor& color)
{
	if (y >= currentClipRect.y1 && y < currentClipRect.y2 &&
		x >= currentClipRect.x1 && x < currentClipRect.x2)
	{
		pp_uint16 col16 = (((pp_uint16)((color.r)>>3)<<11)+((pp_uint16)((color.g)>>2)<<5)+(pp_uint16)((color.b)>>3));
		pp_uint16* buff = (pp_uint16*)buffer+(pitch>>1)*y+x;
		*buff = _16TO15BIT(col16);
	}	
}

void PPGraphics_15BIT::fill(PPRect rect)
{
	
	if (rect.y1 < currentClipRect.y1)
		rect.y1 = currentClipRect.y1;

	if (rect.x1 < currentClipRect.x1)
		rect.x1 = currentClipRect.x1;

	if (rect.y2 > currentClipRect.y2)
		rect.y2 = currentClipRect.y2;

	if (rect.x2 > currentClipRect.x2)
		rect.x2 = currentClipRect.x2;

	pp_uint32 col32 = (((pp_uint32)_16TO15BIT(color16)) << 16) + _16TO15BIT(color16);

	const pp_int32 hPitch = pitch>>1;

	pp_uint16* dest = (pp_uint16*)buffer+hPitch*rect.y1+rect.x1;

	const pp_int32 width = rect.x2 - rect.x1;
	const pp_int32 height = rect.y2 - rect.y1; 
	
	for (pp_int32 y = 0; y < height; y++)
	{
		pp_uint16* buff = dest;
	
		pp_int32 len = width;

		if (reinterpret_cast<size_t> (buff) & 3)
		{
			*buff++ = _16TO15BIT(color16);
			len--;
		}

		pp_uint32* buff32 = (pp_uint32*)buff;

		fill_dword(buff32, col32, len>>1);
		
		if (len&1)
			*((pp_uint16*)(buff32+(len>>1))) = _16TO15BIT(color16);
		
		dest+=hPitch;
	}	

}

void PPGraphics_15BIT::fill()
{

	fill(currentClipRect);

}

void PPGraphics_15BIT::drawHLine(pp_int32 x1, pp_int32 x2, pp_int32 y)
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

	pp_uint32 col32 = (((pp_uint32)_16TO15BIT(color16)) << 16) + _16TO15BIT(color16);

	pp_uint16* buff = (pp_uint16*)buffer+(pitch>>1)*y+lx;

	pp_int32 len = rx-lx;
	
	if (len <= 0)
		return;

	if (reinterpret_cast<size_t> (buff) & 3)
	{
		*buff++ = _16TO15BIT(color16);
		len--;
	}
	
	pp_uint32* buff32 = (pp_uint32*)buff;
	
	fill_dword(buff32, col32, len>>1);
		
	if (len&1)
		*((pp_uint16*)(buff32+(len>>1))) = _16TO15BIT(color16);
}

void PPGraphics_15BIT::drawVLine(pp_int32 y1, pp_int32 y2, pp_int32 x)
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

	pp_uint16* buff = (pp_uint16*)buffer+(pitch>>1)*ly+x;

	for (pp_int32 y = ly; y < ry; y++)
	{
		*buff = _16TO15BIT(color16);
		buff+=pitch>>1;
	}

}

void PPGraphics_15BIT::drawLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{	
	__PPGRAPHICSLINETEMPLATE
}

void PPGraphics_15BIT::drawAntialiasedLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{
	__PPGRAPHICSAALINETEMPLATE
}

void PPGraphics_15BIT::blit(const pp_uint8* src, const PPPoint& p, const PPSize& size, pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity/* = 256*/)
{
	pp_int32 w = size.width;
	pp_int32 h = size.height;
	
	const pp_uint32 bytesPerPixel = bpp;
	
	if (intensity == 256)
	{
		pp_uint16* dst = (pp_uint16*)((pp_uint8*)buffer+(this->pitch*p.y+p.x*BPP)); 
		for (pp_int32 y = 0; y < h; y++)
		{
			for (pp_int32 x = 0; x < w; x++)
			{	
				*dst = (((pp_uint16)((src[0])>>3)<<10)+((pp_uint16)((src[1])>>3)<<5)+(pp_uint16)((src[2])>>3));
				dst++;
				src+=bytesPerPixel;
			}
			dst+=this->pitch/BPP - w;
			src+=pitch - w*bytesPerPixel;
		}
	}
	else
	{
		pp_uint16* dst = (pp_uint16*)((pp_uint8*)buffer+(this->pitch*p.y+p.x*BPP)); 
		for (pp_int32 y = 0; y < h; y++)
		{
			for (pp_int32 x = 0; x < w; x++)
			{	
				*dst = (((pp_uint16)((src[0]*intensity/256)>>3)<<10)+((pp_uint16)((src[1]*intensity/256)>>3)<<5)+(pp_uint16)((src[2]*intensity/256)>>3));
				dst++;
				src+=bytesPerPixel;
			}
			dst+=this->pitch/BPP - w;
			src+=pitch - w*bytesPerPixel;
		}
	}
}

void PPGraphics_15BIT::drawChar(pp_uint8 chr, pp_int32 x, pp_int32 y, bool underlined)
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
	Bitstream* bitstream = currentFont->bitstream;
	pp_uint8* fontbuffer = bitstream->buffer;

	pp_uint16* buff = (pp_uint16*)buffer+(pitch>>1)*y+x;	
	
	const pp_uint32 cchrDim = chr*charDim;
	const pp_uint32 incr = (pitch>>1)-(charWidth);

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
					*buff = _16TO15BIT(color16);
				}
				buff++;
				xChr++;
			}
			
			buff+=incr;
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
					
					pp_uint16* buff = (pp_uint16*)buffer+(pitch>>1)*(y+i)+(x+j);	
					*buff = _16TO15BIT(color16);
				}
				xChr++;

			}
			yChr+=charWidth;
		}

	}

	if (underlined)
		drawHLine(x, x+charWidth, y+charHeight);

}

void PPGraphics_15BIT::drawString(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
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

void PPGraphics_15BIT::drawStringVertical(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
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
