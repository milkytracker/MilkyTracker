/*
 *  ppui/Graphics_OGL.cpp
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

#ifdef __OPENGL__

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

static void setupOrtho(pp_uint32 width, pp_uint32 height)
{
	glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1.0, 1.0);
    glTranslatef(0.5f, 0.5f, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

PPGraphics_OGL::PPGraphics_OGL(pp_int32 w, pp_int32 h) :
	PPGraphicsAbstract(w, h),
	fontCacheEntry(NULL)
{
	setupOrtho(w, h);
	
	glDrawBuffer(GL_FRONT_AND_BACK);
	
	glEnable(GL_SCISSOR_TEST);
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);	
	
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
}

void PPGraphics_OGL::setPixel(pp_int32 x, pp_int32 y)
{
	glBegin(GL_POINTS);
	glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(x, y);
	glEnd();
}

void PPGraphics_OGL::setPixel(pp_int32 x, pp_int32 y, const PPColor& color)
{
	glBegin(GL_POINTS);
	glColor3ub(color.r, color.g, color.b);        
	glVertex2i(x, y);
	glEnd();
}

void PPGraphics_OGL::fill(PPRect rect)
{
	glBegin(GL_QUADS);
	glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(rect.x1, rect.y1);
	
	//glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(rect.x2, rect.y1);
	
	//glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(rect.x2, rect.y2);
	
	//glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(rect.x1, rect.y2);
    glEnd();
}

void PPGraphics_OGL::fill()
{
	fill(currentClipRect);
}

void PPGraphics_OGL::drawHLine(pp_int32 x1, pp_int32 x2, pp_int32 y)
{
	glBegin(GL_LINES);
	glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(x1, y);
	glVertex2i(x2, y);
	glEnd();
}

void PPGraphics_OGL::drawVLine(pp_int32 y1, pp_int32 y2, pp_int32 x)
{
	glBegin(GL_LINES);
	glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(x, y1);
	glVertex2i(x, y2);
	glEnd();
}

void PPGraphics_OGL::drawLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{
	glBegin(GL_LINES);
	glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
}

void PPGraphics_OGL::drawAntialiasedLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{
	glEnable(GL_LINE_SMOOTH);
	glBegin(GL_LINES);
	glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
	glDisable(GL_LINE_SMOOTH);
}

void PPGraphics_OGL::blit(const pp_uint8* src, const PPPoint& p, const PPSize& size, pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity/* = 256*/)
{
}


void PPGraphics_OGL::drawChar(pp_uint8 chr, pp_int32 x, pp_int32 y, bool underlined)
{
	if (!fontCacheEntry || !currentFont)
		return;
		
	glRasterPos2d(x, y);
	
	pp_uint32 offset = (pp_uint32)chr * fontCacheEntry->newWidth * fontCacheEntry->newHeight;
	
	glColor3ub(currentColor.r, currentColor.g, currentColor.b);
	glBitmap(currentFont->getCharWidth(), currentFont->getCharHeight(), 
			 0, currentFont->getCharHeight()-1,
			 0, 0, 
			 (GLubyte*)fontCacheEntry->oglBitmapData+offset);	
	
	if (underlined)
	{
		glBegin(GL_LINES);
		glVertex2i(x, y+currentFont->getCharHeight());
		glVertex2i(x+currentFont->getCharWidth(), y+currentFont->getCharHeight());
		glEnd();
	}
}

