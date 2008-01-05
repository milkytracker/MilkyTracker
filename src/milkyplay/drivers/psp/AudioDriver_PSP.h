/*
 *  AudioDriver_PSP.h
 *  PSPAudio
 *
 *  Created by Shazz
 *  Copyright (c) 2006 milkytracker.net, All rights reserved.
 *
 */
#ifndef __AUDIODRIVER_PSP_H__
#define __AUDIODRIVER_PSP_H__

#include "AudioDriver_COMPENSATE.h"

class AudioDriver_PSP : public AudioDriver_COMPENSATE
{
private:
	mp_uint32	psp_channel;
	bool		didInit;

	static void fill_audio(void *udata, unsigned int numSamples, void *userdata);

public:
			AudioDriver_PSP();

	virtual		~AudioDriver_PSP();

	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     void		start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();

	virtual	    const char* getDriverID() { return "PSPAudio"; }
	virtual		mp_sint32	getPreferredBufferSize() { return 2048; }	
};

#endif /* __AUDIODRIVER_PSP_H__ */
