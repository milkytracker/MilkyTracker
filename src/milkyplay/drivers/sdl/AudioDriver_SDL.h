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
			
	virtual		mp_sint32	initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual		mp_sint32	closeDevice();

	virtual		mp_sint32	start();
	virtual		mp_sint32	stop();

	virtual		mp_sint32	pause();
	virtual		mp_sint32	resume();
	
	virtual		const char*	getDriverID() { return "SDLAudio"; }
	virtual		mp_sint32	getPreferredBufferSize() const { return 2048; }	
};

#endif
