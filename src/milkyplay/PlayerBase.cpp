/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  PlayerBase.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on Thu Jan 20 2005.
 *
 */

#include "PlayerBase.h"
#include "XModule.h"

mp_sint32 PlayerBase::kick()
{
	// if the player hasn't been initialized until now => DO IT
	if (!isInitialized())
	{
		mp_sint32 err = initDevice();		
		if (err != MP_OK)
		{
			return err;
		}
	}
	
	// - configure player: --------------
	// playing => song is not paused yet
	paused = false;
	// playing => song has not stopped yet
	halted = false;
	// set idle mode
	setIdle(idle);
	
	// - configure mixer: ---------------
	// mixer reset
	resetChannelsWithoutMuting();
	
	// start playing (mixer flag)
	startPlay = true;
	
	// mix buffers
	startMixer();
	
	// reset sample counters
	sampleCounter = 0;
	return MP_OK;
}

PlayerBase::PlayerBase(mp_uint32 frequency) : 
	ChannelMixer(32, frequency),
	timeRecord(NULL)
{
	module = NULL;
	
	initialNumChannels = 8;
	
	mainVolume	= 255;
	
	rowcnt		= 0;				// counts through each row in a pattern
	poscnt		= 0;				// counts through each index the pattern index table
	synccnt		= 0;
	lastUnvisitedPos = 0;
	
	startPlay						= false;
	paused							= false;
	halted							= false;
	idle							= false;
	resetOnStopFlag					= false;
	resetMainVolumeOnStartPlayFlag	= true;
	
	adder = BPMCounter = 0;

	patternIndexToPlay = -1;
	
	playMode = PlayMode_Auto;	
	
	reallocTimeRecord();
}

PlayerBase::~PlayerBase()
{ 
	//if (isPlaying())
	//	stopPlaying();
	
	ChannelMixer::closeDevice(); 
	
	delete[] timeRecord;
}

mp_sint32 PlayerBase::adjustFrequency(mp_uint32 frequency)
{
	mp_uint32 lastNumBeatPackets = getNumBeatPackets()+1;

	mp_sint32 res = ChannelMixer::adjustFrequency(frequency);
	
	if (res < 0)
		return res;
	
	// nothing has changed
	if (lastNumBeatPackets == getNumBeatPackets()+1)
		return MP_OK;
				
	reallocTimeRecord();
	
	return MP_OK;
}

mp_sint32 PlayerBase::setBufferSize(mp_uint32 bufferSize)
{
	mp_uint32 lastNumBeatPackets = getNumBeatPackets()+1;

	mp_sint32 res = ChannelMixer::setBufferSize(bufferSize);
	
	if (res < 0)
		return res;
		
	// nothing has changed
	if (lastNumBeatPackets == getNumBeatPackets()+1)
		return MP_OK;

	reallocTimeRecord();
	
	return MP_OK;
}

void PlayerBase::restart(mp_uint32 startPosition/* = 0*/, mp_uint32 startRow/* = 0*/, bool resetMixer/* = true*/, const mp_ubyte* customPanningTable/* = NULL*/, bool playOneRowOnly /* = false*/)
{
	if (module == NULL) 
		return;

	if (resetMixer)
		resetChannelsWithoutMuting();

	// initialise crappy global variables
	baseBpm = 125;
	
	halted = false;
	
	synccnt = 0;
	rowcnt = startRow;
	poscnt = startPosition;
	lastUnvisitedPos = poscnt;
	
	synccnt			= 0;

	this->playOneRowOnly = playOneRowOnly;

	if (resetMainVolumeOnStartPlayFlag)
		mainVolume = module->header.mainvol;

	// Clear position/speed lookup tables
	updateTimeRecord();
}

//////////////////////////////////////////////////////
// setup mixer and start playing
//////////////////////////////////////////////////////
mp_sint32 PlayerBase::startPlaying(XModule *module,
							   bool repeat /* = false*/,
							   mp_uint32 startPosition /* = 0*/, 
							   mp_uint32 startRow /* = 0*/,
							   mp_sint32 numChannels /* = -1*/,
							   const mp_ubyte* customPanningTable /* = NULL*/,
							   bool idle /* = false*/,
							   mp_sint32 patternIndex /* = -1*/,
							   bool playOneRowOnly /* = false*/)
{
	this->module = module;
	
	if (numChannels == -1)
		initialNumChannels = module->header.channum;
	else
		initialNumChannels = numChannels;

	ChannelMixer::setNumChannels(initialNumChannels);	

	this->idle = idle;
	this->repeat = repeat;
	
	mp_sint32 res = allocateStructures();
	
	if (res != MP_OK)
		return res;

	patternIndexToPlay = patternIndex;

	restart(startPosition, startRow, true, customPanningTable, playOneRowOnly);

	return PlayerBase::kick();	
}

mp_sint32 PlayerBase::stopPlaying()
{
	stop();
	
	mp_sint32 err = closeDevice();
	
	module = NULL;
	
	return err;
}

mp_sint32 PlayerBase::pausePlaying()
{
	if (!paused)
	{
		ChannelMixer::pause();
		
		paused = true;
	}
	return MP_OK;
}

mp_sint32 PlayerBase::resumePlaying(bool unpause/* = true*/)
{
	if (paused && unpause)
	{
		paused = false;
		return resume();
	}
	
	if (module) 
	{
		
		// if the player hasn't been initialized until now => DO IT
		if (!isInitialized())
		{
			mp_sint32 err = initDevice();
			if (err != MP_OK)
			{
				return err;
			}
		}
		
		startMixer();
		
		startPlay = true;
		
	}
	
	return MP_OK;
}

void PlayerBase::nextPattern()
{
	if (!module) 
		return;

	if (startPlay && !paused)
	{
		if (poscnt<module->header.ordnum-1)
		{
			ChannelMixer::resetChannelsWithoutMuting();			
			rowcnt = 0;
			poscnt++;
			lastUnvisitedPos = poscnt;
			clearEffectMemory();
		}
	}

}

void PlayerBase::lastPattern()
{
	if (!module) 
		return;

	if (startPlay && !paused)
	{
		if (poscnt>0)
		{
			//memset(chninfo,0,sizeof(TModuleChannel)*module->header.channum);
			ChannelMixer::resetChannelsWithoutMuting();
			rowcnt = 0;
			poscnt--;
			lastUnvisitedPos = poscnt;
			clearEffectMemory();
		}
	}
}

void PlayerBase::setPatternPos(mp_uint32 pos, mp_uint32 row/* = 0*/, bool resetChannels/* = true*/, bool resetFXMemory/* = true*/)
{
	
	if (!module) 
		return;

	if (startPlay && !paused && (pos < module->header.ordnum))
	{
		//memset(chninfo,0,sizeof(TModuleChannel)*module->header.channum);
		if (resetChannels)
			ChannelMixer::resetChannelsWithoutMuting();

		poscnt = pos;
		rowcnt = row;
		lastUnvisitedPos = poscnt;
		
		updateTimeRecord();
		
		if (resetFXMemory)
			clearEffectMemory();
	}
}


void PlayerBase::timerHandler(mp_sint32 currentBeatPacket)
{
	timeRecord[currentBeatPacket] = TimeRecord(poscnt, 
											   rowcnt, 
											   bpm, 
											   tickSpeed, 
											   mainVolume,
											   ticker);
}
