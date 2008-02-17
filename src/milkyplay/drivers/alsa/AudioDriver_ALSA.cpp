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

// Hack to simplify build scripts
#ifdef HAVE_LIBASOUND
#include "AudioDriver_ALSA.h"

void AudioDriver_ALSA::async_direct_callback(snd_async_handler_t *ahandler)
{
	snd_pcm_t *handle = snd_async_handler_get_pcm(ahandler);
	AudioDriver_ALSA* audioDriver = (AudioDriver_ALSA*) snd_async_handler_get_callback_private(ahandler);
	const snd_pcm_channel_area_t *my_areas;
	snd_pcm_uframes_t offset, frames, size;
	snd_pcm_sframes_t avail, commitres;
	snd_pcm_state_t state;
	int first = 0, err;
	
	while (1) {
		state = snd_pcm_state(handle);
		if (state == SND_PCM_STATE_XRUN) {
			err = snd_pcm_recover(handle, -EPIPE, 0);
			if (err < 0) {
				fprintf(stderr, "ALSA: XRUN recovery failed: %s\n", snd_strerror(err));
			}
			first = 1;
		} else if (state == SND_PCM_STATE_SUSPENDED) {
			err = snd_pcm_recover(handle, ESTRPIPE, 0);
			if (err < 0) {
				fprintf(stderr, "ALSA: SUSPEND recovery failed: %s\n", snd_strerror(err));
			}
		}
		avail = snd_pcm_avail_update(handle);
		if (avail < 0) {
			err = snd_pcm_recover(handle, avail, 0);
			if (err < 0) {
				fprintf(stderr, "ALSA: avail update failed: %s\n", snd_strerror(err));
			}
			first = 1;
			continue;
		}
		if (avail < audioDriver->period_size) {
			if (first) {
				first = 0;
				err = snd_pcm_start(handle);
				if (err < 0) {
					fprintf(stderr, "ALSA: Start error: %s\n", snd_strerror(err));
				}
			} else {
				break;
			}
			continue;
		}
		size = audioDriver->period_size;
		while (size > 0) {
			frames = size;
			err = snd_pcm_mmap_begin(handle, &my_areas, &offset, &frames);
			if (err < 0) {
				if ((err = snd_pcm_recover(handle, err, 0)) < 0) {
					fprintf(stderr, "ALSA: MMAP begin avail error: %s\n", snd_strerror(err));
				}
				first = 1;
			}
			//generate_sine(my_areas, offset, frames, &data->phase);
			audioDriver->fillAudioWithCompensation(static_cast<char*> (my_areas->addr) + offset*4, frames * 2);
			commitres = snd_pcm_mmap_commit(handle, offset, frames);
			if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) {
				if ((err = snd_pcm_recover(handle, commitres >= 0 ? -EPIPE : commitres, 0)) < 0) {
					fprintf(stderr, "ALSA: MMAP commit error: %s\n", snd_strerror(err));
				}
				first = 1;
			}
			size -= frames;
		}
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
	snd_pcm_sw_params_t *swparams;
	int err;

	snd_pcm_sw_params_alloca(&swparams);

	if ((err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "ALSA: Failed to open device 'default' (%s)\n", snd_strerror(err));
		return -1;
	}
	
	if ((err = snd_pcm_set_params(pcm,
		SND_PCM_FORMAT_S16,
		SND_PCM_ACCESS_MMAP_INTERLEAVED,
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
	printf("ALSA: Period size = %i frames (requested %i), buffer size = %i frames\n", period_size, bufferSizeInWords / 2, buffer_size);

	/* get the current swparams */
	err = snd_pcm_sw_params_current(pcm, swparams);
	if (err < 0) {
		fprintf(stderr, "ALSA: Unable to determine current swparams for playback: %s\n", snd_strerror(err));
		return -1;
	}
	/* start the transfer when the buffer is almost full: */
	/* (buffer_size / avail_min) * avail_min */
/*	err = snd_pcm_sw_params_set_start_threshold(pcm, swparams, (buffer_size / period_size) * period_size);
	if (err < 0) {
		fprintf(stderr, "ALSA: Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
		return -1;
	}*/

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
	const snd_pcm_channel_area_t *my_areas;
	snd_pcm_uframes_t offset, frames, size;
	snd_async_handler_t *ahandler;
	int err;
	err = snd_async_add_pcm_handler(&ahandler, pcm, async_direct_callback, this);
	if (err < 0) {
		fprintf(stderr, "ALSA: Unable to register async handler (%s)\n", snd_strerror(err));
	}

	for (int count = 0; count < 2; count++) {
		size = period_size;
		while (size > 0) {
			frames = size;
			err = snd_pcm_mmap_begin(pcm, &my_areas, &offset, &frames);
			if (err < 0) {
				if ((err = snd_pcm_recover(pcm, err, 0)) < 0) {
					fprintf(stderr, "ALSA: MMAP begin error: %s\n", snd_strerror(err));
				}
			}
			// Sanity check
			if (my_areas->step != 32 && my_areas->first != 0)
				fprintf(stderr, "ALSA: Unsupported audio format.\n");
				
			memset(my_areas->addr, 0, buffer_size * 4);
			int commitres = snd_pcm_mmap_commit(pcm, offset, frames);
			if (err < 0 || (snd_pcm_uframes_t)commitres != frames) {
				if ((err = snd_pcm_recover(pcm, commitres >= 0 ? -EPIPE : commitres, 0)) < 0) {
					fprintf(stderr, "ALSA: MMAP commit error: %s\n", snd_strerror(err));
				}
			}
			size -= frames;
		}
	}


	err = snd_pcm_start(pcm);
	if (err < 0)
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
