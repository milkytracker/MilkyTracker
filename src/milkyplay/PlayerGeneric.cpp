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
 *  PlayerGeneric.cpp
 *  MilkyPlay 
 *
 *  PlayerGeneric is a wrapper that allocates a suiting type of player
 *	for a module while providing the same player interfaces.
 *	Currently there are three types of players: PlayerFAR, PlayerSTD and PlayerIT
 *
 */
#include "PlayerGeneric.h"
#include "MasterMixer.h"
#include "XModule.h"
#include "AudioDriver_WAVWriter.h"
#include "AudioDriverManager.h"
#include "PlayerBase.h"
#include "PlayerSTD.h"
#ifndef MILKYTRACKER
#include "PlayerIT.h"
#include "PlayerFAR.h"
#endif

#undef __VERBOSE__

class MixerNotificationListener : public MasterMixer::MasterMixerNotificationListener
{
private:
	class PlayerGeneric& player;

public:
	MixerNotificationListener(PlayerGeneric& player) :
		player(player)
	{
	}

	virtual void masterMixerNotification(MasterMixer::MasterMixerNotifications notification)
	{
		player.adjustSettings();
	}
};

void PlayerGeneric::adjustSettings()
{
	mp_uint32 bufferSize = mixer->getBufferSize();
	mp_uint32 sampleRate = mixer->getSampleRate();
	
	this->bufferSize = bufferSize;
	this->frequency = sampleRate;
	
	if (player)
	{
		player->setBufferSize(bufferSize);
		player->adjustFrequency(sampleRate);
	}
}

PlayerBase::PlayerTypes PlayerGeneric::getPreferredPlayerType(XModule* module)
{
	if (module == NULL)
		return PlayerBase::PlayerType_INVALID;
		
	switch (module->getType())
	{
		case XModule::ModuleType_669:
		case XModule::ModuleType_FAR:
#ifndef MILKYTRACKER
			return PlayerBase::PlayerType_FAR;
			break;
#endif
		case XModule::ModuleType_IT:
#ifndef MILKYTRACKER
			return PlayerBase::PlayerType_IT;
			break;
#endif
		case XModule::ModuleType_UNKNOWN: // just assume our standard player can handle this
		//case XModule::ModuleType_669:
		case XModule::ModuleType_AMF:
		case XModule::ModuleType_AMS:
		case XModule::ModuleType_CBA:
		case XModule::ModuleType_DBM:
		case XModule::ModuleType_DIGI:
		case XModule::ModuleType_DSM:
		case XModule::ModuleType_DSm:
		case XModule::ModuleType_DTM_1:
		case XModule::ModuleType_DTM_2:
		case XModule::ModuleType_GDM:
		case XModule::ModuleType_GMC:
		case XModule::ModuleType_IMF:
		case XModule::ModuleType_MDL:
		case XModule::ModuleType_MOD:
		case XModule::ModuleType_MTM:
		case XModule::ModuleType_MXM:
		case XModule::ModuleType_OKT:
		case XModule::ModuleType_PLM:
		case XModule::ModuleType_PSM:
		case XModule::ModuleType_PTM:
		case XModule::ModuleType_S3M:
		case XModule::ModuleType_STM:
		case XModule::ModuleType_SFX:
		case XModule::ModuleType_UNI:
		case XModule::ModuleType_ULT:
		case XModule::ModuleType_XM:
		case XModule::ModuleType_NONE:
			return PlayerBase::PlayerType_Generic;
			break;
			
		default:
			return PlayerBase::PlayerType_INVALID;
	}
}

PlayerBase*	PlayerGeneric::getPreferredPlayer(XModule* module) const
{
	switch (getPreferredPlayerType(module))
	{
#ifndef MILKYTRACKER
		case PlayerBase::PlayerType_FAR:
			return new PlayerFAR(frequency);
		case PlayerBase::PlayerType_IT:
			return new PlayerIT(frequency);
#endif
		case PlayerBase::PlayerType_Generic:
			return new PlayerSTD(frequency);
			
		default:
			return NULL;
	}
}

