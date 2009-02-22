/*
 *  fx/TwisterFX.h
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
 *  TwisterFX.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 26.11.05.
 *
 */

#ifndef TWISTERFX__H
#define TWISTERFX__H

#include "BasicTypes.h"
#include "FXAbstract.h"

class FXInterface;

class TwisterFX : public FXAbstract
{
private:
	FXInterface* fx;
	pp_uint16* vscreen;

	pp_uint32 realw, realh;

public:
	TwisterFX(pp_int32 w, pp_int32 h);
	~TwisterFX();
	
	virtual void update(pp_uint32 syncFrac);
	
	virtual void render(pp_uint8* buffer);
};

#endif
