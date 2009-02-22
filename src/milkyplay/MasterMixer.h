/*
 *  milkyplay/MasterMixer.h
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
 *  MasterMixer.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 14.12.07.
 *
 */

#ifndef __MASTERMIXER_H__
#define __MASTERMIXER_H__

#include "Mixable.h"

class MasterMixer
{
public:
	enum MasterMixerNotifications
	{
		MasterMixerNotificationBufferSizeChanged,
		MasterMixerNotificationSampleRateChanged
	};

	class MasterMixerNotificationListener
	{
	public:
		virtual ~MasterMixerNotificationListener()
		{
		}
	
		virtual void masterMixerNotification(MasterMixerNotifications notification) = 0;
	};

	MasterMixer(mp_uint32 sampleRate, 
				mp_uint32 bufferSize = 0, 
				mp_uint32 numDevices = 1,
				class AudioDriverInterface* audioDriver = 0);
	
	virtual ~MasterMixer();
	
	void setMasterMixerNotificationListener(MasterMixerNotificationListener* listener);

	mp_sint32 openAudioDevice();
	mp_sint32 closeAudioDevice();

	mp_sint32 start();
	mp_sint32 stop();
	mp_sint32 pause();
	mp_sint32 resume();
	
	bool isPlaying() const { return started; }
	bool isPaused() const { return paused; }
	bool isInitialized() const { return initialized; }
	bool isActive() const { return started && !paused; }
	
	mp_sint32 setBufferSize(mp_uint32 bufferSize);
	mp_uint32 getBufferSize() const { return bufferSize; }
	
	mp_sint32 setSampleRate(mp_uint32 sampleRate);
	mp_uint32 getSampleRate() const { return sampleRate; }
	
	bool addDevice(Mixable* device, bool paused = false);
	bool removeDevice(Mixable* device, bool blocking = true); 
	bool isDeviceRemoved(Mixable* device);

	bool pauseDevice(Mixable* device, bool blocking = true);
	bool resumeDevice(Mixable* device);
	bool isDevicePaused(Mixable* device);
		
	void mixerHandler(mp_sword* buffer);
	
	// allows to control the loudness of the resulting output stream
	// by bit-shifting the output *right* (dividing by 2^shift)
	void setSampleShift(mp_sint32 shift) { sampleShift = shift; }
	mp_uint32 getSampleShift() const { return sampleShift; }	

	// disable mixing... you don't need to understand this
	void setDisableMixing(bool disableMixing) { this->disableMixing = disableMixing; }
	
	void setFilterHook(Mixable* filterHook) { this->filterHook = filterHook; }
	Mixable* getFilterHook(Mixable* filterHook) const { return filterHook; }
	
	// some legacy functions used by milkytracker
	const class AudioDriverInterface* getAudioDriver() const { return audioDriver; }
	
	const char*	getCurrentAudioDriverName() const;
	bool setCurrentAudioDriverByName(const char* name);

	const class AudioDriverManager* getAudioDriverManager() const;
	
	mp_sint32 getCurrentSample(mp_sint32 position, mp_sint32 channel);
	mp_sint32 getCurrentSamplePeak(mp_sint32 position, mp_sint32 channel);	
			
private:
	MasterMixerNotificationListener* listener;
	mp_uint32 sampleRate;
	mp_uint32 bufferSize;
	mp_sint32* buffer;
	mp_uint32 sampleShift;
	bool disableMixing;
	mp_uint32 numDevices;
	Mixable* filterHook;

	struct DeviceDescriptor
	{
		Mixable* mixable;
		bool markedForRemoval;
		bool markedForPause;
		bool paused;
	
		DeviceDescriptor() :
			mixable(0),
			markedForRemoval(false),
			markedForPause(false),
			paused(false)
		{
		}
	};
	
	DeviceDescriptor* devices;
	
	mutable class AudioDriverManager* audioDriverManager;
	AudioDriverInterface* audioDriver;
	
	bool initialized;
	bool started;
	bool paused;
	
	void notifyListener(MasterMixerNotifications notification);
	
	void cleanup();
	
	inline void prepareBuffer();
	inline void swapOutBuffer(mp_sword* bufferOut);
};

#endif
