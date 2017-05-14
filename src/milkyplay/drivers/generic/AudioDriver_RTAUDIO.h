/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
	// The following enum is copied from RtAudio.h, keep synchronised
	enum Api
	{
		UNSPECIFIED,    /*!< Search for a working compiled API. */
		LINUX_ALSA,     /*!< The Advanced Linux Sound Architecture API. */
		LINUX_PULSE,    /*!< The Linux PulseAudio API. */
		LINUX_OSS,      /*!< The Linux Open Sound System API. */
		UNIX_JACK,      /*!< The Jack Low-Latency Audio Server API. */
		MACOSX_CORE,    /*!< Macintosh OS-X Core Audio API. */
		WINDOWS_WASAPI, /*!< The Microsoft WASAPI API. */
		WINDOWS_ASIO,   /*!< The Steinberg Audio Stream I/O API. */
		WINDOWS_DS,     /*!< The Microsoft Direct Sound API. */
		RTAUDIO_DUMMY   /*!< A compilable but non-functional API. */
	};
	AudioDriver_RTAUDIO(Api audioApi = UNSPECIFIED);

	virtual		~AudioDriver_RTAUDIO();
			
	virtual		mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual		mp_uint32	getMixFrequency() const;
	virtual		mp_sint32   closeDevice();
	virtual		mp_sint32	start();
	virtual		mp_sint32   stop();
	virtual		mp_sint32   pause();
	virtual		mp_sint32   resume();
	virtual		mp_uint32	getNumPlayedSamples() const;
	virtual		mp_uint32	getBufferPos() const;
	virtual		bool		supportsTimeQuery() const;
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
