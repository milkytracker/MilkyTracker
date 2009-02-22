/*
 *  fx/ParticleFX.h
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

#ifndef PARTICLEFX__H
#define PARTICLEFX__H

#include "FXInterface.h"
#include "Math3d.h"

class ParticleFX : public FXInterface
{
protected:
	struct Particle 
	{
		VectorFP		pos;
		VectorFP		direction;
		int				size;
		int				grow;
		unsigned short*	texture;
		int				textureWidth, textureHeight;
	};

	struct Sprite
	{
		int				tx,ty;
		int				xsize,ysize;
		int				xres,yres;
		int				xoffset,yoffset;
		unsigned short	zval;
		int				flags;
		int				shade;
		unsigned int	texturexres;
		unsigned short	*texture;
	};

	int				FOV;
	int				zOffset;

	Particle*		particles;
	unsigned int	numParticles;

	MatrixFP		rotMatrix;

public:
	ParticleFX(int width, int height, int numParticles, int FOV = -1);
	virtual ~ParticleFX();

	virtual void render(unsigned short* vscreen, unsigned int pitch);
	virtual void update(float syncFrac) = 0;

	void setFOV(int FOV) { this->FOV = FOV; }
	void setZOffset(int zOffset) { this->zOffset = zOffset; }
	void setMatrix(MatrixFP& matrix) { rotMatrix = matrix; }
	MatrixFP getMatrix() { return rotMatrix; }

	static void	drawSprite(unsigned short *buffer, int XMAX, int YMAX, int PITCH, Sprite* spr);

};

#endif
