/*
 *  fx/ParticleEmitter.h
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

#ifndef PARTICLEEMITTER__H
#define PARTICLEEMITTER__H

#include "ParticleFX.h"
#include "Math3d.h"

class ParticleEmitter : public ParticleFX
{
private:
	unsigned short* flareTexture[3];
	unsigned short* temp;
	unsigned short*	buffer;

	unsigned int numEmitters;
	unsigned short* emitterTexture[6];
	Particle* emitters;
	int	currentParticleIndex;

	int maxParticles;

public:
	ParticleEmitter(int width, int height, int FOV, int numEmitters, int maxParticles);
	virtual ~ParticleEmitter();

	void setEmitter(unsigned int index, VectorFP& pos) { if (index < numEmitters) emitters[index].pos = pos; }

	virtual void update(float syncFrac);
	virtual void render(unsigned short* vscreen, unsigned int pitch);

};

#endif
