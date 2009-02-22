/*
 *  fx/TwisterFX.cpp
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

/*
 *  TwisterFX.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 26.11.05.
 *
 */

#include "TwisterFX.h"
//#include "Twister.h"
#include "ParticleBlobs.h"
#include "ParticleScene.h"
#include "TCBSplineTest.h"
#include "TexturedGrid.h"
#include "Twister.h"
#include "PictureGlow.h"

TwisterFX::TwisterFX(pp_int32 w, pp_int32 h) :
	FXAbstract(w,h)
{
	//twister = NULL;
	//vscreen = NULL;
	//fx = new Twister(w, h);

	realw = ((w >> 3) + 1) << 3;
	realh = ((h >> 3) + 1) << 3;
	
	fx = new TexturedGrid(realw, realh, 3);
	vscreen = new pp_uint16[realw * realh];
}

TwisterFX::~TwisterFX()
{
	delete fx;
	delete[] vscreen;
}

void TwisterFX::update(pp_uint32 syncFrac)
{
	fx->update(syncFrac*(1.0f/65536.0f));
}

void TwisterFX::render(pp_uint8* buffer)
{
	fx->render(vscreen, realw);
	
	pp_int32 w = width;
	pp_int32 h = height;
	
	pp_uint8* dst = buffer;
	pp_uint16* src = vscreen;
	
	for (pp_int32 y = 0; y < h; y++)
	{
		for (pp_int32 x = 0; x < w; x++)
		{
			*dst = (*src >> 11)<<3;
			*(dst+1) = (*src >> 5)<<2;
			*(dst+2) = (*src&0x1f)<<3;
			dst+=3;
			src++;
		}
		src+=realw - w;
	}
}
