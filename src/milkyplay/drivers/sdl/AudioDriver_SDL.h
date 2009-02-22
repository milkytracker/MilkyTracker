/*
 *  milkyplay/drivers/sdl/AudioDriver_SDL.h
 *
 *  Copyright 2009 Peter Barth
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
 *  MilkyPlay
 *
 *  Created by Peter Barth on 09.06.05.
 *
 */
#ifndef __AUDIODRIVER_SDL_H__
#define __AUDIODRIVER_SDL_H__

#include <SDL.h>
#include <SDL_audio.h>

#include "AudioDriver_COMPENSATE.h"

class AudioDriver_SDL : public AudioDriver_COMPENSATE
{
private:
	mp_uint32	periodSize;
	
	static void SDLCALL fill_audio(void *udata, Uint8 *stream, int len); 
									 
public:
				AudioDriver_SDL();

	virtual		~AudioDriver_SDL();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     mp_sint32	start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();
	
	virtual		const char* getDriverID() { return "SDLAudio"; }
	virtual		mp_sint32	getPreferredBufferSize() const { return 2048; }	
};

#endif
