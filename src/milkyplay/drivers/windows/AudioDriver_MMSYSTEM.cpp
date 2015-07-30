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
 *  AudioDriver_MMSYSTEM.cpp
 *  MilkyPlay audiodriver
 */
#include "AudioDriver_MMSYSTEM.h"
#include "MasterMixer.h"

DWORD WINAPI AudioDriver_MMSYSTEM::ThreadProc(_In_ LPVOID lpParameter)
{
	AudioDriver_MMSYSTEM* audioDriver = (AudioDriver_MMSYSTEM*)lpParameter;

	while (audioDriver->deviceOpen)
	{
		MSG msg;

		// Blocks until message received from waveOut
		GetMessage(&msg, NULL, 0, 0);

		if (msg.message == WOM_DONE)
		{
			::waveOutUnprepareHeader(audioDriver->hwo, &audioDriver->wavhdr[audioDriver->currentBufferIndex], sizeof(WAVEHDR));

			if (!audioDriver->deviceHasStarted)
				continue;

			EnterCriticalSection(&audioDriver->cs);
			audioDriver->sampleCounterTotal += audioDriver->bufferSize >> 1;
			audioDriver->kick();
			LeaveCriticalSection(&audioDriver->cs);
		}
	}

	return 0;
}

AudioDriver_MMSYSTEM::AudioDriver_MMSYSTEM() :
	AudioDriverBase(),
	paused(false), 
	deviceOpen(false),
	deviceHasStarted(false),
	currentBufferIndex(0),
	sampleCounterTotal(0)
{
	memset(&mixbuff16, 0, sizeof(mixbuff16));
	memset(&wavhdr, 0, sizeof(wavhdr));
	InitializeCriticalSection(&cs);
}

AudioDriver_MMSYSTEM::~AudioDriver_MMSYSTEM() 
{
	if (deviceHasStarted)
		stop();

	for (mp_sint32 c = 0; c < NUMBUFFERS; c++)
	{
		if (mixbuff16[NUMBUFFERS-1-c])
			delete[] mixbuff16[NUMBUFFERS-1-c];
	}
	
	DeleteCriticalSection(&cs);	
}

mp_sint32 AudioDriver_MMSYSTEM::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* pMixer)
{
	mp_sint32 res = AudioDriverBase::initDevice(bufferSizeInWords, mixFrequency, pMixer);
	if (res != MP_OK)
		return res;

	sampleRate = mixFrequency;

	mp_uint32 supportedFormats[] = { WAVE_FORMAT_48S16, 48000,
									 WAVE_FORMAT_4S16,  44100,
									 WAVE_FORMAT_2S16,  22050,
									 WAVE_FORMAT_1S16,  11025 };

	bool modeFound = false;
	DWORD dwFormat = 0;
	
	for (mp_uint32 i = 0; i < sizeof(supportedFormats)>>1; i++)
	{
		if (supportedFormats[i*2+1] == mixFrequency)
		{
			dwFormat = supportedFormats[i*2];
			modeFound = TRUE;
			break;
		}

	}

	if (!modeFound)
		return MP_DEVICE_ERROR;

	WAVEFORMATEX format;

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = NUMCHANNELS;
	format.nSamplesPerSec = mixFrequency;
	format.wBitsPerSample = NUMBITS;
	format.nAvgBytesPerSec = (format.nChannels*format.nSamplesPerSec*format.wBitsPerSample)/8;
	format.nBlockAlign = (format.nChannels*format.wBitsPerSample)/8;
	format.cbSize=0;

	// Find a usable waveOut device and open it
	for (UINT devid = 0; devid < ::waveOutGetNumDevs(); devid++)
	{

		if (devid == ::waveOutGetNumDevs())
			return MP_DEVICE_ERROR;

		if (::waveOutOpen(&hwo,
						  WAVE_MAPPER,
						  &format,
						  0,
						  0,
						  CALLBACK_NULL) == MMSYSERR_NOERROR)
		{
			// Usable device found, stop searching
			break;
		}
	
	}

	UINT waveOutID;
	if (::waveOutGetID(hwo,(LPUINT)&waveOutID) != MMSYSERR_NOERROR)
		return MP_DEVICE_ERROR;

	MMRESULT r = ::waveOutGetDevCaps((UINT)waveOutID,
									 &waveoutcaps,
									 sizeof(waveoutcaps));

	if (r != MMSYSERR_NOERROR) 
		return MP_DEVICE_ERROR;

	if (!(waveoutcaps.dwFormats & dwFormat))
		return MP_DEVICE_ERROR;

	::waveOutReset(hwo);

	if (::waveOutClose(hwo)!= MMSYSERR_NOERROR) 
		return MP_DEVICE_ERROR;

	// Setup thread to handle buffering
	deviceOpen = true;
	bufferThreadHandle = CreateThread(0, 0, &ThreadProc, this, 0, &bufferThreadId);

	if (::waveOutOpen(&hwo,
					  WAVE_MAPPER,
					  &format,
					  bufferThreadId,
					  (DWORD_PTR)this,
					  CALLBACK_THREAD) != MMSYSERR_NOERROR)
	{
		deviceOpen = false;
		return MP_DEVICE_ERROR;
	}
	
	lastSampleIndex = 0;

	for (mp_sint32 c = 0; c < NUMBUFFERS; c++)
	{
		delete[] mixbuff16[c];
		// cope with out of memory someday
		mixbuff16[c] = new mp_sword[bufferSizeInWords];
	}

	deviceHasStarted = false;
	sampleCounterTotal = 0;

	return MP_OK;
}

