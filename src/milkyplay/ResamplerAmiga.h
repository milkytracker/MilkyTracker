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

/* Implementation of Antti Lankila's Amiga sound model, see:
 * http://bel.fi/~alankila/modguide/interpolate.txt
 * (a copy can be found in the resources/reference/Amiga_resampler)
 *
 * TODO: filter volume changes (maybe), add dithering on the output
 *
 * Chris (Deltafire) 13/1/2008
 */

// Amiiiiiiiiiiiiigaaaaaaaaa

#include "computed-blep.h"

template<mp_sint32 filterTable>
class ResamplerAmiga : public ChannelMixer::ResamplerBase
{
private:
	// Some constants
	enum 
	{
		BLEP_SCALE = 17,
		MAX_BLEPS = 32,
		MAX_AGE = 2048,
		PAULA_FREQ = 3546895
	};
	
	// the structure that holds data of bleps
	struct blepState_t 
	{
		mp_sint32 level;
		mp_uword age;
	};
	
	const int* table;
	
	// Storage
	mp_sint32 numChannels;
	
	blepState_t** bleps;
	mp_sint32* currentLevel;
	mp_sint32* activeBleps;
	
	mp_sint32 paulaAdvance;
	
	void cleanUp(mp_sint32 num)
	{
		for (mp_sint32 i = 0; i < num; i++)
			delete[] bleps[i];
			
		delete[] bleps;
		delete[] currentLevel;
		delete[] activeBleps;
	}
	
	void realloc(mp_sint32 oldNum, mp_sint32 newNum)
	{
		cleanUp(oldNum);
		
		bleps = new blepState_t*[newNum];
		for (mp_sint32 i = 0; i < newNum; i++)
			bleps[i] = new blepState_t[MAX_BLEPS];
		currentLevel = new mp_sint32[newNum];
		activeBleps = new mp_sint32[newNum];
	}
	
	void clearState()
	{
		memset(currentLevel, 0, numChannels*sizeof(mp_uint32));
		memset(activeBleps, 0, numChannels*sizeof(mp_sint32));
		
		for (mp_sint32 i = 0; i < numChannels; i++)
			memset(bleps[i], 0, MAX_BLEPS*sizeof(blepState_t));
	}
	
public:
	ResamplerAmiga() : 
		numChannels(0),
		bleps(NULL),
		currentLevel(NULL),
		activeBleps(NULL)
	{
	}
	
	virtual ~ResamplerAmiga()
	{
		cleanUp(numChannels);
	}

	virtual void setFrequency(mp_sint32 frequency)
	{
		paulaAdvance = PAULA_FREQ / frequency;
	}
	
	virtual void setNumChannels(mp_sint32 num) 
	{ 
		// one more channel for the scope dummy 
		num++;	
		realloc(numChannels, num);	
		numChannels = num;		
		clearState();
	}	
	
	inline mp_sint32 interpolate_amiga_8bit(const mp_sint32 sample,
											const mp_sint32 channel,
											const ChannelMixer::TMixerChannel* chn)
	{
		if(sample != currentLevel[channel])
		{
			// We have a newborn blep!
			// Make room for it
			memmove(&bleps[channel][1], &bleps[channel][0], sizeof(blepState_t) * activeBleps[channel]);
			if(++activeBleps[channel] == MAX_BLEPS)
			{
#ifndef WIN32
				fprintf(stderr, "AMIGA: Blep list truncated!\n");
#endif
				activeBleps[channel]--;
			}
			bleps[channel][0].level = sample - currentLevel[channel];
			bleps[channel][0].age = ((chn->fixedtimefrac + (chn->fixedtime & 0xffff))  * paulaAdvance) >> 16;
			currentLevel[channel] = sample;
		}
		
		mp_sint32 s = 0;
		// Age teh bleps!
		for(mp_sint32 i = 0; i < activeBleps[channel]; i++)
		{
			s -= winsinc_integral[filterTable*WINSINCSIZE+bleps[channel][i].age] * bleps[channel][i].level;
			if((bleps[channel][i].age += paulaAdvance) >= MAX_AGE)
				activeBleps[channel]  = i; // It died of old age :(
		}
		
		s >>= (BLEP_SCALE - 8);
		
		s += sample << 8;
		
		return  s;
	}
	
