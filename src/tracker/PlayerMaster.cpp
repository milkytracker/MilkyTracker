/*
 *  tracker/PlayerMaster.cpp
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
 *  PlayerMaster.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 18.12.07.
 *
 */

#include "PlayerMaster.h"
#include "MasterMixer.h"
#include "SimpleVector.h"
#include "PlayerController.h"
#include "PlayerCriticalSection.h"
#include "AudioDriverManager.h"
#include "PlayerSTD.h"
#include "ResamplerHelper.h"

class MasterMixerNotificationListener : public MasterMixer::MasterMixerNotificationListener
{
private:
	class PlayerMaster& playerMaster;

public:
	MasterMixerNotificationListener(PlayerMaster& playerMaster) :
		playerMaster(playerMaster)
	{
	}

	virtual void masterMixerNotification(MasterMixer::MasterMixerNotifications notification)
	{
		playerMaster.adjustSettings();
	}
};

void PlayerMaster::adjustSettings()
{
	pp_int32 bufferSize = mixer->getBufferSize();
	pp_int32 sampleRate = mixer->getSampleRate();

	for (pp_int32 i = 0; i < playerControllers->size(); i++)
	{		
		PlayerSTD* player = playerControllers->get(i)->player;

		player->setBufferSize(bufferSize);
		player->adjustFrequency(sampleRate);

		if (!player->isPlaying())
			player->resumePlaying(false);
	}
}

void PlayerMaster::applySettingsToPlayerController(PlayerController& playerController, const TMixerSettings& settings)
{
	bool wasPlaying = playerController.player->isPlaying();

	PlayerSTD* player = playerController.player;
	
	if (settings.mixerVolume >= 0)
	{
		player->setMasterVolume(settings.mixerVolume);
	}
	
	mp_sint32 resamplerType = player->getResamplerType();			
	mp_sint32 oldResamplerType = resamplerType;

	if (settings.ramping >= 0)
	{
		// ramping flag is stored in the LSB of the resampler type
		if (settings.ramping != 0)
			resamplerType |= 1;
		else
			resamplerType &= ~1;			
	}
	
	// remember if ramping needs to be set
	bool ramping = (resamplerType & 1) != 0;
	
	// now check if the resampler has changed at all
	if (settings.resampler >= 0)
	{
		// we have a class that translates the resampler index (which maps to a
		// human readable name of the resampler) back to an enum that can be
		// set on the ChannelMixer class
		ResamplerHelper resamplerHelper;
		resamplerType = resamplerHelper.getResamplerType(settings.resampler, ramping);
	}
	
	// now let's see if something has changed at all
	if (resamplerType != oldResamplerType)
	{
		playerController.getCriticalSection()->enter();
		player->setResamplerType((ChannelMixer::ResamplerTypes)resamplerType);	
		playerController.getCriticalSection()->leave();
	}
	
	if (!player->isPlaying() && wasPlaying)
		player->resumePlaying(false);	
}

const char* PlayerMaster::getPreferredAudioDriverID()
{
	AudioDriverManager audioDriverManager;
	return audioDriverManager.getPreferredAudioDriver()->getDriverID();
}

pp_uint32 PlayerMaster::getPreferredSampleRate()
{
	AudioDriverManager audioDriverManager;
	return audioDriverManager.getPreferredAudioDriverSampleRate();
}

pp_uint32 PlayerMaster::getPreferredBufferSize()
{
	AudioDriverManager audioDriverManager;
	return audioDriverManager.getPreferredAudioDriverBufferSize();
}

pp_int32 PlayerMaster::roundToNearestPowerOfTwo(pp_int32 v)
{
	for (mp_uint32 i = 0; i < 32; i++)
	{
		if ((unsigned)(1 << i) >= (unsigned)v)
			return (1 << i);
	}
	
	return v;
}

float PlayerMaster::convertBufferSizeToMillis(pp_uint32 sampleRate, pp_uint32 bufferSize)
{
	return ((float)(bufferSize) / (float)sampleRate) * 1000.0f;
	//return (1.0f / (44100.0f / (MP_BEATLENGTH*bufferSize)))*1000.0f;
}

