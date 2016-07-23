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

		frames = audioDriver->period_size;
		err = snd_pcm_mmap_begin(handle, &my_areas, &offset, &frames);
		if (err < 0) {
			if ((err = snd_pcm_recover(handle, err, 0)) < 0) {
				fprintf(stderr, "ALSA: MMAP begin avail error: %s\n", snd_strerror(err));
			}
			first = 1;
		}

		if(frames != audioDriver->period_size)
			fprintf(stderr, "ALSA: Invalid buffer size: %lu (should be %lu), skipping..\n", frames, audioDriver->period_size);
			// Certain audio drivers will periodically request buffers of less than one period when
			// soft-resampling (ie, not running at native frequency).  Milkytracker can't handle this,
			// and bad things happen - so best to warn the user and not process.
			// PS - I've disabled soft-resampling for now (see below) so this shouldn't happen.
			// PPS - The downside is that if the user has the wrong mixer rate, they will get an error
			//       dialog - hopefully they'll read the message on stderr...
		else
			audioDriver->fillAudioWithCompensation(static_cast<char*> (my_areas->addr) + offset*4, frames * 2);

		commitres = snd_pcm_mmap_commit(handle, offset, frames);
		if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) {
			if ((err = snd_pcm_recover(handle, commitres >= 0 ? -EPIPE : commitres, 0)) < 0) {
				fprintf(stderr, "ALSA: MMAP commit error: %s\n", snd_strerror(err));
				// What now?
//				exit(1);
			}
			first = 1;
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
mp_sint32 AudioDriver_ALSA::initDevice(mp_sint32 periodSizeAsSamples, const mp_uint32 mixFrequency, MasterMixer* mixer)
{
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
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
		0, // disallow soft resampling
		(2000000 * static_cast<unsigned long long> (periodSizeAsSamples)) / mixFrequency)) < 0)
			// period size in uS
	{
		fprintf(stderr, "ALSA: Playback open error (%s)\nALSA: Is your mixer frequency correct? Try 48000Hz\nALSA: If you are seeing \"Access type not available for PLAYBACK\" then your audio driver does not support MMAP access, and will not work with this version of MilkyTracker using the ALSA driver (try SDL instead).\n", snd_strerror(err));
		return -1;
	}

	snd_pcm_prepare(pcm);
	period_size = periodSizeAsSamples * 2;
	snd_pcm_get_params(pcm, &buffer_size, &period_size);
	stream = new char[period_size * 2];
	printf("ALSA: Period size = %lu frames (requested %i), buffer size = %lu frames\n", period_size, periodSizeAsSamples / 2, buffer_size);

	/* get the current swparams */
	err = snd_pcm_sw_params_current(pcm, swparams);
	if (err < 0) {
		fprintf(stderr, "ALSA: Unable to determine current swparams for playback: %s\n", snd_strerror(err));
		return -1;
	}

	AudioDriverBase::initDevice(period_size * 2, mixFrequency, mixer);
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

mp_sint32 AudioDriver_ALSA::start()
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

			memset(static_cast<char*> (my_areas->addr) + offset*4, 0, frames * 4);
			int commitres = snd_pcm_mmap_commit(pcm, offset, frames);
			if (err < 0 || (snd_pcm_uframes_t)commitres != frames) {
				if ((err = snd_pcm_recover(pcm, commitres >= 0 ? -EPIPE : commitres, 0)) < 0) {
					fprintf(stderr, "ALSA: MMAP commit error: %s\n", snd_strerror(err));
				}
			}
			size -= frames;
		}
	}

\
	err = snd_pcm_start(pcm);
	if (err < 0)
	{
		fprintf(stderr, "ALSA: Could not start PCM device (%s)\n", snd_strerror(err));
		return -1;
	}

	deviceHasStarted = true;
	return 0;
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
