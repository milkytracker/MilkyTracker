/*
 *  SectionAbout.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 17.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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