mp_sint32 AudioDriver_MMSYSTEM::stop()
{
	if (!deviceHasStarted)
		return MP_OK;

	bool resetVolume = false; 
	DWORD dwVol;
	if (paused)
	{
		::waveOutGetVolume(hwo, &dwVol);
		::waveOutSetVolume(hwo, 0);	
		resetVolume = true;
		resume();
	}

	// wait for buffers to be played
	deviceHasStarted = false;
	
	// wait at least 2 seconds
	const mp_sint32 timeOutVal = 2000;
	
	mp_sint32 timeCounter = 0;
	const mp_sint32 waitTime = 60;
	while (true/* && timeCounter < timeOutVal*/)
	{
		bool notDone = false;
		for (mp_sint32 i = 0;  i < NUMBUFFERS; i++)
		{
			if (! (wavhdr[i].dwFlags & WHDR_DONE))
			{
				notDone = TRUE;
				break;
			}
		}
		
		::Sleep(waitTime);		

		if (!notDone) break;
	}

	for (mp_sint32 i = 0;  i < NUMBUFFERS; i++)
	{
		if (wavhdr[i].dwFlags & WHDR_PREPARED)
		{
			::waveOutUnprepareHeader(hwo,&wavhdr[i],sizeof(WAVEHDR));
			wavhdr[i].dwFlags &= ~WHDR_PREPARED;
			break;
		}
	}

	// reset device
	::waveOutReset(hwo);

	if (resetVolume)
		::waveOutSetVolume(hwo, dwVol);	

	lastSampleIndex = 0;
	sampleCounterTotal = 0;

	return MP_OK;
}

mp_sint32 AudioDriver_MMSYSTEM::closeDevice()
{
	deviceOpen = false;

	if (waveOutClose(hwo)!=MMSYSERR_NOERROR) 
		return MP_DEVICE_ERROR;

	return MP_OK;
}

void AudioDriver_MMSYSTEM::kick()
{
	if (isMixerActive())
		mixer->mixerHandler(mixbuff16[currentBufferIndex]);
	else
		memset(mixbuff16[currentBufferIndex], 0, bufferSize*sizeof(mp_sword)); 

	wavhdr[currentBufferIndex].lpData = (char*)mixbuff16[currentBufferIndex];
	wavhdr[currentBufferIndex].dwBufferLength = (bufferSize*NUMBITS)/8;
	wavhdr[currentBufferIndex].dwFlags = 0;
	wavhdr[currentBufferIndex].dwLoops = 0;
	wavhdr[currentBufferIndex].dwUser = (DWORD_PTR)mixbuff16[currentBufferIndex];

	if (::waveOutPrepareHeader(hwo, &wavhdr[currentBufferIndex], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		return;

	if (::waveOutWrite(hwo, &wavhdr[currentBufferIndex], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		return;

	currentBufferIndex = (currentBufferIndex+1) % NUMBUFFERS;
}

mp_sint32 AudioDriver_MMSYSTEM::start()
{
	deviceHasStarted = true;
	for (int i = 0; i < NUMBUFFERS; i++)
		kick();

	sampleCounterTotal = 0;

	return MP_OK;
}

mp_sint32 AudioDriver_MMSYSTEM::pause()
{
	::waveOutPause(hwo);
	paused = true;
	return MP_OK;
}

mp_sint32 AudioDriver_MMSYSTEM::resume()
{
	::waveOutRestart(hwo);
	paused = false;
	return MP_OK;
}

mp_uint32 AudioDriver_MMSYSTEM::getNumPlayedSamples() const
{
	EnterCriticalSection(&cs);

	MMTIME mmtime;

	mmtime.wType = TIME_SAMPLES;

	if (::waveOutGetPosition(hwo,&mmtime,sizeof(mmtime)) == MMSYSERR_NOERROR)
	{
		if (mmtime.u.sample > lastSampleIndex)
			lastSampleIndex = mmtime.u.sample;
		LeaveCriticalSection(&cs);
		return lastSampleIndex;
	}

	LeaveCriticalSection(&cs);

	return lastSampleIndex;
}

mp_uint32 AudioDriver_MMSYSTEM::getBufferPos() const
{
	mp_sint32 smpPos = (mp_sint32)getNumPlayedSamples() - (mp_sint32)sampleCounterTotal;

	mp_sint32 maxSize = bufferSize >> 1;

	if (smpPos < 0)
		smpPos = 0;

	if (smpPos > maxSize)
		smpPos = maxSize;

	return smpPos;
}
