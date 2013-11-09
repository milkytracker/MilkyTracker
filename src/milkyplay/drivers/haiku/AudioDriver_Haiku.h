/*
 * Copyright (c) 2012, The MilkyTracker Team.
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

#ifndef __AUDIODRIVER_HAIKU_H__
#define __AUDIODRIVER_HAIKU_H__

#include "AudioDriverBase.h"

#include <SupportDefs.h>

class  BSoundPlayer;
struct media_raw_audio_format;

enum {
	kChannelCount = 2
};


class AudioDriver_Haiku : public AudioDriverBase
{
public:
				AudioDriver_Haiku();
	virtual		~AudioDriver_Haiku();

// --- AudioDriverBase API ----------------------------------------------------
public:
	virtual 	mp_sint32   initDevice(mp_sint32 bufferSizeInWords,
								mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual		mp_sint32	closeDevice();

	virtual 	mp_sint32	start();
	virtual		mp_sint32	stop();
	virtual		mp_sint32	pause();
	virtual		mp_sint32	resume();

	virtual		mp_uint32	getNumPlayedSamples() const;
	virtual		bool		supportsTimeQuery() const
								{ return false; }

	virtual		const char* getDriverID()
								{ return "Haiku MediaKit"; }

	virtual		mp_sint32	getPreferredBufferSize() const;

// --- MediaKit playback ------------------------------------------------------
private:
	static		void		_FillBuffer(void* theCookie, void* buffer,
		size_t size, const media_raw_audio_format& format);

				BSoundPlayer* fSoundPlayer;
};

#endif // __AUDIODRIVER_HAIKU_H__
