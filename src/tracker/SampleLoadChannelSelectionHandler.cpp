/*
 *  SampleLoadChannelSelectionHandler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.10.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "SampleLoadChannelSelectionHandler.h"
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
	PPListBox* listBox = tracker.sampleLoadRespondMessageBox->getListBox();
	
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
