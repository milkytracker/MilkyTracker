/*
 *  tracker/PlayerLogic.cpp
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
 *  PlayerLogic.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 21.12.07.
 *
 */

#include "PlayerLogic.h"
#include "Tracker.h"
#include "PlayerMaster.h"
#include "PlayerController.h"
#include "ModuleEditor.h"
#include "RecorderLogic.h"

#include "ListBox.h"
#include "Screen.h"
#include "PatternEditorControl.h"

PlayerLogic::PlayerLogic(Tracker& tracker) :
	tracker(tracker),
	liveSwitch(false),
	stopBackgroundOnPlay(false),
	tracePlay(false),
	rowPlay(false)
{
}

void PlayerLogic::setLiveSwitch(bool liveSwitch)
{
	this->liveSwitch = liveSwitch;
	if (!liveSwitch)
		tracker.playerMaster->resetQueuedPositions();
}

void PlayerLogic::playSong(pp_int32 row/* = 0*/)
{
	tracker.initPlayback();

	if (stopBackgroundOnPlay)
		stopAll();

	pp_int32 index = tracker.moduleEditor->getCurrentOrderIndex();				
	tracker.playerController->playSong(index, row, tracker.muteChannels);
}

void PlayerLogic::playPattern()
{
	tracker.initPlayback();

	if (stopBackgroundOnPlay)
		stopAll();

	tracker.playerController->playPattern(tracker.moduleEditor->getCurrentPatternIndex(),
										  tracker.moduleEditor->getCurrentOrderIndex(),
										  0,
										  tracker.muteChannels);
}

void PlayerLogic::playPosition(bool rowOnly/* = false*/)
{
	if (!rowOnly)
		tracker.initPlayback();

	// when the replay engine is already in row playing mode,
	// we don't need to restart the song, just update the current position to the cursor
	if (rowOnly && 
		tracker.playerController->isPlaying() && 
		tracker.playerController->isPlayingRowOnly())
	{
		tracker.playerController->setCurrentPatternIndex(tracker.moduleEditor->getCurrentPatternIndex());
		tracker.updateSongRow(false);
		tracker.playerController->readjustSpeed(false);
		return;
	}

	if (stopBackgroundOnPlay)
		stopAll();

	tracker.playerController->playPattern(tracker.moduleEditor->getCurrentPatternIndex(),
										  tracker.moduleEditor->getCurrentOrderIndex(),
										  tracker.getPatternEditorControl()->getCurrentRow(),
										  tracker.muteChannels,
										  rowOnly);
}

void PlayerLogic::stopPlayer(PlayerController& playerController)
{
	playerController.stop();
	playerController.resetPlayTimeCounter();
}

void PlayerLogic::stopSong()
{
	tracker.recorderLogic->reset();

	stopPlayer(*tracker.playerController);

	tracker.moduleEditor->cleanUnusedPatterns();

	tracker.getPatternEditorControl()->setSongPosition(-1, -1);
	tracker.screen->paintControl(tracker.getPatternEditorControl());
	tracker.updateSpeed();
}

void PlayerLogic::stopAll()
{
	for (pp_int32 i = 0; i < tracker.playerMaster->getNumPlayerControllers(); i++)
	{
		if (tracker.playerController != tracker.playerMaster->getPlayerController(i))
			tracker.playerMaster->getPlayerController(i)->pause();
	}
		//stopPlayer(*tracker.playerMaster->getPlayerController(i));
}

void PlayerLogic::storePosition()
{
	// does the current pattern match the selected order list index?
	backupIndex = tracker.isEditingCurrentOrderlistPattern() ? tracker.getOrderListBoxIndex() : tracker.moduleEditor->getCurrentPatternIndex();

	// indicate if we were editing a pattern different from the one in the orderlist
	backupIndex |= (tracker.isEditingCurrentOrderlistPattern() ? 0x10000 : 0);
	
	backupRow = tracker.getPatternEditorControl()->getCurrentRow();
}

void PlayerLogic::restorePosition()
{
	pp_int32 index = backupIndex & 0xFFFF;
	
	tracker.screen->pauseUpdate(true);
	
	if (backupIndex & 0x10000)
	{
		tracker.setOrderListIndex(index);
	}
	else
	{
		tracker.moduleEditor->setCurrentPatternIndex(index);
		tracker.updatePattern();
	}

	tracker.screen->pauseUpdate(false);
	
	tracker.getPatternEditorControl()->setRow(backupRow);
	
	tracker.screen->paint();
}

void PlayerLogic::playTrace()
{
	if (!tracePlay)
	{
		storePosition();

		tracePlay = true;
		
		if (tracker.isEditingCurrentOrderlistPattern())
			playSong(tracker.getPatternEditorControl()->getCurrentRow());
		else
			playPosition();
	}
}

void PlayerLogic::playRow()
{
	rowPlay = true;
	playPosition(true);	
	tracker.getPatternEditorControl()->advanceRow(true);
}

void PlayerLogic::ensureSongStopped(bool bResetMainVolume, bool suspend)
{
	if (suspend)
	{
		tracker.playerController->suspendPlayer(bResetMainVolume);
	}
	else
	{
		tracker.playerController->stopSample();
		// make sure song is stopped, but don't reset the main volume
		tracker.playerController->stop(bResetMainVolume);
	}	
}

void PlayerLogic::ensureSongPlaying(bool continuePlaying)
{
	if (tracker.playerController->isSuspended())
		tracker.playerController->resumePlayer(continuePlaying);
	else
		tracker.playerController->continuePlaying();
}

#define CONTINUEPATTERN \
	if (tracker.playerController->isPlayingPattern() && \
		!tracker.playerController->isPlayingRowOnly()) \
		tracker.playerController->playPattern(tracker.moduleEditor->getCurrentPatternIndex(), \
		tracker.moduleEditor->getCurrentOrderIndex(), \
		0, \
		tracker.muteChannels); 

#define CONTINUE \
	CONTINUEPATTERN \
	else if (tracker.playerController->isPlaying() && \
			!tracker.playerController->isPlayingRowOnly() && \
			tracker.shouldFollowSong()) \
		tracker.playerController->playSong(tracker.moduleEditor->getCurrentOrderIndex(), \
		0, \
		tracker.muteChannels);

void PlayerLogic::continuePlayingPattern()
{
	if (liveSwitch)
	{
		tracker.playerController->setNextPatternToPlay(tracker.moduleEditor->getCurrentPatternIndex());
		return;
	}

	if (tracker.playerController->isPlayingPattern() && 
		!tracker.playerController->isPlayingRowOnly()) 
		tracker.playerController->playPattern(tracker.moduleEditor->getCurrentPatternIndex(),
											  tracker.moduleEditor->getCurrentOrderIndex(), 
											  -1, 
											  tracker.muteChannels); 
}

void PlayerLogic::continuePlayingSong()
{
	if (liveSwitch)
	{
		tracker.playerController->setNextOrderToPlay(tracker.moduleEditor->getCurrentOrderIndex());
		return;
	}

	CONTINUE
}

void PlayerLogic::finishTraceAndRowPlay()
{
	if (tracePlay)
	{
		stopSong();
		tracePlay = false;
		restorePosition();
	}
	else if (rowPlay)
	{
		rowPlay = false;
	}
}

void PlayerLogic::playNote(class PlayerController& playerController, 
						   pp_uint8 chn, 
						   pp_int32 note, pp_int32 i, pp_int32 vol/* = -1*/)
{
	playerController.playNote(chn, note, i, vol);
}
						   
