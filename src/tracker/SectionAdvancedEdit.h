/*
 *  SectionAdvancedEdit.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.05.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */
#ifndef SECTIONADVANCEDEDIT__H
#define SECTIONADVANCEDEDIT__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionUpperLeft.h"

class PPControl;
class Tracker;
class PPCheckBox;

class SectionAdvancedEdit : public SectionUpperLeft
{
private:
	pp_int32 splitTrackNumSubsequentChannels;
	PPCheckBox* checkBoxSplitTrack;
	PPCheckBox* checkBoxSplitTrackNoteOff;

public:
	SectionAdvancedEdit(Tracker& tracker);
	virtual ~SectionAdvancedEdit();

	// Derived from SectionAbstract
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	virtual void init() { SectionUpperLeft::init(); }
	virtual void init(pp_int32 x, pp_int32 y);
	virtual void show(bool bShow) { SectionUpperLeft::show(bShow); }
	virtual void update(bool repaint = true);
	
	friend class Tracker;
};

#endif

