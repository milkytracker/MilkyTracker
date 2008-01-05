#include "Graphics.h"
#include "Font.h"
#include "fastfill.h"

#define BPP 4

PPGraphics_ARGB32::PPGraphics_ARGB32(pp_int32 w, pp_int32 h, pp_int32 p, void* buff) :
	PPGraphicsAbstract(w, h, p, buff)
{
}

void PPGraphics_ARGB32::setPixel(pp_int32 x, pp_int32 y)
{
	if (y >= currentClipRect.y1 && y < currentClipRect.y2 &&
		x >= currentClipRect.x1 && x < currentClipRect.x2)
	{

		pp_uint8* buff = (pp_uint8*)buffer+pitch*y+x*BPP;
		
		buff[1] = (pp_uint8)currentColor.r;
		buff[2] = (pp_uint8)currentColor.g;
		buff[3] = (pp_uint8)currentColor.b;		

	}	
}

void PPGraphics_ARGB32::setPixel(pp_int32 x, pp_int32 y, const PPColor& color)
{
	if (y >= currentClipRect.y1 && y < currentClipRect.y2 &&
		x >= currentClipRect.x1 && x < currentClipRect.x2)
	{
		pp_uint8* buff = (pp_uint8*)buffer+pitch*y+x*BPP;
		
		buff[1] = color.r;
		buff[2] = color.g;
		buff[3] = color.b;		
	}	
}

void PPGraphics_ARGB32::fill(PPRect rect)
{
	
	if (rect.y1 < currentClipRect.y1)
		rect.y1 = currentClipRect.y1;

	if (rect.x1 < currentClipRect.x1)
		rect.x1 = currentClipRect.x1;

	if (rect.y2 > currentClipRect.y2)
		rect.y2 = currentClipRect.y2;

	if (rect.x2 > currentClipRect.x2)
		rect.x2 = currentClipRect.x2;

	pp_int32 len = (rect.x2-rect.x1);

	if (len <= 0)
		return;

	pp_uint8 r = (pp_uint8)currentColor.r;
	pp_uint8 g = (pp_uint8)currentColor.g;
	pp_uint8 b = (pp_uint8)currentColor.b;

#ifdef __ppc__
	pp_uint32 rgb1 = 
		(((pp_uint32)r) << 16) +
		(((pp_uint32)g) << 8) +
		(((pp_uint32)b));	
#else
	pp_uint32 rgb1 = 
		(((pp_uint32)b) << 24) +
		(((pp_uint32)g) << 16) +
		(((pp_uint32)r) << 8);	
#endif	
	pp_int32 inc = (pitch-len*BPP) / BPP;

	pp_uint8* buff = (pp_uint8*)buffer+(pitch*rect.y1+rect.x1*BPP); 
	pp_uint32* ptr = reinterpret_cast<pp_uint32*>(buff);
				
	for (pp_int32 y = rect.y1; y < rect.y2; y++)
	{
		fill_dword(ptr, rgb1, len);
		ptr+=len;		
		ptr+=inc;
	}	

}

void PPGraphics_ARGB32::fill()
{
	fill(currentClipRect);
}

void PPGraphics_ARGB32::drawHLine(pp_int32 x1, pp_int32 x2, pp_int32 y)
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
		
	if (lx > currentClipRect.x2)
		return;
		
	if (rx < currentClipRect.x1)
		return;

	pp_int32 len = (rx-lx);

	if (len <= 0)
		return;

	pp_uint8* buff = (pp_uint8*)buffer+pitch*y+lx*4;

#ifdef __ppc__
	pp_uint32 rgb1 = (((pp_uint32)currentColor.r) << 16) +
					 (((pp_uint32)currentColor.g) << 8) +
					 (((pp_uint32)currentColor.b));
#else
	pp_uint32 rgb1 = (((pp_uint32)currentColor.b) << 24) +
					 (((pp_uint32)currentColor.g) << 16) +
					 (((pp_uint32)currentColor.r) << 8);	
#endif

	fill_dword(reinterpret_cast<pp_uint32*>(buff), rgb1, len);
}

void PPGraphics_ARGB32::drawVLine(pp_int32 y1, pp_int32 y2, pp_int32 x)
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
		
	pp_int32 len = (ry-ly);

	if (len <= 0)
		return;

	pp_uint8* buff = (pp_uint8*)buffer+pitch*(ly)+x*4;

#ifdef __ppc__
	pp_uint32 rgb1 = (((pp_uint32)currentColor.r) << 16) +
					 (((pp_uint32)currentColor.g) << 8) +
					 (((pp_uint32)currentColor.b));
#else
	pp_uint32 rgb1 = (((pp_uint32)currentColor.b) << 24) +
					 (((pp_uint32)currentColor.g) << 16) +
					 (((pp_uint32)currentColor.r) << 8);	
#endif

	fill_dword_vertical(reinterpret_cast<pp_uint32*>(buff), rgb1, len, pitch);
}

void PPGraphics_ARGB32::drawLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{
	__PPGRAPHICSLINETEMPLATE
}

void PPGraphics_ARGB32::drawAntialiasedLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2)
{	
	__PPGRAPHICSAALINETEMPLATE
}

