/*
 *  tracker/SectionQuickOptions.h
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
 *  SectionQuickOptions.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.05.05.
 *
 */
#ifndef SECTIONQUICKOPTIONS__H
#define SECTIONQUICKOPTIONS__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionUpperLeft.h"

class SectionQuickOptions : public SectionUpperLeft
{
private:
	class PPCheckBox* checkBoxKeepSettings;
	class DialogPanning* dialogPanning;
	
	pp_uint8* oldPanning;

public:
	SectionQuickOptions(class Tracker& tracker);
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

