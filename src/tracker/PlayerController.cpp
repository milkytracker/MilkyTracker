/*
 *  tracker/PlayerController.cpp
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
 *  PlayerController.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Tue Mar 15 2005.
 *
 */

#include "PlayerController.h"
#include "PlayerMaster.h"
#include "MilkyPlay.h"
#include "ResamplerMacros.h"
#include "PPSystem.h"
#include "PlayerCriticalSection.h"
#include "ModuleEditor.h"

class PlayerStatusTracker : public PlayerSTD::StatusEventListener
{
public:
	PlayerStatusTracker(PlayerController& playerController) :
		playerController(playerController)
	{
		clearUpdateCommandBuff();
	}
	
	// this is being called from the player callback in a serialized fashion
	virtual void playerTickStarted(PlayerSTD& player, XModule& module) 
	{ 
		while (rbReadIndex < rbWriteIndex)
		{			
			mp_sint32 idx = rbReadIndex & (UPDATEBUFFSIZE-1);			
			
			// handle notes that are played from external source
			// i.e. keyboard playback
			switch (updateCommandBuff[idx].code)
			{
				case UpdateCommandCodeNote:
				{
					UpdateCommandNote* command = reinterpret_cast<UpdateCommandNote*>(&updateCommandBuff[idx]);
					mp_sint32 note = command->note;
					if (note)
					{
						player.playNote(command->channel, note, 
										command->ins, 
										command->volume);
						command->note = 0;
					}
					break;
				}

				case UpdateCommandCodeSample:
				{
					UpdateCommandSample* command = reinterpret_cast<UpdateCommandSample*>(&updateCommandBuff[idx]);
					const TXMSample* smp = command->smp;
					if (smp)
					{
						playSampleInternal(player, command->channel, smp, command->currentSamplePlayNote,
										   command->rangeStart,  command->rangeEnd);
						command->smp = NULL;
					}
					
					break;
				}
				
			}
			rbReadIndex++;
		}
	}	
	
	virtual void patternEndReached(PlayerSTD& player, XModule& module, mp_sint32& newOrderIndex) 
	{ 
		handleQueuedPositions(player, newOrderIndex);
	}

	void playNote(mp_ubyte chn, mp_sint32 note, mp_sint32 ins, mp_sint32 vol/* = -1*/)
	{	
		// fill ring buffer with note entries
		// the callback will query these notes and play them 
		mp_sint32 idx = rbWriteIndex & (UPDATEBUFFSIZE-1);
		UpdateCommandNote* command = reinterpret_cast<UpdateCommandNote*>(&updateCommandBuff[idx]);
		command->channel = chn;
		command->ins = ins;
		command->volume = vol;
		command->note = note;
		// code needs to be filled in last
		command->code = UpdateCommandCodeNote;
		rbWriteIndex++;
	}

	void playSample(mp_ubyte chn, const TXMSample& smp, mp_sint32 currentSamplePlayNote, mp_sint32 rangeStart, mp_sint32 rangeEnd)
	{
		// fill ring buffer with sample playback entries
		mp_sint32 idx = rbWriteIndex & (UPDATEBUFFSIZE-1);
		UpdateCommandSample* command = reinterpret_cast<UpdateCommandSample*>(&updateCommandBuff[idx]);
		command->channel = chn;
		command->currentSamplePlayNote = currentSamplePlayNote;
		command->rangeStart = rangeStart;
		command->rangeEnd = rangeEnd;
		command->smp = &smp;
		command->code = UpdateCommandCodeSample;
		rbWriteIndex++;
	}
				  
private:
	PlayerController& playerController;
	
	void handleQueuedPositions(PlayerSTD& player, mp_sint32& poscnt)
	{
		// there is a queued position
		if (playerController.nextOrderIndexToPlay != -1)
		{
			// the new order the queued one
			poscnt = playerController.nextOrderIndexToPlay;
			// the queued one becomes invalid
			playerController.nextOrderIndexToPlay = -1;
			// we're no longer in pattern play mode...
			playerController.patternPlay = false;
			// ... that's why we're going to set the pattern index to -1
			playerController.patternIndex = -1;
			player.setPatternToPlay(-1);
		}
		// there is a queued pattern
		else if (playerController.nextPatternIndexToPlay != -1)
		{
			// the new pattern index
			player.setPatternToPlay(playerController.nextPatternIndexToPlay);
			// we're no in pattern play mode
			playerController.patternPlay = true;
			// that's our new pattern
			playerController.patternIndex = playerController.nextPatternIndexToPlay;
			// the queued one becomes invalid
			playerController.nextPatternIndexToPlay = -1;
		}
	}
	
