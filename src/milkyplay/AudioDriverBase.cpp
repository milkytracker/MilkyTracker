/*
 *  AudioDriverBase.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 15.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
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
