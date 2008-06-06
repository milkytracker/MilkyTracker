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
    glTranslatef(.5, .5, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

PPGraphics_OGL::PPGraphics_OGL(pp_int32 w, pp_int32 h) :
	PPGraphicsAbstract(w, h)
{
	setupOrtho(w, h);
	
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
	glVertex2i(x, y+1);
	glEnd();
}

void PPGraphics_OGL::setPixel(pp_int32 x, pp_int32 y, const PPColor& color)
{
	glBegin(GL_POINTS);
	glColor3ub(color.r, color.g, color.b);        
	glVertex2i(x, y+1);
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
