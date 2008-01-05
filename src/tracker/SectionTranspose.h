/*
 *  SectionTranspose.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.04.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SECTIONTRANSPOSE__H
#define SECTIONTRANSPOSE__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionUpperLeft.h"
#include "PatternEditorTools.h"

class PPControl;
class Tracker;

class SectionTranspose : public SectionUpperLeft
{
private:
	pp_int32 currentInstrument;
	pp_int32 currentInstrumentRangeStart;
	pp_int32 currentInstrumentRangeEnd;

	pp_int32 currentNote;
	pp_int32 currentNoteRangeStart;
	pp_int32 currentNoteRangeEnd;
	
	pp_int32 currentTransposeAmount;
	
	PatternEditorTools::TransposeParameters tp;

public:
	SectionTranspose(Tracker& tracker);
	virtual ~SectionTranspose();

	void setCurrentInstrument(pp_int32 instrument, bool redraw = true);
	
	const PatternEditorTools::TransposeParameters& getTransposeParameters() { return tp; }
	
	// Derived from SectionAbstract
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	virtual void init() { SectionUpperLeft::init(); }
	virtual void init(pp_int32 x, pp_int32 y);
	virtual void show(bool bShow);
	virtual void update(bool repaint = true);
	
	friend class Tracker;
};

#endif

