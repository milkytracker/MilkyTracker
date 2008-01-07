/*
 *  milkyplay/ResamplerFast.h
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
 *  ResamplerFast.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.11.07.
 *
 */

#ifndef __RESAMPLERFAST_H__
#define __RESAMPLERFAST_H__

#include "ResamplerMacros.h"

/////////////////////////////////////////////////////////
//		SIMPLE MIXER, NO INTERPOLATION, NO RAMPING     //
/////////////////////////////////////////////////////////
class ResamplerSimple : public ChannelMixer::ResamplerBase
{
public:
	virtual bool isRamping() { return false; }
	virtual bool supportsFullChecking() { return true; }
	virtual bool supportsNoChecking() { return true; }

	virtual void addBlockFull(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		FULLMIXER_TEMPLATE(FULLMIXER_8BIT_NORMAL,FULLMIXER_16BIT_NORMAL,16, 0);
	}
	
	virtual void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;

		mp_sint32 smppos = chn->smppos;
		const mp_sint32 smpadd = (chn->flags&MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		const mp_sint32 basepos = smppos;
		mp_sint32 posfixed = chn->smpposfrac;

		mp_sint32 fp = smpadd*count;
		MP_INCREASESMPPOS(chn->smppos,chn->smpposfrac,fp,16);

		if ((voll == 0) && (volr == 0)) return;
		
		mp_sint32 sd1,sd2;
		
		NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_NORMAL,NOCHECKMIXER_16BIT_NORMAL);
	}
};

/////////////////////////////////////////////////////////
//			NO INTERPOLATION BUT VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
class ResamplerSimpleRamp : public ChannelMixer::ResamplerBase
{
public:
	virtual bool isRamping() { return true; }
	virtual bool supportsFullChecking() { return true; }
	virtual bool supportsNoChecking() { return true; }

	virtual void addBlockFull(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		
		mp_sint32 rampFromVolStepL = chn->rampFromVolStepL;
		mp_sint32 rampFromVolStepR = chn->rampFromVolStepR;		

		if (rampFromVolStepL || rampFromVolStepR)
		{
			FULLMIXER_TEMPLATE(FULLMIXER_8BIT_NORMAL_RAMP(true),FULLMIXER_16BIT_NORMAL_RAMP(true), 16, 0);
		}
		else
		{
			FULLMIXER_TEMPLATE(FULLMIXER_8BIT_NORMAL_RAMP(false),FULLMIXER_16BIT_NORMAL_RAMP(false), 16, 1);
		}
			
		chn->finalvoll = voll;
		chn->finalvolr = volr;
	}
	
	virtual void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;

		mp_sint32 rampFromVolStepL = chn->rampFromVolStepL;
		mp_sint32 rampFromVolStepR = chn->rampFromVolStepR;		

		mp_sint32 smppos = chn->smppos;
		const mp_sint32 smpadd = (chn->flags&MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		const mp_sint32 basepos = smppos;
		mp_sint32 posfixed = chn->smpposfrac;

		mp_sint32 fp = smpadd*count;
		MP_INCREASESMPPOS(chn->smppos,chn->smpposfrac,fp,16);

		if ((voll == 0 && rampFromVolStepL == 0) && (volr == 0 && rampFromVolStepR == 0)) return;
		
		mp_sint32 sd1,sd2;
		
		if (rampFromVolStepL || rampFromVolStepR)
		{
			NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_NORMAL_RAMP(true),NOCHECKMIXER_16BIT_NORMAL_RAMP(true));
		}
		else
		{
			NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_NORMAL_RAMP(false),NOCHECKMIXER_16BIT_NORMAL_RAMP(false));
		}
		
		chn->finalvoll = voll;
		chn->finalvolr = volr;	
	}
};

/////////////////////////////////////////////////////////
//			INTERPOLATION AND NO VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
class ResamplerLerp : public ChannelMixer::ResamplerBase
{
public:
	virtual bool isRamping() { return false; }
	virtual bool supportsFullChecking() { return true; }
	virtual bool supportsNoChecking() { return true; }