PlayerGeneric::PlayerGeneric(mp_sint32 frequency, AudioDriverInterface* audioDriver/* = NULL*/) :
	mixer(NULL),
	player(NULL),
	frequency(frequency),
	audioDriver(audioDriver),
	audioDriverName(NULL)
{
	listener = new MixerNotificationListener(*this);

	bufferSize = 0;
	sampleShift = 0;
	
	resamplerType = MIXER_NORMAL;

	idle = false;
	playOneRowOnly = false;
	paused = false;
	repeat = false;
	resetOnStopFlag = false;
	autoAdjustPeak = false;
	disableMixing = false;
	allowFilters = false;
#ifdef __FORCEPOWEROFTWOBUFFERSIZE__
	compensateBufferFlag = true;
#else
	compensateBufferFlag = false;
#endif
	masterVolume = panningSeparation = numMaxVirChannels = 256;
	resetMainVolumeOnStartPlayFlag = true;
	playMode = PlayMode_Auto;

	// Special playmode settings
	options[PlayModeOptionPanning8xx] = true;
	options[PlayModeOptionPanningE8x] = false;
	options[PlayModeOptionForcePTPitchLimit] = true;

	AudioDriverManager audioDriverManager;
	const char* defaultName = audioDriverManager.getPreferredAudioDriver()->getDriverID();
	if (defaultName)
	{
		audioDriverName = new char[strlen(defaultName)+1];
		strcpy(audioDriverName, defaultName);
	}
}
	
PlayerGeneric::~PlayerGeneric()
{
	if (mixer)
		delete mixer;

	if (player)
	{
		if (mixer->isActive() && !mixer->isDeviceRemoved(player))
			mixer->removeDevice(player);
		delete player;
	}

	delete[] audioDriverName;
	
	delete listener;
}

// -- wrapping mixer specific stuff ----------------------
void PlayerGeneric::setResamplerType(ResamplerTypes type)
{
	resamplerType = type;
	if (player)
		player->setResamplerType(type);
}

void PlayerGeneric::setResamplerType(bool interpolation, bool ramping)
{
	if (interpolation)
	{
		if (ramping)
			resamplerType = MIXER_LERPING_RAMPING;
		else
			resamplerType = MIXER_LERPING;
	}
	else
	{
		if (ramping)
			resamplerType = MIXER_NORMAL_RAMPING;
		else
			resamplerType = MIXER_NORMAL;
	}

	if (player)
		player->setResamplerType(resamplerType);
}

ChannelMixer::ResamplerTypes PlayerGeneric::getResamplerType() const
{
	if (player)
		return player->getResamplerType();
		
	return resamplerType;
}
	
void PlayerGeneric::setSampleShift(mp_sint32 shift)
{
	sampleShift = shift;
	if (mixer)
		mixer->setSampleShift(shift);
}

mp_sint32 PlayerGeneric::getSampleShift() const
{
	if (mixer)
		return mixer->getSampleShift();
	
	return sampleShift;
}

void PlayerGeneric::setPeakAutoAdjust(bool b)
{
	this->autoAdjustPeak = b;
}

mp_sint32 PlayerGeneric::adjustFrequency(mp_uint32 frequency)
{
	this->frequency = frequency;
	
	mp_sint32 res = MP_OK;
	
	if (mixer)
		res = mixer->setSampleRate(frequency);
	
	return res;
}

mp_sint32 PlayerGeneric::getMixFrequency() const
{
	if (player)
		return player->getMixFrequency();
		
	return frequency;
}

mp_sint32 PlayerGeneric::beatPacketsToBufferSize(mp_uint32 numBeats)
{
	return ChannelMixer::beatPacketsToBufferSize(getMixFrequency(), numBeats);
}