	void playSampleInternal(PlayerSTD& player, mp_ubyte chn, const TXMSample* smp, mp_sint32 currentSamplePlayNote, 
							mp_sint32 rangeStart, mp_sint32 rangeEnd)
	{
		pp_int32 period = player.getlogperiod(currentSamplePlayNote+1, smp->relnote, smp->finetune) << 8;
		pp_int32 freq = player.getlogfreq(period); 

		player.setFreq(chn,freq);
		player.setVol(chn, (smp->vol*512)/255);
		player.setPan(chn, 128);

		player.chninfo[chn].flags |= 0x100; // CHANNEL_FLAGS_UPDATE_IGNORE

		pp_int32 flags = (smp->type >> 2) & 4;
		
		if (rangeStart == -1 && rangeEnd == -1)
		{
			flags |= smp->type & 3;
			
			if (flags & 3)
				player.playSample(chn, smp->sample, smp->samplen, 0, 0, false, smp->loopstart, smp->loopstart+smp->looplen, flags);
			else
				player.playSample(chn, smp->sample, smp->samplen, 0, 0, false, 0, smp->samplen, flags);
		}
		else
		{
			if (rangeStart == -1 || rangeEnd == -1)
				return;
			
			if (rangeEnd > (signed)smp->samplen)
				rangeEnd = smp->samplen;

			player.playSample(chn, smp->sample, smp->samplen, rangeStart, 0, false, 0, rangeEnd, flags);
		}
	}

	void clearUpdateCommandBuff()
	{
		memset(updateCommandBuff, 0, sizeof(updateCommandBuff));
		rbReadIndex = rbWriteIndex = 0;
	}
	
	enum
	{
		// must be 2^n
		UPDATEBUFFSIZE = 128
	};
	
	enum UpdateCommandCodes
	{
		UpdateCommandCodeInvalid = 0,
		UpdateCommandCodeNote,
		UpdateCommandCodeSample,
	};
	
	struct UpdateCommand
	{
		mp_ubyte code;			
		mp_uint32 data[8];
		void* pdata[8];
	};

	struct UpdateCommandNote
	{
		mp_ubyte code;			
		mp_sint32 note;
		mp_sint32 channel;
		mp_sint32 ins;
		mp_sint32 volume;
		mp_uint32 data[4];
		void* pdata[8];
	};
	
	struct UpdateCommandSample
	{
		mp_ubyte code;
		mp_uint32 currentSamplePlayNote;
		mp_uint32 rangeStart;
		mp_uint32 rangeEnd;
		mp_sint32 channel;
		mp_uint32 data[4];
		const TXMSample* smp;
		void* pdata[7];
	};
	
	UpdateCommand updateCommandBuff[UPDATEBUFFSIZE];
	
	mp_sint32 rbReadIndex;
	mp_sint32 rbWriteIndex;	
};

void PlayerController::assureNotSuspended()
{
	if (mixer->isDevicePaused(player))
	{
		mixer->resumeDevice(player);
		if (suspended)
			suspended = false;
	}
}

void PlayerController::reset()
{
	if (!player)
		return;

	// reset internal player variables (effect memory) and looping information
	player->BPMCounter = 0;
	player->reset();
	// reset mixer channels (stop playing channels)
	player->resetChannelsFull();
}

bool PlayerController::detachDevice()
{
	if (!mixer->isDeviceRemoved(player))
	{
		return mixer->removeDevice(player);
	}
	return false;
}

PlayerController::PlayerController(MasterMixer* mixer, bool fakeScopes) :
	mixer(mixer),
	player(NULL),
	module(NULL),
	criticalSection(NULL),
	playerStatusTracker(new PlayerStatusTracker(*this)),
	patternPlay(false), playRowOnly(false),
	patternIndex(-1), 
	nextOrderIndexToPlay(-1),
	nextPatternIndexToPlay(-1),
	lastPosition(-1), lastRow(-1),
	suspended(false),
	firstRecordChannelCall(true),
	numPlayerChannels(TrackerConfig::numPlayerChannels),
	numVirtualChannels(TrackerConfig::numVirtualChannels),
	totalPlayerChannels(numPlayerChannels + numVirtualChannels + 2),
	useVirtualChannels(TrackerConfig::useVirtualChannels),
	multiChannelKeyJazz(true),
	multiChannelRecord(true),
	mixerDataCacheSize(fakeScopes ? 0 : 512*2),
	mixerDataCache(fakeScopes ? NULL : new mp_sint32[mixerDataCacheSize])
{
	criticalSection = new PlayerCriticalSection(*this);

	player = new PlayerSTD(mixer->getSampleRate(), playerStatusTracker);
	player->setPlayMode(PlayerBase::PlayMode_FastTracker2);
	player->resetMainVolumeOnStartPlay(false);
	player->setBufferSize(mixer->getBufferSize());

	currentPlayingChannel = useVirtualChannels ? numPlayerChannels : 0;
	
	pp_uint32 i;
	
	for (i = 0; i < sizeof(muteChannels) / sizeof(bool); i++)
		muteChannels[i] = false;
										
	for (i = 0; i < sizeof(recordChannels) / sizeof(bool); i++)
		recordChannels[i] = false;

	for (i = 0; i < sizeof(panning) / sizeof(mp_ubyte); i++)
	{
		switch (i & 3)
		{
			case 0:
				panning[i] = 0;
				break;
			case 1:
				panning[i] = 255;
				break;
			case 2:
				panning[i] = 255;
				break;
			case 3:
				panning[i] = 0;
				break;
		}
	}
}

