/////////////////////////////////////////////////////////////////
//
//	Graphics class
//
/////////////////////////////////////////////////////////////////
#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "GraphicsAbstract.h"

#define __EMPTY__

#define SUBCLASS_GRAPHICSABSTRACT(prologue, name, epilogue) \
class name : public PPGraphicsAbstract \
{ \
private: \
	prologue \
public: \
	name(pp_int32 w, pp_int32 h, pp_int32 p, void* buff); \
	virtual void setPixel(pp_int32 x, pp_int32 y); \
	virtual void setPixel(pp_int32 x, pp_int32 y, const PPColor& color); \
	virtual void fill(PPRect r); \
	virtual void fill(); \
	virtual void drawHLine(pp_int32 x1, pp_int32 x2, pp_int32 y); \
	virtual void drawVLine(pp_int32 y1, pp_int32 y2, pp_int32 x); \
	virtual void drawLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2); \
	virtual void drawAntialiasedLine(pp_int32 x1, pp_int32 y1, pp_int32 x2, pp_int32 y2); \
	virtual void blit(SimpleBitmap& bitmap, PPPoint p); \
	virtual void blit(const pp_uint8* src, const PPPoint& p, const PPSize& size, pp_uint32 pitch, pp_uint32 bpp, pp_int32 intensity = 256); \
	virtual void drawChar(pp_uint8 chr, pp_int32 x, pp_int32 y, bool underlined = false); \
	virtual void drawString(const char* str, pp_int32 x, pp_int32 y, bool underlined = false); \
	virtual void drawStringVertical(const char* str, pp_int32 x, pp_int32 y, bool underlined = false); \
	epilogue \
}; \

// trans is level of transparency - 0 = opaque, 255 = transparent
static inline void set_pixel_transp(PPGraphicsAbstract* g, pp_int32 x, pp_int32 y, PPColor* col, unsigned char trans)
{
	PPColor newColor;
	newColor.r = (col->r * (unsigned int)(255-trans))>>8;
	newColor.g = (col->g * (unsigned int)(255-trans))>>8;
	newColor.b = (col->b * (unsigned int)(255-trans))>>8;
	g->setPixel(x, y, newColor);
}

SUBCLASS_GRAPHICSABSTRACT(__EMPTY__,PPGraphics_BGR24,__EMPTY__)
SUBCLASS_GRAPHICSABSTRACT(__EMPTY__,PPGraphics_RGB24,__EMPTY__)
SUBCLASS_GRAPHICSABSTRACT(__EMPTY__,PPGraphics_ARGB32,__EMPTY__)
SUBCLASS_GRAPHICSABSTRACT(__EMPTY__,PPGraphics_BGRA32,__EMPTY__)
SUBCLASS_GRAPHICSABSTRACT(__EMPTY__,PPGraphics_RGBA32,__EMPTY__)
SUBCLASS_GRAPHICSABSTRACT(__EMPTY__,PPGraphics_16BIT,__EMPTY__)
SUBCLASS_GRAPHICSABSTRACT(__EMPTY__,PPGraphics_15BIT,__EMPTY__)
SUBCLASS_GRAPHICSABSTRACT(__EMPTY__,PPGraphics_BGR24_SLOW,__EMPTY__)

#define PROLOGUE \
	pp_uint32 bitPosR, bitPosG, bitPosB;
	
#define EPILOGUE \
	void setComponentBitpositions(pp_uint32 bitPosR, pp_uint32 bitPosG, pp_uint32 bitPosB) \
	{ \
		this->bitPosR = bitPosR; this->bitPosG = bitPosG; this->bitPosB = bitPosB; \
	}

SUBCLASS_GRAPHICSABSTRACT(PROLOGUE,PPGraphics_24bpp_generic,EPILOGUE)
SUBCLASS_GRAPHICSABSTRACT(PROLOGUE,PPGraphics_32bpp_generic,EPILOGUE)

#undef EPILOGUE
#undef PROLOGUE

#undef SUBCLASS_GRAPHICSABSTRACT

#undef __EMPTY__

#endif
