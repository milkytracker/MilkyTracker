/*
 *  tracker/SectionUpperLeft.h
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
 *  SectionUpperLeft.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.04.05.
 *
 */

#ifndef SECTIONUPPERLEFT__H
#define SECTIONUPPERLEFT__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionAbstract.h"

class PPControl;
class Tracker;

class SectionUpperLeft : public SectionAbstract
{
protected:
	enum Constants
	{
		UPPERLEFTSECTIONHEIGHT = 118
	};

	PPControl* sectionContainer;

	virtual void showSection(bool bShow);
	
public:
	SectionUpperLeft(Tracker& tracker, PPDialogBase* dialog = NULL, DialogResponder* responder = NULL);
	virtual ~SectionUpperLeft();

	// Derived from SectionAbstract
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event) = 0;
	
	virtual void init();
	
	virtual void init(pp_int32 x, pp_int32 y) = 0;
	
	virtual void show(bool bShow);

	virtual void update(bool repaint = true) = 0;
	
	PPControl* getSectionContainer() { return sectionContainer; }
};

#endif

