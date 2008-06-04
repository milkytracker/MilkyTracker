/*
 *  ppui/Graphics_OGL.cpp
 *
 *  Copyright 2008 Peter Barth
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

PPGraphics_OGL::PPGraphics_OGL(pp_int32 w, pp_int32 h, pp_int32 p, void* buff) :
	PPGraphicsAbstract(w, h)
{
}

void PPGraphics_OGL::setPixel(pp_int32 x, pp_int32 y)
{
}

void PPGraphics_OGL::setPixel(pp_int32 x, pp_int32 y, const PPColor& color)
{
}

void PPGraphics_OGL::fill(PPRect rect)
{
}

void PPGraphics_OGL::fill()
{
	fill(currentClipRect);
}

void PPGraphics_OGL::drawHLine(pp_int32 x1, pp_int32 x2, pp_int32 y)
{
}

void PPGraphics_OGL::drawVLine(pp_int32 y1, pp_int32 y2, pp_int32 x)
{
}

void PPGraphics_OGL::drawLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{
}

void PPGraphics_OGL::drawAntialiasedLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{
}

void PPGraphics_OGL::blit(const pp_uint8* src, const PPPoint& p, const PPSize& size, pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity/* = 256*/)
{
}


void PPGraphics_OGL::drawChar(pp_uint8 chr, pp_int32 x, pp_int32 y, bool underlined)
{
}

void PPGraphics_OGL::drawString(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
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

void PPGraphics_OGL::drawStringVertical(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
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