PlayerController::~PlayerController()
{
	delete[] mixerDataCache;

	if (player)
	{
		detachDevice();
		delete player;
	}
	
	delete playerStatusTracker;
	
	delete criticalSection;
}

void PlayerController::attachModuleEditor(ModuleEditor* moduleEditor)
{
	this->moduleEditor = moduleEditor;
	this->module = moduleEditor->getModule();

	if (!player)
		return;

	if (!mixer->isDeviceRemoved(player))
		mixer->removeDevice(player);

	ASSERT(sizeof(muteChannels)/sizeof(bool) >= (unsigned)totalPlayerChannels);

	player->startPlaying(module, true, 0, 0, totalPlayerChannels, panning, true);	

	// restore muting
	for (mp_sint32 i = 0; i < numPlayerChannels; i++)
		player->muteChannel(i, muteChannels[i]);
	
	mixer->addDevice(player);
}

void PlayerController::playSong(mp_sint32 startIndex, mp_sint32 rowPosition, bool* muteChannels)
{
	if (!player)
		return;

	if (!module)
		return;

	if (!module->isModuleLoaded())
		return;

	assureNotSuspended();

	if (!suspended)
		criticalSection->enter(false);

	readjustSpeed();

	reset();

	player->setPatternToPlay(-1);
	setNextOrderToPlay(-1);
	setNextPatternToPlay(-1);

	// muting has been reset, restore it
	for (mp_sint32 i = 0; i < numPlayerChannels; i++)
	{
		player->muteChannel(i, muteChannels[i]);
		this->muteChannels[i] = muteChannels[i];
	}
	player->restart(startIndex, rowPosition, false, panning);
	player->setIdle(false);
	//resetPlayTimeCounter();

	patternPlay = false;
	playRowOnly = false;
	patternIndex = 0;

	criticalSection->leave(false);
}

void PlayerController::playPattern(mp_sint32 index, mp_sint32 songPosition, mp_sint32 rowPosition, bool* muteChannels, bool playRowOnly/* = false*/)
{
	if (!player)
		return;

	if (!module)
		return;

	if (!module->isModuleLoaded())
		return;

	assureNotSuspended();

	if (!suspended)
		criticalSection->enter(false);

	readjustSpeed();
	
	reset();

	setCurrentPatternIndex(index);
	setNextOrderToPlay(-1);
	setNextPatternToPlay(-1);

	// muting has been reset, restore it
	for (mp_sint32 i = 0; i < numPlayerChannels; i++)
	{
		player->muteChannel(i, muteChannels[i]);
		this->muteChannels[i] = muteChannels[i];
	}
	
	if (rowPosition == -1)
	{
		rowPosition = player->getRow();
		if (rowPosition >= module->phead[index].rows)
			rowPosition = 0;
	}
	
	player->restart(songPosition, rowPosition, false, panning, playRowOnly);
	player->setIdle(false);
	//resetPlayTimeCounter();

	patternPlay = true;
	this->playRowOnly = playRowOnly;
	patternIndex = index;

	criticalSection->leave(false);
}

void PlayerController::setCurrentPatternIndex(mp_sint32 index)
{
	if (player)
		player->setPatternToPlay(index);
}

void PlayerController::stop(bool bResetMainVolume/* = true*/)
{
	if (!player)
		return;

	if (!module)
		return;

	if (!suspended)
		criticalSection->enter(false);

	if (isPlaying() && !playRowOnly)
	{
		lastPosition = player->getOrder();
		lastRow = player->getRow();
		wasPlayingPattern = isPlayingPattern();
	}
	else
	{
		lastPosition = -1;
		lastRow = -1;
		wasPlayingPattern = false;
	}

	patternPlay = false;
	playRowOnly = false;
	
	readjustSpeed();

	player->setIdle(true);
	reset();
	player->restart(0, 0, true, panning);

	// muting has been reset, restore it
	for (mp_sint32 i = 0; i < numPlayerChannels; i++)
	{
		player->muteChannel(i, muteChannels[i]);
		this->muteChannels[i] = muteChannels[i];
	}

	// reset internal variables	
	if (bResetMainVolume)
		resetMainVolume();	
		
	setNextOrderToPlay(-1);
	setNextPatternToPlay(-1);

	criticalSection->leave(false);
}

