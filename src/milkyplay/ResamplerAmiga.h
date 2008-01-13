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

***  ACHTUNG!!!! ***

This is a work-in-progress, I have still to implement volume ramping and some other little bits.

- Chris (Deltafire) 13/1/2008

	*/

// Amiiiiiiiiiiiiigaaaaaaaaa

#include "computed-blep.h"

template<bool ramping>
class ResamplerAmiga : public ChannelMixer::ResamplerBase
{
private:
	// Some constants
	enum 
	{
		BLEP_SCALE = 17,
		MAX_CHANNELS = 33,	// Extra channel is used for 'scopes
		MAX_BLEPS = 32,
		MAX_AGE = 2048,
		filterTable = 0, // TODO: Make this user selectable (see computed-blep.h)
		paulaAdvance = 74// TODO: This needs to be calculated from mixer freq
	};
	
	// the structure that holds data of bleps
	struct blepState_t 
	{
		mp_sint32 level;
		mp_uword age;
	};
	
	// Storage
	blepState_t bleps[MAX_CHANNELS][MAX_BLEPS];
	mp_uint32 currentLevel[MAX_CHANNELS];
	mp_sint32 activeBleps[MAX_CHANNELS];
	
public:
	ResamplerAmiga()
	{
		memset(bleps, 0, sizeof(bleps));
		memset(currentLevel, 0, sizeof(currentLevel));
		memset(activeBleps, 0, sizeof(activeBleps));
	}
	
	inline mp_sint32 interpolate_amiga_8bit(const mp_sint32 sample,
									   const mp_sint32 smppos,
									   const ChannelMixer::TMixerChannel* chn)
	{
		const mp_sint32 channel = chn->index;
		if(sample != currentLevel[channel])
		{
			// We have a newborn blep!
			// Make room for it
			memmove(&bleps[channel][1], &bleps[channel][0], sizeof(blepState_t) * activeBleps[channel]);
			if(++activeBleps[channel] == MAX_BLEPS)
			{
				fprintf(stderr, "AMIGA: Blep list truncated!\n");
				activeBleps[channel]--;
			}
			bleps[channel][0].level = sample - currentLevel[channel];
			bleps[channel][0].age = ((chn->fixedtimefrac + (chn->smppos & 0xffff))  * paulaAdvance) >> 16;
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
											const mp_sint32 smppos,
		   									const ChannelMixer::TMixerChannel* chn)
	{
		const mp_sint32 channel = chn->index;
		if(sample != currentLevel[channel])
		{
			// We have a newborn blep!
			// Make room for it
			memmove(&bleps[channel][1], &bleps[channel][0], sizeof(blepState_t) * activeBleps[channel]);
			if(++activeBleps[channel] == MAX_BLEPS)
			{
				fprintf(stderr, "AMIGA: Blep list truncated!\n");
				activeBleps[channel]--;
			}
			bleps[channel][0].level = sample - currentLevel[channel];
			bleps[channel][0].age = ((chn->fixedtimefrac + (chn->smppos & 0xffff))  * paulaAdvance) >> 16;
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

	virtual bool isRamping() { return ramping; }
	virtual bool supportsFullChecking() { return false; }
	virtual bool supportsNoChecking() { return true; }
	
	inline void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		// adding some local variables, will be faster to access than attributes of chn
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		
		const mp_sint32 rampFromVolStepL = ramping ? chn->rampFromVolStepL : 0;
		const mp_sint32 rampFromVolStepR = ramping ? chn->rampFromVolStepR : 0;
		
		mp_sint32 smppos = chn->smppos;
		mp_sint32 smpposfrac = chn->smpposfrac;
		const mp_sint32 smpadd = (chn->flags&ChannelMixer::MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		
		mp_sint32 fp = smpadd*count;
		MP_INCREASESMPPOS(chn->smppos,chn->smpposfrac,fp,16);
		
		if (chn->flags & 4)
		{
			// 16 bit
			const mp_sword* sample = ((const mp_sword*) chn->sample) + smppos;
			smppos = smpposfrac;
			
			while (count--)
			{
				mp_sint32 s = sample[smppos>>16];
				
				s = interpolate_amiga_16bit(s, smppos, chn);
				
				(*buffer++)+=(s*(voll>>15))>>15; 
				(*buffer++)+=(s*(volr>>15))>>15; 
				
				// ramping is a template parameter
				// see explanation above
				if (ramping)
				{
					voll+=rampFromVolStepL; 
					volr+=rampFromVolStepR; 
				}
				
				smppos+=smpadd;
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
				
				s = interpolate_amiga_8bit(s, smppos, chn);
				
				// Really, the volume should be applied before the interpolation
				(*buffer++)+=(s*(voll>>15))>>15;
				(*buffer++)+=(s*(volr>>15))>>15;
				
				// ramping is a template parameter
				// see explanation above
				if (ramping)
				{
					voll+=rampFromVolStepL; 
					volr+=rampFromVolStepR; 
				}
				
				smppos+=smpadd;
			}
			
		}
		
		if (ramping)
		{
			chn->finalvoll = voll;
			chn->finalvolr = volr;	
		}
	}
};
