/*
 *  SectionAbstract.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.04.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SECTIONABSTRACT__H
#define SECTIONABSTRACT__H

#include "BasicTypes.h"
#include "Event.h"
#include "Screen.h"
#include "Tracker.h"

class PPControl;

class SectionAbstract : public EventListenerInterface
{
private:
	PPControl* lastFocusedControl;

protected:
	Tracker& tracker;
	bool initialised;
	
protected:
	virtual void showSection(bool bShow) = 0;
	
public:
	SectionAbstract(Tracker& theTracker) :
		lastFocusedControl(NULL),
		tracker(theTracker),
		initialised(false)
	{
	}
	
	virtual ~SectionAbstract() 
	{
	}

	// PPEvent listener
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event) = 0;
	
	virtual void init() = 0;
	
	virtual void init(pp_int32 x, pp_int32 y) = 0;
	virtual void show(bool bShow)
	{
#ifdef __LOWRES__
		if (bShow)
			lastFocusedControl = tracker.screen->getFocusedControl();
		else
			tracker.screen->setFocus(lastFocusedControl);
#endif
	}
	
	virtual void update(bool repaint = true) = 0;
	
	virtual void notifyInstrumentSelect(pp_int32 index) {}
	virtual void notifySampleSelect(pp_int32 index) {}
	virtual void notifyTabSwitch() {}
	
	friend class Tracker;

#ifdef __LOWRES__
private:
	PPSize oldInstrumentListSize;
	PPPoint oldInstrumentListLocation;
	PPSize oldSampleListSize;
	PPPoint oldSampleListLocation;
	PPSize oldInstrumentListContainerSize;
	PPPoint oldInstrumentListContainerLocation;
	PPPoint oldControlLocations[4];
	bool visibility[10];

protected:
	enum
	{
		REPLACEDINSTRUMENTLISTBOXESHEIGHT = 37+15
	};

	void replaceAndResizeInstrumentListContainer(pp_int32 listBoxContainerHeight);
	void replaceInstrumentListBoxes(bool b, pp_int32 listBoxContainerHeight = REPLACEDINSTRUMENTLISTBOXESHEIGHT);
#endif
};

#endif