void PlayerController::continuePlaying()
{	
	continuePlaying(true);
}

void PlayerController::continuePlaying(bool assureNotSuspended)
{
	if (lastPosition == -1 || lastRow == -1)
		return;
		
	if (!player)
		return;

	if (!module)
		return;

	if (!module->isModuleLoaded())
		return;
		
	if (assureNotSuspended)
		this->assureNotSuspended();

	if (!suspended)
		criticalSection->enter(false);

	readjustSpeed();
	
	if (wasPlayingPattern)
		player->setPatternToPlay(patternIndex);
	else
		player->setPatternToPlay(-1);

	for (mp_sint32 i = 0; i < numPlayerChannels; i++)
		player->muteChannel(i, muteChannels[i]);
		
	player->restart(lastPosition, lastRow, false, panning);
	player->setIdle(false);

	patternPlay = wasPlayingPattern;
	playRowOnly = false;
	
	criticalSection->leave(false);
}

void PlayerController::restartPlaying()
{
	if (!player)
		return;

	if (!module)
		return;

	if (!module->isModuleLoaded())
		return;

	lastPosition = lastRow = 0;
}

bool PlayerController::isPlaying() const
{
	if (!player)
		return false;

	if (!module)
		return false;

	return player->isPlaying() && (!player->isIdle()) && !player->hasSongHalted();	
}

bool PlayerController::isPlayingRowOnly() const
{
	if (!player)
		return false;

	if (!module)
		return false;

	return playRowOnly;	
}

bool PlayerController::isActive() const
{
	if (!player)
		return false;

	if (!module)
		return false;

	return player->isPlaying() && (!player->isIdle());	
}

bool PlayerController::isPlayingPattern(mp_sint32 index) const
{
	if (!player)
		return false;

	if (index < 0)
		return false;
		
	if (player->getPatternToPlay() < 0)
		return false;

	return isPlayingPattern() && (player->getPatternToPlay() == index);
}

void PlayerController::setNextOrderToPlay(mp_sint32 orderIndex)
{
	nextOrderIndexToPlay = orderIndex; 
	if (orderIndex != -1)
		nextPatternIndexToPlay = -1;
}

mp_sint32 PlayerController::getNextOrderToPlay() const
{
	return nextOrderIndexToPlay;
}

void PlayerController::setNextPatternToPlay(mp_sint32 patternIndex)
{
	nextPatternIndexToPlay = patternIndex; 
	if (patternIndex != -1)
		nextOrderIndexToPlay = -1;
}

mp_sint32 PlayerController::getNextPatternToPlay() const
{
	return nextPatternIndexToPlay;
}

void PlayerController::pause()
{
	if (player)
		player->pausePlaying();
}

void PlayerController::unpause()
{
	if (player)
		player->resumePlaying();
}
	
bool PlayerController::isPaused() const
{
	if (player)
		return player->isPaused();
		
	return false;
}

void PlayerController::getSpeed(mp_sint32& BPM, mp_sint32& speed)
{
	if (player && player->isPlaying())
	{
		BPM = player->getTempo();
		speed = player->getSpeed();
	}
	else if (module)
	{
		BPM = module->header.speed;
		speed = module->header.tempo;
	}
	else
	{
		BPM = 125;
		speed = 6;
	}
}

void PlayerController::setSpeed(mp_sint32 BPM, mp_sint32 speed, bool adjustModuleHeader/* = true*/)
{
	if (!player)
		return;

	if (BPM < 32)
		BPM = 32;
	if (BPM > 255)
		BPM = 255;

	if (speed < 1)
		speed = 1;
	if (speed > 31)
		speed = 31;

	if (player->isPlaying())
	{
		player->setTempo(BPM);
		player->setSpeed(speed);
		// this is a MUST!!!
		pp_uint32 bpmRate = player->getbpmrate(BPM);
		player->adder = bpmRate;
	}
	
	if (module && adjustModuleHeader)
	{
		module->header.speed = BPM;
		module->header.tempo = speed;
	}
}

void PlayerController::readjustSpeed(bool adjustModuleHeader/* = true*/)
{
	mp_sint32 speed, bpm;
	getSpeed(bpm, speed);
	setSpeed(bpm, speed, adjustModuleHeader);
}

void PlayerController::playSample(const TXMSample& smp, mp_sint32 currentSamplePlayNote, mp_sint32 rangeStart/* = -1*/, mp_sint32 rangeEnd/* = -1*/)
{
	if (!player)
		return;

	assureNotSuspended();

	if (player->isPlaying())
	{
		pp_int32 i = numPlayerChannels + numVirtualChannels + 1;

		playerStatusTracker->playSample(i, smp, currentSamplePlayNote, rangeStart, rangeEnd);		
	}	

}

