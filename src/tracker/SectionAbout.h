/*
 *  tracker/SectionAbout.h
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
 *  SectionAbout.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 17.11.05.
 *
 */

#ifndef SECTIONABOUT__H
#define SECTIONABOUT__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionUpperLeft.h"

class PPControl;
class Tracker;

class SectionAbout : public SectionUpperLeft
{
public:
	SectionAbout(Tracker& tracker);
	virtual ~SectionAbout();

	// Derived from SectionAbstract
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	virtual void init() { SectionUpperLeft::init(); }
	virtual void init(pp_int32 x, pp_int32 y);
	virtual void show(bool bShow) { SectionUpperLeft::show(bShow); }
	virtual void update(bool repaint = true);
	
	friend class Tracker;
};


#endif