mp_sint32 PlayerGeneric::adjustBufferSize(mp_uint32 numBeats)
{
	return setBufferSize(beatPacketsToBufferSize(numBeats));
}

mp_sint32 PlayerGeneric::setBufferSize(mp_uint32 bufferSize)
{
	mp_sint32 res = 0;
	
	this->bufferSize = bufferSize;
	
	if (mixer)
	{
		// If we're told to compensate the samples until we 
		// we reached 2^n buffer sizes
		if (compensateBufferFlag)
		{
			for (mp_uint32 i = 0; i < 16; i++)
			{
				if ((unsigned)(1 << i) >= (unsigned)bufferSize)
				{
					bufferSize = 1 << i;
					break;
				}
			}
		}		
	
		res = mixer->setBufferSize(bufferSize);
	}
	
	return res;
}
	
mp_sint32 PlayerGeneric::setPowerOfTwoCompensationFlag(bool b)
{
	if (mixer && compensateBufferFlag != b)
	{
		compensateBufferFlag = b;
		setBufferSize(bufferSize);
	}

	return MP_OK;
}

bool PlayerGeneric::getPowerOfTwoCompensationFlag() const
{
	return compensateBufferFlag;
}

const char*	PlayerGeneric::getCurrentAudioDriverName() const
{
	if (mixer)
		return mixer->getCurrentAudioDriverName();

	return audioDriverName;
}
	
bool PlayerGeneric::setCurrentAudioDriverByName(const char* name)
{
	if (name == NULL)
		return false;

	if (mixer)
	{
		bool res = mixer->setCurrentAudioDriverByName(name);

		if (audioDriverName)
			delete[] audioDriverName;

		const char* curDrvName = getCurrentAudioDriverName();
		ASSERT(curDrvName);
		audioDriverName = new char[strlen(curDrvName)+1];
		strcpy(audioDriverName, curDrvName);
		return res;
	}

	AudioDriverManager audioDriverManager;
	if (audioDriverManager.getAudioDriverByName(name))
	{
		if (audioDriverName)
			delete[] audioDriverName;

		audioDriverName = new char[strlen(name)+1];
		strcpy(audioDriverName, name);		
		return true;
	}

	return false;
}


bool PlayerGeneric::isInitialized() const
{
	if (mixer)
		return mixer->isInitialized();
		
	return false;
}

bool PlayerGeneric::isPlaying() const
{
	if (mixer)
		return mixer->isPlaying();
		
	return false;
}
	
mp_int64 PlayerGeneric::getSampleCounter() const
{
	if (player)
		return player->getSampleCounter();
	
	return 0;
}

void PlayerGeneric::resetSampleCounter()
{
	if (player)
		player->resetSampleCounter();
}
	
mp_sint32 PlayerGeneric::getCurrentSamplePosition() const
{
	if (mixer && mixer->getAudioDriver())
		return mixer->getAudioDriver()->getBufferPos();
	
	return 0;
}

mp_sint32 PlayerGeneric::getCurrentBeatIndex()
{
	if (player)
		return player->getBeatIndexFromSamplePos(getCurrentSamplePosition());
	
	return 0;
}
	
mp_sint32 PlayerGeneric::getCurrentSample(mp_sint32 position, mp_sint32 channel)
{
	if (mixer)
		return mixer->getCurrentSample(position, channel);
	
	return 0;
}

mp_sint32 PlayerGeneric::getCurrentSamplePeak(mp_sint32 position, mp_sint32 channel)
{
	if (mixer)
		return mixer->getCurrentSamplePeak(position, channel);

	return 0;
}

void PlayerGeneric::resetChannels()
{
	if (player)
		player->resetChannelsFull();
}

mp_sint32 PlayerGeneric::getNumAllocatedChannels() const
{
	if (player)
		return player->getNumAllocatedChannels();

	return 0;
}

