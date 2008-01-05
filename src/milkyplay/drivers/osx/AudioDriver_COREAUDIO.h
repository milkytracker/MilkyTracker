/*
 *  AudioDriver.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Fri Sep 10 2004.
 *  Copyright (c) 2004 milkytracker.net. All rights reserved.
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
