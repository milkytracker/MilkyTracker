/*
 *  AudioDriver_WAVWriter.h
 *  Audio Test
 *
 *  Created by Peter Barth on 29.07.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef __AUDIODRIVER_WAVWRITER_H__
#define __AUDIODRIVER_WAVWRITER_H__

#include "AudioDriver_NULL.h"
#include "XMFile.h"

class WAVWriter : public AudioDriver_NULL
{
private:
	XMFile*		f;
	mp_sint32	mixFreq;
	
public:
				WAVWriter(const SYSCHAR* fileName);

	virtual		~WAVWriter();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual		const char* getDriverID() { return "WAVWriter"; }

	virtual		void		advance();

	bool					isOpen() { return f != NULL; }
};

#endif
