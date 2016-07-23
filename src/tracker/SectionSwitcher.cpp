/*
 *  tracker/SectionSwitcher.cpp
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
 *  SectionSwitcher.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 09.04.08.
 *
 */

#include "SectionSwitcher.h"
#include "Tracker.h"
#include "Screen.h"
#include "Container.h"
#include "SectionInstruments.h"
#include "SectionSamples.h"
#include "ScopesControl.h"
#include "PatternEditorControl.h"

#include "ControlIDs.h"

SectionSwitcher::SectionSwitcher(Tracker& tracker) :
	tracker(tracker),
	bottomSection(ActiveBottomSectionNone),
#ifdef __LOWRES__
	lowerSectionPage(ActiveLowerSectionPageMain),
#endif
	currentUpperSection(NULL)
{
}

// General bottom sections show/hide
void SectionSwitcher::showBottomSection(ActiveBottomSections section, bool paint/* = true*/)
{
	switch (bottomSection)
	{
		case ActiveBottomSectionInstrumentEditor:
			tracker.sectionInstruments->show(false);
			break;
		case ActiveBottomSectionSampleEditor:
			tracker.sectionSamples->show(false);
			break;
		case ActiveBottomSectionNone:
			break;
	}

	if (bottomSection != section)
		bottomSection = section;
	else
		bottomSection = ActiveBottomSectionNone;
	
	switch (bottomSection)
	{
		case ActiveBottomSectionInstrumentEditor:
			tracker.sectionInstruments->show(true);
			break;
		case ActiveBottomSectionSampleEditor:
			tracker.sectionSamples->show(true);
			break;
		case ActiveBottomSectionNone:
			tracker.rearrangePatternEditorControl();
			break;
	}	
	
	if (paint)
		tracker.screen->paint();
}

void SectionSwitcher::showUpperSection(SectionAbstract* section, bool hideSIP/* = true*/)
{
	tracker.screen->pauseUpdate(true);
	if (currentUpperSection)
	{
		currentUpperSection->show(false);
	}
	if (section)
	{
		if (hideSIP)
			tracker.hideInputControl();

		section->show(true);
	}
	tracker.screen->pauseUpdate(false);
	tracker.screen->update();
	currentUpperSection = section;
}

#ifdef __LOWRES__
void SectionSwitcher::showSubMenu(ActiveLowerSectionPages section, bool repaint/* = true*/)
{
	// Hide everything first
	tracker.showSongSettings(false);
	tracker.showMainOptions(false);
	tracker.screen->getControlByID(CONTAINER_INSTRUMENTLIST)->show(false);	
	tracker.screen->getControlByID(CONTAINER_LOWRES_TINYMENU)->show(false);
	tracker.screen->getControlByID(CONTAINER_LOWRES_JAMMENU)->show(false);
	
	tracker.scopesControl->show(false);
	tracker.screen->getControlByID(CONTAINER_SCOPECONTROL)->show(false);
	
	// Last active page was the "Jam"-section so the pattern editor has probably been resized
	// Check if it was resized and if so, restore original size
	if (lastLowerSectionPage == ActiveLowerSectionPageJam && 
		section != ActiveLowerSectionPageJam &&
		patternEditorSize != tracker.getPatternEditorControl()->getSize())
	{
		tracker.getPatternEditorControl()->setSize(patternEditorSize);
	}	
	
	switch (section)
	{
		case ActiveLowerSectionPageMain:
			tracker.showMainOptions(true);
			tracker.hideInputControl(false);
			break;
		case ActiveLowerSectionPageSong:
			tracker.showSongSettings(true);
			tracker.hideInputControl(false);
			break;
		case ActiveLowerSectionPageInstruments:
			tracker.screen->getControlByID(CONTAINER_INSTRUMENTLIST)->show(true);	
			tracker.screen->getControlByID(CONTAINER_LOWRES_TINYMENU)->show(true);
			tracker.hideInputControl(false);
			break;
		case ActiveLowerSectionPageScopes:
			tracker.scopesControl->show(true);
			tracker.screen->getControlByID(CONTAINER_SCOPECONTROL)->show(true);
			tracker.updateScopesControlButtons();
			tracker.hideInputControl(false);
			break;
		case ActiveLowerSectionPageJam:
		{
			PPControl* control = tracker.screen->getControlByID(CONTAINER_LOWRES_JAMMENU);
			ASSERT(control);
			patternEditorSize = tracker.getPatternEditorControl()->getSize();
			PPSize size(tracker.screen->getWidth(), control->getLocation().y);
			if (tracker.getPatternEditorControl()->getSize() != size)
				tracker.getPatternEditorControl()->setSize(size);
			tracker.hideInputControl();
			tracker.screen->getControlByID(CONTAINER_LOWRES_JAMMENU)->show(true);
			break;
		}
	}
	
	if (repaint)
		tracker.screen->paint();
}


void SectionSwitcher::switchToSubMenu(ActiveLowerSectionPages lsPageNew)
{
	// same page, nothing to do
	if (lsPageNew == lowerSectionPage)
		return;
	
	// remember what was currently active
	lastLowerSectionPage = lowerSectionPage;
	// apply new page
	lowerSectionPage = lsPageNew;
				
	updateSubMenusButtons(false);
	// make it visible
	showSubMenu(lowerSectionPage);
}

void SectionSwitcher::hideBottomSection() 
{ 
	if (bottomSection != ActiveBottomSectionNone)
		showBottomSection(ActiveBottomSectionNone, false);
}

void SectionSwitcher::updateSubMenusButtons(bool repaint/* = true*/)
{
	PPContainer* container = static_cast<PPContainer*>(tracker.screen->getControlByID(CONTAINER_LOWRES_MENUSWITCH));

	for (pp_int32 i = 0; i < tracker.NUMSUBMENUS(); i++)
		static_cast<PPButton*>(container->getControlByID(BUTTON_0+i))->setPressed(false);
	
	static_cast<PPButton*>(container->getControlByID(BUTTON_0+lowerSectionPage))->setPressed(true);

	if (repaint)
		tracker.screen->paintControl(container);
}

#endif
