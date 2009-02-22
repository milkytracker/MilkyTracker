/*
 *  milkyplay/drivers/psp/AudioDriver_PSP.h
 *
 *  Copyright 2009 Peter Barth
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
 *  AudioDriver_PSP.h
 *  PSPAudio
 *
 *  Created by Shazz
 *
 */
#ifndef __AUDIODRIVER_PSP_H__
#define __AUDIODRIVER_PSP_H__

#include "AudioDriver_COMPENSATE.h"

class AudioDriver_PSP : public AudioDriver_COMPENSATE
{
private:
	mp_uint32	psp_channel;
	bool		didInit;

	static void fill_audio(void *udata, unsigned int numSamples, void *userdata);

public:
			AudioDriver_PSP();

	virtual		~AudioDriver_PSP();

	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     mp_sint32	start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();

	virtual	    const char* getDriverID() { return "PSPAudio"; }

	virtual		mp_sint32	getPreferredSampleRate() const { return 44100; }
	virtual		mp_sint32	getPreferredBufferSize() const { return 1024; }	
};

#endif /* __AUDIODRIVER_PSP_H__ */
