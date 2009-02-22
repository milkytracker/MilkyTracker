/*
 *  fx/ParticleBlobs.h
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

#ifndef PARTICLEBLOBS__H
#define PARTICLEBLOBS__H

#include "ParticleFX.h"
#include "Math3d.h"


class ParticleBlobs : public ParticleFX
{
	unsigned short* flareTexture[3];
	unsigned short* temp;
	unsigned short*	buffer;


public:
	ParticleBlobs(int width, int height);
	virtual ~ParticleBlobs();

	virtual void render(unsigned short* vscreen, unsigned int pitch);
	virtual void update(float syncFrac);

};

#endif