	virtual void addBlockFull(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		FULLMIXER_TEMPLATE(FULLMIXER_8BIT_LERP,FULLMIXER_16BIT_LERP, 16, 0);
	}
	
	virtual void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;

		mp_sint32 smppos = chn->smppos;
		const mp_sint32 smpadd = (chn->flags&MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		const mp_sint32 basepos = smppos;
		mp_sint32 posfixed = chn->smpposfrac;

		mp_sint32 fp = smpadd*count;
		MP_INCREASESMPPOS(chn->smppos,chn->smpposfrac,fp,16);

		if ((voll == 0) && (volr == 0)) return;
		
		mp_sint32 sd1,sd2;
		
		NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP,NOCHECKMIXER_16BIT_LERP);
	}
};

/////////////////////////////////////////////////////////
//  INTERPOLATION, VOLUME RAMPING AND LOW PASS FILTER  //
/////////////////////////////////////////////////////////
class ResamplerLerpRampFilter : public ChannelMixer::ResamplerBase
{
public:
	virtual bool isRamping() { return true; }
	virtual bool supportsFullChecking() { return true; }
	virtual bool supportsNoChecking() { return true; }

	virtual void addBlockFull(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		
		mp_sint32 rampFromVolStepL = chn->rampFromVolStepL;
		mp_sint32 rampFromVolStepR = chn->rampFromVolStepR;		
		
		if (chn->cutoff != MP_INVALID_VALUE && chn->resonance != MP_INVALID_VALUE)
		{
			const mp_sint32 a = chn->a;
			const mp_sint32 b = chn->b;
			const mp_sint32 c = chn->c;
			
			mp_sint32 currsample = chn->currsample;
			mp_sint32 prevsample = chn->prevsample;
			
			if (rampFromVolStepL || rampFromVolStepR)
			{
				FULLMIXER_TEMPLATE(FULLMIXER_8BIT_LERP_RAMP_FILTER(true),FULLMIXER_16BIT_LERP_RAMP_FILTER(true), 16, 0);
			}
			else
			{
				FULLMIXER_TEMPLATE(FULLMIXER_8BIT_LERP_RAMP_FILTER(false),FULLMIXER_16BIT_LERP_RAMP_FILTER(false), 16, 1);
			}

			chn->currsample = currsample;
			chn->prevsample = prevsample;
		}
		else
		{
			if (rampFromVolStepL || rampFromVolStepR)
			{
				FULLMIXER_TEMPLATE(FULLMIXER_8BIT_LERP_RAMP(true),FULLMIXER_16BIT_LERP_RAMP(true), 16, 2);
			}
			else
			{
				FULLMIXER_TEMPLATE(FULLMIXER_8BIT_LERP_RAMP(false),FULLMIXER_16BIT_LERP_RAMP(false), 16, 3);
			}
		}
		
		chn->finalvoll = voll;
		chn->finalvolr = volr;
	}
	
	virtual void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		
		mp_sint32 rampFromVolStepL = chn->rampFromVolStepL;
		mp_sint32 rampFromVolStepR = chn->rampFromVolStepR;		
		
		mp_sint32 smppos = chn->smppos;
		const mp_sint32 smpadd = (chn->flags&MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		const mp_sint32 basepos = smppos;
		mp_sint32 posfixed = chn->smpposfrac;
		
		mp_sint32 fp = smpadd*count;
		MP_INCREASESMPPOS(chn->smppos,chn->smpposfrac,fp,16);
		
		mp_sint32 sd1,sd2;
		
		if (chn->cutoff != MP_INVALID_VALUE && chn->resonance != MP_INVALID_VALUE)
		{
			const mp_sint32 a = chn->a;
			const mp_sint32 b = chn->b;
			const mp_sint32 c = chn->c;
			
			mp_sint32 currsample = chn->currsample;
			mp_sint32 prevsample = chn->prevsample;
			
			// check if ramping has to be performed
			if (rampFromVolStepL || rampFromVolStepR)
			{
				NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP_RAMP_FILTER(true),NOCHECKMIXER_16BIT_LERP_RAMP_FILTER(true));
			}
			else
			{
				NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP_RAMP_FILTER(false),NOCHECKMIXER_16BIT_LERP_RAMP_FILTER(false));
			}
			
