/*
 *  milkyplay/drivers/osx/AudioDriver_COREAUDIO.cpp
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
 *  AudioDriver_COREAUDIO.cpp
 *  MilkyPlay
 *
 *	The base of this code was a MikMod CoreAudio driver by Fruitz^Dojo
 *  It was heavily modified and improved to deal with some strange behaviour
 *  of my G4 Cube USB speakers. It seems to be rather solid now.
 */
#include "AudioDriver_COREAUDIO.h"
#include "MasterMixer.h"
#include <memory.h>

#define CHECK_ERROR(ERRNO, RESULT) \
	if (RESULT != kAudioHardwareNoError) \
	{ \
		lastError = ERRNO; \
		return -1; \
	}
	
#define SET_PROPS() \
	if (AudioDeviceSetProperty (soundDeviceID, NULL, 0, 0, \
								kAudioDevicePropertyStreamFormat, \
								myPropertySize, &mySoundBasicDescription)) \
	{ \
		CHECK_ERROR \
		( \
			MPERR_OSX_BAD_PROPERTY, \
			AudioDeviceGetProperty (soundDeviceID, 0, 0, \
									kAudioDevicePropertyStreamFormat, \
									&myPropertySize, &mySoundBasicDescription) \
		); \
	}

#define MPERR_DETECTING_DEVICE			-1
#define MPERR_OSX_UNKNOWN_DEVICE		-2
#define MPERR_OSX_BAD_PROPERTY			-3
#define MPERR_OSX_UNSUPPORTED_FORMAT	-4
#define MPERR_OSX_BUFFER_ALLOC			-5
#define MPERR_OSX_ADD_IO_PROC			-6
#define MPERR_OUT_OF_MEMORY				-7
#define MPERR_OSX_DEVICE_START			-8

OSStatus AudioDriver_COREAUDIO::OSX_AudioIOProc16Bit (AudioDeviceID inDevice, 
													  const AudioTimeStamp* inNow,
													  const AudioBufferList* inInputData,
													  const AudioTimeStamp* inInputTime,
													  AudioBufferList* outOutputData, 
													  const AudioTimeStamp* inOutputTime,
													  void *inClientData)
{
    register float*	myOutBuffer = (float *) outOutputData->mBuffers[0].mData;
    
	AudioDriver_COREAUDIO* audioDriver = reinterpret_cast<AudioDriver_COREAUDIO*>(inClientData);

    MasterMixer* mixer = audioDriver->mixer;
	
    register SInt16* myInBuffer = (SInt16*)audioDriver->compensateBuffer;
    register UInt32	size = (outOutputData->mBuffers[0].mDataByteSize / 
							outOutputData->mBuffers[0].mNumberChannels) / 
								sizeof(float);
	
	audioDriver->sampleCounter+=size;	
	
    if (audioDriver->isMixerActive())
        mixer->mixerHandler(myInBuffer);
	else
		memset(myInBuffer, 0, size*MP_NUMCHANNELS*sizeof(mp_sword));
		
    register UInt32	i;

	if (audioDriver->mono)
	{
		for (i = 0; i < size; i++)
			myOutBuffer[i] = (myInBuffer[i*2]+myInBuffer[i*2+1])*(1.0f/(32768.0f*2.0f));			
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			myOutBuffer[i*2] = (myInBuffer[i*2])*(1.0f/32768.0f);			
			myOutBuffer[i*2+1] = (myInBuffer[i*2+1])*(1.0f/32768.0f);			
		}
	}
	
    return noErr;
}

AudioDriver_COREAUDIO::AudioDriver_COREAUDIO() :
	AudioDriverBase(),
	sampleCounter(0),
	compensateBuffer(NULL),
	IOProcIsInstalled(0),
	deviceHasStarted(false)
{
}

AudioDriver_COREAUDIO::~AudioDriver_COREAUDIO() 
{
	delete[] compensateBuffer;
}