mp_sint32 PlayerGeneric::getNumActiveChannels() const
{
	if (player)
		return player->getNumActiveChannels();

	return 0;
}

// -- wrapping player specific stuff ----------------------
void PlayerGeneric::setPlayMode(PlayModes mode)
{
	playMode = mode;
	if (player)
		player->setPlayMode(mode);
}

PlayerGeneric::PlayModes PlayerGeneric::getPlayMode() const
{
	if (player)
		return player->getPlayMode();
		
	return playMode;
}

void PlayerGeneric::enable(PlayModeOptions option, bool b)
{
	ASSERT(option>=PlayModeOptionFirst && option<PlayModeOptionLast);
	options[option] = b;
	
	if (player)
		player->enable(option, b);
}

bool PlayerGeneric::isEnabled(PlayModeOptions option) const
{
	ASSERT(option>=PlayModeOptionFirst && option<PlayModeOptionLast);
	
	if (!player)
		return options[option];
	else 
		return player->isEnabled(option);
}

void PlayerGeneric::restart(mp_uint32 startPosition/* = 0*/, mp_uint32 startRow/* = 0*/, bool resetMixer/* = true*/, const mp_ubyte* customPanningTable/* = NULL*/, bool playOneRowOnly/* = false*/)
{
	if (player)
		player->restart(startPosition, startRow, resetMixer, customPanningTable, playOneRowOnly);
}

void PlayerGeneric::reset()
{
	if (player)
		player->reset();
}

void PlayerGeneric::resetAllSpeed()
{
	if (player)
		player->resetAllSpeed();
}

mp_sint32 PlayerGeneric::startPlaying(XModule* module, 
									  bool repeat/* = false*/, 
									  mp_uint32 startPosition/* = 0*/, 
									  mp_uint32 startRow/* = 0*/,
									  mp_sint32 numChannels/* = -1*/, 
									  const mp_ubyte* customPanningTable/* = NULL*/,
									  bool idle/* = false*/,
									  mp_sint32 patternIndex/* = -1*/,
									  bool playOneRowOnly/* = false*/)
{
	this->idle = idle;
	this->repeat = repeat;
	this->playOneRowOnly = playOneRowOnly;

	if (mixer == NULL)
	{
		mixer = new MasterMixer(frequency, bufferSize, 1, audioDriver);
		mixer->setMasterMixerNotificationListener(listener);
		mixer->setSampleShift(sampleShift);
		if (audioDriver == NULL)
			mixer->setCurrentAudioDriverByName(audioDriverName);
	}

	if (!player || player->getType() != getPreferredPlayerType(module))
	{
		if (player)
		{
			if (!mixer->isDeviceRemoved(player))
				mixer->removeDevice(player);
			delete player;
		}
		
		player = getPreferredPlayer(module);
		
		if (player)
		{
			// apply our own "state" to the state of the newly allocated player
			player->resetMainVolumeOnStartPlay(resetMainVolumeOnStartPlayFlag);
			player->resetOnStop(resetOnStopFlag);
			player->setBufferSize(bufferSize);
			player->setResamplerType(resamplerType);
			player->setMasterVolume(masterVolume);
			player->setPanningSeparation(panningSeparation);
			player->setPlayMode(playMode);

			for (mp_sint32 i = PlayModeOptionFirst; i < PlayModeOptionLast; i++)
				player->enable((PlayModeOptions)i, options[i]);			
			
			player->setDisableMixing(disableMixing);
			player->setAllowFilters(allowFilters);
			//if (paused)
			//	player->pausePlaying();

			// adjust number of virtual channels if necessary
			setNumMaxVirChannels(numMaxVirChannels);
		}		
	}
	
	if (player && mixer)
	{
		if (!mixer->isDeviceRemoved(player))
			mixer->removeDevice(player);
			
		player->startPlaying(module, repeat, startPosition, startRow, numChannels, customPanningTable, idle, patternIndex, playOneRowOnly);
		
		mixer->addDevice(player);
		
		if (!mixer->isPlaying())
			return mixer->start();
	}


	return MP_OK;
}

