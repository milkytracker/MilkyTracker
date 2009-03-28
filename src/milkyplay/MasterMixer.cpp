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
 *  MasterMixer.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 14.12.07.
 *
 */

#include "MasterMixer.h"
#include "MilkyPlayCommon.h"
#include "AudioDriverBase.h"
#include "AudioDriverManager.h"

enum
{
	BlockTimeOut = 5000
};

MasterMixer::MasterMixer(mp_uint32 sampleRate, 
						 mp_uint32 bufferSize/* = 0*/, 
						 mp_uint32 numDevices/* = 1*/,
						 AudioDriverInterface* audioDriver/* = 0*/) :
	listener(0),
	sampleRate(sampleRate),
	bufferSize(bufferSize),
	buffer(0),
	sampleShift(0),
	disableMixing(false),
	numDevices(numDevices),
	filterHook(0),
	devices(new DeviceDescriptor[numDevices]),
	audioDriverManager(0),
	audioDriver(audioDriver),
	initialized(false),
	started(false),
	paused(false)
{
}

MasterMixer::~MasterMixer()
{
	cleanup();

	delete audioDriverManager;
	delete[] devices;
}

void MasterMixer::setMasterMixerNotificationListener(MasterMixerNotificationListener* listener) 
{ 
	this->listener = listener; 
}

mp_sint32 MasterMixer::openAudioDevice()
{
	if (initialized)
		return 0;
		
	if (audioDriver == 0)
	{
		if (audioDriverManager == 0)
			audioDriverManager = new AudioDriverManager();
			
		audioDriver = audioDriverManager->getPreferredAudioDriver();
	}
		
	if (bufferSize == 0)
	{
		bufferSize = audioDriver->getPreferredBufferSize();
		notifyListener(MasterMixerNotificationBufferSizeChanged);
	}
		
	cleanup();
	
	mp_sint32 res = audioDriver->initDevice(bufferSize*MP_NUMCHANNELS, sampleRate, this);
	if (res < 0)
		return res;

	if (res > 0)
	{
		// if the result is positive it reflects the number of 16 bit words
		// in the obtained buffer => divide by MP_NUMCHANNELS is the correct buffer size
		bufferSize = res / MP_NUMCHANNELS;
		notifyListener(MasterMixerNotificationBufferSizeChanged);		
	}

	if (audioDriver->getMixFrequency() != sampleRate)
	{
		sampleRate = audioDriver->getMixFrequency();
		notifyListener(MasterMixerNotificationSampleRateChanged);
	}
	
	buffer = new mp_sint32[bufferSize*MP_NUMCHANNELS];	
	
	initialized = true;	
	return 0;
}

mp_sint32 MasterMixer::closeAudioDevice()
{
	if (started)
		stop();
	
	if (!initialized)
		return 0;

	mp_sint32 res = 0;
	if (audioDriver)
	{
		res = audioDriver->closeDevice();
		if (res == 0)
			initialized = false;
	}
	else
	{
		initialized = false;
	}

	return res;
}

mp_sint32 MasterMixer::start()
{
	if (started)
		return 0;
	
	mp_sint32 res = 0;

	if (!initialized)
	{
		res = openAudioDevice();
		if (res < 0)
			return res;
	}
		
	res = audioDriver->start();	
	if (res != 0)
		return res;
	
	started = true;
	return 0;
}

mp_sint32 MasterMixer::stop()
{	
	if (!started)
		return 0;

	mp_sint32 res = audioDriver->stop();
	if (res == 0)
		started = false;
		
	return res;
}

mp_sint32 MasterMixer::pause()
{
	mp_sint32 res = audioDriver->pause();	
	paused = true;	
	return res;
}

mp_sint32 MasterMixer::resume()
{
	paused = false;
	return audioDriver->resume();
}

mp_sint32 MasterMixer::setBufferSize(mp_uint32 bufferSize)
{
	if (bufferSize != this->bufferSize)
	{		
		mp_sint32 res = closeAudioDevice();
		if (res != 0)
			return res;
		
		this->bufferSize = bufferSize;
		delete[] buffer;
		buffer = NULL;
		
		notifyListener(MasterMixerNotificationBufferSizeChanged);
	}
	return 0;
}

mp_sint32 MasterMixer::setSampleRate(mp_uint32 sampleRate)
{
	if (sampleRate != this->sampleRate)
	{
		mp_sint32 res = closeAudioDevice();
		if (res != 0)
			return res;
			
		this->sampleRate = sampleRate;

		notifyListener(MasterMixerNotificationSampleRateChanged);
	}
	return 0;
}

bool MasterMixer::addDevice(Mixable* device, bool paused/* = false*/)
{
	for (mp_uint32 i = 0; i < numDevices; i++)
	{
		if (devices[i].mixable == NULL)
		{
			devices[i].mixable = device;
			devices[i].markedForRemoval = false;
			devices[i].markedForPause = false;
			devices[i].paused = paused;
			return true;
		}
	}

	return false;
}

