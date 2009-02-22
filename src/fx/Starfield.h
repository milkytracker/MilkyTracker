/*
 *  fx/Starfield.h
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
 *  Fire.h
 *  MyFirstCarbonProject
 *
 *  Created by Peter Barth on Wed Feb 20 2002.
 *
 */

#ifndef STARFIELD__H
#define STARFIELD__H

#include "BasicTypes.h"
#include "FXAbstract.h"
#include "Math3d.h"

class Starfield : public FXAbstract
{
private:
	VectorFP* stars;

	const pp_int32 numStars;

public:
	Starfield(pp_int32 w, pp_int32 h);
	virtual ~Starfield();

	virtual void render(pp_uint8* buffer);

	virtual void update(pp_uint32 syncFrac);
	
	void setPixel(pp_int32 x, pp_int32 y, const PPColor& color, pp_uint8* buffer);
};

#endif