void PlayerController::stopSample()
{
	if (!player)
		return;

	if (player->isPlaying())
	{
		// doesn't seem to be a critical race condition
		pp_int32 i = numPlayerChannels + numVirtualChannels + 1;

		player->stopSample(i);

		player->chninfo[i].flags &= ~0x100; // CHANNEL_FLAGS_UPDATE_IGNORE		
	}	

}

void PlayerController::stopInstrument(mp_sint32 insIndex)
{
	if (!player)
		return;

	if (player->isPlaying())
	{
		// doesn't seem to be a critical race condition
		for (pp_int32 i = 0; i < numPlayerChannels + numVirtualChannels; i++)
		{
			if (player->chninfo[i].ins == insIndex)
			{
				player->stopSample(i);
				player->chninfo[i].flags &= ~0x100; // CHANNEL_FLAGS_UPDATE_IGNORE
			}
		}
	}	
}

void PlayerController::playNote(mp_ubyte chn, mp_sint32 note, mp_sint32 i, mp_sint32 vol/* = -1*/)
{
	if (!player)
		return;
		
	assureNotSuspended();

	// note playing goes synchronized in the playback callback
	playerStatusTracker->playNote(chn, note, i, vol);
}

void PlayerController::suspendPlayer(bool bResetMainVolume/* = true*/, bool stopPlaying/* = true*/)
{
	if (!player || suspended || mixer->isDeviceRemoved(player))
		return;

	mixer->pauseDevice(player);
	suspended = true;

	if (stopPlaying)
	{
		stopSample();
		stop(bResetMainVolume);	
	}
}
	
void PlayerController::resumePlayer(bool continuePlaying)
{
	if (!player)
		return;

	if (continuePlaying)
		this->continuePlaying(!suspended);

	if (suspended)
	{
		mixer->resumeDevice(player);
		suspended = false;
	}
}

void PlayerController::muteChannel(mp_sint32 c, bool m)
{
	muteChannels[c] = m;
	
	if (player)
		player->muteChannel(c, m);
}

bool PlayerController::isChannelMuted(mp_sint32 c)
{
	return muteChannels[c];
	// do not poll the state from the player it will be resetted when the
	// player stops playing of a song 
	/*if (player)
	{
		if (!player->getPlayerInstance())
			return false;

		return static_cast<PlayerSTD*>(player->getPlayerInstance())->isChannelMuted(c);
	}
	
	return false;*/
}

void PlayerController::recordChannel(mp_sint32 c, bool m)
{
	recordChannels[c] = m;
}

bool PlayerController::isChannelRecording(mp_sint32 c)
{
	return recordChannels[c];
}

void PlayerController::reallocateChannels(mp_sint32 moduleChannels/* = 32*/, mp_sint32 virtualChannels/* = 0*/)
{
    
    // channels might be changed, we need to make sure playing is stopped firstly
    bool paused = false;
    if (player && module) {
        paused = player->isPaused();
        stop(false);
    }
    
	numPlayerChannels = moduleChannels;
	numVirtualChannels = virtualChannels;
    totalPlayerChannels = numPlayerChannels + (numVirtualChannels >= 0 ? numVirtualChannels : 0) + 2;

    if (player && module) {
        // reattaching will cause the desired channels to be allocated
        attachModuleEditor(moduleEditor);
        if (paused)
            player->pausePlaying();
        continuePlaying();
    }
}

void PlayerController::setUseVirtualChannels(bool bUseVirtualChannels)
{
	useVirtualChannels = bUseVirtualChannels;

	currentPlayingChannel = useVirtualChannels ? numPlayerChannels : 0;
}

void PlayerController::resetFirstPlayingChannel()
{
	for (pp_int32 i = 0; i < module->header.channum; i++)
	{
		if (recordChannels[i])
		{
			currentPlayingChannel = i;
			break;
		}
	}
}

mp_sint32 PlayerController::getNextPlayingChannel(mp_sint32 currentChannel)
{
	// if we're using virtual channels for instrument playback
	// the virtual channels are located in the range 
	// [numPlayerChannels .. numPlayerChannels + numVirtualChannels]
	if (useVirtualChannels)
	{
		if (currentPlayingChannel < numPlayerChannels)
			currentPlayingChannel = numPlayerChannels-1;
	
		mp_sint32 res = currentPlayingChannel++;

		if (currentPlayingChannel >= numPlayerChannels + numVirtualChannels)
			currentPlayingChannel = numPlayerChannels;

		return res;
	}
	// if we're not using virtual channels for instrument playback
	// just use the module channels and cut notes which are playing
	else if (multiChannelKeyJazz)
	{
		mp_sint32 res = currentPlayingChannel/*++*/;
		//if (currentPlayingChannel >= module->header.channum)
		//	currentPlayingChannel = 0;

		bool found = false;
		for (pp_int32 i = currentPlayingChannel+1; i < currentPlayingChannel + 1 + module->header.channum; i++)
		{
			pp_int32 c = i % module->header.channum;
			if (recordChannels[c])
			{
				currentPlayingChannel = c;
				found = true;
				break;
			}
		}
		
		return found ? res : currentChannel;
	}
	
	return currentChannel;
}

