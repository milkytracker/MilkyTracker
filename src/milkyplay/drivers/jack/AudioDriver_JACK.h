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
 *  AudioDriver_JACK.h
 *  JACK Audio
 *
 *  Created by Christopher O'Neill on 8/12/07
 *
 *
 */
#ifndef __AUDIODRIVER_JACK_H__
#define __AUDIODRIVER_JACK_H__

#include "AudioDriver_COMPENSATE.h"
#include <jack/jack.h>

class AudioDriver_JACK : public AudioDriver_COMPENSATE
{
private:
	jack_client_t *hJack;
	jack_port_t *leftPort, *rightPort;
	mp_sword *rawStream;
	int jackFrames;
	bool paused;
	void *libJack;

	static int jackProcess(jack_nframes_t nframes, void *arg);

	// Jack library functions
	jack_client_t *(*jack_client_new) (const char *client_name);
	int (*jack_client_close) (jack_client_t *client);
	int (*jack_set_process_callback) (jack_client_t *client,
					JackProcessCallback process_callback,
					void *arg);
	int (*jack_activate) (jack_client_t *client);
	int (*jack_deactivate) (jack_client_t *client);
	jack_port_t *(*jack_port_register) (jack_client_t *client,
					const char *port_name,
					const char *port_type,
					unsigned long flags,
					unsigned long buffer_size);
	void *(*jack_port_get_buffer) (jack_port_t *, jack_nframes_t);
	jack_nframes_t (*jack_get_buffer_size) (jack_client_t *);
	jack_nframes_t (*jack_get_sample_rate) (jack_client_t *);
	const char ** (*jack_get_ports) (jack_client_t *, 
					const char *port_name_pattern, 
					const char *type_name_pattern, 
					unsigned long flags);
	int (*jack_connect) (jack_client_t *, 
					const char *source_port, 
					const char *destination_port);
	const char* (*jack_port_name) (const jack_port_t *);  


public:
				AudioDriver_JACK();

	virtual		~AudioDriver_JACK();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     mp_sint32	start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();
	
	virtual		bool		supportsPowerOfTwoCompensation() { return true; }

	virtual		const char* getDriverID() { return "JACK"; }
	virtual		mp_sint32	getPreferredBufferSize() const { return 2048; }
	
};

#endif
