/*
 *  AudioDriver_JACK.h
 *  JACK Audio
 *
 *  Created by Christopher O'Neill on 8/12/07
 *
 *  Copyright (c) 2007 milkytracker.net, All rights reserved.
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

public:
				AudioDriver_JACK();

	virtual		~AudioDriver_JACK();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     void		start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();
	
	virtual		bool		supportsPowerOfTwoCompensation() { return true; }

	virtual		const char* getDriverID() { return "JACK"; }
	virtual		mp_sint32	getPreferredBufferSize() { return 2048; }
	
};

#endif
