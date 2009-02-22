/*
 *  tracker/Zapper.cpp
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
 *  Zapper.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 26.12.07.
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
	tracker.moduleEditor->reloadCurrentPattern();
	tracker.sectionHDRecorder->adjustOrders();
	// stop song with resetting main volume
	tracker.ensureSongStopped(true, false);
	tracker.updateSongInfo(false);
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
	tracker.moduleEditor->reloadCurrentPattern();
	tracker.sectionHDRecorder->adjustOrders();
	// stop song with resetting main volume
	tracker.ensureSongStopped(true, false);			
	tracker.updateSongInfo(false);
	tracker.signalWaitState(false);
}

void Zapper::zapPattern()
{
	tracker.signalWaitState(true);
	tracker.playerController->resetPlayTimeCounter();
	tracker.getPatternEditor()->clearPattern();
	tracker.updateSongInfo(false);
	tracker.signalWaitState(false);	
}

void Zapper::zapInstruments()
{
	tracker.signalWaitState(true);
	tracker.playerController->resetPlayTimeCounter();
	tracker.moduleEditor->createEmptySong(false, true);
	tracker.updateSongInfo(false);
	tracker.signalWaitState(false);
}