bool MasterMixer::removeDevice(Mixable* device, bool blocking/* = true*/)
{
	for (mp_uint32 i = 0; i < numDevices; i++)
	{
		if (devices[i].mixable == device)
		{
			if (!started)
			{
				devices[i].markedForRemoval = false;
				devices[i].mixable = 0;
				return true;				
			}
		
			devices[i].markedForRemoval = true;
			
			if (blocking)
			{
				// this is going to loop infinitely when the audio device is not running
				double waitMillis = ((double)(bufferSize/2) / (double)sampleRate) * 1000.0 * 2.0;
				if (waitMillis < 1.0)
					waitMillis = 1.0;
				if (waitMillis > (double)BlockTimeOut)
					waitMillis = (double)BlockTimeOut;
				
				mp_uint32 time = 0;
				const mp_uint32 sleepTime = 10;
				while (devices[i].mixable && time < (mp_uint32)waitMillis)
				{
					audioDriver->msleep(sleepTime);
					time+=sleepTime;
				}
				
				// timeout
				if (time >= waitMillis && devices[i].mixable)
				{
					devices[i].mixable = 0;
					devices[i].markedForRemoval = false;
				}
			}
			
			return true;
		}
	}

	return false;
}

bool MasterMixer::isDeviceRemoved(Mixable* device)
{
	for (mp_uint32 i = 0; i < numDevices; i++)
	{
		if (devices[i].mixable == device)
		{
			return false;
		}
	}

	return true;
}

bool MasterMixer::pauseDevice(Mixable* device, bool blocking/* = true*/)
{
	for (mp_uint32 i = 0; i < numDevices; i++)
	{
		if (devices[i].mixable == device)
		{
			if (!started)
			{
				devices[i].markedForPause = false;
				devices[i].paused = true;
				return true;				
			}

			devices[i].markedForPause = true;

			if (blocking)
			{
				// this is going to loop infinitely when the audio device is not running
				double waitMillis = ((double)(bufferSize/2) / (double)sampleRate) * 1000.0 * 2.0;
				if (waitMillis < 1.0)
					waitMillis = 1.0;
				if (waitMillis > (double)BlockTimeOut)
					waitMillis = (double)BlockTimeOut;
				
				mp_uint32 time = 0;
				const mp_uint32 sleepTime = 10;
				while (!devices[i].paused && time < (mp_uint32)waitMillis)
				{
					audioDriver->msleep(sleepTime);
					time+=sleepTime;
				}
				
				// timeout
				if (time >= waitMillis && !devices[i].paused)
				{
					devices[i].paused = true;
					devices[i].markedForPause = false;
				}
			}

			return true;
		}
	}
	
	return false;
}

bool MasterMixer::resumeDevice(Mixable* device)
{
	for (mp_uint32 i = 0; i < numDevices; i++)
	{
		if (devices[i].mixable == device && devices[i].paused)
		{
			devices[i].paused = false;
			return true;
		}
	}
	
	return false;
}

bool MasterMixer::isDevicePaused(Mixable* device)
{
	for (mp_uint32 i = 0; i < numDevices; i++)
	{
		if (devices[i].mixable == device)
		{
			return devices[i].paused;
		}
	}

	return false;
}

void MasterMixer::mixerHandler(mp_sword* buffer)
{
	if (!disableMixing)
		prepareBuffer();
	
	const register mp_sint32 numDevices = this->numDevices;
	const register mp_uint32 bufferSize = this->bufferSize;
	mp_sint32* mixBuffer = this->buffer;
	
	DeviceDescriptor* device = this->devices;	
	for (mp_sint32 i = 0; i < numDevices; i++, device++)
	{
		if (device->markedForRemoval && device->mixable)
		{
			device->markedForRemoval = false;
			device->mixable = 0;
		}  
		else if (device->mixable && device->markedForPause)
		{
			device->markedForPause = false;
			device->paused = true;
		}
		else if (device->mixable && !device->paused)
		{
			device->mixable->mix(mixBuffer, bufferSize);
		}
	}
	
	if (!disableMixing)
		swapOutBuffer(buffer);
}

void MasterMixer::notifyListener(MasterMixerNotifications notification)
{
	if (listener)
		listener->masterMixerNotification(notification);
}

void MasterMixer::cleanup()
{
	if (started)
		stop();

	if (initialized)
		closeAudioDevice();

	if (buffer)
	{
		delete[] buffer;	
		buffer = 0;
	}
}

inline void MasterMixer::prepareBuffer()
{
	memset(buffer, 0, bufferSize*MP_NUMCHANNELS*sizeof(mp_sint32)); 
}

