/*
 *  milkyplay/drivers/windows/AudioDriver_MMSYSTEM.h
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
	bool		deviceHasStarted;

	HWAVEOUT	hwo;
	WAVEOUTCAPS waveoutcaps;
	WAVEHDR		wavhdr[NUMBUFFERS];
	mp_sword*	mixbuff16[NUMBUFFERS];
	mp_sint32   currentBufferIndex;

	mp_uint32	sampleCounterTotal;

	bool		timeEmulation;
	
	mp_uint32	sampleRate;

	mutable	mp_uint32			lastSampleIndex;
	mutable	mp_uint32			lastTimeInMillis;
	mutable	mp_uint32			timeInSamples;
	mutable	mp_uint32			sampleCounter;
	mutable	CRITICAL_SECTION	cs;

	static void CALLBACK waveOutProc(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);

	void		kick();

public:
				AudioDriver_MMSYSTEM(bool timeEmulation = false);
	virtual		~AudioDriver_MMSYSTEM();
			
	virtual		mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual		mp_sint32   closeDevice();

	virtual		mp_sint32	start();
	virtual		mp_sint32   stop();

	virtual		mp_sint32   pause();
	virtual		mp_sint32   resume();

	virtual		mp_uint32	getNumPlayedSamples() const;
	virtual		mp_uint32	getBufferPos() const;
	virtual		bool		supportsTimeQuery() { return true; }

	virtual		const char* getDriverID() { return timeEmulation ? "WaveOut (Vista)" : "WaveOut (old)"; }

	virtual		mp_sint32	getPreferredSampleRate() const { return 44100; }
	virtual		mp_sint32	getPreferredBufferSize() const { return 8192; }
};

#endif