void PPGraphics_ARGB32::blit(SimpleBitmap& bitmap, PPPoint p)
{
	pp_uint8* src = (pp_uint8*)bitmap.buffer;

	for (int y = 0; y < bitmap.height; y++)
		for (int x = 0; x < bitmap.width; x++)
		{
		
			if (src[0]||src[1]||src[2])
			{
				pp_uint8* buff = (pp_uint8*)buffer+((y+p.y)*pitch+(x+p.x)*BPP);
				buff[1] = src[0];
				buff[2] = src[1];
				buff[3] = src[2];
			}

			src+=3;

		}
}

void PPGraphics_ARGB32::blit(const pp_uint8* src, const PPPoint& p, const PPSize& size, pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity/* = 256*/)
{
	pp_int32 w = size.width;
	pp_int32 h = size.height;
	
	pp_uint8* dst = (pp_uint8*)buffer+(this->pitch*p.y+p.x*BPP); 
	
	const pp_uint32 bytesPerPixel = bpp;
	
	if (intensity != 256)
	{
		for (pp_int32 y = 0; y < h; y++)
		{
			for (pp_int32 x = 0; x < w; x++)
			{		
				dst[1] = (src[0]*intensity)>>8;
				dst[2] = (src[1]*intensity)>>8;
				dst[3] = (src[2]*intensity)>>8;
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
				dst[1] = src[0];
				dst[2] = src[1];
				dst[3] = src[2];
				dst+=BPP;
				src+=bytesPerPixel;
			}
			dst+=this->pitch - w*BPP;
			src+=pitch - w*bytesPerPixel;
		}
	}
}


void PPGraphics_ARGB32::drawChar(pp_uint8 chr, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
{

	if (currentFont == NULL)
		return;

	const pp_int32 charWidth = (signed)currentFont->getCharWidth();
	const pp_int32 charHeight = (signed)currentFont->getCharHeight(); 

	if (x + (signed)charWidth < currentClipRect.x1 ||
		x > currentClipRect.x2 ||
		y + (signed)charHeight < currentClipRect.y1 ||
		y > currentClipRect.y2)
		return;

	const pp_int32 charDim = currentFont->charDim;

	pp_uint8 r = (pp_uint8)currentColor.r;
	pp_uint8 g = (pp_uint8)currentColor.g;
	pp_uint8 b = (pp_uint8)currentColor.b;

	Bitstream* bitstream = currentFont->bitstream;
	pp_uint8* fontbuffer = bitstream->buffer;

#ifdef __ppc__
	pp_uint32 rgb1 = (((pp_uint32)r) << 16) +
					 (((pp_uint32)g) << 8) +
					 (((pp_uint32)b));
#else
	pp_uint32 rgb1 = (((pp_uint32)b) << 24) +
					 (((pp_uint32)g) << 16) +
					 (((pp_uint32)r) << 8);	
#endif

	const pp_uint32 cchrDim = chr*charDim;

	if (x>= currentClipRect.x1 && x + charWidth < currentClipRect.x2 &&
		y>= currentClipRect.y1 && y + charHeight < currentClipRect.y2)
	{
		pp_uint8* buff = (pp_uint8*)buffer + y*pitch + x*BPP;
		
		pp_uint32 yChr = cchrDim;
		for (pp_uint32 i = 0; i < (unsigned)charHeight; i++)
		{
			pp_uint32 xChr = yChr;
			for (pp_uint32 j = 0; j < (unsigned)charWidth; j++)
			{
				if ((fontbuffer[xChr>>3]>>(xChr&7)&1))
				{
					*reinterpret_cast<pp_uint32*>(buff) = rgb1;
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
		for (pp_int32 i = 0; i < charHeight; i++)
		{
			pp_uint32 xChr = yChr;
			for (pp_int32 j = 0; j < charWidth; j++)
			{
				
				if (y+(signed)i >= currentClipRect.y1 && y+(signed)i < currentClipRect.y2 &&
					x+(signed)j >= currentClipRect.x1 && x+(signed)j < currentClipRect.x2 &&
					(fontbuffer[xChr>>3]>>(xChr&7)&1))
				{
					
					pp_uint8* buff = (pp_uint8*)buffer+((y+i)*pitch+(x+j)*BPP);
					
					*reinterpret_cast<pp_uint32*>(buff) = rgb1;					
				}
				xChr++;

			}
			yChr+=charWidth;
		}

	}

	if (underlined)
		drawHLine(x, x+charWidth, y+charHeight);
}

void PPGraphics_ARGB32::drawString(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
{
	if (currentFont == NULL)
		return;

	const pp_int32 charWidth = (signed)currentFont->getCharWidth();
	const pp_int32 charHeight = (signed)currentFont->getCharHeight();

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

void PPGraphics_ARGB32::drawStringVertical(const char* str, pp_int32 x, pp_int32 y, bool underlined/* = false*/)
{
	if (currentFont == NULL)
		return;

	const pp_int32 charWidth = (signed)currentFont->getCharWidth();
	const pp_int32 charHeight = (signed)currentFont->getCharHeight();

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

