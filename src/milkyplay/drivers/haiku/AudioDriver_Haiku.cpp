/*
 * Copyright (c) 2012, The MilkyTracker Team.
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

#include "AudioDriver_Haiku.h"
#include "MasterMixer.h"

#include <MediaDefs.h>
#include <MediaRoster.h>
#include <OS.h>
#include <SoundPlayer.h>

#include <stdio.h>
#include <string.h>


AudioDriver_Haiku::AudioDriver_Haiku()
	:
	AudioDriverBase(),
	fSoundPlayer(NULL)
{
}


AudioDriver_Haiku::~AudioDriver_Haiku()
{
	delete fSoundPlayer;
}


// #pragma mark - AudioDriverBase API


mp_sint32
AudioDriver_Haiku::initDevice(mp_sint32 bufferSizeInWords,
	mp_uint32 mixFrequency, MasterMixer* mixer)
{
	mp_sint32 result = AudioDriverBase::initDevice(bufferSizeInWords,
		mixFrequency, mixer);
	if (result < 0)
		return result;

	media_raw_audio_format format;
	format.frame_rate    = mixFrequency;
	format.channel_count = kChannelCount;
	format.format        = media_raw_audio_format::B_AUDIO_SHORT;
	format.byte_order    = B_MEDIA_LITTLE_ENDIAN;
	format.buffer_size   = bufferSizeInWords * sizeof(mp_sword);

	fSoundPlayer = new BSoundPlayer(&format, "Milky sound", _FillBuffer, NULL,
		this);

	status_t status = fSoundPlayer->InitCheck();
	if (status != B_OK) {
		fprintf(stderr, "AudioDriver_Haiku: failed to initialize "
			 "BSoundPlayer\n");
		return MP_DEVICE_ERROR;
	}

	return MP_OK;
}


mp_sint32
AudioDriver_Haiku::closeDevice()
{
	fSoundPlayer->SetHasData(false);
	fSoundPlayer->Stop(true);
	delete fSoundPlayer;
	fSoundPlayer = NULL;
	return MP_OK;
}


mp_sint32
AudioDriver_Haiku::start()
{
	status_t status =  fSoundPlayer->Start();
	if (status != B_OK) {
		fprintf(stderr, "AudioDriver_Haiku: failed to start BSoundPlayer\n");
		return MP_DEVICE_ERROR;
	}
	fSoundPlayer->SetHasData(true);
	return MP_OK;
}


mp_sint32
AudioDriver_Haiku::stop()
{
	fSoundPlayer->SetHasData(false);
	fSoundPlayer->Stop(true);
	return MP_OK;
}


mp_sint32
AudioDriver_Haiku::pause()
{
	fSoundPlayer->SetHasData(false);
}


mp_sint32
AudioDriver_Haiku::resume()
{
	fSoundPlayer->SetHasData(true);
}


mp_uint32
AudioDriver_Haiku::getNumPlayedSamples() const
{
	bigtime_t time = fSoundPlayer->CurrentTime();
	float samples = ((float)time / 1000000.0f) * mixFrequency;
	return (mp_uint32)samples;
}


mp_sint32
AudioDriver_Haiku::getPreferredBufferSize() const
{
	float frameRate = getPreferredSampleRate();
	return BMediaRoster::Roster()->AudioBufferSizeFor(kChannelCount,
		media_raw_audio_format::B_AUDIO_SHORT, frameRate, B_UNKNOWN_BUS);
}


// #pragma mark - MediaKit playback


void
AudioDriver_Haiku::_FillBuffer(void* theCookie, void* buffer, size_t size,
	const media_raw_audio_format& format)
{
	AudioDriver_Haiku* audioDriver = (AudioDriver_Haiku*)theCookie;

	MasterMixer* mixer = audioDriver->mixer;

	if (audioDriver->isMixerActive())
		mixer->mixerHandler((mp_sword*)buffer);
	else
		memset(buffer, 0, size);
}
