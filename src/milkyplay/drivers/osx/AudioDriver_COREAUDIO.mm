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
#include <Foundation/Foundation.h>

#define CHECK_ERROR(ERRNO, RESULT) \
	if (RESULT != kAudioHardwareNoError) \
	{ \
		lastError = ERRNO; \
		return MP_DEVICE_ERROR; \
	}

#define MPERR_DETECTING_DEVICE			-1
#define MPERR_OSX_UNKNOWN_DEVICE		-2
#define MPERR_OSX_BAD_PROPERTY			-3
#define MPERR_OSX_UNSUPPORTED_FORMAT	-4
#define MPERR_OSX_BUFFER_ALLOC			-5
#define MPERR_OSX_ADD_IO_PROC			-6
#define MPERR_OUT_OF_MEMORY				-7
#define MPERR_OSX_DEVICE_START			-8

OSStatus AudioDriver_COREAUDIO::audioIOProc(AudioObjectID			inDevice,
											const AudioTimeStamp*	inNow,
											const AudioBufferList*	inInputData,
											const AudioTimeStamp*	inInputTime,
											AudioBufferList*		outOutputData,
											const AudioTimeStamp*	inOutputTime,
											void*					inClientData)
{
	float* outputBuffer = (float *) outOutputData->mBuffers[0].mData;

	AudioDriver_COREAUDIO* audioDriver = reinterpret_cast<AudioDriver_COREAUDIO*>(inClientData);
	MasterMixer* mixer = audioDriver->mixer;
	mp_sword* inputBuffer = audioDriver->compensateBuffer;
	UInt32 numFrames = outOutputData->mBuffers[0].mDataByteSize /
					   outOutputData->mBuffers[0].mNumberChannels /
					   sizeof(float);

	audioDriver->sampleCounter += numFrames;

	if (audioDriver->isMixerActive())
		mixer->mixerHandler(inputBuffer);
	else
		memset(inputBuffer, 0, numFrames * MP_NUMCHANNELS * sizeof(mp_sword));

	// Core Audio always uses floats for its buffers, so we need to do a conversion
	if (audioDriver->mono)
	{
		for (int i = 0; i < numFrames; i++)
		{
			outputBuffer[i] = (inputBuffer[i * 2] + inputBuffer[i * 2 + 1]) / (32768.0f * 2.0f);
		}
	}
	else
	{
		UInt32 channelsPerFrame = outOutputData->mBuffers[0].mNumberChannels;

		for (int i = 0; i < numFrames; i++)
		{
			outputBuffer[i * channelsPerFrame]     = inputBuffer[i * 2] / 32768.0f;
			outputBuffer[i * channelsPerFrame + 1] = inputBuffer[i * 2 + 1] / 32768.0f;
		}
	}

	return kAudioHardwareNoError;
}

AudioDriver_COREAUDIO::AudioDriver_COREAUDIO() :
	AudioDriverBase(),
	defaultDevice(true),
	driverID(NULL),
	sampleCounter(0),
	compensateBuffer(NULL),
	audioIOProcID(NULL),
	deviceHasStarted(false)
{
}

AudioDriver_COREAUDIO::AudioDriver_COREAUDIO(AudioObjectID deviceID) :
	AudioDriverBase(),
	defaultDevice(false),
	driverID(NULL),
	sampleCounter(0),
	compensateBuffer(NULL),
	audioDeviceID(deviceID),
	audioIOProcID(NULL),
	deviceHasStarted(false)
{
}

AudioDriver_COREAUDIO::~AudioDriver_COREAUDIO()
{
	delete[] compensateBuffer;
	if (driverID)
		delete[] driverID;
}

// --------------------------------------------------
//  Get the name of the sound device from Core Audio
// --------------------------------------------------
const char* AudioDriver_COREAUDIO::getDriverID()
{
	if (defaultDevice)
		return "Default Output Device";

	if (!driverID)
	{
		OSStatus err = noErr;
		mp_uint32 dataSize = sizeof(CFStringRef);
		CFStringRef deviceName;

		AudioObjectPropertyAddress propertyAddress =
		{
			kAudioObjectPropertyName,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMaster
		};

		err = AudioObjectGetPropertyData(audioDeviceID,
										 &propertyAddress,
										 0, NULL,
										 &dataSize,
										 &deviceName);

		if (err != kAudioHardwareNoError)
			return "Unnamed Device";

		CFIndex strLen = CFStringGetLength(deviceName);
		CFIndex bufSize = CFStringGetMaximumSizeForEncoding(strLen, kCFStringEncodingUTF8);
		driverID = new char[bufSize];
		CFStringGetCString(deviceName, driverID, bufSize, kCFStringEncodingUTF8);

		CFRelease(deviceName);
	}

	return driverID;
}

