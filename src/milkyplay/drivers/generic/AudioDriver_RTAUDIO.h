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

#include "AudioDriver_COMPENSATE.h"

class AudioDriver_RTAUDIO : public AudioDriverInterface
{
private:
	AudioDriverInterface* impl;
	
public:
	enum Api 
	{
		UNSPECIFIED,    /*!< Search for a working compiled API. */
		LINUX_ALSA,     /*!< The Advanced Linux Sound Architecture API. */
		LINUX_OSS,      /*!< The Linux Open Sound System API. */
		UNIX_JACK,      /*!< The Jack Low-Latency Audio Server API. */
		MACOSX_CORE,    /*!< Macintosh OS-X Core Audio API. */
		WINDOWS_ASIO,   /*!< The Steinberg Audio Stream I/O API. */
		WINDOWS_DS,     /*!< The Microsoft Direct Sound API. */
		RTAUDIO_DUMMY   /*!< A compilable but non-functional API. */
	};
	
	enum Version
	{
		V3,
		V4
	};
	
	
#ifdef __OSX_PANTHER__
				AudioDriver_RTAUDIO(Api audioApi = UNSPECIFIED, Version version = V3);
#else
				AudioDriver_RTAUDIO(Api audioApi = UNSPECIFIED, Version version = V4);
#endif

	virtual		~AudioDriver_RTAUDIO();
			
	virtual		mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual		mp_uint32	getMixFrequency() const;
	virtual		mp_sint32   closeDevice();
	virtual		void		start();
	virtual		mp_sint32   stop();
	virtual		mp_sint32   pause();
	virtual		mp_sint32   resume();
	virtual		mp_uint32	getNumPlayedSamples() const;
	virtual		mp_uint32	getBufferPos() const;
	virtual		bool		supportsTimeQuery();
	virtual		const char* getDriverID();
	virtual		void		advance();
	virtual		mp_sint32	getPreferredSampleRate() const;
	virtual		mp_sint32	getPreferredBufferSize() const;
	virtual		void		msleep(mp_uint32 msecs);
	virtual		bool		isMixerActive();
	virtual		void		setIdle(bool idle);
	
	void createRt4Instance(Api audioApi = UNSPECIFIED);
	void createRt3Instance(Api audioApi = UNSPECIFIED);
};

#endif
