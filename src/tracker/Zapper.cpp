/*
 *  Zapper.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 26.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Zapper.h"
#include "Tracker.h"
#include "PlayerController.h"
#include "ModuleEditor.h"
#include "ModuleServices.h"
#include "PatternEditor.h"
#include "SectionHDRecorder.h"

void Zapper::zapAll()
{
	if (!tracker.checkForChanges())
		return;
	tracker.signalWaitState(true);
	tracker.playerController->resetPlayTimeCounter();
	tracker.moduleEditor->createEmptySong(true, 
										  true, 
										  tracker.playerController->getPlayMode() == PlayerController::PlayMode_FastTracker2 ? 8 : 4);
	tracker.moduleEditor->getModuleServices()->resetEstimatedSongLength();
	tracker.sectionHDRecorder->adjustOrders();
	// stop song with resetting main volume
	tracker.ensureSongStopped(true, false);
	tracker.signalWaitState(false);
}

void Zapper::zapSong()
{
	tracker.signalWaitState(true);
	tracker.playerController->resetPlayTimeCounter();
	tracker.moduleEditor->createEmptySong(true, 
								  false,
								  tracker.playerController->getPlayMode() == PlayerController::PlayMode_FastTracker2 ? 8 : 4);
	tracker.moduleEditor->getModuleServices()->resetEstimatedSongLength();
	tracker.sectionHDRecorder->adjustOrders();
	// stop song with resetting main volume
	tracker.ensureSongStopped(true, false);			
	tracker.signalWaitState(false);
}

void Zapper::zapPattern()
{
	tracker.signalWaitState(true);
	tracker.playerController->resetPlayTimeCounter();
	tracker.getPatternEditor()->clearPattern();
	tracker.signalWaitState(false);	
}

void Zapper::zapInstrument()
{
	tracker.signalWaitState(true);
	tracker.playerController->resetPlayTimeCounter();
	tracker.moduleEditor->createEmptySong(false, true);
	tracker.signalWaitState(false);
}
