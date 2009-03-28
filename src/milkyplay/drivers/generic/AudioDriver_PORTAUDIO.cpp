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
 *  AudioDriver_PORTAUDIO.cpp
 *  SDL Audio
 *
 *  Created by Peter Barth on 09.06.05.
 *
 *  Christopher O'Neill 16/3/06:
 *    fill_audio() rewritten to solve issues on certain systems
 *
 *
 */

#include "AudioDriver_PORTAUDIO.h"

const char*	AudioDriver_PORTAUDIO::driverNames[] =
{
	"WASAPI (PortAudio)"
};

int	AudioDriver_PORTAUDIO::refCount = 0;

int AudioDriver_PORTAUDIO::patestCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
	framesPerBuffer<<=2;
	
	AudioDriver_PORTAUDIO* audioDriver = (AudioDriver_PORTAUDIO*)userData;
	
	// Base class can handle this
	audioDriver->fillAudioWithCompensation((char*)outputBuffer, framesPerBuffer);	
	return paContinue;
}


AudioDriver_PORTAUDIO::AudioDriver_PORTAUDIO() :
	AudioDriver_COMPENSATE(),
	stream(NULL)
{
	if (++refCount == 1)
		Pa_Initialize();
}

AudioDriver_PORTAUDIO::~AudioDriver_PORTAUDIO() 
{
	if (stream)
		closeDevice();

	if (--refCount == 0)
	    Pa_Terminate();
}

// On error return a negative value
// If the requested buffer size can be served return 0, 
// otherwise return the number of 16 bit words contained in the obtained buffer
mp_sint32 AudioDriver_PORTAUDIO::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
		return res;

	int channels = 2;
	int sampleRate = mixFrequency;
	int bufferSize = bufferSizeInWords / channels;

    PaStreamParameters outputParameters;
    PaError err;

    //printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);
    
    err = Pa_Initialize();
    if( err != paNoError ) return -1;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    outputParameters.channelCount = channels;       /* stereo output */
    outputParameters.sampleFormat = paInt16; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              sampleRate,
			  bufferSize,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              patestCallback,
              this);

    if( err != paNoError ) return -1;

#ifndef WIN32
	printf("Audio buffer: Wanted %d bytes, got %d\n", bufferSizeInWords / channels * 4, bufferSize * 4);
#endif

	// If we got what we requested, return 0,
	// otherwise return the actual number of samples * number of channels
	return (bufferSizeInWords / channels == bufferSize) ? 0 : bufferSize * channels;
}

mp_sint32 AudioDriver_PORTAUDIO::stop()
{
    PaError err = Pa_StopStream( stream );
    if( err != paNoError ) return -1;
	deviceHasStarted = false;
	return 0;
}

mp_sint32 AudioDriver_PORTAUDIO::closeDevice()
{	
	if (deviceHasStarted)
		stop();
	//Pa_Sleep(1000);
    PaError err = Pa_CloseStream( stream );
    if( err != paNoError ) return -1;
	stream = NULL;
	//Pa_Sleep(1000);
	return 0;
}

mp_sint32 AudioDriver_PORTAUDIO::start()
{
	// hopefully this works
	// no error checking performed
	PaError err = Pa_StartStream( stream );
	if (err != paNoError)
	{
		deviceHasStarted = false;
		return -1;
	}

	deviceHasStarted = true;
	return 0;
}

mp_sint32 AudioDriver_PORTAUDIO::pause()
{
	return 0;
}

mp_sint32 AudioDriver_PORTAUDIO::resume()
{
	return 0;
}
