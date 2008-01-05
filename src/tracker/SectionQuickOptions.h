/*
 *  SectionQuickOptions.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.05.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */
#ifndef SECTIONQUICKOPTIONS__H
#define SECTIONQUICKOPTIONS__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionUpperLeft.h"

class PPControl;
class Tracker;
class PPCheckBox;
class PanningSettingsContainer;

class SectionQuickOptions : public SectionUpperLeft
{
private:
	PPCheckBox* checkBoxKeepSettings;
	PanningSettingsContainer* panningSettingsContainer;
	
	pp_uint8* oldPanning;

public:
	SectionQuickOptions(Tracker& tracker);
	virtual ~SectionQuickOptions();

	// Derived from SectionAbstract
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	virtual void init() { SectionUpperLeft::init(); }
	virtual void init(pp_int32 x, pp_int32 y);
	virtual void show(bool bShow);
	virtual void update(bool repaint = true);

	virtual void notifyTabSwitch();

	bool setKeepSettings(bool b);
	bool keepSettings();

private:
	void updateControlStates();

	void saveOldPanning();
	void restoreOldPanning();
	
	friend class Tracker;
};

#endif

