/*
 *  ppui/Graphics_OGL.h
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
 
#ifndef __GRAPHICSOGL_H__
#define __GRAPHICSOGL_H__

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
 
class PPGraphics_OGL : public PPGraphicsAbstract 
{ 
public: 
	PPGraphics_OGL(pp_int32 w, pp_int32 h); 
	virtual void setPixel(pp_int32 x, pp_int32 y); 
	virtual void setPixel(pp_int32 x, pp_int32 y, const PPColor& color); 
	virtual void fill(PPRect r); 
	virtual void fill(); 
	virtual void drawHLine(pp_int32 x1, pp_int32 x2, pp_int32 y); 
	virtual void drawVLine(pp_int32 y1, pp_int32 y2, pp_int32 x); 
	virtual void drawLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2); 
	virtual void drawAntialiasedLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2); 
	virtual void blit(const pp_uint8* src, const PPPoint& p, const PPSize& size, pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity = 256); 
	virtual void drawChar(pp_uint8 chr, pp_int32 x, pp_int32 y, bool underlined = false); 
	virtual void drawString(const char* str, pp_int32 x, pp_int32 y, bool underlined = false); 
	virtual void drawStringVertical(const char* str, pp_int32 x, pp_int32 y, bool underlined = false); 
	virtual void fillVerticalShaded(PPRect r, const PPColor& colSrc, const PPColor& colDst, bool invertShading);
	virtual void setFont(PPFont* font);

protected:
	virtual void validateRect();
	
private:
	struct FontCacheEntry
	{
		PPFont* font;
		pp_uint8* oldFontBits;
		pp_uint8* oglBitmapData;
		GLuint listOffset;

		pp_uint32 newWidth;
		pp_uint32 newHeight;
		
		FontCacheEntry() :
			font(NULL),
			oldFontBits(NULL),
			oglBitmapData(NULL),
			newWidth(0),
			newHeight(0)
		{
		}
		
		~FontCacheEntry();
		
		void createFromFont(PPFont* font);
	};
	
	FontCacheEntry fontCache[4];	
	FontCacheEntry* fontCacheEntry;
}; 
 
#endif
