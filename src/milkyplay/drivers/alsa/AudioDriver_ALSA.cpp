/*
 *  milkyplay/drivers/ALSA/AudioDriver_ALSA.cpp
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
 *  AudioDriver_ALSA.cpp
 *  ALSA Audio
 *
 *  Created by Christopher O'Neill on 19/1/2008
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBASOUND
#include "AudioDriver_ALSA.h"

void AudioDriver_ALSA::async_direct_callback(snd_async_handler_t *ahandler)
{
	// TODO: use mmap access for efficiency
	AudioDriver_ALSA* audioDriver = (AudioDriver_ALSA*) snd_async_handler_get_callback_private(ahandler);
	snd_pcm_t *pcm = audioDriver->pcm;
	snd_pcm_sframes_t avail;
	int err;
	const snd_pcm_uframes_t period_size = audioDriver->period_size;
	char *stream = audioDriver->stream;

	avail = snd_pcm_avail_update(pcm);
	while (avail >= period_size) {
		audioDriver->fillAudioWithCompensation(stream, period_size * 4);
		err = snd_pcm_writei(pcm, (void *) stream, period_size);
		if (err < 0)
		{
			fprintf(stderr, "ALSA: Write error (%s)\n", snd_strerror(err));
			break;
		}
		else if (err != period_size)
			fprintf(stderr, "ALSA: Write error: written %i expected %li\n", err, period_size);
		avail = snd_pcm_avail_update(pcm);
	}
}

AudioDriver_ALSA::AudioDriver_ALSA() :
	AudioDriver_COMPENSATE()
{
}

AudioDriver_ALSA::~AudioDriver_ALSA()
{
}

// On error return a negative value
// If the requested buffer size can be served return 0, 
// otherwise return the number of 16 bit words contained in the obtained buffer
mp_sint32 AudioDriver_ALSA::initDevice(mp_sint32 bufferSizeInWords, const mp_uint32 mixFrequency, MasterMixer* mixer)
{
	AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	snd_pcm_uframes_t buffer_size;
	snd_pcm_sw_params_t *swparams;
	int err;

	snd_pcm_sw_params_alloca(&swparams);

	if ((err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "ALSA: Failed to open device 'default' (%s)\n", snd_strerror(err));
		return -1;
	}
	
	if ((err = snd_pcm_set_params(pcm,
		SND_PCM_FORMAT_S16,
		SND_PCM_ACCESS_RW_INTERLEAVED,
		2, // channels
		mixFrequency,
		1, // allow soft resampling
		(2000000 * bufferSizeInWords) / mixFrequency)) < 0)
	{
		fprintf(stderr, "ALSA: Playback open error (%s)\n", snd_strerror(err));
		return -1;
	}
	snd_pcm_get_params(pcm, &buffer_size, &period_size);
	stream = new char[period_size * 4];
	printf("ALSA: Buffer size = %i samples (requested %i) (buffer size = %i)\n", period_size, bufferSizeInWords / 2, buffer_size);

	/* get the current swparams */
	err = snd_pcm_sw_params_current(pcm, swparams);
	if (err < 0) {
		fprintf(stderr, "ALSA: Unable to determine current swparams for playback: %s\n", snd_strerror(err));
		return -1;
	}
	/* start the transfer when the buffer is almost full: */
	/* (buffer_size / avail_min) * avail_min */
	err = snd_pcm_sw_params_set_start_threshold(pcm, swparams, (buffer_size / period_size) * period_size);
	if (err < 0) {
		fprintf(stderr, "ALSA: Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
		return -1;
	}


	return period_size * 2;		// 2 = number of channels
}

mp_sint32 AudioDriver_ALSA::stop()
{
	snd_pcm_drop(pcm);
	deviceHasStarted = false;
	return 0;
}

mp_sint32 AudioDriver_ALSA::closeDevice()
{
	snd_pcm_close(pcm);
	delete[] stream;
	stream = NULL;
	deviceHasStarted = false;
	return 0;
}

void AudioDriver_ALSA::start()
{
	snd_async_handler_t *ahandler;
	int err;

	err = snd_async_add_pcm_handler(&ahandler, pcm, async_direct_callback, this);
	if(err < 0)
	{
		fprintf(stderr, "ALSA: Could not add PCM hander (%s)\n", snd_strerror(err));
	}

	memset(stream, 0, period_size * 4);
 	for (mp_sint32 count = 0; count < 2; count++)
 	{
		err = snd_pcm_writei(pcm, stream, period_size);
		if(err < 0)
			fprintf(stderr, "ALSA: Initial write error (%s)\n", snd_strerror(err));
		if(err != period_size)
			fprintf(stderr, "ALSA: Initial write error (written %i expected %li)\n", err, period_size);
	}


	err = snd_pcm_start(pcm);
	if(err < 0)
	{
		fprintf(stderr, "ALSA: Could not start PCM device (%s)\n", snd_strerror(err));
	}
	
	deviceHasStarted = true;
}

mp_sint32 AudioDriver_ALSA::pause()
{
	snd_pcm_pause(pcm, true);
	return 0;
}

mp_sint32 AudioDriver_ALSA::resume()
{
	snd_pcm_pause(pcm, false);
	return 0;
}

#endif
