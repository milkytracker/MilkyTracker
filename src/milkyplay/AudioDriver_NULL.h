/*
 *  AudioDriver_NULL.h
 *  Audio Test
 *
 *  Created by Peter Barth on 29.07.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef __AUDIODRIVER_NULL_H__
#define __AUDIODRIVER_NULL_H__

#include "AudioDriverBase.h"
#include "MilkyPlayTypes.h"

class AudioDriver_NULL : public AudioDriverBase
{
protected:
	mp_uint32	numSamplesWritten;
	mp_sword*	compensateBuffer;
	
public:
				AudioDriver_NULL();

	virtual		~AudioDriver_NULL();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     void		start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();

	virtual		mp_uint32	getNumPlayedSamples() const { return numSamplesWritten; };

	virtual		const char* getDriverID() { return "NULL"; }
	virtual		mp_sint32	getPreferredBufferSize() { return 0; }

	virtual		void		advance();

};

#endif
