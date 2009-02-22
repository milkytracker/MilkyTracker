/*
 *  tracker/SectionAbstract.h
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
 *  SectionAbstract.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.04.05.
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

	class PPDialogBase* dialog;
	class DialogResponder* responder;
	
protected:
	virtual void showSection(bool bShow) = 0;

	void showMessageBox(pp_uint32 id, const PPString& text, bool yesnocancel = false);
	
public:
	SectionAbstract(Tracker& theTracker, PPDialogBase* dialog = NULL, DialogResponder* responder = NULL) :
		lastFocusedControl(NULL),
		tracker(theTracker),
		initialised(false),
		dialog(dialog),
		responder(responder)
	{
	}
	
	virtual ~SectionAbstract();

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