	// Due to 32-bit limitations the 16-bit resampler is less precise
	// PS, copy & paste is quicker than messing around with templates ;)
	inline mp_sint32 interpolate_amiga_16bit(const mp_sint32 sample,
											 const mp_sint32 channel,
											 const ChannelMixer::TMixerChannel* chn)
	{
		if(sample != currentLevel[channel])
		{
			// We have a newborn blep!
			// Make room for it
			memmove(&bleps[channel][1], &bleps[channel][0], sizeof(blepState_t) * activeBleps[channel]);
			if(++activeBleps[channel] == MAX_BLEPS)
			{
#ifndef WIN32
				fprintf(stderr, "AMIGA: Blep list truncated!\n");
#endif
				activeBleps[channel]--;
			}
			bleps[channel][0].level = sample - currentLevel[channel];
			bleps[channel][0].age = ((chn->fixedtimefrac + (chn->fixedtime & 0xffff))  * paulaAdvance) >> 16;
			currentLevel[channel] = sample;
		}
		
		mp_sint32 s = 0;
		// Age teh bleps!
		for(mp_sint32 i = 0; i < activeBleps[channel]; i++)
		{
			s -= (winsinc_integral[filterTable*WINSINCSIZE+bleps[channel][i].age]>>3) * bleps[channel][i].level;
			if((bleps[channel][i].age += paulaAdvance) >= MAX_AGE)
				activeBleps[channel]  = i; // It died of old age :(
		}
		
		s >>= BLEP_SCALE - 3;
		
		s += sample;
		
		return  s;
	}

	virtual bool isRamping() { return false; }
	virtual bool supportsFullChecking() { return false; }
	virtual bool supportsNoChecking() { return true; }
	
	inline void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		// adding some local variables, will be faster to access than attributes of chn
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		
		mp_sint32 smppos = chn->smppos;
		mp_sint32 smpposfrac = chn->smpposfrac;
		const mp_sint32 smpadd = (chn->flags&ChannelMixer::MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		
		mp_sint32 fp = smpadd*count;
		MP_INCREASESMPPOS(chn->smppos,chn->smpposfrac,fp,16);
		
		// when the channel index is -1 we're going to use the dummy channel
		const mp_sint32 channel = (chn->index == -1 ? this->numChannels - 1 : chn->index);
		
		if (chn->flags & 4)
		{
			// 16 bit
			const mp_sword* sample = ((const mp_sword*) chn->sample) + smppos;
			smppos = smpposfrac;
			
			while (count--)
			{
				mp_sint32 s = sample[smppos>>16];
				
				s = interpolate_amiga_16bit(s, channel, chn);
				
				(*buffer++)+=(s*(voll>>15))>>15; 
				(*buffer++)+=(s*(volr>>15))>>15; 
				
				smppos+=smpadd;
				MP_INCREASESMPPOS(chn->fixedtime, chn->fixedtimefrac, smpadd, 16);
			}
		} 
		else 
		{
			// 8 bit
			const mp_sbyte* sample = &chn->sample[smppos];
			smppos = smpposfrac;
			
			while (count--)
			{
				mp_sint32 s = sample[smppos>>16];
				
				s = interpolate_amiga_8bit(s, channel, chn);
				
				// Really, the volume should be applied before the interpolation
				(*buffer++)+=(s*(voll>>15))>>15;
				(*buffer++)+=(s*(volr>>15))>>15;
				
				smppos+=smpadd;
				// advance time, this is necessary because it's not being done
				// when not being used, so we'll do it here
				MP_INCREASESMPPOS(chn->fixedtime, chn->fixedtimefrac, smpadd, 16);
			}
		}
	}
};
