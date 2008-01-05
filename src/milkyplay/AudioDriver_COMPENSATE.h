/*
 *  AudioDriver_COMPENSATE.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 22.04.06.
 *  Copyright 2006 milkytracker.net. All rights reserved.
 *
 */
#ifndef __AUDIODRIVER_COMPENSATE_H__
#define __AUDIODRIVER_COMPENSATE_H__

#include "AudioDriverBase.h"
#include "MilkyPlayCommon.h"
#include "MasterMixer.h"

class AudioDriver_COMPENSATE : public AudioDriverBase
{
protected:
	bool		deviceHasStarted;
	mp_uint32	sampleCounter;
	
public:
	AudioDriver_COMPENSATE() :
		deviceHasStarted(false),
		sampleCounter(0)
	{
	}

	virtual		~AudioDriver_COMPENSATE()
	{
	}

	virtual		mp_uint32	getNumPlayedSamples() const { return sampleCounter; }
	
	void fillAudioWithCompensation(char* stream, int length)
	{
		// sanity check
		if (!this->deviceHasStarted)
			return;
		
		MasterMixer* mixer = this->mixer;

		// Attention: Sample buffer MUST be 16 bit stereo, otherwise this will not work
		this->sampleCounter+=length>>2;
		//mixer->updateSampleCounter(length>>2);

		if (isMixerActive())
			mixer->mixerHandler((mp_sword*)stream);
		else
			memset(stream, 0, length);
	}
};

#endif

