/*
 *  fx/TexturedGrid.h
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

#ifndef TEXTUREDGRID__H
#define TEXTUREDGRID__H

#include "FXInterface.h"

struct VectorFP;

class TexturedGrid : public FXInterface
{
protected:
	int				fov;
	int				gridshift;
	int				gridsize;		// must be 1 << GRIDSHIFT

	VectorFP*		grid;
	int				gridWidth, gridHeight;

	unsigned short* texture;
	unsigned short* buffer;

	int				sintab[1024];

public:
	TexturedGrid(int width, int height, int gridshift);
	virtual ~TexturedGrid();

	void setTexture(unsigned short* texture) { this->texture = texture; }

	// from FXInterface
	virtual void render(unsigned short* vscreen, unsigned int pitch);
	virtual void update(float syncFrac);

};

#endif
