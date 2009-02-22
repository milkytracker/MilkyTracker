/*
 *  tracker/SectionTranspose.h
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
 *  SectionTranspose.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.04.05.
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
	
private:
	void handleTransposeSong();
	void transposeSong();

	// Responder should be friend
	friend class DialogResponderTranspose;	

	friend class Tracker;
};

#endif

