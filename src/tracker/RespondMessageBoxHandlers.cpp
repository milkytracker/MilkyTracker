/*
 *  tracker/RespondMessageBoxHandlers.cpp
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
 *  RespondMessageBoxHandlers.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *
 */

#include "RespondMessageBoxHandlers.h"
#include "Tracker.h"
#include "RespondMessageBoxChannelSelector.h"
#include "ListBox.h"
#include "ModuleEditor.h"
#include "PlayerController.h"
#include "SectionSamples.h"
#include "ControlIDs.h"
#include "ScopesControl.h"

// Load either left or right sample from a list of choices
pp_int32 SampleLoadChannelSelectionHandler::ActionOkay(PPObject* sender)
{
	PPListBox* listBox = static_cast<RespondMessageBoxChannelSelector*>(tracker.respondMessageBox)->getListBox();
	
	tracker.signalWaitState(true);

	if (preferredFileName.length())	
		tracker.loadingParameters.res = tracker.moduleEditor->loadSample(fileName, 
																		 tracker.listBoxInstruments->getSelectedIndex(), 
																		 tracker.listBoxSamples->getSelectedIndex(), 
																		 listBox->getSelectedIndex(),
																		 preferredFileName);
	else
		tracker.loadingParameters.res = tracker.moduleEditor->loadSample(fileName, 
																		 tracker.listBoxInstruments->getSelectedIndex(), 
																		 tracker.listBoxSamples->getSelectedIndex(), 
																		 listBox->getSelectedIndex());
	
	tracker.sectionSamples->updateAfterLoad();
	
	return !tracker.finishLoading();
}

// Mixdown all channels into one
pp_int32 SampleLoadChannelSelectionHandler::ActionUser1(PPObject* sender)
{
	tracker.signalWaitState(true);
	
	// mixdown samples
	if (preferredFileName.length())	
		tracker.loadingParameters.res = tracker.moduleEditor->loadSample(fileName, 
																		 tracker.listBoxInstruments->getSelectedIndex(), 
																		 tracker.listBoxSamples->getSelectedIndex(), 
																		 -1,
																		 preferredFileName);
	else
		tracker.loadingParameters.res = tracker.moduleEditor->loadSample(fileName, 
																		 tracker.listBoxInstruments->getSelectedIndex(), 
																		 tracker.listBoxSamples->getSelectedIndex(), 
																		 -1);
																	 
	tracker.sectionSamples->updateAfterLoad();

	return !tracker.finishLoading();
}

pp_int32 SampleLoadChannelSelectionHandler::ActionCancel(PPObject* sender)
{
	tracker.loadingParameters.abortLoading = true;
	tracker.finishLoading();
	return 0;
}

void SampleLoadChannelSelectionHandler::setCurrentFileName(const PPSystemString& fileName)
{
	this->fileName = fileName;
}

void SampleLoadChannelSelectionHandler::setPreferredFileName(const PPSystemString& fileName)
{
	this->preferredFileName = fileName;
}