// ----------------------------------------------------------------------
//  Gets number of output devices, creates an array of output device IDs
// ----------------------------------------------------------------------
OSStatus AudioDriver_COREAUDIO::getAudioDevices(mp_uint32 &numDevices, AudioObjectID* &deviceIDs)
{
	OSStatus err = noErr;
	mp_uint32 dataSize = 0;
	mp_uint32 numAudioDevices = 0;
	mp_uint32 numOutputDevices = 0;

	AudioObjectPropertyAddress propertyAddress =
	{
		kAudioHardwarePropertyDevices,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};

	// Find out how many audio devices exist on the system (including input devices)
	err = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &dataSize);
	if (err) return err;

	// Temporary arrays to hold device IDs
	numAudioDevices = dataSize / sizeof(AudioObjectID);
	AudioObjectID allDeviceIDs[numAudioDevices];
	AudioObjectID outputDeviceIDs[numAudioDevices];

	// Get all of the device IDs
	err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &dataSize, allDeviceIDs);
	if (err) return err;

	// Check each device ID for output channels
	propertyAddress.mSelector = kAudioDevicePropertyStreams;
	propertyAddress.mScope = kAudioDevicePropertyScopeOutput;
	for (int i = 0; i < numAudioDevices; i++)
	{
		dataSize = 0;

		err = AudioObjectGetPropertyDataSize(allDeviceIDs[i], &propertyAddress, 0, NULL, &dataSize);
		if (err) return err;

		mp_uint32 numOutputChannels = dataSize / sizeof(AudioObjectID);

		// Skip this device ID if there are no output channels
		if (numOutputChannels < 1)
			continue;

		// Otherwise add it to our array
		outputDeviceIDs[numOutputDevices] = allDeviceIDs[i];
		numOutputDevices++;
	}

	// Store device count and output device IDs
	numDevices = numOutputDevices;
	deviceIDs = new AudioObjectID[numOutputDevices];
	memcpy(deviceIDs, outputDeviceIDs, numOutputDevices * sizeof(AudioObjectID));

	return kAudioHardwareNoError;
}