void PlayerController::initRecording()
{
	firstRecordChannelCall = true;
}

mp_sint32 PlayerController::getNextRecordingChannel(mp_sint32 currentChannel)
{
	if (currentChannel < 0 || currentChannel >= TrackerConfig::MAXCHANNELS)
		return -1;

	if (firstRecordChannelCall && recordChannels[currentChannel])
	{
		firstRecordChannelCall = false;
		return currentChannel;
	}
	else
	{
		for (pp_int32 i = currentChannel+1; i < currentChannel + 1 + module->header.channum; i++)
		{
			pp_int32 c = i % module->header.channum;
			if (recordChannels[c])
				return c;
		}
		//return (currentChannel+1)%module->header.channum;
	}
	return currentChannel;
}

mp_sint32 PlayerController::getSongMainVolume()
{
	if (!player || !module)
		return 255;

	return player->getSongMainVolume();
}

void PlayerController::resetMainVolume()
{
	if (!player || !module)
		return;
		
	player->setSongMainVolume((mp_ubyte)module->header.mainvol);
}

mp_int64 PlayerController::getPlayTime()
{
	if (!player)
		return 0;

	float freq = (float)player->getMixFrequency();
	
	return (mp_int64)(player->getSampleCounter()/freq);
}

void PlayerController::resetPlayTimeCounter()
{
	if (!player)
		return;

	player->resetSampleCounter();
}

void PlayerController::setPanning(mp_ubyte chn, mp_ubyte pan)
{
	if (!player)
		return;

	panning[chn] = pan;
	
	if (player && player->isPlaying())
	{
		for (mp_sint32 i = 0; i < TrackerConfig::numPlayerChannels; i++)
			player->setPanning((mp_ubyte)i, panning[i]);
	}
}

void PlayerController::getPosition(mp_sint32& pos, mp_sint32& row)
{
	mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());	
	player->getPosition(pos, row, index);
}

void PlayerController::getPosition(mp_sint32& order, mp_sint32& row, mp_sint32& ticker)
{
	mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());	
	player->getPosition(order, row, ticker, index);
}

void PlayerController::setPatternPos(mp_sint32 pos, mp_sint32 row)
{
	player->setPatternPos(pos, row, false, false);
}

void PlayerController::switchPlayMode(PlayModes playMode, bool exactSwitch/* = true*/)
{
	if (!player)
		return;
	
	switch (playMode)
	{
		case PlayMode_ProTracker2:
			if (exactSwitch)
			{
				player->enable(PlayerSTD::PlayModeOptionPanningE8x, false);
				player->enable(PlayerSTD::PlayModeOptionPanning8xx, false);
				player->enable(PlayerSTD::PlayModeOptionForcePTPitchLimit, true);
			}
			player->setPlayMode(PlayerBase::PlayMode_ProTracker2);
			break;
		case PlayMode_ProTracker3:
			if (exactSwitch)
			{
				player->enable(PlayerSTD::PlayModeOptionPanningE8x, false);
				player->enable(PlayerSTD::PlayModeOptionPanning8xx, false);
				player->enable(PlayerSTD::PlayModeOptionForcePTPitchLimit, true);
			}
			player->setPlayMode(PlayerBase::PlayMode_ProTracker3);
			break;
		case PlayMode_FastTracker2:
			if (exactSwitch)
			{
				player->enable(PlayerSTD::PlayModeOptionPanningE8x, false);
				player->enable(PlayerSTD::PlayModeOptionPanning8xx, true);
				player->enable(PlayerSTD::PlayModeOptionForcePTPitchLimit, false);
			}
			player->setPlayMode(PlayerBase::PlayMode_FastTracker2);
			break;
			
		default:
			ASSERT(false);
	}
	
	//stop();
	//continuePlaying();
}

PlayerController::PlayModes PlayerController::getPlayMode()
{
	if (!player)
		return PlayMode_Auto;
	
	switch (player->getPlayMode())
	{
		case PlayerBase::PlayMode_ProTracker2:
			return PlayMode_ProTracker2;
		case PlayerBase::PlayMode_ProTracker3:
			return PlayMode_ProTracker3;
		case PlayerBase::PlayMode_FastTracker2:
			return PlayMode_FastTracker2;
		default:
			ASSERT(false);	
	}
	
	return PlayMode_Auto;
}

