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
 *  AudioDriver_MMSYSTEM.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Fri Sep 10 2004.
 *
 */
#ifndef __AUDIODRIVER_MMSYSTEM_H__
#define __AUDIODRIVER_MMSYSTEM_H__

#include <windows.h>
#ifndef _WIN32_WCE 
	#include <mmsystem.h>
#endif

#include "AudioDriverBase.h"

class AudioDriver_MMSYSTEM : public AudioDriverBase
{
private:
    enum
    {
        // WAV Header & mixing buffer info
        NUMCHANNELS = MP_NUMCHANNELS,
        NUMBITS = MP_NUMBITS,
        NUMBYTES = MP_NUMBYTES,
        NUMBUFFERS = 2
    };

	bool		paused;
	bool		deviceOpen;
	bool		deviceHasStarted;

	DWORD		bufferThreadId;
	HANDLE		bufferThreadHandle;

	HWAVEOUT	hwo;
	WAVEOUTCAPS waveoutcaps;
	WAVEHDR		wavhdr[NUMBUFFERS];
	mp_sword*	mixbuff16[NUMBUFFERS];
	mp_sint32   currentBufferIndex;

	mp_uint32	sampleCounterTotal;
		
	mp_uint32	sampleRate;

	mutable	mp_uint32			lastSampleIndex;
	mutable	mp_uint32			timeInSamples;
	mutable	mp_uint32			sampleCounter;
	mutable	CRITICAL_SECTION	cs;

	static DWORD WINAPI			ThreadProc(_In_ LPVOID lpParameter);
	void						kick();

public:
				AudioDriver_MMSYSTEM();
	virtual		~AudioDriver_MMSYSTEM();
			
	virtual		mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual		mp_sint32   closeDevice();

	virtual		mp_sint32	start();
	virtual		mp_sint32   stop();

	virtual		mp_sint32   pause();
	virtual		mp_sint32   resume();

	virtual		mp_uint32	getNumPlayedSamples() const;
	virtual		mp_uint32	getBufferPos() const;
	virtual		bool		supportsTimeQuery() const { return true; }

	virtual		const char* getDriverID() { return "WaveOut"; }

	virtual		mp_sint32	getPreferredSampleRate() const { return 44100; }
	virtual		mp_sint32	getPreferredBufferSize() const { return 8192; }
};

#endif
