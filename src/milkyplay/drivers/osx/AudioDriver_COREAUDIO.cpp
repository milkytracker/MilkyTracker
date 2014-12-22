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
		return MP_DEVICE_ERROR; \
	}

#define SET_PROPS() \
	if (AudioObjectSetPropertyData (soundDeviceID, \
									&mySoundPropertyAddress, \
									0, \
									NULL, \
									myPropertySize, &mySoundBasicDescription)) \
	{ \
		CHECK_ERROR \
		( \
			MPERR_OSX_BAD_PROPERTY, \
			AudioObjectSetPropertyData (soundDeviceID, \
										&mySoundPropertyAddress, \
										0, \
										NULL, \
										myPropertySize, &mySoundBasicDescription) \
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
	{
		mixer->mixerHandler(myInBuffer);
	}
	else
	{
		memset(myInBuffer, 0, size*MP_NUMCHANNELS*sizeof(mp_sword));
	}

	register UInt32	i;

	if (audioDriver->mono)
	{
		for (i = 0; i < size; i++)
		{
			myOutBuffer[i] = (myInBuffer[i*2]+myInBuffer[i*2+1])*(1.0f/(32768.0f*2.0f));
		}
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
	{
		return res;
	}

	sampleCounter = 0;

	AudioStreamBasicDescription mySoundBasicDescription;
	AudioObjectPropertyAddress mySoundPropertyAddress;
	UInt32 myPropertySize, myBufferFrameSize;

	// Get the default output device...
	myPropertySize = sizeof (soundDeviceID);
	mySoundPropertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	mySoundPropertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
	mySoundPropertyAddress.mElement = kAudioObjectPropertyElementMaster;
	
	CHECK_ERROR
	(
		MPERR_DETECTING_DEVICE,
		AudioObjectGetPropertyData (kAudioObjectSystemObject,
									&mySoundPropertyAddress,
									0,  NULL,
									&myPropertySize, &soundDeviceID)
	);

	if (soundDeviceID == kAudioDeviceUnknown)
	{
		lastError = MPERR_OSX_UNKNOWN_DEVICE;
		return MP_DEVICE_ERROR;
	}

	// Get the device format...
	myPropertySize = sizeof (mySoundBasicDescription);
	mySoundPropertyAddress.mSelector = kAudioDevicePropertyStreamFormat;
	CHECK_ERROR
	(
		MPERR_OSX_BAD_PROPERTY,
		AudioObjectGetPropertyData (soundDeviceID,
									&mySoundPropertyAddress,
									0,  NULL,
									&myPropertySize, &mySoundBasicDescription)
	);

	// Try the selected mix frequency, if failure return device error...
	if (mySoundBasicDescription.mSampleRate != mixFrequency)
	{
		mySoundBasicDescription.mSampleRate = mixFrequency;
		SET_PROPS();
	}

	// Try selected channels, if failure return device error...
	if (mySoundBasicDescription.mChannelsPerFrame != 2)
	{
		mySoundBasicDescription.mChannelsPerFrame = 2;
		SET_PROPS();
	}

	// Linear PCM is required...
	if (mySoundBasicDescription.mFormatID != kAudioFormatLinearPCM)
	{
		lastError = MPERR_OSX_UNSUPPORTED_FORMAT;
		return MP_DEVICE_ERROR;
	}

	if (mySoundBasicDescription.mChannelsPerFrame > 2 ||
			mySoundBasicDescription.mChannelsPerFrame < 1)
	{
		return MP_DEVICE_ERROR;
	}

	// Force stereo -> mono conversion if driver only supports mono
	if (mySoundBasicDescription.mChannelsPerFrame == 1)
	{
		mono = true;
	}

	gAudioIOProc = OSX_AudioIOProc16Bit;
	
	// Find valid frame size range
	AudioValueRange frameSizeRange;
	myPropertySize = sizeof(AudioValueRange);
	mySoundPropertyAddress.mSelector = kAudioDevicePropertyBufferFrameSizeRange;
	CHECK_ERROR
	(
		MPERR_OSX_BUFFER_ALLOC,
		AudioObjectGetPropertyData(soundDeviceID,
								   &mySoundPropertyAddress,
								   0, NULL,
								   &myPropertySize, &frameSizeRange)
	 );
	
	// Number of frames = buffer size / 2
	myBufferFrameSize = bufferSizeInWords >> 1 >> (mono ? 1 : 0);
	
	// If requested buffer size is too large, set it to the device's maximum
	if (myBufferFrameSize > frameSizeRange.mMaximum)
		myBufferFrameSize = frameSizeRange.mMaximum;
	
	// Set buffer size
	mySoundPropertyAddress.mSelector = kAudioDevicePropertyBufferFrameSize;
	CHECK_ERROR
	(
		MPERR_OSX_BUFFER_ALLOC,
		AudioObjectSetPropertyData (soundDeviceID,
									&mySoundPropertyAddress,
									0,  NULL,
									sizeof(myBufferFrameSize), &myBufferFrameSize)
	);
	
	bufferSize = myBufferFrameSize;

	// Add our audio IO procedure...
	CHECK_ERROR
	(
		MPERR_OSX_ADD_IO_PROC,
		AudioDeviceCreateIOProcID (soundDeviceID, gAudioIOProc, (void*)this, &gAudioIOProcID)
	);
	
	// If the IOProcID is null, something went wrong
	assert (gAudioIOProcID != NULL);

	IOProcIsInstalled = true;
	deviceHasStarted = false;

	if (compensateBuffer)
	{
		delete[] compensateBuffer;
	}
	compensateBuffer = new mp_sword[bufferSizeInWords];

	printf("Core Audio: Wanted %d bytes, got %d\n", bufferSizeInWords / 2 * 4, myBufferFrameSize * 4);
	
	// If we got what we requested, return MP_OK,
	// otherwise return the actual number of frames * number of channels
	return bufferSizeInWords / 2 == (signed) bufferSize ? MP_OK : myBufferFrameSize * 2;
}

mp_sint32 AudioDriver_COREAUDIO::stop()
{
	AudioDeviceStop (soundDeviceID, gAudioIOProcID);

	deviceHasStarted = false;
	return MP_OK;
}

mp_sint32 AudioDriver_COREAUDIO::closeDevice()
{
	if (IOProcIsInstalled)
	{
		AudioDeviceDestroyIOProcID (soundDeviceID, gAudioIOProcID);
		deviceHasStarted = false;
	}

	return MP_OK;
}

mp_sint32 AudioDriver_COREAUDIO::start()
{
	// Start the audio IO Proc...
	if (AudioDeviceStart (soundDeviceID, gAudioIOProcID))
	{
		lastError = MPERR_OSX_DEVICE_START;
		return MP_DEVICE_ERROR;
	}
	deviceHasStarted = true;
	return MP_OK;
}

mp_sint32 AudioDriver_COREAUDIO::pause()
{
	AudioDeviceStop (soundDeviceID, gAudioIOProcID);
	deviceHasStarted = false;
	return MP_OK;
}

mp_sint32 AudioDriver_COREAUDIO::resume()
{
	if (!deviceHasStarted)
	{
		// Start the audio IO Proc...
		if (AudioDeviceStart (soundDeviceID, gAudioIOProcID))
		{
			lastError = MPERR_OSX_DEVICE_START;
			return MP_DEVICE_ERROR;
		}
		deviceHasStarted = true;
	}
	return MP_OK;
}
