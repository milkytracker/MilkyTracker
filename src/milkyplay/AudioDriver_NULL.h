/*
 *  milkyplay/AudioDriver_NULL.h
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
 *  AudioDriver_NULL.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 29.07.05.
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
	virtual		mp_sint32	getPreferredBufferSize() const { return 0; }

	virtual		void		advance();

};

#endif
