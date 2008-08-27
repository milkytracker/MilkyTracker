/*
 *  milkyplay/drivers/sdl/AudioDriver_SDL.cpp
 *
 *  Copyright 2008 Peter Barth, Christopher O'Neill
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
 *  AudioDriver_SDL.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 09.06.05.
 *
 *  Christopher O'Neill 16/3/06:
 *    fill_audio() rewritten to solve issues on certain systems
 *
 *
 */

#include "AudioDriver_SDL.h"

void SDLCALL AudioDriver_SDL::fill_audio(void *udata, Uint8 *stream, int length) 
{
	AudioDriver_SDL* audioDriver = (AudioDriver_SDL*)udata;

	if(length>>2 != audioDriver->periodSize)
		fprintf(stderr, "SDL: Invalid buffer size: %i (should be %i), skipping..\n", length >> 2, audioDriver->periodSize);
		// See comment in AudioDriver_ALSA.cpp
	else
		audioDriver->fillAudioWithCompensation((char*)stream, length);
}

AudioDriver_SDL::AudioDriver_SDL() :
	AudioDriver_COMPENSATE()
{
}

AudioDriver_SDL::~AudioDriver_SDL() 
{
}

// On error return a negative value
// If the requested buffer size can be served return 0, 
// otherwise return the number of 16 bit words contained in the obtained buffer
mp_sint32 AudioDriver_SDL::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	SDL_AudioSpec	wanted, obtained, saved;
	char name[32];
	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
		return res;

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
		return -1;
	}

	printf("SDL: Using audio driver: %s\n", SDL_AudioDriverName(name, 32));

	if(wanted.format != obtained.format)
	{
		fprintf(stderr, "SDL: Audio driver doesn't support 16-bit signed samples!\n");
		return -1;
	}
	
	if (wanted.channels != obtained.channels)
	{
		fprintf(stderr, "SDL: Failed to obtain requested audio format.  Suggested format:\n");
		fprintf(stderr, "SDL: Frequency: %d\nChannels: %d\n", obtained.freq, obtained.channels);
		return -1;
	}
	
	// fallback for obtained sample rate
	if (wanted.freq != obtained.freq)
		this->mixFrequency = obtained.freq;

	printf("SDL: Buffer size = %i samples (requested %i)\n", obtained.samples, finalWantedSize / wanted.channels);

	periodSize = obtained.samples;
	// If we got what we requested, return 0,
	// otherwise return the actual number of samples * number of channels
	return (bufferSizeInWords / wanted.channels == obtained.samples) ? 0 : obtained.samples * obtained.channels;
}

mp_sint32 AudioDriver_SDL::stop()
{
	SDL_PauseAudio(1); 
	deviceHasStarted = false;
	return 0;
}

mp_sint32 AudioDriver_SDL::closeDevice()
{
	SDL_CloseAudio(); 
	deviceHasStarted = false;
	return 0;
}

mp_sint32 AudioDriver_SDL::start()
{
	SDL_PauseAudio(0);
	deviceHasStarted = true;
	return 0;
}

mp_sint32 AudioDriver_SDL::pause()
{
	SDL_PauseAudio(1); 
	return 0;
}

mp_sint32 AudioDriver_SDL::resume()
{
	SDL_PauseAudio(0); 
	return 0;
}
