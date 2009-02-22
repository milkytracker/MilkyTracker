/*
 *  milkyplay/drivers/ALSA/AudioDriver_ALSA.h
 *
 *  Copyright 2009 Peter Barth, Christopher O'Neill
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
 *  AudioDriver_ALSA.h
 *  ALSA Audio
 *
 *  Created by Christopher O'Neill on 19/1/2008
 *
 */
#ifndef __AUDIODRIVER_ALSA_H__
#define __AUDIODRIVER_ALSA_H__

#include "AudioDriver_COMPENSATE.h"
#include <alsa/asoundlib.h>

class AudioDriver_ALSA : public AudioDriver_COMPENSATE
{
private:
	snd_pcm_t *pcm;
	char *stream;
	snd_pcm_uframes_t period_size;

	static void async_direct_callback(snd_async_handler_t *ahandler);

public:
				AudioDriver_ALSA();

	virtual		~AudioDriver_ALSA();
			
	virtual     mp_sint32   initDevice(mp_sint32 periodSizeAsSamples, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     mp_sint32	start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();
	
	virtual		const char* getDriverID() { return "ALSA"; }
	virtual		mp_sint32	getPreferredBufferSize() const { return 2048; }
};

#endif
