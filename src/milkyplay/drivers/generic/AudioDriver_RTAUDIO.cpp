/*
 *  milkyplay/drivers/generic/AudioDriver_RTAUDIO.cpp
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
 *  AudioDriver_RTAUDIO.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 09.06.05.
 *
 *  Christopher O'Neill 16/3/06:
 *    fill_audio() rewritten to solve issues on certain systems
 *
 *
 */

#include "AudioDriver_RTAUDIO.h"

AudioDriver_RTAUDIO::AudioDriver_RTAUDIO(Api audioApi/* = UNSPECIFIED*/, Version version/* = V4*/) :
	impl(0)
{
	switch (version)
	{
		case V3:
			createRt3Instance(audioApi);
			break;
		case V4:
			createRt4Instance(audioApi);
			break;
	}
}

AudioDriver_RTAUDIO::~AudioDriver_RTAUDIO() 
{
	delete impl;
}

mp_sint32 AudioDriver_RTAUDIO::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	return impl->initDevice(bufferSizeInWords, mixFrequency, mixer);
}

mp_uint32 AudioDriver_RTAUDIO::getMixFrequency() const
{
	return impl->getMixFrequency();
}

mp_sint32   AudioDriver_RTAUDIO::closeDevice()
{
	return impl->closeDevice();
}

mp_sint32	AudioDriver_RTAUDIO::start()
{
	return impl->start();
}

mp_sint32   AudioDriver_RTAUDIO::stop()
{
	return impl->stop();
}

mp_sint32   AudioDriver_RTAUDIO::pause()
{
	return impl->pause();
}

mp_sint32   AudioDriver_RTAUDIO::resume()
{
	return impl->resume();
}

mp_uint32	AudioDriver_RTAUDIO::getNumPlayedSamples() const
{
	return impl->getNumPlayedSamples();
}

mp_uint32	AudioDriver_RTAUDIO::getBufferPos() const
{
	return impl->getBufferPos();
}

bool		AudioDriver_RTAUDIO::supportsTimeQuery()
{
	return impl->supportsTimeQuery();
}

const char* AudioDriver_RTAUDIO::getDriverID()
{
	return impl->getDriverID();
}

void		AudioDriver_RTAUDIO::advance()
{
	impl->advance();
}

mp_sint32	AudioDriver_RTAUDIO::getPreferredSampleRate() const
{
	return impl->getPreferredSampleRate();
}

mp_sint32	AudioDriver_RTAUDIO::getPreferredBufferSize() const
{
	return impl->getPreferredBufferSize();
}

void		AudioDriver_RTAUDIO::msleep(mp_uint32 msecs)
{
	impl->msleep(msecs);
}

bool		AudioDriver_RTAUDIO::isMixerActive()
{
	return impl->isMixerActive();
}

void		AudioDriver_RTAUDIO::setIdle(bool idle)
{
	impl->setIdle(idle);
}
