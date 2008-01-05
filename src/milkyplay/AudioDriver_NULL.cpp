/*
 *  AudioDriver_NULL.cpp
 *  Audio Test
 *
 *  Created by Peter Barth on 29.07.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "AudioDriver_NULL.h"
#include "MasterMixer.h"

AudioDriver_NULL::AudioDriver_NULL() :
	numSamplesWritten(0),
	compensateBuffer(0)
{
}

AudioDriver_NULL::~AudioDriver_NULL() 
{
	delete[] compensateBuffer;
}

mp_sint32 AudioDriver_NULL::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
		return res;

	numSamplesWritten = 0;
	
	delete[] compensateBuffer;
	compensateBuffer = new mp_sword[bufferSizeInWords];

	return 0;
}

mp_sint32 AudioDriver_NULL::stop()
{
	return 0;
}

mp_sint32 AudioDriver_NULL::closeDevice()
{
	return 0;
}

void AudioDriver_NULL::start()
{
}

mp_sint32 AudioDriver_NULL::pause()
{
	return 0;
}

mp_sint32 AudioDriver_NULL::resume()
{
	return 0;
}

void AudioDriver_NULL::advance()
{
	numSamplesWritten+=bufferSize / MP_NUMCHANNELS;	
	
    if (mixer->isPlaying())
        mixer->mixerHandler(compensateBuffer);	
}

