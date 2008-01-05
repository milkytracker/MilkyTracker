#ifndef PICTUREGLOW__H
#define PICTUREGLOW__H

#include "FXInterface.h"
#include "Filter.h"

class PictureGlow : public FXInterface
{
private:
	unsigned short* pictureBuffer;

	int scale, resShift;

	unsigned short* glowBuffer1;
	unsigned short* glowBuffer2;

public:
	PictureGlow(int w, int h, int rShift, unsigned short* picture) :
		resShift(resShift),
		pictureBuffer(picture)
	{
		width = w;
		height = h;
		scale = 0;

		glowBuffer1 = new unsigned short[w*h];
		glowBuffer2 = new unsigned short[w*h];
	}

	~PictureGlow()
	{
		delete glowBuffer1;
		delete glowBuffer2;
	}

	void setScale(int scale) { this->scale = scale; }

	virtual void render(unsigned short* vscreen, unsigned int pitch)
	{
		int x,y;
		
		unsigned int* dstDW = (unsigned int*)vscreen;
		unsigned int* srcDW = (unsigned int*)pictureBuffer;

		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width>>1; x++)
			{
				*dstDW = *srcDW;
				srcDW++; dstDW++;
			}
			dstDW+=(pitch-width)>>1;
		}

		Filter::glow(vscreen, width, height, pitch, glowBuffer1, glowBuffer2, resShift, scale);
	}

	virtual void update(float syncFrac) { scale-=(int)(syncFrac*1024.0f); if (scale < 0) scale = 0; }


};

#endif