void PlayerController::enablePlayModeOption(PlayModeOptions option, bool b)
{
	switch (option)
	{
		case PlayModeOptionPanning8xx:
			player->enable(PlayerSTD::PlayModeOptionPanning8xx, b);			
			break;
		case PlayModeOptionPanningE8x:
			player->enable(PlayerSTD::PlayModeOptionPanningE8x, b);
			break;
		case PlayModeOptionForcePTPitchLimit:
			player->enable(PlayerSTD::PlayModeOptionForcePTPitchLimit, b);
			break;
		default:
			ASSERT(false);
	}
}

bool PlayerController::isPlayModeOptionEnabled(PlayModeOptions option)
{
	if (!player)
		return false;
	
	switch (option)
	{
		case PlayModeOptionPanning8xx:
			return player->isEnabled(PlayerSTD::PlayModeOptionPanning8xx);			
		case PlayModeOptionPanningE8x:
			return player->isEnabled(PlayerSTD::PlayModeOptionPanningE8x);
		case PlayModeOptionForcePTPitchLimit:
			return player->isEnabled(PlayerSTD::PlayModeOptionForcePTPitchLimit);
		default:
			ASSERT(false);
			return false;
	}
}

mp_sint32 PlayerController::getAllNumPlayingChannels()
{
	if (!player)
		return 0;
		
	return player->mixerNumAllocatedChannels;
}

mp_sint32 PlayerController::getPlayerNumPlayingChannels()
{
	if (!player)
		return 0;
		
	return player->initialNumChannels;
}

mp_sint32 PlayerController::getCurrentSamplePosition()
{
	if (mixer && mixer->getAudioDriver())
		return mixer->getAudioDriver()->getBufferPos();
	
	return 0;
}

mp_sint32 PlayerController::getCurrentBeatIndex()
{
	if (player)
		return player->getBeatIndexFromSamplePos(getCurrentSamplePosition());
	
	return 0;
}

bool PlayerController::isSamplePlaying(const TXMSample& smp, mp_sint32 channel, mp_sint32& pos, mp_sint32& vol, mp_sint32& pan)
{
	if (!player)
		return false;
		
	ChannelMixer* mixer = player;

	// this rather critical
	// maybe someday this entire decision should go into the
	// player or mixer class itself, so I don't need to access it here
	pp_int32 j = getCurrentBeatIndex();
	pos = mixer->channel[channel].timeRecord[j].smppos;
	
	// compare sample from sample editor against sample from current mixer channel
	if (pos >= 0 && 
		(void*)mixer->channel[channel].timeRecord[j].sample == (void*)smp.sample)
	{
		vol = (mixer->channel[channel].timeRecord[j].volPan & 0xFFFF) >> 1;
		pan = (mixer->channel[channel].timeRecord[j].volPan) >> 16;
		return true;
	}
	
	return false;
}

bool PlayerController::isEnvelopePlaying(const TEnvelope& envelope, mp_sint32 envelopeType, mp_sint32 channel, mp_sint32& pos)
{
	if (!player)
		return false;

	ChannelMixer* mixer = player;

	const PlayerSTD::TPrEnv* env = NULL;
	
	switch (envelopeType)
	{
		case 0:
			env = &player->chninfo[channel].venv;
			break;
		case 1:
			env = &player->chninfo[channel].penv;
			break;
	}
	
	pp_int32 j = getCurrentBeatIndex();
	pos = env->timeRecord[j].pos;
	
	if (env && env->timeRecord[j].envstruc && env->timeRecord[j].envstruc == &envelope)
	{
		
		if ((env->timeRecord[j].envstruc->num && 
			 !(env->timeRecord[j].envstruc->type & 4) &&
			 pos >= env->timeRecord[j].envstruc->env[env->timeRecord[j].envstruc->num-1][0]) ||
			!(mixer->channel[channel].timeRecord[j].volPan & 0xFFFF))
		{
			pos = -1;
		}

		return true;
	}
	
	return false;
}

bool PlayerController::isNotePlaying(mp_sint32 ins, mp_sint32 channel, mp_sint32& note, bool& muted)
{
	if (!player)
		return false;

	const PlayerSTD::TModuleChannel* chnInf = &player->chninfo[channel];
	
	if (player->channel[channel].flags&ChannelMixer::MP_SAMPLE_PLAY)
	{
		if (chnInf->ins == ins && chnInf->keyon && chnInf->note)
		{
			muted = (player->channel[channel].flags & ChannelMixer::MP_SAMPLE_MUTE) != 0;
			note = chnInf->note;
			return true;
		}
	}
	
	return false;
}

