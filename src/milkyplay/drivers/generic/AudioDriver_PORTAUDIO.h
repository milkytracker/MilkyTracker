/*
 *  AudioDriver_PORTAUDIO.h
 *  SDL Audio
 *
 *  Created by Peter Barth on 09.06.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */
#ifndef __AUDIODRIVER_PORTAUDIO_H__
#define __AUDIODRIVER_PORTAUDIO_H__

#include "portaudio.h"
#include "AudioDriver_COMPENSATE.h"

class AudioDriver_PORTAUDIO : public AudioDriver_COMPENSATE
{
private:
	static const char*	driverNames[];

	static int			refCount;

	static int			patestCallback(const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData);

	PaStream*			stream;

public:
				AudioDriver_PORTAUDIO();

	virtual		~AudioDriver_PORTAUDIO();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     void		start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();
	
	virtual		const char* getDriverID() { return driverNames[0]; }
};

#endif
