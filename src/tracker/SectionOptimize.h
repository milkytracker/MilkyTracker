/*
 *  SectionOptimize.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 17.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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
class RespondMessageBoxListBox;

class SectionOptimize : public SectionUpperLeft
{
private:
	RespondMessageBoxListBox* respondMessageBox;
	
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