#define FULLMIXER_8BIT_NORMAL_TEMP \
	if (sample) { \
		sd1 = ((mp_sbyte)sample[smppos])<<8; \
		sd2 = ((mp_sbyte)sample[smppos+1])<<8; \
		sd1 =((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; \
		y = (sd1*vol)>>9; \
	} \
	else { \
		y = 0; \
 	} \
	fetcher.fetchSampleData(y);

#define FULLMIXER_16BIT_NORMAL_TEMP \
	if (sample) { \
		sd1 = ((mp_sword*)(sample))[smppos]; \
		sd2 = ((mp_sword*)(sample))[smppos+1]; \
		sd1 =((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; \
		y = (sd1*vol)>>9; \
	} \
	else { \
		y = 0; \
 	} \
	fetcher.fetchSampleData(y);

void PlayerController::grabSampleData(mp_uint32 chnIndex, mp_sint32 count, mp_sint32 fMul, SampleDataFetcher& fetcher)
{
	if (!player)
		return;	

	if (mixerDataCache && (count * 2 > mixerDataCacheSize))
	{
		delete[] mixerDataCache;
		mixerDataCacheSize = count * 2 * 2;		
		mixerDataCache = new mp_sint32[mixerDataCacheSize];
	}
	
	ChannelMixer* mixer = player;

	ChannelMixer::TMixerChannel* chn = &mixer->channel[chnIndex];
	
	pp_int32 j = getCurrentBeatIndex();

	if (chn->flags & ChannelMixer::MP_SAMPLE_PLAY)
	{
		// this is critical
		// it might be that the audio thread modifies the data as we are
		// accessing it... So in the worst case we're getting a sample 
		// but the channel state data does belong to another sample already
		// in that case we're displaying garbage...
		// BUT it's important that we only access sample data
		// within the range of the current sample we have
		ChannelMixer::TMixerChannel channel;	
		channel.sample = chn->timeRecord[j].sample;
		
		if (channel.sample == NULL)
			goto resort;
		
		channel.smplen = TXMSample::getSampleSizeInSamples((mp_ubyte*)channel.sample);
		channel.flags = chn->timeRecord[j].flags;
		channel.smppos = chn->timeRecord[j].smppos % channel.smplen;
		channel.smpposfrac = chn->timeRecord[j].smpposfrac;
		channel.smpadd = chn->timeRecord[j].smpadd;
		channel.loopend = channel.loopendcopy = chn->timeRecord[j].loopend % (channel.smplen+1);
		channel.loopstart = chn->timeRecord[j].loopstart % (channel.smplen+1);
		if (channel.loopstart >= channel.loopend)
			channel.flags &= ~3;
		channel.vol = chn->timeRecord[j].volPan & 0xFFFF;
		channel.pan = chn->timeRecord[j].volPan >> 16;
		channel.fixedtimefrac = chn->timeRecord[j].fixedtimefrac;
		channel.cutoff = ChannelMixer::MP_INVALID_VALUE;
		channel.resonance = ChannelMixer::MP_INVALID_VALUE;
//		channel.index = chnIndex; Uncomment this if you like crackly audio
		// The scopes (which I assume is what this function is for) are currently
		// using channel 33, to avoid interfering with playback
		
		channel.smpadd = (channel.smpadd*fMul) / (!count ? 1 : count);		
		chn = &channel;
				
		if (mixerDataCache && channel.smpadd <= 65536)
		{
			memset(mixerDataCache, 0, count*2*sizeof(mp_sint32));

			channel.rsmpadd = (mp_sint32)((1.0 / channel.smpadd) * 65536.0);

			// we only need the left channel as no panning is involved
			channel.finalvoll = (channel.vol*128*256)<<6; 
			channel.finalvolr = 0;
			channel.rampFromVolStepL = channel.rampFromVolStepR = 0;
			
			player->getCurrentResampler()->addChannel(chn, mixerDataCache, count, count);
		
			for (mp_sint32 i = 0; i < count; i++)
				fetcher.fetchSampleData(mixerDataCache[i*2]);
		}
		else
		{
			pp_int32 vol = chn->vol;	
			mp_sint32 y;		
			FULLMIXER_TEMPLATE(FULLMIXER_8BIT_NORMAL_TEMP, FULLMIXER_16BIT_NORMAL_TEMP, 16, 0);
		}		
	}
	else
	{
resort:
		for (mp_sint32 i = 0; i < count; i++)
			fetcher.fetchSampleData(0);
	}
}

bool PlayerController::hasSampleData(mp_uint32 chnIndex)
{
	if (!player)
		return false;	

	ChannelMixer* mixer = player;

	ChannelMixer::TMixerChannel* chn = &mixer->channel[chnIndex];

	pp_int32 j = getCurrentBeatIndex();

	return ((chn->timeRecord[j].flags & ChannelMixer::MP_SAMPLE_PLAY) && (chn->timeRecord[j].volPan & 0xFFFF));
}
