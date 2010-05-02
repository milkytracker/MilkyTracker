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
 *  AudioDriverBase.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on Fri Feb 25 2005.
 *
 */
#ifndef __AUDIODRIVERBASE_H__
#define __AUDIODRIVERBASE_H__

#include "MilkyPlayTypes.h"
#include "MilkyPlayResults.h"

// WAV Header & mixing buffer info
#define MP_NUMCHANNELS 2
#define MP_NUMBITS 16
#define MP_NUMBYTES (NUMBITS>>3)

class MasterMixer;

class AudioDriverInterface
{
public:
	virtual ~AudioDriverInterface()
	{
	}

	// init device, hook resources
	virtual		mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer) = 0;

	virtual		mp_uint32	getMixFrequency() const = 0;
	
	// close device, unhook resources
	virtual		mp_sint32   closeDevice() = 0;

	// start device
	virtual		mp_sint32	start() = 0;
	// stop device 
	virtual		mp_sint32   stop() = 0;

	// pause the device:
	// this is actually a little bit awkward, because it was meant to
	// be an interface for waveOutPause (win32) which immediately pauses the 
	// device no matter how large the buffer size is whereas stop waits 
	// until all pending buffers have been played which is not suitable for 
	// a client player application.
	// You *should* implement this anyway, just set a flag and don't 
	// output any audio when the flag is set or something
	virtual		mp_sint32   pause() = 0;
	// resume the device from paused state
	virtual		mp_sint32   resume() = 0;

	// if the device supports query of how many samples are played since 
	// start has been called, return number of samples here
	virtual		mp_uint32	getNumPlayedSamples() const = 0;
	// returns the position within the buffer
	virtual		mp_uint32	getBufferPos() const = 0;
	// if the device supports query of how many samples are played since 
	// start has been called, return true here
	virtual		bool		supportsTimeQuery() const = 0;

	// should be kinda unique
	virtual		const char* getDriverID() = 0;

	// required by wav/null drivers, ignore if you're not writing a wav writer
	virtual		void		advance() = 0;

	// return preferred sample rate here
	virtual		mp_sint32	getPreferredSampleRate() const = 0;
	
	// should return preferred buffer size for preferred sample rate (see above)
	virtual		mp_sint32	getPreferredBufferSize() const = 0;
	
	virtual		void		msleep(mp_uint32 msecs) = 0;
	virtual		bool		isMixerActive() = 0;
	virtual		void		setIdle(bool idle) = 0;
};

// -------------------------------------------------------------------------
// Important note: On construction an AudioDriver instance should not
// hook any real resources, so it should be possible to instantiate
// different audio driver implementations on the same plattform
// accessing the same audio device without interfering each other.
// -------------------------------------------------------------------------
// For error codes see MilkyPlayResults.h
// -------------------------------------------------------------------------
class AudioDriverBase : public AudioDriverInterface
{
protected:
	MasterMixer*	mixer;	
	mp_sint32		bufferSize;
	mp_uint32		mixFrequency;
	bool			mono;

	bool			idle;
	bool			markedAsIdle;

public:
				AudioDriverBase() :
					mixer(0),
					bufferSize(0),
					mixFrequency(1),
					mono(false),
					idle(false),
					markedAsIdle(false)
				{
				}
			
	virtual     ~AudioDriverBase() {}
	
	// init device, hook resources
	virtual		mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer) 
	{ 
		this->mixer = mixer; 
		bufferSize = bufferSizeInWords;
		this->mixFrequency = mixFrequency;
		return MP_OK; 
	}
	
				mp_uint32	getMixFrequency() const { return mixFrequency; }
	
	// if the device supports query of how many samples are played since 
	// start has been called, return number of samples here
	virtual		mp_uint32	getNumPlayedSamples() const { return 0; }
	// returns the position within the buffer
	virtual		mp_uint32	getBufferPos() const { return 0; }
	// if the device supports query of how many samples are played since 
	// start has been called, return true here
	virtual		bool		supportsTimeQuery() const { return false; }

	// required by wav/null drivers, ignore if you're not writing a wav writer
	virtual		void		advance() { }

	// return preferred sample rate here
	virtual		mp_sint32	getPreferredSampleRate() const { return 48000; }
	
	virtual		void		msleep(mp_uint32 msecs);
	virtual		bool		isMixerActive();	
	virtual		void		setIdle(bool idle);
};

#endif
