/*
 *  milkyplay/AudioDriverBase.cpp
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
 *  AudioDriverBase.cpp
 MilkyPlay
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
