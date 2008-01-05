/*
 *  AudioDriver_RTAUDIO.h
 *  SDL Audio
 *
 *  Created by Peter Barth on 09.06.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */
#ifndef __AUDIODRIVER_RTAUDIO_H__
#define __AUDIODRIVER_RTAUDIO_H__

#include "RtAudio.h"
#include "RtError.h"
#include "AudioDriver_COMPENSATE.h"

class AudioDriver_RTAUDIO : public AudioDriver_COMPENSATE
{
private:
	static const char*	driverNames[];

	RtAudio*			audio;
	RtAudio::Api selectedAudioApi;
	
	static int			fill_audio(void* stream, void*, unsigned int length,  double streamTime, RtAudioStreamStatus status, void *udata);
									 									 
public:
				AudioDriver_RTAUDIO(RtAudio::Api audioApi = RtAudio::UNSPECIFIED);

	virtual		~AudioDriver_RTAUDIO();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     void		start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();

	virtual		const char* getDriverID() { return driverNames[selectedAudioApi]; }
	virtual		mp_sint32	getPreferredBufferSize();
};

#endif
