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
 *  AudioDriver.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on Fri Sep 10 2004.
 *
 */
#ifndef __AUDIODRIVER_COREAUDIO_H__
#define __AUDIODRIVER_COREAUDIO_H__

#include <stdio.h>
#include <CoreAudio/AudioHardware.h>

#include "AudioDriverBase.h"

class AudioDriver_COREAUDIO : public AudioDriverBase
{
private:
	bool			defaultDevice;
	char*			driverID;
	mp_uint32		sampleCounter;
	mp_sword*		compensateBuffer;

	AudioObjectID	audioDeviceID;

	mp_sint32		lastError;

	AudioDeviceIOProcID audioIOProcID;

	bool			deviceHasStarted;

	static OSStatus audioIOProc(AudioObjectID			inDevice,
								const AudioTimeStamp*	inNow,
								const AudioBufferList*	inInputData,
								const AudioTimeStamp*	inInputTime,
								AudioBufferList*		outOutputData,
								const AudioTimeStamp*	inOutputTime,
								void*					inClientData);

public:
				AudioDriver_COREAUDIO();
				AudioDriver_COREAUDIO(AudioObjectID deviceID);
	virtual		~AudioDriver_COREAUDIO();
			
	virtual		mp_sint32	initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer);
	virtual		mp_sint32	closeDevice();

	virtual		mp_sint32	start();
	virtual		mp_sint32	stop();

	virtual		mp_sint32	pause();
	virtual		mp_sint32	resume();

	virtual		mp_uint32	getNumPlayedSamples() const { return sampleCounter; }
	
	virtual		const char*	getDriverID();
	virtual		mp_sint32	getPreferredBufferSize() const { return 1024; }
	
	static		OSStatus	getAudioDevices(mp_uint32 &numDevices, AudioObjectID* &deviceIDs);
};

#endif
