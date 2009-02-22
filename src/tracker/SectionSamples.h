/*
 *  tracker/SectionSamples.h
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
 *  SectionSamples.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 15.04.05.
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

	pp_int32 getCurrentSamplePlayNote() const { return currentSamplePlayNote; }

	bool isVisible() const { return visible; }

	void setOffsetText(pp_uint32 ID, pp_uint32 offset);

private:
	void handleClearSample();
	void handleCropSample();
	void handleMinimizeSample();
	void handleConvertSampleResolution();

	// Responder should be friend
	friend class DialogResponderSamples;	

	friend class Tracker;
};

#endif