void PPGraphics_OGL::drawString(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
{
	if (!fontCacheEntry || !currentFont)
		return;

	glColor3ub(currentColor.r, currentColor.g, currentColor.b);

//	pp_int32 charWidth = (signed)currentFont->getCharWidth();
//	pp_int32 charHeight = (signed)currentFont->getCharHeight();
//	const GLubyte* data = (const GLubyte*)fontCacheEntry->oglBitmapData;
	
	glRasterPos2d(x, y);

	// Testing - No underline or \n support
	// Note: Even when using display lists, the opengl renderer is desperately slow
	//       (on my Nvidia at least), it's unusable at fullscreen resolutions. Perhaps
	//       using point-sprites would be quicker..  - Chris
	
	//glPushAttrib(GL_LIST_BIT);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, (GLubyte *) str);
	//glPopAttrib();

/*	pp_int32 sx = x;

	while (*str) 
	{
		switch (*str)
		{
			case '\xf4':
				glBegin(GL_POINTS);
				glVertex2i(x+(charWidth>>1), y+(charHeight>>1));
				glEnd();
				break;
			case '\n':
				y+=charHeight;
				x=sx-charWidth;
				glRasterPos2d(x, y);
				break;
			default:
			{
				pp_uint32 offset = ((pp_uint8)*str) * size;
				//glRasterPos2d(x, y);
				glBitmap(charWidth, charHeight, 0, charHeight-1, charWidth, 0, data+offset);	
				if (underlined)
				{
					glBegin(GL_LINES);
					glVertex2i(x, y+charHeight);
					glVertex2i(x+charWidth, y+charHeight);
					glEnd();
				}
			}
		}
        x += charWidth;
        str++;
    }*/
}

void PPGraphics_OGL::drawStringVertical(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
{
	if (!fontCacheEntry || !currentFont)
		return;

	glColor3ub(currentColor.r, currentColor.g, currentColor.b);

	pp_int32 charWidth = (signed)currentFont->getCharWidth();
	pp_int32 charHeight = (signed)currentFont->getCharHeight();
	const GLubyte* data = (const GLubyte*)fontCacheEntry->oglBitmapData;
	const pp_uint32 size = fontCacheEntry->newWidth * fontCacheEntry->newHeight;

    while (*str) 
	{
		switch (*str)
		{
			case '\xf4':
				glBegin(GL_POINTS);
				glVertex2i(x+(charWidth>>1), y+(charHeight>>1));
				glEnd();
				break;
			default:
			{
				pp_uint32 offset = ((pp_uint8)*str) * size;
				glRasterPos2d(x, y);
				glBitmap(charWidth, charHeight, 0, charHeight-1, 0, 0, data+offset);	
				if (underlined)
				{
					glBegin(GL_LINES);
					glVertex2i(x, y+charHeight);
					glVertex2i(x+charWidth, y+charHeight);
					glEnd();
				}
			}
		}
        y += charHeight;
        str++;
    }
}

void PPGraphics_OGL::fillVerticalShaded(PPRect r, const PPColor& colSrc, const PPColor& colDst, bool invertShading)
{
	if (invertShading)
	{
		glBegin(GL_QUADS);
		glColor3ub(colDst.r, colDst.g, colDst.b);        
		glVertex2i(r.x1, r.y1);
		
		//glColor3ub(colSrc.r, colSrc.g, colSrc.b);        
		glVertex2i(r.x2, r.y1);
		
		glColor3ub(colSrc.r, colSrc.g, colSrc.b);        
		glVertex2i(r.x2, r.y2);
		
		//glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
		glVertex2i(r.x1, r.y2);
		glEnd();
	}
	else
	{
		glBegin(GL_QUADS);
		glColor3ub(colSrc.r, colSrc.g, colSrc.b);        
		glVertex2i(r.x1, r.y1);
		
		//glColor3ub(colSrc.r, colSrc.g, colSrc.b);        
		glVertex2i(r.x2, r.y1);
		
		glColor3ub(colDst.r, colDst.g, colDst.b);        
		glVertex2i(r.x2, r.y2);
		
		//glColor3ub(currentColor.r, currentColor.g, currentColor.b);        
		glVertex2i(r.x1, r.y2);
		glEnd();
	}
}

void PPGraphics_OGL::validateRect()
{
	PPGraphicsAbstract::validateRect();

	glScissor(currentClipRect.x1, 
			  this->height-currentClipRect.y1-(currentClipRect.y2-currentClipRect.y1), 
			  currentClipRect.x2-currentClipRect.x1,
			  currentClipRect.y2-currentClipRect.y1);
}

void PPGraphics_OGL::setFont(PPFont* font)
{
	PPGraphicsAbstract::setFont(font);
	
	pp_int32 charWidth = (signed)font->getCharWidth();
	pp_int32 charHeight = (signed)font->getCharHeight();
	
	
	bool found = false;
	for (pp_int32 i = 0; i < sizeof(fontCache) / sizeof(FontCacheEntry); i++)
	{
		if (fontCache[i].font && fontCache[i].oldFontBits == font->fontBits)
		{
			found = true;
			fontCacheEntry = &fontCache[i];
			glListBase(fontCacheEntry->listOffset);
			break;
		}
	}

	if (!found)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_LSB_FIRST, true);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, ((font->getCharWidth()+7) / 8) * 8);

		pp_int32 slot = 0;
		for (pp_int32 i = 0; i < sizeof(fontCache) / sizeof(FontCacheEntry); i++)
		{
			if (!fontCache[i].font)
			{
				slot = i;
				break;
			}
		}
		
		fontCache[slot].createFromFont(font);
		fontCacheEntry = &fontCache[slot];
		
		fontCacheEntry->listOffset = glGenLists(256);
		glListBase(fontCacheEntry->listOffset);
		const GLubyte* data = (const GLubyte*)fontCacheEntry->oglBitmapData;
		const pp_uint32 size = fontCacheEntry->newWidth * fontCacheEntry->newHeight;
		

		for(pp_uint32 i = 0; i < 256; i++)
		{
			glNewList(fontCacheEntry->listOffset + i, GL_COMPILE);
			glBitmap(charWidth, charHeight, 0, charHeight-1, charWidth, 0, data+i*size);
			glEndList();
		}
	}

}

void PPGraphics_OGL::FontCacheEntry::createFromFont(PPFont* font)
{
	this->font = font;
	this->oldFontBits = font->fontBits;

	newWidth = (font->getCharWidth() + 7) / 8;
	newHeight = font->getCharHeight();
	
	pp_uint32 size = newWidth*newHeight*256;
	
	delete[] oglBitmapData;
	oglBitmapData = new pp_uint8[size];
	memset(oglBitmapData, 0, size);
	
	Bitstream stream(oglBitmapData, size);
	
	pp_uint32 dst = 0;
	for (pp_uint32 i = 0; i < 256; i++)
	{
		for (pp_uint32 y = 0; y < font->getCharHeight(); y++)
		{			
			if ((dst & 7) != 0)
				dst = ((dst+7)/8)*8;
			for (pp_uint32 x = 0; x < font->getCharWidth(); x++)
			{
				stream.write(dst, font->getPixelBit(i, x, font->getCharHeight() - 1 - y));
				dst++;
			}
		}
	}
}

PPGraphics_OGL::FontCacheEntry::~FontCacheEntry()
{
	delete[] oglBitmapData;
}

#endif