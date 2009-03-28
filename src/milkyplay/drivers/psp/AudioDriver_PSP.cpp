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
 *  AudioDriver_PSP.cpp
 *  PSP Audio
 *
 *  Created by Shazz
 *
 *
 *
 */

#include "AudioDriver_PSP.h"
#include <stdio.h>
#include "MasterMixer.h"

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspaudio.h>
#include <pspdisplay.h>
#include <pspctrl.h>

#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include <pspaudio.h>

#define PSP_AUDIO_CHANNEL	0
#define PSP_NB_CHANNELS		2

#define PSP_NUM_AUDIO_CHANNELS 4
/** This is the number of frames you can update per callback, a frame being
 * 1 sample for mono, 2 samples for stereo etc. */
#define PSP_NUM_AUDIO_SAMPLES 1024
#define PSP_VOLUME_MAX 0x8000

typedef void (* pspAudioCallback_t)(void *buf, unsigned int reqn, void *pdata);

typedef struct {
  int threadhandle;
  int handle;
  int volumeleft;
  int volumeright;
  pspAudioCallback_t callback;
  void *pdata;
} psp_audio_channelinfo;

typedef int (* pspAudioThreadfunc_t)(int args, void *argp);

// some function forward declarations
static int pspAudioInit();
static void pspAudioEnd();
static void pspAudioSetChannelCallback(int channel, pspAudioCallback_t callback, void *pdata);
static void pspAudioSetVolume(int channel, int left, int right);

// ------------------------------ driver starts here ------------------------------
void AudioDriver_PSP::fill_audio(void *udata, unsigned int numSamples, void *userdata)
{
	AudioDriver_PSP * audioDriver = (AudioDriver_PSP *)userdata;

	if(audioDriver->deviceHasStarted && audioDriver->mixer->isPlaying())
	{
		audioDriver->fillAudioWithCompensation((char*)udata, numSamples*4);
	}
   	else
   	{
		//  Not Playing , so clear buffer
		short * _buf = (short *) udata;
		unsigned int count;
		for (count = 0; count < numSamples * 2; count++)
	    		*(_buf + count) = 0;
    }
}

AudioDriver_PSP::AudioDriver_PSP() :
	AudioDriver_COMPENSATE(),
	didInit(false)
{
}

AudioDriver_PSP::~AudioDriver_PSP()
{
	stop();
	if (didInit)
	{
		pspAudioEnd();
	}
}

// On error return a negative value
// If the requested buffer size can be served return 0,
mp_sint32 AudioDriver_PSP::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
		return res;
	
	if (!didInit)
	{
		//printf("Opening audio PSPdriver, buffer size %u bytes at %u Hz\n", bufferSizeInWords, mixFrequency);
		pspAudioInit();
		didInit = true;
	}
	
	return PSP_NUM_AUDIO_SAMPLES*PSP_NB_CHANNELS;
}

mp_sint32 AudioDriver_PSP::stop()
{
	deviceHasStarted = false;
	pspAudioSetChannelCallback(PSP_AUDIO_CHANNEL, 0, NULL);
	return 0;
}

mp_sint32 AudioDriver_PSP::closeDevice()
{
	if (deviceHasStarted)
	{
		deviceHasStarted = false;
		pspAudioSetChannelCallback(PSP_AUDIO_CHANNEL, 0, NULL);
	}
	return 0;
}

mp_sint32 AudioDriver_PSP::start()
{
	deviceHasStarted = true;
	pspAudioSetChannelCallback(PSP_AUDIO_CHANNEL, fill_audio, (void*)this);
	return 0;
}

mp_sint32 AudioDriver_PSP::pause()
{
	deviceHasStarted = false;
	pspAudioSetChannelCallback(PSP_AUDIO_CHANNEL, 0, NULL);
	return 0;
}

mp_sint32 AudioDriver_PSP::resume()
{
	if (!deviceHasStarted)
	{
		deviceHasStarted = true;
		pspAudioSetChannelCallback(PSP_AUDIO_CHANNEL, fill_audio, (void*)this);
	}
	return 0;
}

// ------------------------------ modified PSP audiolib here ------------------------------
static int audio_ready=0;
static short audio_sndbuf[PSP_NUM_AUDIO_CHANNELS][2][PSP_NUM_AUDIO_SAMPLES][2];

static psp_audio_channelinfo AudioStatus[PSP_NUM_AUDIO_CHANNELS];

static volatile int audio_terminate=0;

static SceUID semaID;

static void enterCriticalSection()
{
	if(semaID > 0) {
		sceKernelWaitSema(semaID, 1, NULL);
	}
}

static void leaveCriticalSection()
{
	if(semaID > 0) {
		sceKernelSignalSema(semaID, 1);
	}
}

static void pspAudioSetVolume(int channel, int left, int right)
{
  AudioStatus[channel].volumeright = right;
  AudioStatus[channel].volumeleft  = left;
}

