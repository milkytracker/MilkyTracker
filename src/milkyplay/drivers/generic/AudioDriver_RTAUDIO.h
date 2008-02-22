/*
 *  milkyplay/drivers/generic/AudioDriver_RTAUDIO.h
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
 *  AudioDriver_RTAUDIO.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 09.06.05.
 *
 */
#ifndef __AUDIODRIVER_RTAUDIO_H__
#define __AUDIODRIVER_RTAUDIO_H__

#include "RtAudio.h"
#include "RtError.h"
#include "AudioDriver_COMPENSATE.h"

class AudioDriver_RTAUDIO : public AudioDriver_COMPENSATE
{
private:
	static const char*	driverNames[];

	RtAudio*			audio;
	RtAudio::Api selectedAudioApi;
	
	static int			fill_audio(void* stream, void*, unsigned int length,  double streamTime, RtAudioStreamStatus status, void *udata);
									 									 
public:
				AudioDriver_RTAUDIO(RtAudio::Api audioApi = RtAudio::UNSPECIFIED);

	virtual		~AudioDriver_RTAUDIO();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     void		start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();

	virtual		const char* getDriverID() { return driverNames[selectedAudioApi]; }
	virtual		mp_sint32	getPreferredBufferSize() const;
};

#endif
