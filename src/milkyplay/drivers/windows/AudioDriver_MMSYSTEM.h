/*
 *  AudioDriver_MMSYSTEM.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Fri Sep 10 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
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

	virtual		void		start();
	virtual		mp_sint32   stop();

	virtual		mp_sint32   pause();
	virtual		mp_sint32   resume();

	virtual		mp_uint32	getNumPlayedSamples() const;
	virtual		mp_uint32	getBufferPos() const;
	virtual		bool		supportsTimeQuery() { return true; }

	virtual		const char* getDriverID() { return timeEmulation ? "WaveOut (Vista)" : "WaveOut (old)"; }
	virtual		mp_sint32	getPreferredBufferSize() { return 8192; }
};

#endif
