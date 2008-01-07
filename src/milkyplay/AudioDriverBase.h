/*
 *  milkyplay/AudioDriverBase.h
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
 *  AudioDriverBase.h
 *  Audio Test
 *
 *  Created by Peter Barth on Fri Feb 25 2005.
 *
 */
#ifndef __AUDIODRIVERBASE_H__
#define __AUDIODRIVERBASE_H__

#include "MilkyPlayTypes.h"

// WAV Header & mixing buffer info
#define MP_NUMCHANNELS 2
#define MP_NUMBITS 16
#define MP_NUMBYTES (NUMBITS>>3)

class MasterMixer;

// -------------------------------------------------------------------------
// Important note: On construction an AudioDriver instance should not
// hook any real resources, so it should be possible to instantiate
// different audio driver implementations on the same plattform
// accessing the same audio device without interfering each other.
// -------------------------------------------------------------------------
// all critical functions are supposed to return win32 waveOut legacy API 
// error codes which are:
//  0 = no error
// -1 = no free device
// -2 = can't get device ID
// -3 = can't get device capabilities
// -4 = device can't handle requested format
// -5 = can't close device
// -6 = can't open device
// -7 = out of memory
// -8 = unknown error
// -------------------------------------------------------------------------
class AudioDriverBase
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
		return 0; 
	}
	
				mp_uint32	getMixFrequency() const { return mixFrequency; }
	
	// close device, unhook resources
	virtual		mp_sint32   closeDevice() = 0;

	// start device
	virtual		void		start() = 0;
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
	virtual		mp_uint32	getNumPlayedSamples() const { return 0; }
	// returns the position within the buffer
	virtual		mp_uint32	getBufferPos() const { return 0; }
	// if the device supports query of how many samples are played since 
	// start has been called, return true here
	virtual		bool		supportsTimeQuery() { return false; }

	// should be kinda unique
	virtual		const char* getDriverID() = 0;

	// required by wav/null drivers, ignore if you're not writing a wav writer
	virtual		void		advance() { }

	// should return preferred buffer size for 44.1khz
	virtual		mp_sint32	getPreferredBufferSize() = 0;
	
	virtual		void		msleep(mp_uint32 msecs);
	virtual		bool		isMixerActive();	
	virtual		void		setIdle(bool idle);
};

#endif
