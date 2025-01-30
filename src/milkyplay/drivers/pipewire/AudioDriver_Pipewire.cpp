/*
 * Copyright (c) 2025, The MilkyTracker Team.
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
 * AudioDriver_Pipewire.cpp
 * Pipewire Audio driver for MilkyTracker
 * (it's not that good)
 *
 * Created by Evan Walter on 27/1/2025
 */

#include "pipewire/stream.h"
#include <cstdint>
#ifdef HAVE_PIPEWIRE
#include "AudioDriver_Pipewire.h"
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
void AudioDriver_Pipewire::on_stream_param_change(void *userdata, uint32_t id, const struct spa_pod *param) {
	AudioDriver_Pipewire *driver = (AudioDriver_Pipewire *) userdata;
	struct pw_stream *stream = driver->stream;
	uint8_t params_pod_buf[1024];
	struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(params_pod_buf, sizeof(params_pod_buf));
	const struct spa_pod *params[2];
	if (param == NULL || id != SPA_PARAM_Format)
		return;

	printf("Pipewire: negotiating buffer format\n");

	params[0] = (struct spa_pod *) spa_pod_builder_add_object(&builder,
			SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers,
			SPA_PARAM_BUFFERS_buffers, SPA_POD_Int(1),
			SPA_PARAM_BUFFERS_blocks, SPA_POD_Int(1),
			SPA_PARAM_BUFFERS_size, SPA_POD_Int(2 * sizeof(int16_t) * driver->period),
			SPA_PARAM_BUFFERS_stride, SPA_POD_Int(2 * sizeof(int16_t)),
			SPA_PARAM_BUFFERS_dataType, SPA_POD_CHOICE_FLAGS_Int(1 << SPA_DATA_MemPtr),
			SPA_PARAM_BUFFERS_metaType, SPA_POD_Int(0),
			SPA_PARAM_BUFFERS_align, SPA_POD_Int(sizeof(int)));


	pw_stream_update_params(stream, params, 1);
	printf("Pipewire: stream parameters updated\n");
}
 
void AudioDriver_Pipewire::on_process(void *userdata)
{
	struct pw_buffer *b;
	struct spa_buffer *buf_spa;
	int16_t *dst;

	AudioDriver_Pipewire *driver = (AudioDriver_Pipewire *) userdata;
	const size_t frame_size = sizeof(int16_t) * 2;

	if ((b = pw_stream_dequeue_buffer(driver->stream)) == NULL) 
	{
		return;
	}

	// SPA info for the buffer we're to fill this time
	buf_spa = b->buffer;
	int max_buf_size = buf_spa->datas[0].maxsize;
	if ((dst = (int16_t *) buf_spa->datas[0].data) == NULL)
		return;
	if (max_buf_size != frame_size * driver->period)
		return;

	struct spa_chunk *chunk = buf_spa->datas[0].chunk;
	chunk->offset = 0;
	chunk->stride = frame_size;
	chunk->size = max_buf_size;

	driver->fillAudioWithCompensation((char *) dst, max_buf_size);

	pw_stream_queue_buffer(driver->stream, b);
}

AudioDriver_Pipewire::AudioDriver_Pipewire() :
	AudioDriver_COMPENSATE()
{ 
}

AudioDriver_Pipewire::~AudioDriver_Pipewire()
{
}

mp_sint32 AudioDriver_Pipewire::initDevice(mp_sint32 periodSizeAsSamples, const mp_uint32 mixFrequency, MasterMixer* mixer)
{
	const struct spa_pod *params[1];
	uint8_t buffer[1024];
	struct spa_pod_builder b; 
	events = new pw_stream_events();
	events->version = PW_VERSION_STREAM_EVENTS;
	events->process = AudioDriver_Pipewire::on_process;
	events->param_changed = AudioDriver_Pipewire::on_stream_param_change;
	period = periodSizeAsSamples;

	printf("Pipewire: Beginning device init\n");

	b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
	pw_init(NULL, NULL);

	printf("Pipewire: Initialized Pipewire\n");

	pw_loop *loop = pw_loop_new(NULL);
	thread_loop = pw_thread_loop_new_full(loop, "MilkyTracker", NULL);

	printf("Pipewire: Created thread loop\n");

	pw_thread_loop_lock(thread_loop);
	pw_loop *inner_loop = pw_thread_loop_get_loop(thread_loop);
	pw_thread_loop_unlock(thread_loop);

	stream = pw_stream_new_simple(
			inner_loop,
			"audio-src",
			pw_properties_new(
				PW_KEY_MEDIA_TYPE, "Audio",
				PW_KEY_MEDIA_CATEGORY, "Playback",
				NULL),
			events,
			this);

	printf("Pipewire: Created stream\n");

	// We need to spell out the whole thing ourselves: since C++
	// is really picky about struct designator syntax, the macro from PW
	// doesn't work
	spa_audio_info_raw audio_init_params;
	audio_init_params.format = SPA_AUDIO_FORMAT_S16;
	audio_init_params.rate = mixFrequency;
	audio_init_params.channels = 2;

	params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
			&audio_init_params);

	pw_stream_flags flags = (pw_stream_flags) (
			(int) PW_STREAM_FLAG_AUTOCONNECT 
			| (int) PW_STREAM_FLAG_MAP_BUFFERS 
			// | (int) PW_STREAM_FLAG_DRIVER
			);

	pw_stream_connect(
			stream,
			PW_DIRECTION_OUTPUT,
			PW_ID_ANY,
			flags,
			params, 1);

	printf("Pipewire: Connected stream\n");

	deviceHasStarted = 0;
	AudioDriverBase::initDevice(periodSizeAsSamples, mixFrequency, mixer);
	return periodSizeAsSamples * 2;
}

mp_sint32 AudioDriver_Pipewire::closeDevice()
{
	printf("Pipewire: closing device\n");
	if (thread_loop) 
	{
		pw_thread_loop_stop(thread_loop);
		pw_stream_destroy(stream);
		pw_thread_loop_destroy(thread_loop);
		pw_deinit();
		printf("Pipewire: app closed\n");
	}
	return 0;
}


mp_sint32 AudioDriver_Pipewire::start() 
{
	if (!deviceHasStarted)
	{
		pw_thread_loop_lock(thread_loop);
		pw_thread_loop_start(thread_loop);
		pw_thread_loop_unlock(thread_loop);
		deviceHasStarted = 1;
		printf("Pipewire: started device\n");
	}
	printf("Pipewire: exiting start\n");
	return 0;
}

mp_sint32 AudioDriver_Pipewire::stop() 
{
	printf("Pipewire: stopping device\n");
	if (deviceHasStarted) 
	{
		pw_thread_loop_stop(thread_loop);
		deviceHasStarted = 0;
		printf("Pipewire: stopped device\n");
	}
	printf("Pipewire: exiting stop function\n");
	return 0;
}

mp_sint32 AudioDriver_Pipewire::pause()
{
	stop();
	return 0;
}

mp_sint32 AudioDriver_Pipewire::resume()
{
	start();
	return 0;
}

#endif
