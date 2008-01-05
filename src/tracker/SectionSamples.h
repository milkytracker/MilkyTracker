/*
 *  SectionSamples.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 15.04.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SECTIONSAMPLES__H
#define SECTIONSAMPLES__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionAbstract.h"

class PPControl;
class SampleEditorControl;

class SectionSamples : public SectionAbstract
{
private:
	PPContainer* containerEntire;

	bool visible;
	SampleEditorControl* sampleEditorControl;
	pp_int32 currentSamplePlayNote;

	PPSize oldInstrumentListSize;
	PPPoint oldInstrumentListLocation;
	PPSize oldSampleListSize;
	PPPoint oldSampleListLocation;
	PPSize oldInstrumentListContainerSize;
	PPPoint oldInstrumentListContainerLocation;
	PPPoint p[4];
	
	bool showRangeOffsets;
	pp_uint32 offsetFormat;
	
protected:
	virtual void showSection(bool bShow);

public:
	SectionSamples(Tracker& tracker);
	virtual ~SectionSamples();

	// PPEvent listener
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual void init();

	virtual void init(pp_int32 x, pp_int32 y);

	void realign();	
	
	virtual void show(bool bShow);
	virtual void update(bool repaint = true);

	virtual void notifyTabSwitch();
	
	void refresh(bool repaint = true);
	void realUpdate(bool repaint, bool force, bool reAttach);
	void updateSampleWindow(bool repaint = true);
	void updateAfterLoad();
	
	SampleEditorControl* getSampleEditorControl(bool forceAttach = true);
	
	void resetSampleEditor();
	
	bool isEmptySample();
	
	void setOffsetFormat(pp_uint32 offsetFormat);
	pp_uint32 getOffsetFormat() const { return offsetFormat; }
	void toggleOffsetFormat();

	bool isVisible() const { return visible; }

	void setOffsetText(pp_uint32 ID, pp_uint32 offset);

	friend class Tracker;
};

#endif