void PlayerGeneric::setPatternToPlay(mp_sint32 patternIndex)
{
	if (player)
		player->setPatternToPlay(patternIndex);
}

mp_sint32 PlayerGeneric::stopPlaying()
{
	if (player)
		player->stopPlaying();

	if (mixer)
		return mixer->stop();
		
	return MP_OK;
}

bool PlayerGeneric::hasSongHalted() const
{
	if (player)
		return player->hasSongHalted();
		
	return true;
}

void PlayerGeneric::setIdle(bool idle)
{
	this->idle = idle;
	if (player)
		player->setIdle(idle);
}

bool PlayerGeneric::isIdle() const
{
	if (player)
		return player->isIdle();
		
	return idle;
}

void PlayerGeneric::setRepeat(bool repeat)
{
	this->repeat = repeat;
	if (player)
		player->setRepeat(repeat);
}

bool PlayerGeneric::isRepeating() const
{
	if (player)
		return player->isRepeating();
		
	return repeat;
}
	
mp_sint32 PlayerGeneric::pausePlaying()
{
	paused = true;
	if (mixer)
		return mixer->pause();
		
	return MP_OK;
}

mp_sint32 PlayerGeneric::resumePlaying()
{
	if (player && !player->isPlaying())
		player->resumePlaying();

	if (mixer && mixer->isPaused())
		return mixer->resume();
	else if (mixer && !mixer->isPlaying())
		return mixer->start();

	return MP_OK;
}

bool PlayerGeneric::isPaused() const
{
	if (mixer)
		return mixer->isPaused();
		
	return paused;
}

void PlayerGeneric::setDisableMixing(bool b)
{
	disableMixing = b;

	if (player)
		player->setDisableMixing(disableMixing);
}

void PlayerGeneric::setAllowFilters(bool b)
{
	allowFilters = b;

	if (player)
		player->setAllowFilters(allowFilters);
}

bool PlayerGeneric::getAllowFilters() const
{
	if (player)
		return player->getAllowFilters();
		
	return allowFilters;
}

// volume control
void PlayerGeneric::setMasterVolume(mp_sint32 vol)
{
	masterVolume = vol;
	if (player)
		player->setMasterVolume(vol);
}

mp_sint32 PlayerGeneric::getMasterVolume() const
{
	if (player)
		return player->getMasterVolume();
		
	return masterVolume;
}

// panning control
void PlayerGeneric::setPanningSeparation(mp_sint32 separation)
{
	panningSeparation = separation;
	if (player)
		player->setPanningSeparation(separation);
}

mp_sint32 PlayerGeneric::getPanningSeparation() const
{
	if (player)
		return player->getPanningSeparation();
		
	return panningSeparation;
}

mp_sint32 PlayerGeneric::getSongMainVolume() const
{
	if (player)
	{
		mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());
		return player->getSongMainVolume(index);
	}
	
	return 255;
}

mp_sint32 PlayerGeneric::getRow() const
{
	if (player)
	{
		mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());
		return player->getRow(index);
	}
	
	return 0;
}

mp_sint32 PlayerGeneric::getOrder() const
{
	if (player)
	{
		mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());
		return player->getOrder(index);
	}
		
	return 0;
}

void PlayerGeneric::getPosition(mp_sint32& order, mp_sint32& row) const
{
	if (player)
	{
		mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());	
		player->getPosition(order, row, index);
		return;
	}
	
	order = row = 0;
}

mp_sint32 PlayerGeneric::getLastUnvisitedPosition() const
{
	if (player)
		return player->getLastUnvisitedPosition();
	
	return 0;
}

void PlayerGeneric::getPosition(mp_sint32& order, mp_sint32& row, mp_sint32& ticker) const
{
	if (player)
	{
		mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());
		player->getPosition(order, row, ticker, index);
		return;
	}
	
	order = row = ticker = 0;
}