			chn->currsample = currsample;
			chn->prevsample = prevsample;
		}
		else
		{
			if ((voll == 0 && rampFromVolStepL == 0) && (volr == 0 && rampFromVolStepR == 0)) return;
			
			// check if ramping has to be performed
			if (rampFromVolStepL || rampFromVolStepR)
			{
				NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP_RAMP(true),NOCHECKMIXER_16BIT_LERP_RAMP(true));
			}
			else
			{
				NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP_RAMP(false),NOCHECKMIXER_16BIT_LERP_RAMP(false));
			}
		}
		
		chn->finalvoll = voll;
		chn->finalvolr = volr;	
	}
};

/////////////////////////////////////////////////////////
//  Testing purposes                                   //
/////////////////////////////////////////////////////////
class ResamplerDummy : public ChannelMixer::ResamplerBase
{
public:
	virtual bool isRamping() { return false; }
	virtual bool supportsFullChecking() { return false; }
	virtual bool supportsNoChecking() { return true; }

	virtual void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		
		//const mp_sint32 rampFromVolStepL = chn->rampFromVolStepL;
		//const mp_sint32 rampFromVolStepR = chn->rampFromVolStepR;		
		
		mp_sint32 smppos = chn->smppos;
		mp_sint32 smpposfrac = chn->smpposfrac;
		const mp_sint32 smpadd = (chn->flags&MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		
		mp_sint32 sd1,sd2;

		const mp_sint32 flags = chn->flags;
		const mp_sint32 loopstart = chn->loopstart;
		const mp_sint32 loopend = chn->loopend;
		const mp_sint32 smplen = chn->smplen;
		
		mp_sint32 fixedtimefrac = chn->fixedtimefrac;
		const mp_sint32 timeadd = chn->smpadd;
	
		if (!(flags&4)) 
		{
			const mp_sbyte* sample = chn->sample;
			while (count--)
			{
				/*sd1 = sample[smppos] << 8; 
				sd2 = sample[smppos+1] << 8; 
				
				sd1 = ((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; 
				
				(*buffer++)+=((sd1*(voll>>15))>>15); 
				(*buffer++)+=((sd1*(volr>>15))>>15); 
				
				voll+=rampFromVolStepL; 
				volr+=rampFromVolStepR; */
				
				mp_sint32 ofsf, v0, v1, v2, v3;
				
				v1 = sample[smppos] << 8;
				v2 = sample[smppos + 1] << 8;			
				
				v0 = sample[smppos - 1] << 8;
				v3 = sample[smppos + 2] << 8;
				ofsf = smpposfrac + 65536;
				v3 += -3*v2 + 3*v1 - v0;
				v3 = ChannelMixer::fixedmul(v3, (ofsf - 2*65536) / 6);
				v3 += v2 - v1 - v1 + v0;
				v3 = ChannelMixer::fixedmul(v3, (ofsf - 65536) >> 1);
				v3 += v1 - v0;
				v3 = ChannelMixer::fixedmul(v3, ofsf);
				v3 += v0;	
				
				(*buffer++)+=((v3*(voll>>15))>>15); 
				(*buffer++)+=((v3*(volr>>15))>>15); 
				
				//voll+=rampFromVolStepL; 
				//volr+=rampFromVolStepR; 
				
				MP_INCREASESMPPOS(smppos, smpposfrac, smpadd, 16);
			}
		}
		else
		{
			const mp_sword* sample = (const mp_sword*)chn->sample; 
			while (count--)
			{
				mp_sint32 ofsf, v0, v1, v2, v3;
								
				v1 = sample[smppos];
				v2 = sample[smppos + 1];			
				
				v0 = sample[smppos - 1];
				v3 = sample[smppos + 2];
				ofsf = smpposfrac + 65536;
				v3 += -3*v2 + 3*v1 - v0;
				v3 = ChannelMixer::fixedmul(v3, (ofsf - 2*65536) / 6);
				v3 += v2 - v1 - v1 + v0;
				v3 = ChannelMixer::fixedmul(v3, (ofsf - 65536) >> 1);
				v3 += v1 - v0;
				v3 = ChannelMixer::fixedmul(v3, ofsf);
				v3 += v0;			
				
				(*buffer++)+=((v3*(voll>>15))>>15); 
				(*buffer++)+=((v3*(volr>>15))>>15); 
				
				//voll+=rampFromVolStepL; 
				//volr+=rampFromVolStepR; 
				
				MP_INCREASESMPPOS(smppos, smpposfrac, smpadd, 16);
			}
		}
		
		chn->smppos = smppos;
		chn->smpposfrac = smpposfrac;

		chn->fixedtimefrac = fixedtimefrac;

		/*if (!(chn->flags&4)) 
		{ 
			const mp_sbyte* sample = chn->sample + basepos; 
			while (count--)
			{
				sd1 = sample[posfixed>>16]<<8; 
				sd2 = sample[(posfixed>>16)+1]<<8; 
				
				sd1 =((sd1<<12)+((posfixed>>4)&0xfff)*(sd2-sd1))>>12; 
				
				(*buffer++)+=((sd1*(voll>>15))>>15); 
				(*buffer++)+=((sd1*(volr>>15))>>15); 
				
				voll+=rampFromVolStepL; 
				volr+=rampFromVolStepR; 
				posfixed+=smpadd;
			}
		} 
		else 
		{ 
			const mp_sword* sample = (const mp_sword*)chn->sample + basepos; 
			while (count--)
			{
				sd1 = sample[posfixed>>16]; 
				sd2 = sample[(posfixed>>16)+1]; 
				
				sd1 =((sd1<<12)+((posfixed>>4)&0xfff)*(sd2-sd1))>>12; 
				
				(*buffer++)+=((sd1*(voll>>15))>>15); 
				(*buffer++)+=((sd1*(volr>>15))>>15); 
				
				voll+=rampFromVolStepL; 
				volr+=rampFromVolStepR; 
				posfixed+=smpadd;
			}
		} */
		
/*		
		if (chn->cutoff != MP_INVALID_VALUE && chn->resonance != MP_INVALID_VALUE)
		{
			const mp_sint32 a = chn->a;
			const mp_sint32 b = chn->b;
			const mp_sint32 c = chn->c;
			
			mp_sint32 currsample = chn->currsample;
			mp_sint32 prevsample = chn->prevsample;
			
			// check if ramping has to be performed
			if (rampFromVolStepL || rampFromVolStepR)
			{
				NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP_RAMP_FILTER(true),NOCHECKMIXER_16BIT_LERP_RAMP_FILTER(true));
			}
			else
			{
				NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP_RAMP_FILTER(false),NOCHECKMIXER_16BIT_LERP_RAMP_FILTER(false));
			}
			
			chn->currsample = currsample;
			chn->prevsample = prevsample;
		}
		else
		{
			if ((voll == 0 && rampFromVolStepL == 0) && (volr == 0 && rampFromVolStepR == 0)) return;
			
			// check if ramping has to be performed
			if (rampFromVolStepL || rampFromVolStepR)
			{
				NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP_RAMP(true),NOCHECKMIXER_16BIT_LERP_RAMP(true));
			}
			else
			{
				NOCHECKMIXER_TEMPLATE(NOCHECKMIXER_8BIT_LERP_RAMP(false),NOCHECKMIXER_16BIT_LERP_RAMP(false));
			}
		}*/
		
		//chn->finalvoll = voll;
		//chn->finalvolr = volr;	
	}
};

#endif
