/*
 *  milkyplay/ResamplerCubic.h
 *
 *  Copyright 2008 Peter Barth, Christopher O'Neill
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

/* Implementation of Antti Lankila's Amiga sound model, see:
 * http://bel.fi/~alankila/modguide/interpolate.txt
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
		activeBleps(NULL),
		paulaAdvance(74)
	{
	}
	
	~ResamplerAmiga()
	{
		cleanUp(numChannels);
	}

	// FIXME: setFrequency isn't being called at intialisation
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
			s -= winsinc_integral[filterTable][bleps[channel][i].age] * bleps[channel][i].level;
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
			s -= (winsinc_integral[filterTable][bleps[channel][i].age]>>3) * bleps[channel][i].level;
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
