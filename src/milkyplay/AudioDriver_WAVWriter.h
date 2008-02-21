/*
 *  milkyplay/AudioDriver_WAVWriter.h
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
 *  AudioDriver_WAVWriter.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 29.07.05.
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
