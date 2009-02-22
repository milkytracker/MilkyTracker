/*
 *  tracker/SectionOptimize.h
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
 *  SectionOptimize.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 17.11.05.
 *
 */

#ifndef SECTIONOPTIMIZE__H
#define SECTIONOPTIMIZE__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionUpperLeft.h"
#include "PatternEditorTools.h"

class PPControl;
class Tracker;
class DialogListBox;

class SectionOptimize : public SectionUpperLeft
{
private:
	void refresh();

	void optimize(bool evaluate = false);

	PatternEditorTools::OperandOptimizeParameters getOptimizeParameters();

	void zeroOperandsTrack();
	void zeroOperandsPattern();
	void zeroOperandsSong();
	void zeroOperandsBlock();

	void fillOperandsTrack();
	void fillOperandsPattern();
	void fillOperandsSong();
	void fillOperandsBlock();

	PatternEditorTools::RelocateParameters getRelocateParameters();

	void relocateCommandsTrack();
	void relocateCommandsPattern();
	void relocateCommandsSong();
	void relocateCommandsBlock();

	struct OptimizeSamplesResult
	{
		pp_int32 numConvertedSamples;
		pp_int32 numMinimizedSamples;
		
		OptimizeSamplesResult() :
			numConvertedSamples(0),
			numMinimizedSamples(0)
		{
		}
	};
	
	OptimizeSamplesResult optimizeSamples(bool convertTo8Bit, bool minimize, bool evaluate);

public:
	SectionOptimize(Tracker& tracker);
	virtual ~SectionOptimize();

	// Derived from SectionAbstract
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	virtual void init() { SectionUpperLeft::init(); }
	virtual void init(pp_int32 x, pp_int32 y);
	virtual void show(bool bShow) { SectionUpperLeft::show(bShow); }
	virtual void update(bool repaint = true);
	
	static pp_uint32 getNumFlagGroups();
	static pp_uint32 getDefaultFlags(pp_uint32 groupIndex);

	pp_uint32 getOptimizeCheckBoxFlags(pp_uint32 groupIndex);
	void setOptimizeCheckBoxFlags(pp_uint32 groupIndex, pp_uint32 flags);

	friend class Tracker;
};


#endif
