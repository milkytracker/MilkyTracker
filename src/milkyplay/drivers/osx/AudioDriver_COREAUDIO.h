/*
 *  milkyplay/drivers/osx/AudioDriver_COREAUDIO.h
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
 *  AudioDriver.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Fri Sep 10 2004.
 *
 */
#ifndef __AUDIODRIVER_COREAUDIO_H__
#define __AUDIODRIVER_COREAUDIO_H__

#include <CoreAudio/AudioHardware.h>

#include "AudioDriverBase.h"

class AudioDriver_COREAUDIO : public AudioDriverBase
{
private:
	mp_uint32		sampleCounter;
	mp_sword*		compensateBuffer;

	AudioDeviceID	soundDeviceID;
	mp_sint32		lastError;
	Boolean			IOProcIsInstalled;

	OSStatus		(*gAudioIOProc) (AudioDeviceID, const AudioTimeStamp *,
									 const AudioBufferList *, const AudioTimeStamp *,
									 AudioBufferList *, const AudioTimeStamp *, void *);
									 
	static OSStatus OSX_AudioIOProc16Bit (AudioDeviceID inDevice,
										  const AudioTimeStamp* inNow,
										  const AudioBufferList* inInputData,
										  const AudioTimeStamp* inInputTime,
										  AudioBufferList* outOutputData, 
										  const AudioTimeStamp *inOutputTime,
										  void* inClientData);
										  
	bool		deviceHasStarted;
									 
public:
				AudioDriver_COREAUDIO();
	virtual		~AudioDriver_COREAUDIO();
			
	virtual     mp_sint32   initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual     mp_sint32   closeDevice();

	virtual     void		start();
	virtual     mp_sint32   stop();

	virtual     mp_sint32   pause();
	virtual     mp_sint32   resume();

	virtual		mp_uint32	getNumPlayedSamples() const { return sampleCounter; }
	
	virtual		const char* getDriverID() { return "CoreAudio"; }
	virtual		mp_sint32	getPreferredBufferSize() { return 1024; }		
};

#endif
