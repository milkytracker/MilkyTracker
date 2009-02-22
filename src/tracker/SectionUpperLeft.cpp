/*
 *  tracker/SectionUpperLeft.cpp
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
 *  SectionUpperLeft.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Wed May 04 2005.
 *
 */

#include "SectionUpperLeft.h"
#include "Tracker.h"
#include "PatternEditorControl.h"
#include "Container.h"

SectionUpperLeft::SectionUpperLeft(Tracker& theTracker, PPDialogBase* dialog/* = NULL*/, DialogResponder* responder/* = NULL*/) :
	SectionAbstract(theTracker, dialog, responder),
	sectionContainer(NULL)
{
}

SectionUpperLeft::~SectionUpperLeft()
{
}

void SectionUpperLeft::showSection(bool bShow)
{
	sectionContainer->show(bShow);		
}

void SectionUpperLeft::init()
{
#ifndef __LOWRES__
	init(0,0);
#else
	init(0,tracker.screen->getHeight()-UPPERLEFTSECTIONHEIGHT);
#endif
}

void SectionUpperLeft::show(bool bShow)
{
	SectionAbstract::show(bShow);

	PPScreen* screen = tracker.screen;
	
	if (!initialised)
	{
		init();
	}

	if (initialised)
	{
		if (bShow)
		{
			tracker.showMainMenu(false, false);
			update(false);
#ifdef __LOWRES__
			// resize pattern editor control
			PatternEditorControl* control = tracker.getPatternEditorControl();
			if (control)
			{
				control->setLocation(PPPoint(0, 0));

				pp_int32 offsety = tracker.inputContainerCurrent->isVisible() ? tracker.inputContainerCurrent->getSize().height : 0;

				control->setSize(PPSize(screen->getWidth(),screen->getHeight()-(UPPERLEFTSECTIONHEIGHT+offsety)));
			}
#endif
		}
		else
		{
			tracker.showMainMenu(true, false);
			tracker.rearrangePatternEditorControl();
		}
		
		showSection(bShow);
		
		screen->paint();
	}	
}
