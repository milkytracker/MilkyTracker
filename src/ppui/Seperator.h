/*
 *  ppui/Seperator.h
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
 *  Seperator.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.03.05.
 *
 */
#ifndef SEPERATOR__H
#define SEPERATOR__H

#include "BasicTypes.h"
#include "Control.h"

class PPSeperator : public PPControl
{
private:
	bool horizontal;
	
	const PPColor* color;	
	
public:
	PPSeperator(pp_int32 id, PPScreen* parentScreen, 
				const PPPoint& location, pp_uint32 size, 
				const PPColor& theColor, bool horizontal = true);
	
	void setColor(const PPColor& color) { this->color = &color; }

	virtual void paint(PPGraphicsAbstract* graphics);
};

#endif