mp_sint32 AudioDriver_COREAUDIO::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
		return res;

	// Reset the sample counter
	sampleCounter = 0;

	UInt32 propertySize;
	AudioObjectPropertyAddress propertyAddress =
	{
		0,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};

	if (defaultDevice)
	{
		// Get the default output device
		propertySize = sizeof (audioDeviceID);
		propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
		CHECK_ERROR
		(
			MPERR_DETECTING_DEVICE,
			AudioObjectGetPropertyData(kAudioObjectSystemObject,
									   &propertyAddress,
									   0, NULL,
									   &propertySize, &audioDeviceID)
		);

		if (audioDeviceID == kAudioDeviceUnknown)
		{
			lastError = MPERR_OSX_UNKNOWN_DEVICE;
			return MP_DEVICE_ERROR;
		}
	}

	// Get the stream format
	AudioStreamBasicDescription basicDescription;
	propertySize = sizeof (basicDescription);
	propertyAddress.mSelector = kAudioStreamPropertyVirtualFormat;
	CHECK_ERROR
	(
		MPERR_OSX_BAD_PROPERTY,
		AudioObjectGetPropertyData(audioDeviceID,
								   &propertyAddress,
								   0, NULL,
								   &propertySize, &basicDescription)
	);

	// Linear PCM is required
	if (basicDescription.mFormatID != kAudioFormatLinearPCM)
	{
		lastError = MPERR_OSX_UNSUPPORTED_FORMAT;
		return MP_DEVICE_ERROR;
	}

	// Check we actually have an output channel
	if (basicDescription.mChannelsPerFrame < 1)
		return MP_DEVICE_ERROR;

	// Check the requested mix frequency
	if (basicDescription.mSampleRate != mixFrequency)
	{
		// Get the supported sample rates
		CHECK_ERROR
		(
			 MPERR_OSX_BAD_PROPERTY,
			 AudioObjectGetPropertyDataSize(audioDeviceID,
											&propertyAddress,
											0, NULL,
											&propertySize)
		);

		UInt32 numRates = propertySize / sizeof(AudioValueRange);
		AudioValueRange supportedSampleRates[numRates];

		propertyAddress.mSelector = kAudioDevicePropertyAvailableNominalSampleRates;
		CHECK_ERROR
		(
			MPERR_OSX_BAD_PROPERTY,
			AudioObjectGetPropertyData(audioDeviceID,
									   &propertyAddress,
									   0, NULL,
									   &propertySize, &supportedSampleRates)
		);

		// See if our requested mix frequency is supported by the device
		AudioValueRange* newSampleRate = NULL;
		for (size_t i = 0; i < numRates; i++)
		{
			if (supportedSampleRates[i].mMinimum == mixFrequency)
			{
				newSampleRate = &supportedSampleRates[i];
				break;
			}
		}

		// No: bail out
		if (!newSampleRate)
		{
			NSLog(@"Core Audio: Device doesn't support mix frequency of %dHz\n", mixFrequency);
			lastError = MPERR_OSX_UNSUPPORTED_FORMAT;
			return MP_DEVICE_ERROR;
		}

		// Yes: set it
		propertySize = sizeof (AudioValueRange);
		propertyAddress.mSelector = kAudioDevicePropertyNominalSampleRate;
		CHECK_ERROR
		(
			MPERR_OSX_BAD_PROPERTY,
			AudioObjectSetPropertyData(audioDeviceID,
									   &propertyAddress,
									   0, NULL,
									   propertySize, newSampleRate)
		);
	}

	// Force stereo -> mono conversion if driver only supports mono
	if (basicDescription.mChannelsPerFrame == 1)
		mono = true;

	// Find valid frame size range
	AudioValueRange frameSizeRange;
	propertySize = sizeof(AudioValueRange);
	propertyAddress.mSelector = kAudioDevicePropertyBufferFrameSizeRange;
	CHECK_ERROR
	(
		MPERR_OSX_BUFFER_ALLOC,
		AudioObjectGetPropertyData(audioDeviceID,
								   &propertyAddress,
								   0, NULL,
								   &propertySize, &frameSizeRange)
	);

	// Number of frames = buffer size / 2
	UInt32 bufferFrameSize = bufferSizeInWords >> 1 >> (mono ? 1 : 0);

	// If requested buffer size is too large, set it to the device's maximum
	if (bufferFrameSize > frameSizeRange.mMaximum)
		bufferFrameSize = frameSizeRange.mMaximum;

	// Set buffer size
	propertyAddress.mSelector = kAudioDevicePropertyBufferFrameSize;
	CHECK_ERROR
	(
		MPERR_OSX_BUFFER_ALLOC,
		AudioObjectSetPropertyData(audioDeviceID,
								   &propertyAddress,
								   0, NULL,
								   sizeof(bufferFrameSize), &bufferFrameSize)
	);

	bufferSize = bufferFrameSize;

	// Add our audio IO procedure
	CHECK_ERROR
	(
		MPERR_OSX_ADD_IO_PROC,
		AudioDeviceCreateIOProcID(audioDeviceID, audioIOProc, (void*)this, &audioIOProcID)
	);

	// If the IOProcID is null, something went wrong
	if (!audioIOProcID)
	{
		lastError = MPERR_OSX_ADD_IO_PROC;
		return MP_DEVICE_ERROR;
	}

	deviceHasStarted = false;

	// Allocate a buffer for the mixer to fill during our IOProc
	if (compensateBuffer)
		delete[] compensateBuffer;

	compensateBuffer = new mp_sword[bufferSizeInWords];

	NSLog(@"Core Audio: Wanted %d bytes, got %d\n", bufferSizeInWords / 2 * 4, bufferFrameSize * 4);

	// If we got what we requested, return MP_OK, otherwise return the actual number of frames * number of channels
	return bufferSizeInWords / 2 == (signed) bufferSize ? MP_OK : bufferFrameSize * 2;
}

mp_sint32 AudioDriver_COREAUDIO::closeDevice()
{
	if (audioIOProcID)
	{
		AudioDeviceDestroyIOProcID(audioDeviceID, audioIOProcID);
		deviceHasStarted = false;
	}
	return MP_OK;
}

mp_sint32 AudioDriver_COREAUDIO::start()
{
	if (!deviceHasStarted)
	{
		// Start the audio IO Proc
		if (AudioDeviceStart(audioDeviceID, audioIOProcID))
		{
			lastError = MPERR_OSX_DEVICE_START;
			return MP_DEVICE_ERROR;
		}
		deviceHasStarted = true;
	}
	return MP_OK;
}

mp_sint32 AudioDriver_COREAUDIO::stop()
{
	AudioDeviceStop(audioDeviceID, audioIOProcID);
	deviceHasStarted = false;
	return MP_OK;
}

mp_sint32 AudioDriver_COREAUDIO::pause()
{
	return stop();
}

mp_sint32 AudioDriver_COREAUDIO::resume()
{
	return start();
}
