/*
 *  fx/PictureGlow.h
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
		resShift(0),
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
		delete[] glowBuffer1;
		delete[] glowBuffer2;
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
