/*
 *  milkyplay/AudioDriver_COMPENSATE.h
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
 *  AudioDriver_COMPENSATE.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 22.04.06.
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

