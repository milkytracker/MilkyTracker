/*
 *  AudioDriver_SDL.h
 *  SDL Audio
 *
 *  Created by Peter Barth on 09.06.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */
#ifndef __AUDIODRIVER_SDL_H__
#define __AUDIODRIVER_SDL_H__

#ifdef WIN32
	#include "SDL.h"
	#include "SDL_audio.h"
#else
	#include <SDL/SDL.h>
	#include <SDL/SDL_audio.h>
#endif

#include "AudioDriver_COMPENSATE.h"

class AudioDriver_SDL : public AudioDriver_COMPENSATE
{
private:
	SDL_AudioSpec	wanted; 
	
	static void fill_audio(void *udata, Uint8 *stream, int len); 
									 
public:
				AudioDriver_SDL();

	virtual		~AudioDriver_SDL();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     void		start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();
	
	virtual		const char* getDriverID() { return "SDLAudio"; }
	virtual		mp_sint32	getPreferredBufferSize() { return 2048; }	
};

#endif
