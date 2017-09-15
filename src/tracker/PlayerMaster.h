/*
 *  tracker/PlayerMaster.h
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
 *  PlayerMaster.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 18.12.07.
 *
 */

#ifndef __PLAYERMASTER_H__
#define __PLAYERMASTER_H__

#include "BasicTypes.h"
#include "TrackerConfig.h"

struct TMixerSettings
{
	// Negative values means ignore 
	pp_int32 mixFreq;
	// Negative values means ignore 
	pp_int32 bufferSize;
	// Negative values means ignore 
	pp_int32 mixerVolume;
	// Negative values means ignore 
	pp_int32 mixerShift;
	// 0 = false, 1 = true, negative values means ignore 
	pp_int32 powerOfTwoCompensation;
	// 0 = none, 1 = linear, 2 = lagrange, negative values means ignore 
	pp_int32 resampler;
	// 0 = false, 1 = true, negative values means ignore 
	pp_int32 ramping;
	// NULL means ignore
	char* audioDriverName;
    // default number of player channels
    pp_uint32 numPlayerChannels;
	// 0 means disable virtual channels, negative value means ignore
	pp_int32 numVirtualChannels;

	TMixerSettings() :
		mixFreq(-1),
		bufferSize(-1),
		mixerVolume(-1),
		mixerShift(-1),
		powerOfTwoCompensation(-1),
		resampler(-1),
		ramping(-1),
		audioDriverName(NULL),
        numPlayerChannels(TrackerConfig::numPlayerChannels),
		numVirtualChannels(-1)
	{
	}

	~TMixerSettings()
	{
		delete[] audioDriverName; 
	}

	void setAudioDriverName(const char* name)
	{
		delete[] audioDriverName; 
		if (name)
		{
			audioDriverName = new char[strlen(name)+1];
			strcpy(audioDriverName, name);
		}
		else 
			audioDriverName = NULL;
	}

	bool operator==(const TMixerSettings& source)
	{
		if (mixFreq != source.mixFreq)
			return false;

		if (bufferSize != source.bufferSize)
			return false;

		if (mixerVolume != source.mixerVolume)
			return false;

		if (mixerShift != source.mixerShift)
			return false;

		if (powerOfTwoCompensation != source.powerOfTwoCompensation)
			return false;

		if (resampler != source.resampler)
			return false;

		if (ramping != source.ramping)
			return false;

        if (numPlayerChannels != source.numPlayerChannels) {
            return false;
        }
            
		if (numVirtualChannels != source.numVirtualChannels)
			return false;

		return strcmp(audioDriverName, source.audioDriverName) == 0;
	}
	
	bool operator!=(const TMixerSettings& source)
	{
		return !(*this == source);
	}

};

template<class Type>
class PPSimpleVector;
class PlayerController;

class PlayerMaster
{
private:
	enum 
	{
		DefaultMaxDevices = 32,
	};

	class MasterMixer* mixer;
	class MasterMixerNotificationListener* listener;
	PPSimpleVector<PlayerController>* playerControllers;
	
	TMixerSettings currentSettings;
	
	pp_uint32 oldBufferSize;
	bool forcePowerOfTwoBufferSize;

	pp_uint8 panning[TrackerConfig::MAXCHANNELS];

	bool multiChannelKeyJazz;
	bool multiChannelRecord;
	
	void adjustSettings();
	void applySettingsToPlayerController(PlayerController& playerController, const TMixerSettings& settings);
	
public:
	PlayerMaster(pp_uint32 numDevices = DefaultMaxDevices);
	~PlayerMaster();
	
	static const char* getPreferredAudioDriverID();
	static pp_uint32 getPreferredSampleRate();
	static pp_uint32 getPreferredBufferSize();
	static pp_int32 roundToNearestPowerOfTwo(pp_int32 v);	
	static float convertBufferSizeToMillis(pp_uint32 sampleRate, pp_uint32 bufferSize);

	PlayerController* createPlayerController(bool fakeScopes);
	bool destroyPlayerController(PlayerController* playerController);	
	
	pp_int32 getNumPlayerControllers() const;
	PlayerController* getPlayerController(pp_int32 index);
	
	// Delegating some player functionality
	bool applyNewMixerSettings(const TMixerSettings& settings, bool allowMixerRestart);
	
	// Delegating some audio driver enumeration code
	const char* getFirstDriverName() const;
	const char* getNextDriverName() const;
	const char* getCurrentDriverName() const;
	bool setCurrentDriverByName(const char* name);
	
	// this will be delegated to all playercontrollers
	void reallocateChannels(pp_int32 moduleChannels = 32, pp_int32 virtualChannels = 0);
	void setUseVirtualChannels(bool useVirtualChannels);

	// see above
	void setMultiChannelKeyJazz(bool b);
	void setMultiChannelRecord(bool b);
	
	bool start();
	bool stop(bool detachPlayers);
	
	void getCurrentSamplePeak(pp_int32& left, pp_int32& right);
	
	void resetQueuedPositions();
	
	friend class MasterMixerNotificationListener;
};

#endif
