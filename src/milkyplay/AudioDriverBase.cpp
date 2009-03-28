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
 *  AudioDriverBase.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 15.12.07.
 *
 */

#include "AudioDriverBase.h"
#include "MasterMixer.h"

#if defined(__PSP__)
#include <pspkernel.h>
#elif !defined(WIN32) && !defined(_WIN32_WCE)
#include <unistd.h>
#else
#include <windows.h>
#endif

void AudioDriverBase::msleep(mp_uint32 msecs)
{
#ifdef WIN32
	::Sleep(msecs);
#elif defined(__PSP__)
	sceKernelDelayThreadCB(msecs*1000);
#elif defined(__AROS__)
	// usleep is not implemented on AROS
	if(msecs < 1000) msecs = 1000;
	sleep(msecs/1000);
#else
	usleep(msecs*1000);
#endif
}

bool AudioDriverBase::isMixerActive()
{
	if (idle)
		return false;
		
	if (markedAsIdle)
	{
		markedAsIdle = false;
		idle = true;
		return false;
	}
	
	return (mixer && mixer->isPlaying());
}

void AudioDriverBase::setIdle(bool idle)
{
	if (mixer == 0)
		return;

	if (idle)
	{
		if (markedAsIdle || this->idle)
			return;
			
		markedAsIdle = true;
		
		// this is going to loop infinitely when the audio device is not running
		double waitMillis = ((double)(bufferSize/2) / (double)mixFrequency) * 1000.0 * 2.0;
		if (waitMillis < 1.0)
			waitMillis = 1.0;
		if (waitMillis > 5000.0)
		waitMillis = 5000.0;

		mp_uint32 time = 0;
		const mp_uint32 sleepTime = 10;
		while (!this->idle && time <= (mp_uint32)waitMillis)
		{
			msleep(sleepTime);
			time+=sleepTime;
		}
		if (!this->idle)
			this->idle = true;
	}
	else
	{
		this->idle = false;
		this->markedAsIdle = false;
	}
}
