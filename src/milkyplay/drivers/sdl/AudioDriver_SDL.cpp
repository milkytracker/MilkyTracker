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
 *
 *  AudioDriver_SDL.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 09.06.05.
 *
 *  12/5/14 - Dale Whinham
 *    - Port to SDL2
 *
 *  16/3/06 - Christopher O'Neill
 *    - fill_audio() rewritten to solve issues on certain systems
 *
 */

#include "AudioDriver_SDL.h"

void SDLCALL AudioDriver_SDL::fill_audio(void *udata, Uint8 *stream, int length)
{
	AudioDriver_SDL* audioDriver = (AudioDriver_SDL*)udata;

	if(length>>2 != audioDriver->periodSize)
	{
		fprintf(stderr, "SDL: Invalid buffer size: %i (should be %i), skipping..\n", length >> 2, audioDriver->periodSize);
	}
	// See comment in AudioDriver_ALSA.cpp
	else
	{
		audioDriver->fillAudioWithCompensation((char*)stream, length);
	}
}

AudioDriver_SDL::AudioDriver_SDL() :
	AudioDriver_COMPENSATE()
{
}

AudioDriver_SDL::~AudioDriver_SDL()
{
}

// On error return a negative value
// If the requested buffer size can be served return MP_OK,
// otherwise return the number of 16 bit words contained in the obtained buffer
mp_sint32 AudioDriver_SDL::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	SDL_AudioSpec	wanted, obtained, saved;
	char name[32];
	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
	{
		return res;
	}

	wanted.freq = mixFrequency;
	wanted.format = AUDIO_S16SYS;
	wanted.channels = 2; /* 1 = mono, 2 = stereo */
	wanted.samples = bufferSizeInWords / wanted.channels; /* Good low-latency value for callback */

	wanted.callback = fill_audio;
	wanted.userdata = (void*)this;

	mp_sint32 finalWantedSize = wanted.samples * wanted.channels;

	// Some soundcard drivers modify the wanted structure, so we copy it here
	memcpy(&saved, &wanted, sizeof(wanted));

	if(SDL_OpenAudio(&wanted, &obtained) < 0)
	{
		memcpy(&wanted, &saved, sizeof(wanted));
		fprintf(stderr, "SDL: Failed to open audio device! (buffer = %d bytes)..\n", saved.samples*4);
		fprintf(stderr, "SDL: Try setting \"Force 2^n sizes\" in the config menu and restarting.\n");
		return MP_DEVICE_ERROR;
	}

	printf("SDL: Using audio driver: %s\n", SDL_GetCurrentAudioDriver());

	if(wanted.format != obtained.format)
	{
		fprintf(stderr, "SDL: Audio driver doesn't support 16-bit signed samples!\n");
		return MP_DEVICE_ERROR;
	}

	if (wanted.channels != obtained.channels)
	{
		fprintf(stderr, "SDL: Failed to obtain requested audio format.  Suggested format:\n");
		fprintf(stderr, "SDL: Frequency: %d\nChannels: %d\n", obtained.freq, obtained.channels);
		return MP_DEVICE_ERROR;
	}

	// fallback for obtained sample rate
	if (wanted.freq != obtained.freq)
	{
		this->mixFrequency = obtained.freq;
	}

	printf("SDL: Buffer size = %i samples (requested %i)\n", obtained.samples, finalWantedSize / wanted.channels);

	periodSize = obtained.samples;
	// If we got what we requested, return MP_OK,
	// otherwise return the actual number of samples * number of channels
	return (bufferSizeInWords / wanted.channels == obtained.samples) ? MP_OK : obtained.samples * obtained.channels;
}

mp_sint32 AudioDriver_SDL::stop()
{
	SDL_PauseAudio(1);
	deviceHasStarted = false;
	return MP_OK;
}

mp_sint32 AudioDriver_SDL::closeDevice()
{
	SDL_CloseAudio();
	deviceHasStarted = false;
	return MP_OK;
}

mp_sint32 AudioDriver_SDL::start()
{
	SDL_PauseAudio(0);
	deviceHasStarted = true;
	return MP_OK;
}

mp_sint32 AudioDriver_SDL::pause()
{
	SDL_PauseAudio(1);
	return MP_OK;
}

mp_sint32 AudioDriver_SDL::resume()
{
	SDL_PauseAudio(0);
	return MP_OK;
}
