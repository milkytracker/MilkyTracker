/*
 *  tracker/SectionAdvancedEdit.h
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
 *  SectionAdvancedEdit.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.05.05.
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

