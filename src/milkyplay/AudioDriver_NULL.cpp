/*
 *  milkyplay/AudioDriver_NULL.cpp
 *
 *  Copyright 2008 Peter Barth
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
 *  AudioDriver_NULL.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 29.07.05.
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