inline void MasterMixer::swapOutBuffer(mp_sword* bufferOut)
{
	if (filterHook)
		filterHook->mix(buffer, bufferSize);

	register mp_sint32* bufferIn = buffer;
	const register mp_sint32 sampleShift = this->sampleShift; 
	const register mp_sint32 lowerBound = -((128<<sampleShift)*256); 
	const register mp_sint32 upperBound = ((128<<sampleShift)*256)-1;
	const register mp_sint32 bufferSize = this->bufferSize*MP_NUMCHANNELS;
	
	for (mp_sint32 i = 0; i < bufferSize; i++)
	{
		mp_sint32 b = *bufferIn++;
		if (b>upperBound) b = upperBound; 
		else if (b<lowerBound) b = lowerBound; 
		*bufferOut++ = b>>sampleShift;
	}
	
	/*
	mp_sint32* buffer32 = mixbuff32;
	mp_sint32 lsampleShift = sampleShift; 
	mp_sint32 lowerBound = -((128<<sampleShift)*256); 
	mp_sint32 upperBound = ((128<<sampleShift)*256)-1;
	
	if (!autoAdjustPeak)
	{
		for (mp_uint32 i = 0; i < mixBufferSize*MP_NUMCHANNELS; i++)
		{
			mp_sint32 b = *buffer32++;
			if (b>upperBound) b = upperBound; 
			else if (b<lowerBound) b = lowerBound; 
			*buffer16++ = b>>lsampleShift;
		}
	}
	else
	{
		lsampleShift = sampleShift; 
		lowerBound = -((128<<sampleShift)*256); 
		upperBound = ((128<<sampleShift)*256)-1;
		mp_sint32 lastPeakValue = 0;
		
		for (mp_uint32 i = 0; i < mixBufferSize*MP_NUMCHANNELS; i++)
		{
			mp_sint32 b = *buffer32++;
	
			if (b>upperBound) 
			{
				if (abs(b) > lastPeakValue)
					lastPeakValue = abs(b);
					
				b = upperBound; 
			}
			else if (b<lowerBound) 
			{
				if (abs(b) > lastPeakValue)
					lastPeakValue = abs(b);

				b = lowerBound; 
			}
			*buffer16++ = b>>lsampleShift;
		}
		
		if (lastPeakValue)
		{
			float v = 32768.0f*(1<<lsampleShift) / (float)lastPeakValue;
			masterVolume = (mp_sint32)((float)masterVolume*v);
			if (masterVolume > 256)
				masterVolume = 256;
		}
		//else
		//{
		//	masterVolume = 256;
		//	sampleShift = 0;
		//}
		
	}*/
}

const char*	MasterMixer::getCurrentAudioDriverName() const
{
	if (audioDriver)
		return audioDriver->getDriverID();

	return NULL;
}

bool MasterMixer::setCurrentAudioDriverByName(const char* name)
{
	bool result = true;

	if (audioDriverManager == 0)
		audioDriverManager = new AudioDriverManager();

	AudioDriverInterface* newAudioDriver = audioDriverManager->getAudioDriverByName(name);
	if (newAudioDriver == 0)
	{
		newAudioDriver = audioDriverManager->getPreferredAudioDriver();	
		result = false;
	}

	// Same instance, don't allocate new audio driver
	if (audioDriver == newAudioDriver)
		return result;

	mp_sint32 err = 0;
	if (initialized)
	{
		err = closeAudioDevice();
	}
	
	audioDriver = newAudioDriver;	

	return result;
}

const AudioDriverManager* MasterMixer::getAudioDriverManager() const
{
	if (audioDriverManager == 0)
		audioDriverManager = new AudioDriverManager();

	return audioDriverManager;
}

mp_sint32 MasterMixer::getCurrentSample(mp_sint32 position, mp_sint32 channel)
{
	if (position < 0)
	{
		position = abs(position);
	}
	if (position > (mp_sint32)bufferSize-1)
	{
		position %= bufferSize*2;
		position -= bufferSize;
		position = bufferSize-1-position;
	}
	
	mp_sint32 val = (mp_sword)this->buffer[position*MP_NUMCHANNELS+channel];
	if (val < -32768)
		val = -32768;
	if (val > 32767)
		val = 32767;

	return val;
}

mp_sint32 MasterMixer::getCurrentSamplePeak(mp_sint32 position, mp_sint32 channel)
{
	if (audioDriver == 0)
		return 0;

	const mp_uint32 mixBufferSize = bufferSize;
	const mp_sint32* mixbuff32 = this->buffer;

	if (audioDriver->supportsTimeQuery())
	{
		mp_sword peak = 0;
		
		for (mp_sint32 p = position-mixBufferSize; p <= position; p++)
		{
			mp_sword s = getCurrentSample(p, channel);
			if (s > peak)
				peak = s;
			if (-s > peak)
				peak = -s;				
		}
		return peak;
	}	
	else
	{
		mp_sword peak = 0;
		for (mp_uint32 pos = 0; pos < mixBufferSize; pos++)
		{
			mp_sint32 s = mixbuff32[pos*MP_NUMCHANNELS+channel];
			if (s < -32768)
				s = -32768;
			if (s > 32767)
				s = 32767;
			if (s > peak)
				peak = s;
			if (-s > peak)
				peak = -s;
		}	
		
		return peak;
	}
}
