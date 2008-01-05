/*
 *  SectionAbout.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 17.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "SectionAbout.h"
#include "Tracker.h"
#include "TrackerConfig.h"
#include "PlayerController.h"
#include "PlayerGeneric.h"
#include "AnimatedFXControl.h"

#include "ControlIDs.h"

enum ControlIDs
{
	FXCONTROL					= 9000
};

SectionAbout::SectionAbout(Tracker& theTracker) :
	SectionUpperLeft(theTracker)
{
}

SectionAbout::~SectionAbout()
{
}

pp_int32 SectionAbout::handleEvent(PPObject* sender, PPEvent* event)
{
	PPScreen* screen = tracker.screen;

	if (event->getID() == eCommand || event->getID() == eCommandRepeat)
	{

		switch (reinterpret_cast<PPControl*>(sender)->getID())
		{
			case FXCONTROL:
				if (event->getID() != eCommand)
					break;

				show(false);
				break;
		}
		
	}

	return 0;
}

void SectionAbout::init(pp_int32 px, pp_int32 py)
{
	PPScreen* screen = tracker.screen;
	
	AnimatedFXControl* ctrl = new AnimatedFXControl(FXCONTROL, tracker.screen, this, PPPoint(px, py), PPSize(320, UPPERLEFTSECTIONHEIGHT));
	ctrl->setBorderColor(TrackerConfig::colorThemeMain);
	screen->addControl(ctrl);
	
	sectionContainer = ctrl;

	initialised = true;

	showSection(false);
}

void SectionAbout::update(bool repaint/* = true*/)
{
}
