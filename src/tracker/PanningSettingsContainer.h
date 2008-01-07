/*
 *  tracker/PanningSettingsContainer.h
 *
 *  Copyright 2008 Peter Barth
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
 *  PanningSettingsContainer.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.03.06.
 *
 */

#ifndef __PANNINGSETTINGSCONTAINER_H__
#define __PANNINGSETTINGSCONTAINER_H__

#include "BasicTypes.h"
#include "Event.h"

class PPScreen;
class PPContainer;

class PanningSettingsContainer : public EventListenerInterface
{
private:
	PPContainer* container;
	PPScreen* screen;
	EventListenerInterface* eventListener;

	pp_uint8* panning;
	pp_uint32 numChannels;
	
public:
	PanningSettingsContainer(PPScreen* theScreen, EventListenerInterface* theEventListener, pp_uint32 channels);
	virtual ~PanningSettingsContainer();

	void init();

	void show(bool b);

	// PPEvent listener
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	void setPanning(pp_uint32 chn, pp_uint8 pan, bool repaint = true);
	pp_uint8 getPanning(pp_uint32 chn) { return panning[chn]; }

private:
	void applyPanningAmiga();
	void applyPanningMilky();
	void applyPanningMono();
};

#endif

