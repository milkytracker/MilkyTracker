/*
 *  tracker/SectionAbout.cpp
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
 *  SectionAbout.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 17.11.05.
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
