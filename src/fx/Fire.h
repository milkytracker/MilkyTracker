/*
 *  fx/Fire.h
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

#ifndef FIRE__H
#define FIRE__H

#include "BasicTypes.h"
#include "FXAbstract.h"

class Fire : public FXAbstract
{
private:
	pp_uint16* workbuffer;
	pp_uint8 colorLUT[256][3];

	void buildColorLUT();

public:
	Fire(pp_int32 w, pp_int32 h);
	virtual ~Fire();

	virtual void render(pp_uint8* buffer);
};

#endif