mp_int64 PlayerGeneric::getSyncCount() const
{
	if (player)
		return player->getSyncCount();
		
	return 0;
}

mp_uint32 PlayerGeneric::getSyncSampleCounter() const
{
	if (player)
		return player->getSyncSampleCounter();
		
	return 0;
}

void PlayerGeneric::nextPattern()
{
	if (player)
		player->nextPattern();
}

void PlayerGeneric::lastPattern()
{
	if (player)
		player->lastPattern();
}

void PlayerGeneric::setPatternPos(mp_uint32 pos, mp_uint32 row/* = 0*/, bool resetChannels/* = true*/, bool resetFXMemory/* = true*/)
{
	if (player)
		player->setPatternPos(pos, row, resetChannels, resetFXMemory);
}

mp_sint32 PlayerGeneric::getTempo() const
{
	if (player)
	{
		mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());
		return player->getTempo(index);
	}
		
	return 0;
}

mp_sint32 PlayerGeneric::getSpeed() const
{
	if (player)
	{
		mp_uint32 index = player->getBeatIndexFromSamplePos(getCurrentSamplePosition());
		return player->getSpeed(index);
	}
		
	return 0;
}

void PlayerGeneric::resetOnStop(bool b) 
{
	resetOnStopFlag = b;
	if (player)
		player->resetOnStop(b); 
}

void PlayerGeneric::resetMainVolumeOnStartPlay(bool b)
{
	resetMainVolumeOnStartPlayFlag = b;
	if (player)
		player->resetMainVolumeOnStartPlay(b); 
}

struct PeakAutoAdjustFilter : public Mixable
{
	mp_uint32 mixerShift;
	mp_uint32 masterVolume;
	mp_sint32 lastPeakValue;
	
	PeakAutoAdjustFilter() : 
		mixerShift(0),
		masterVolume(256),
		lastPeakValue(0)
	{
	}
	
	virtual void mix(mp_sint32* buffer, mp_uint32 bufferSize)
	{
		const mp_sint32* buffer32 = buffer;
		
		for (mp_uint32 i = 0; i < bufferSize*MP_NUMCHANNELS; i++)
		{
			mp_sint32 b = *buffer32++;
			
			if (abs(b) > lastPeakValue)
				lastPeakValue = abs(b);					
		}
	}
	
	void calculateMasterVolume()
	{
		if (lastPeakValue)
		{
			float v = 32768.0f*(1<<mixerShift) / (float)lastPeakValue;
			masterVolume = (mp_sint32)((float)masterVolume*v);
			if (masterVolume > 256)
				masterVolume = 256;
		}
	}
};