PlayerMaster::PlayerMaster(pp_uint32 numDevices/* = DefaultMaxDevices*/) :
	listener(NULL),
	oldBufferSize(getPreferredBufferSize()),
	forcePowerOfTwoBufferSize(false),
	multiChannelKeyJazz(true),
	multiChannelRecord(true)	
{
	listener = new MasterMixerNotificationListener(*this);

	mixer = new MasterMixer(44100, getPreferredBufferSize(), numDevices);
	mixer->setMasterMixerNotificationListener(listener);
	mixer->setSampleShift(1);

	playerControllers = new PPSimpleVector<PlayerController>();

	for (pp_uint32 i = 0; i < sizeof(panning) / sizeof(pp_uint8); i++)
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

PlayerMaster::~PlayerMaster()
{
	delete playerControllers;
	delete mixer;
	delete listener;
}

PlayerController* PlayerMaster::createPlayerController(bool fakeScopes)
{
	if (playerControllers->size() >= DefaultMaxDevices)
		return NULL;

	PlayerController* playerController = new PlayerController(mixer, fakeScopes);

	applySettingsToPlayerController(*playerController, currentSettings);

	playerController->setMultiChannelKeyJazz(this->multiChannelKeyJazz);
	playerController->setMultiChannelRecord(this->multiChannelRecord);

	if (currentSettings.numVirtualChannels >= 0)
	{
		if (currentSettings.numVirtualChannels)
		{
			playerController->setUseVirtualChannels(true);
			playerController->reallocateChannels(32, currentSettings.numVirtualChannels);
		}
		else
		{
			playerController->setUseVirtualChannels(false);
			playerController->reallocateChannels(32, 0);
		}
	}

	playerControllers->add(playerController);
	return playerController;
}

bool PlayerMaster::destroyPlayerController(PlayerController* playerController)
{
	for (pp_int32 i = 0; i < playerControllers->size(); i++)
	{
		if (playerControllers->get(i) == playerController)
		{
			playerControllers->remove(i);
			return true;
		}
	}
	
	return false;
}

pp_int32 PlayerMaster::getNumPlayerControllers() const
{
	return playerControllers->size();
}

PlayerController* PlayerMaster::getPlayerController(pp_int32 index)
{
	if (index >= 0 && index < playerControllers->size())
		return playerControllers->get(index);
		
	return NULL;
}

bool PlayerMaster::applyNewMixerSettings(const TMixerSettings& settings, bool allowMixerRestart)
{
	bool res = true;
	bool restart = false;

	if (settings.audioDriverName)
	{
		if (getCurrentDriverName() == NULL ||
			strcmp(getCurrentDriverName(), 
				   settings.audioDriverName) != 0)
		{
			currentSettings.setAudioDriverName(settings.audioDriverName);
		
			mixer->setCurrentAudioDriverByName(settings.audioDriverName);
			restart = true;
		}
	}

	if (settings.mixerShift >= 0)
	{
		currentSettings.mixerShift = settings.mixerShift;
		mixer->setSampleShift(settings.mixerShift);
	}

	if (settings.powerOfTwoCompensation >= 0)
	{
		currentSettings.powerOfTwoCompensation = settings.powerOfTwoCompensation;
		forcePowerOfTwoBufferSize = settings.powerOfTwoCompensation != 0;
		if (oldBufferSize)
		{
			mixer->setBufferSize(forcePowerOfTwoBufferSize ? 
								 roundToNearestPowerOfTwo(oldBufferSize) :
								 oldBufferSize);
			restart = true;
		}
	}

	if (settings.mixFreq >= 0)
	{
		currentSettings.mixFreq = settings.mixFreq;
		mixer->setSampleRate(settings.mixFreq);
		restart = true;
	}

	// this has to take place AFTER setting the frequency
	if (settings.bufferSize >= 0)
	{
		currentSettings.bufferSize = settings.bufferSize;
		oldBufferSize = settings.bufferSize;
		mixer->setBufferSize(forcePowerOfTwoBufferSize ? 
							 roundToNearestPowerOfTwo(oldBufferSize) :
							 oldBufferSize);
		restart = true;
	}
	
	if (settings.mixerVolume >= 0)
		currentSettings.mixerVolume = settings.mixerVolume;

	if (settings.ramping >= 0)
		currentSettings.ramping = settings.ramping;

	if (settings.resampler >= 0)
		currentSettings.resampler = settings.resampler;
	
	// take over settings like sample rate and buffer size 
	// those are retrieved from the master mixer and set for all players
	// accordingly
	adjustSettings();

	// adjust settings which are not dependent by the master mixer
	for (pp_int32 i = 0; i < playerControllers->size(); i++)
		applySettingsToPlayerController(*playerControllers->get(i), currentSettings);

	if (settings.numPlayerChannels != 0 || settings.numVirtualChannels >= 0)
	{
        if (settings.numPlayerChannels != 0)
            currentSettings.numPlayerChannels = settings.numPlayerChannels;

        if (settings.numVirtualChannels >= 0)
            currentSettings.numVirtualChannels = settings.numVirtualChannels;

		if (res)
		{
			if (currentSettings.numVirtualChannels)
			{
				setUseVirtualChannels(true);
				reallocateChannels(currentSettings.numPlayerChannels, currentSettings.numVirtualChannels);
			}
			else
			{
				setUseVirtualChannels(false);
				reallocateChannels(currentSettings.numPlayerChannels, 0);
			}
		}
	}
	
	if (allowMixerRestart && restart && !mixer->isPlaying())
	{
		if (mixer->start() < 0)
			res = false;
	}
	
	return res;
}

const char* PlayerMaster::getFirstDriverName() const
{
	return mixer->getAudioDriverManager()->getFirstDriverName();
}

const char* PlayerMaster::getNextDriverName() const
{
	return mixer->getAudioDriverManager()->getNextDriverName();
}

const char* PlayerMaster::getCurrentDriverName() const
{
	return mixer->getCurrentAudioDriverName();
}

void PlayerMaster::reallocateChannels(mp_sint32 moduleChannels/* = 32*/, mp_sint32 virtualChannels/* = 0*/)
{
	TrackerConfig::numPlayerChannels = moduleChannels;
	TrackerConfig::numVirtualChannels = virtualChannels;
	TrackerConfig::totalPlayerChannels = TrackerConfig::numPlayerChannels + TrackerConfig::numVirtualChannels + 2;

	for (pp_int32 i = 0; i < playerControllers->size(); i++)
	{		
		playerControllers->get(i)->reallocateChannels(moduleChannels, virtualChannels);
	}
}

void PlayerMaster::setUseVirtualChannels(bool useVirtualChannels)
{
	TrackerConfig::useVirtualChannels = useVirtualChannels;

	for (pp_int32 i = 0; i < playerControllers->size(); i++)
	{		
		playerControllers->get(i)->setUseVirtualChannels(useVirtualChannels);
	}
}

void PlayerMaster::setMultiChannelKeyJazz(bool b)
{
	multiChannelKeyJazz = b;
	for (pp_int32 i = 0; i < playerControllers->size(); i++)
	{		
		playerControllers->get(i)->setMultiChannelKeyJazz(b);
	}
}

void PlayerMaster::setMultiChannelRecord(bool b)
{
	multiChannelRecord = b;
	for (pp_int32 i = 0; i < playerControllers->size(); i++)
	{		
		playerControllers->get(i)->setMultiChannelRecord(b);
	}
}


bool PlayerMaster::start()
{
	return mixer->start() == 0;
}

bool PlayerMaster::stop(bool detachPlayers)
{
	// removes playing devices from master mixer before stopping
	// the master mixer audio device
	if (detachPlayers)
	{
		for (pp_int32 i = 0; i < playerControllers->size(); i++)
			playerControllers->get(i)->detachDevice();
	}

	return mixer->stop() == 0;
}
	
void PlayerMaster::getCurrentSamplePeak(pp_int32& left, pp_int32& right)
{
	if (!mixer->isActive())
	{
		left = right = 0;
		return;
	}

	mp_sint32 pos = mixer->getAudioDriver()->getBufferPos();
	
	left = mixer->getCurrentSamplePeak(pos, 0);
	right = mixer->getCurrentSamplePeak(pos, 1);
}

void PlayerMaster::resetQueuedPositions()
{
	for (pp_int32 i = 0; i < playerControllers->size(); i++)
	{		
		playerControllers->get(i)->setNextOrderToPlay(-1);
		playerControllers->get(i)->setNextPatternToPlay(-1);
	}
}

