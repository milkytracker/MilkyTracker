/*
 *  SectionUpperLeft.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Wed May 04 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "SectionUpperLeft.h"
#include "Tracker.h"
#include "PatternEditorControl.h"
#include "Container.h"

SectionUpperLeft::SectionUpperLeft(Tracker& theTracker) :
	SectionAbstract(theTracker),
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