//static void pspAudioChannelThreadCallback(int channel, void *buf, unsigned int reqn)
//{
//	pspAudioCallback_t callback;
//	callback=AudioStatus[channel].callback;
//}


static void pspAudioSetChannelCallback(int channel, pspAudioCallback_t callback, void *pdata)
{
	enterCriticalSection();
	volatile psp_audio_channelinfo *pci = &AudioStatus[channel];
	pci->callback=0;
	pci->pdata=pdata;
	pci->callback=callback;
	leaveCriticalSection();
}

static int pspAudioOutBlocking(unsigned int channel, unsigned int vol1, unsigned int vol2, void *buf)
{
	if (!audio_ready) return -1;
	if (channel>=PSP_NUM_AUDIO_CHANNELS) return -1;
	if (vol1>PSP_VOLUME_MAX) vol1=PSP_VOLUME_MAX;
	if (vol2>PSP_VOLUME_MAX) vol2=PSP_VOLUME_MAX;
	return sceAudioOutputPannedBlocking(AudioStatus[channel].handle,vol1,vol2,buf);
}

static int AudioChannelThread(SceSize args, void *argp)
{
	volatile int bufidx=0;
	int channel=*(int *)argp;
	
	while (audio_terminate==0) {
		void *bufptr=&audio_sndbuf[channel][bufidx];
		pspAudioCallback_t callback;

		enterCriticalSection();

		callback=AudioStatus[channel].callback;
		if (callback) {
			callback(bufptr, PSP_NUM_AUDIO_SAMPLES, AudioStatus[channel].pdata);
		} else {
			unsigned int *ptr=(unsigned int*)bufptr;
			int i;
			for (i=0; i<PSP_NUM_AUDIO_SAMPLES; ++i) *(ptr++)=0;
		}
		
		leaveCriticalSection();
		
		pspAudioOutBlocking(channel,AudioStatus[channel].volumeleft,AudioStatus[channel].volumeright,bufptr);
		bufidx=(bufidx?0:1);
	}
	sceKernelExitThread(0);
	return 0;
}



/******************************************************************************/



static int pspAudioInit()
{
	int i,ret;
	int failed=0;
	char str[32];

	audio_terminate=0;
	audio_ready=0;

	for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
    AudioStatus[i].handle = -1;
    AudioStatus[i].threadhandle = -1;
    AudioStatus[i].volumeright = PSP_VOLUME_MAX;
    AudioStatus[i].volumeleft  = PSP_VOLUME_MAX;
    AudioStatus[i].callback = 0;
    AudioStatus[i].pdata = 0;
	}
	for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
		if ((AudioStatus[i].handle = sceAudioChReserve(-1,PSP_NUM_AUDIO_SAMPLES,0))<0) 
      failed=1;
	}
	if (failed) {
		for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
			if (AudioStatus[i].handle != -1) 
        sceAudioChRelease(AudioStatus[i].handle);
			AudioStatus[i].handle = -1;
		}
		return -1;
	}
	audio_ready = 1;
	strcpy(str,"audiot0");
	for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
		str[6]='0'+i;
		AudioStatus[i].threadhandle = sceKernelCreateThread(str,&AudioChannelThread,0x12,0x10000,0,NULL);
		if (AudioStatus[i].threadhandle < 0) {
			AudioStatus[i].threadhandle = -1;
			failed=1;
			break;
		}
		ret=sceKernelStartThread(AudioStatus[i].threadhandle,sizeof(i),&i);
		if (ret!=0) {
			failed=1;
			break;
		}
	}
	if (failed) {
		audio_terminate=1;
		for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
			if (AudioStatus[i].threadhandle != -1) {
				//sceKernelWaitThreadEnd(AudioStatus[i].threadhandle,NULL);
				sceKernelDeleteThread(AudioStatus[i].threadhandle);
			}
			AudioStatus[i].threadhandle = -1;
		}
		audio_ready=0;
		return -1;
	}
	else
	{
		semaID = sceKernelCreateSema("AudioSema", 0, 1, 255, NULL);		
	}
	return 0;
}


//static void pspAudioEndPre()
//{
//	audio_ready=0;
//	audio_terminate=1;
//}


static void pspAudioEnd()
{
	int i;
	audio_ready=0;
	audio_terminate=1;

	for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
		if (AudioStatus[i].threadhandle != -1) {
			//sceKernelWaitThreadEnd(AudioStatus[i].threadhandle,NULL);
			sceKernelDeleteThread(AudioStatus[i].threadhandle);
		}
		AudioStatus[i].threadhandle = -1;
	}

	for (i=0; i<PSP_NUM_AUDIO_CHANNELS; i++) {
		if (AudioStatus[i].handle != -1) {
			sceAudioChRelease(AudioStatus[i].handle);
			AudioStatus[i].handle = -1;
		}
	}
	
	if (semaID > 0) 
		sceKernelDeleteSema(semaID);
}
/**************************************************************************************/