// export to 16bit stereo WAV
mp_sint32 PlayerGeneric::exportToWAV(const SYSCHAR* fileName, XModule* module, 
									 mp_sint32 startOrder/* = 0*/, mp_sint32 endOrder/* = -1*/, 
									 const mp_ubyte* mutingArray/* = NULL*/, mp_uint32 mutingNumChannels/* = 0*/,
									 const mp_ubyte* customPanningTable/* = NULL*/,
									 AudioDriverBase* preferredDriver/* = NULL*/,
									 mp_sint32* timingLUT/* = NULL*/)
{
	PlayerBase* player = NULL;
	
	AudioDriverBase* wavWriter = preferredDriver;
	bool isWAVWriterDriver = false;
	
	if (wavWriter == NULL)
	{
		wavWriter = new WAVWriter(fileName);
		isWAVWriterDriver = true;
	
		if (!static_cast<WAVWriter*>(wavWriter)->isOpen())
		{
			delete wavWriter;
			return MP_DEVICE_ERROR;
		}
	}
	
	MasterMixer mixer(frequency, bufferSize, 1, wavWriter);
	mixer.setSampleShift(sampleShift);
	mixer.setDisableMixing(disableMixing);
	
	player = getPreferredPlayer(module);
	
	PeakAutoAdjustFilter filter;
	if (autoAdjustPeak)
		mixer.setFilterHook(&filter);
		
	if (player)
	{
		player->adjustFrequency(frequency);
		player->resetOnStop(resetOnStopFlag);
		player->setBufferSize(bufferSize);
		player->setResamplerType(resamplerType);
		player->setMasterVolume(masterVolume);
		player->setPlayMode(playMode);
		player->setDisableMixing(disableMixing);
		player->setAllowFilters(allowFilters);		
#ifndef MILKYTRACKER
		if (player->getType() == PlayerBase::PlayerType_IT)
		{
			static_cast<PlayerIT*>(player)->setNumMaxVirChannels(numMaxVirChannels);
		}
#endif
		mixer.addDevice(player);
	}

	if (player)
	{
		if (mutingArray && mutingNumChannels > 0 && mutingNumChannels <= module->header.channum)
		{
			for (mp_uint32 i = 0; i < mutingNumChannels; i++)
				player->muteChannel(i, mutingArray[i] == 1);
		}
		player->startPlaying(module, false, startOrder, 0, -1, customPanningTable, false, -1);
		
		mixer.start();
	}

	if (endOrder == -1 || endOrder < startOrder || endOrder > module->header.ordnum - 1)
		endOrder = module->header.ordnum - 1;		

	mp_sint32 curOrderPos = startOrder;
	if (timingLUT)
	{
		for (mp_sint32 i = 0; i < module->header.ordnum; i++)
			timingLUT[i] = -1;
		
		timingLUT[curOrderPos] = 0;
	}

	while (!player->hasSongHalted() && player->getOrder(0) <= endOrder)
	{
		wavWriter->advance();

		if (player->getOrder(0) != curOrderPos)
		{
#ifdef __VERBOSE__
			printf("%f\n", (float)wavWriter->getNumPlayedSamples() / (float)getMixFrequency());
#endif
			curOrderPos = player->getOrder(0);
			if (timingLUT && curOrderPos < module->header.ordnum && timingLUT[curOrderPos] == -1)
				timingLUT[curOrderPos] = wavWriter->getNumPlayedSamples();			
		}
	}

	player->stopPlaying();
	
	mixer.stop();
	// important step, otherwise destruction of the audio driver will cause
	// trouble if the mixer instance is removed from this function's stack 
	// and trys to access the driver which is no longer existant
	mixer.closeAudioDevice();

	// Sync value
	sampleShift = mixer.getSampleShift();
	filter.mixerShift = sampleShift;
	filter.calculateMasterVolume();
	masterVolume = filter.masterVolume;

	delete player;

	mp_sint32 numWrittenSamples = wavWriter->getNumPlayedSamples();
	
	if (isWAVWriterDriver)
		delete wavWriter;

	return numWrittenSamples;
}

bool PlayerGeneric::grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const
{
	if (player)
		return player->grabChannelInfo(chn, channelInfo);
		
	return false;
}

void PlayerGeneric::setNumMaxVirChannels(mp_sint32 max)
{
	numMaxVirChannels = max;
#ifndef MILKYTRACKER
	if (player)
	{
		if (player->getType() == PlayerBase::PlayerType_IT)
		{
			static_cast<PlayerIT*>(player)->setNumMaxVirChannels(max);
		}
	}
#endif
}

mp_sint32 PlayerGeneric::getNumMaxVirChannels() const
{
#ifndef MILKYTRACKER
	if (player)
	{
		if (player->getType() == PlayerBase::PlayerType_IT)
		{
			return static_cast<PlayerIT*>(player)->getNumMaxVirChannels();
		}
	}
#endif
	return numMaxVirChannels;
}

// milkytracker
void PlayerGeneric::setPanning(mp_ubyte chn, mp_ubyte pan)
{
	if (player)
		player->setPanning(chn, pan);
}
