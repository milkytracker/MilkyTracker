/*
 *  milkyplay/drivers/jack/AudioDriver_JACK.cpp
 *
 *  Copyright 2008 Christopher O'Neill
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
 *  AudioDriver_JACK.cpp
 *  JACK Audio
 *
 *  Created by Christopher O'Neill on 8/12/07
 *
 *
 */

#include "config.h"
#ifdef HAVE_LIBJACK
#include <dlfcn.h>
#include<stdio.h>
#include<string.h>
#include<assert.h>
#include "AudioDriver_JACK.h"

int AudioDriver_JACK::jackProcess(jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *leftBuffer, *rightBuffer;
	AudioDriver_JACK* audioDriver = (AudioDriver_JACK*)arg;

	if(audioDriver->paused) return 0;

	// This could change, we need to setup a callback to deal with it
	// But for now, just panic
	assert(nframes == audioDriver->jackFrames);
	
	leftBuffer = (jack_default_audio_sample_t*) audioDriver->jack_port_get_buffer(audioDriver->leftPort, nframes);
	rightBuffer = (jack_default_audio_sample_t*) audioDriver->jack_port_get_buffer(audioDriver->rightPort, nframes);

	audioDriver->fillAudioWithCompensation((char*)audioDriver->rawStream, nframes * 4);

	// JACK uses non-interleaved floating-point samples, we need to convert
	for(int out = 0, in = 0; in < nframes; in++)
	{
		leftBuffer[in] = audioDriver->rawStream[out++] * (1.0/32768.0);
		rightBuffer[in] = audioDriver->rawStream[out++] * (1.0/32768.0);
	}
}

AudioDriver_JACK::AudioDriver_JACK() :
	AudioDriver_COMPENSATE(),
	paused(false),
	rawStream(NULL)
{
}

AudioDriver_JACK::~AudioDriver_JACK()
{
	if(rawStream) delete[] rawStream;
}

// On error return a negative value
// If the requested buffer size can be served return 0, 
// otherwise return the number of 16 bit words contained in the obtained buffer
mp_sint32 AudioDriver_JACK::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	// First load libjack
	libJack = dlopen("libjack.so", RTLD_LAZY);
	if(!libJack) {
		fprintf(stderr, "JACK: Can't load libjack (is it installed?)\n");
		return -1;
	}

	// Get function addresses
	// Each function has to be cast.. surely there must be an easier way?
	dlerror();
	jack_port_get_buffer = (void* (*)(jack_port_t*, jack_nframes_t))
			dlsym(libJack, "jack_port_get_buffer");
	jack_client_new = (jack_client_t* (*)(const char*))
			dlsym(libJack, "jack_client_new");
	jack_port_register = (jack_port_t* (*)(jack_client_t*, const char*, const char*, long unsigned int, long unsigned int))
			dlsym(libJack, "jack_port_register");
	jack_set_process_callback = (int (*)(jack_client_t*, int (*)(jack_nframes_t, void*), void*))
			dlsym(libJack, "jack_set_process_callback");
	jack_get_buffer_size = (jack_nframes_t (*)(jack_client_t*))
			dlsym(libJack, "jack_get_buffer_size");
	jack_deactivate = (int (*)(jack_client_t*))
			dlsym(libJack, "jack_deactivate");
	jack_client_close = (int (*)(jack_client_t*))
			dlsym(libJack, "jack_client_close");
	jack_activate = (int (*)(jack_client_t*))
			dlsym(libJack, "jack_activate");
	jack_get_sample_rate = (jack_nframes_t (*)(jack_client_t *))
			dlsym(libJack, "jack_get_sample_rate");
	if(dlerror()) {
		fprintf(stderr, "JACK: An error occured whilst loading symbols, aborting.\n");
		return -1;
	}

	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
		return res;

	// Connect to JACK
	hJack = jack_client_new("Milkytracker");
	if (!hJack) {
		fprintf(stderr, "JACK: Can't connect to JACK (is it running?)\n");
		return -1;
	}

	// Register ports
	leftPort = jack_port_register(hJack, "Left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	rightPort = jack_port_register(hJack, "Right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	if(!leftPort || !rightPort)
	{
		fprintf(stderr, "JACK: Failed to register ports\n");
		return -1;
	}
	
	// Set callback
	jack_set_process_callback(hJack, jackProcess, (void *) this);

	// Get buffer-size
	jackFrames = jack_get_buffer_size(hJack);
	bufferSize = jackFrames * 2;
	this->mixFrequency = jack_get_sample_rate(hJack);
	printf("JACK: Mixer frequency: %i\n", this->mixFrequency);
	//delete[] rawStream; // pailes: make sure this isn't allocated yet
	assert(!rawStream);		// If it is allocated, something went wrong and we need to know about it
	rawStream = new mp_sword[bufferSize];
	printf("JACK: Latency = %i frames\n", jackFrames);
	return bufferSize;
}

mp_sint32 AudioDriver_JACK::stop()
{
	jack_deactivate(hJack);
	deviceHasStarted = false;
	return 0;
}

mp_sint32 AudioDriver_JACK::closeDevice()
{
	deviceHasStarted = false;
	jack_client_close(hJack);
	if(rawStream) delete[] rawStream;
	rawStream = NULL;
	dlclose(libJack);
	libJack = NULL;
	return 0;
}

void AudioDriver_JACK::start()
{
	jack_activate(hJack);
	deviceHasStarted = true;
}

mp_sint32 AudioDriver_JACK::pause()
{
	paused = true;
	return 0;
}

mp_sint32 AudioDriver_JACK::resume()
{
	paused = false;
	return 0;
}
#endif
