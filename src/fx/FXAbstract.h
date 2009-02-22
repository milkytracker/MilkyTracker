/*
 *  fx/FXAbstract.h
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
 *  FXAbstract.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.11.05.
 *
 */

#ifndef FXABSTRACT__H
#define FXABSTRACT__H

#include "BasicTypes.h"

class FXAbstract
{
protected:
	pp_uint32 width, height;
	
public:
	FXAbstract(pp_uint32 w, pp_uint32 h) :
		width(w), height(h)
	{
	}
	virtual ~FXAbstract() {}
	
	virtual void update(pp_uint32 syncFrac) { }
	
	virtual void render(pp_uint8* buffer) = 0;
};

#endif

