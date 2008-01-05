/*
 *  SectionUpperLeft.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.04.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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
	SectionUpperLeft(Tracker& tracker);
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

