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

AudioDriver_RTAUDIO::AudioDriver_RTAUDIO(Api audioApi/* = UNSPECIFIED*/) :
	impl(0)
{
#ifdef __OSX_PANTHER__
	createRt3Instance(audioApi);
#else
	createRt4Instance(audioApi);
#endif
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

bool		AudioDriver_RTAUDIO::supportsTimeQuery() const
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