mp_sint32 AudioDriver_COREAUDIO::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
		return res;
	
	sampleCounter = 0;

    AudioStreamBasicDescription mySoundBasicDescription;
    UInt32 myPropertySize, myBufferByteCount;

    // get the device...
    myPropertySize = sizeof (soundDeviceID);
    CHECK_ERROR
    (
        MPERR_DETECTING_DEVICE,
        AudioHardwareGetProperty (kAudioHardwarePropertyDefaultOutputDevice,
                                  &myPropertySize, &soundDeviceID)
    );
	
    if (soundDeviceID == kAudioDeviceUnknown)
    {
        lastError = MPERR_OSX_UNKNOWN_DEVICE;
        return -1;
    }

    // get the device format...
    myPropertySize = sizeof (mySoundBasicDescription);
    CHECK_ERROR
    (
        MPERR_OSX_BAD_PROPERTY,
        AudioDeviceGetProperty (soundDeviceID, 0, 0, kAudioDevicePropertyStreamFormat,
                                &myPropertySize, &mySoundBasicDescription)
    );

    // try the selected mix frequency, if failure, fall back to native frequency...
    if (mySoundBasicDescription.mSampleRate != mixFrequency)
    {
        mySoundBasicDescription.mSampleRate = mixFrequency;
        SET_PROPS ();
        //md_mixfreq = mySoundBasicDescription.mSampleRate;
    }

    // try selected channels, if failure select native channels...
	if (mySoundBasicDescription.mChannelsPerFrame != 2)
	{
		mySoundBasicDescription.mChannelsPerFrame = 2;
		SET_PROPS();
	}

    // linear PCM is required...
    if (mySoundBasicDescription.mFormatID != kAudioFormatLinearPCM)
    {
        lastError = MPERR_OSX_UNSUPPORTED_FORMAT;
        return -1;
    }

    // get the device format again and verify if we could set the settings
    myPropertySize = sizeof (mySoundBasicDescription);
    CHECK_ERROR
    (
        MPERR_OSX_BAD_PROPERTY,
        AudioDeviceGetProperty (soundDeviceID, 0, 0, kAudioDevicePropertyStreamFormat,
                                &myPropertySize, &mySoundBasicDescription)
    );

	if (mySoundBasicDescription.mChannelsPerFrame > 2 ||
		mySoundBasicDescription.mChannelsPerFrame < 1)
		return -1;

	// force some stereo -> mono conversion
	if (mySoundBasicDescription.mChannelsPerFrame == 1)
		mono = true;

	// couldn't set sample rate, fall back
	if (mySoundBasicDescription.mSampleRate != mixFrequency)
		this->mixFrequency = (mp_uint32)mySoundBasicDescription.mSampleRate;

	gAudioIOProc = OSX_AudioIOProc16Bit;	
    
	myBufferByteCount = (bufferSizeInWords >> (mono ? 1 : 0))* sizeof(float);
    CHECK_ERROR
    (
        MPERR_OSX_BUFFER_ALLOC,
        AudioDeviceSetProperty (soundDeviceID, NULL, 0, 0, kAudioDevicePropertyBufferSize,
                                sizeof(myBufferByteCount), &myBufferByteCount)
    );
    
    // add our audio IO procedure....
    CHECK_ERROR
    (
        MPERR_OSX_ADD_IO_PROC,
        AudioDeviceAddIOProc (soundDeviceID, gAudioIOProc, (void*)this)
    );
	
    IOProcIsInstalled = 1;

	deviceHasStarted = false;

	if (compensateBuffer)
		delete[] compensateBuffer;
	compensateBuffer = new mp_sword[bufferSizeInWords];

	return 0;
}

mp_sint32 AudioDriver_COREAUDIO::stop()
{
	AudioDeviceStop (soundDeviceID, gAudioIOProc);

	deviceHasStarted = false;
	return 0;
}

mp_sint32 AudioDriver_COREAUDIO::closeDevice()
{
    if (IOProcIsInstalled)
    {
        AudioDeviceRemoveIOProc (soundDeviceID, gAudioIOProc);
		deviceHasStarted = false;
    }

	return 0;
}

mp_sint32 AudioDriver_COREAUDIO::start()
{
	// start the audio IO Proc...
	if (AudioDeviceStart (soundDeviceID, gAudioIOProc))
	{
		lastError = MPERR_OSX_DEVICE_START;
		return -1;
	}
	deviceHasStarted = true;
	return 0;
}

mp_sint32 AudioDriver_COREAUDIO::pause()
{
	AudioDeviceStop (soundDeviceID, gAudioIOProc);
	deviceHasStarted = false;
	return 0;
}

mp_sint32 AudioDriver_COREAUDIO::resume()
{
	if (!deviceHasStarted)
	{
		// start the audio IO Proc...
		if (AudioDeviceStart (soundDeviceID, gAudioIOProc))
		{
			lastError = MPERR_OSX_DEVICE_START;
			return -1;
		}
		deviceHasStarted = true;
     }	
	return 0;
}
