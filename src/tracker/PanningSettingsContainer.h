/*
 *  PanningSettingsContainer.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.03.06.
 *  Copyright 2006 milkytracker.net, All rights reserved.
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

