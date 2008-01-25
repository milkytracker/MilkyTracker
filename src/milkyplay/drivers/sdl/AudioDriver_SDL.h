/*
 *  milkyplay/drivers/sdl/AudioDriver_SDL.h
 *
 *  Copyright 2008 Peter Barth
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
 *  AudioDriver_SDL.h
 *  SDL Audio
 *
 *  Created by Peter Barth on 09.06.05.
 *
 */
#ifndef __AUDIODRIVER_SDL_H__
#define __AUDIODRIVER_SDL_H__

// You might want to change this
#ifdef __OSX_SILLYNESS__
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#else
#include <SDL.h>
#include <SDL_audio.h>
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
