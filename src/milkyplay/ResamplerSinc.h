/*
 *  milkyplay/ResamplerSinc.h
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
 *  ResamplerSinc.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.01.08.
 *
 */

#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////
// sinc resamplers based on:                                                         //
// http://www.cs.princeton.edu/courses/archive/spr07/cos325/src/TimeStuf/srconvrt.c  //
///////////////////////////////////////////////////////////////////////////////////////
#ifndef M_PI 
#define M_PI 3.14159265358979323846 
#endif

#define fpmul ChannelMixer::fixedmul

#define advancePos(CHN) \
	if (((((CHN.flags&3) == 0 || (CHN.flags&3) == 1)) && !(CHN.flags&MP_SAMPLE_BACKWARD)) || \
		((CHN.flags&3) == 2 && (CHN.flags&MP_SAMPLE_BACKWARD) == 0)) \
	{ \
		CHN.smppos++; \
		/* stop playing if necessary */ \
		if (CHN.smppos>=CHN.loopend) \
		{ \
			if ((CHN.flags & 3) == 0) \
			{ \
				if (CHN.flags & MP_SAMPLE_ONESHOT) \
				{ \
					CHN.flags &= ~MP_SAMPLE_ONESHOT; \
					CHN.flags |= 1; \
					CHN.loopend = CHN.loopendcopy; \
					CHN.smppos = CHN.loopstart; \
				} \
				else \
				{ \
					CHN.flags&=~MP_SAMPLE_PLAY; \
				} \
			} \
			else if ((CHN.flags & 3) == 1) \
			{ \
				CHN.smppos = CHN.loopstart; \
			} \
			else \
			{ \
				CHN.flags|=MP_SAMPLE_BACKWARD; \
			} \
		}\
	} \
	/* bi-dir loop */ \
	else \
	{ \
		CHN.smppos--; \
		if (CHN.loopstart>=CHN.smppos) \
		{ \
			if ((CHN.flags & 3) == 0) \
			{ \
				CHN.flags&=~MP_SAMPLE_PLAY; \
			} \
			else if ((CHN.flags & 3) == 1) \
			{ \
				CHN.smppos = CHN.loopend; \
			} \
			else \
			{ \
				CHN.flags&=~MP_SAMPLE_BACKWARD; \
			} \
		} \
	} 

// double precision sinc without window function

template<bool ramping, mp_sint32 windowSize, class bufferType, mp_uint32 shift>
class SincResamplerDummy
{
private:
	static inline double sinc(double x)
	{
		double temp;
		if (x==0.0) return 1.0;
		else {
			temp = M_PI * x;
			return sin(temp) / (temp);
		}
	}
	
	enum 
	{
		WINDOWSIZE = windowSize, // must be even
		WIDTH = (WINDOWSIZE / 2)
	};	

public:
	static inline void addBlock(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		const bufferType* sample = (const bufferType*)chn->sample;

		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		
		const mp_sint32 rampFromVolStepL = ramping ? chn->rampFromVolStepL : 0;
		const mp_sint32 rampFromVolStepR = ramping ? chn->rampFromVolStepR : 0;		
		
		mp_sint32 smppos = chn->smppos;
		mp_sint32 smpposfrac = chn->smpposfrac;
		const mp_sint32 smpadd = (chn->flags&MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		
		const mp_sint32 flags = chn->flags;
		const mp_sint32 loopstart = chn->loopstart;
		const mp_sint32 loopend = chn->loopend;
		const mp_sint32 loopendcopy = chn->loopendcopy;
		const mp_sint32 smplen = chn->smplen;
		
		mp_sint32 fixedtimefrac = chn->fixedtimefrac;
		const mp_sint32 timeadd = chn->smpadd;
	
		while (count--)
		{
			double temp1 = 0;
			
			ChannelMixer::TMixerChannel pos;
			const double time_now = fixedtimefrac * (1.0 / 65536.0);
		
			if (abs(smpadd)<65536) 
			{
				pos.smppos = smppos; 
				pos.loopstart = loopstart;
				pos.loopend = loopend;
				pos.loopendcopy = loopendcopy;
				pos.flags = smpadd < 0 ? (flags & ~MP_SAMPLE_BACKWARD) : ((flags & ~MP_SAMPLE_BACKWARD) | MP_SAMPLE_BACKWARD); 
				if (!(((flags & 3) && pos.smppos > loopstart && pos.smppos< loopend)))
				{
					pos.loopstart = 0;
					pos.loopend = smplen;
					pos.flags &= ~3;
				}
																								
				double time = time_now;
			
				mp_sint32 j;				
				for (j = 0; j<WIDTH; j++)
				{
					//double w = 0.42 - 0.5 * cos(2.0*M_PI*(WIDTH-1-j)/(WIDTH*2)) + 0.08*cos(4.0*M_PI*(WIDTH-1-j)/(WIDTH*2));
				
					temp1 += (sample[pos.smppos]) * sinc(time);											

					time++;
					advancePos(pos);
					if (!(pos.flags & MP_SAMPLE_PLAY))
						break;
				}

				pos.smppos = smppos; 
				pos.flags = smpadd > 0 ? (flags & ~MP_SAMPLE_BACKWARD) : ((flags & ~MP_SAMPLE_BACKWARD) | MP_SAMPLE_BACKWARD); 

				time = time_now;

				for (j = 1; j<WIDTH; j++)
				{							
					//double w = 0.42 - 0.5 * cos(2.0*M_PI*(j-1+WIDTH)/(WIDTH*2)) + 0.08*cos(4.0*M_PI*(j-1+WIDTH)/(WIDTH*2));

					advancePos(pos);
					time--;
					if (!(pos.flags & MP_SAMPLE_PLAY))
						break;

					temp1 += (sample[pos.smppos]) * sinc(time);				
				}
			}
			else 
			{					
				double factor = smpadd * (1.0 / 65536.0);
				double one_over_factor = 1.0 / factor;

				pos.smppos = smppos; 
				pos.loopstart = loopstart;
				pos.loopend = loopend;
				pos.loopendcopy = loopendcopy;
				pos.flags = smpadd < 0 ? (flags & ~MP_SAMPLE_BACKWARD) : ((flags & ~MP_SAMPLE_BACKWARD) | MP_SAMPLE_BACKWARD); 
				if (!(((flags & 3) && pos.smppos > loopstart && pos.smppos< loopend)))
				{
					pos.loopstart = 0;
					pos.loopend = smplen;
					pos.flags &= ~3;
				}
									
				double time = time_now;
			
				mp_sint32 j;				
				for (j = 0; j<WIDTH; j++)
				{
					//double w = 0.42 - 0.5 * cos(2.0*M_PI*(WIDTH-1-j)/(WIDTH*2)) + 0.08*cos(4.0*M_PI*(WIDTH-1-j)/(WIDTH*2));
				
					temp1 += (sample[pos.smppos]) * one_over_factor * sinc(one_over_factor * time);											

					advancePos(pos);
					time++;
					if (!(pos.flags & MP_SAMPLE_PLAY))
						break;
				}

				pos.smppos = smppos; 
				pos.flags = smpadd > 0 ? (flags & ~MP_SAMPLE_BACKWARD) : ((flags & ~MP_SAMPLE_BACKWARD) | MP_SAMPLE_BACKWARD);

				time = time_now;

				for (j = 1; j<WIDTH; j++)
				{							
					//double w = 0.42 - 0.5 * cos(2.0*M_PI*(j-1+WIDTH)/(WIDTH*2)) + 0.08*cos(4.0*M_PI*(j-1+WIDTH)/(WIDTH*2));

					advancePos(pos);
					time--;
					if (!(pos.flags & MP_SAMPLE_PLAY))
						break;

					temp1 += (sample[pos.smppos]) * one_over_factor * sinc(one_over_factor * time);					
				}
			}
			
			
			mp_sint32 final = (mp_sint32)(temp1*(1 << (16-shift)));
			
			(*buffer++)+=((final*(voll>>15))>>15); 
			(*buffer++)+=((final*(volr>>15))>>15); 

			if (ramping)
			{
				voll+=rampFromVolStepL; 
				volr+=rampFromVolStepR; 
			}
			
			MP_INCREASESMPPOS(smppos, smpposfrac, smpadd, 16);
			fixedtimefrac=(fixedtimefrac+timeadd) & 65535;
		}
		
		chn->smppos = smppos;
		chn->smpposfrac = smpposfrac;

		chn->fixedtimefrac = fixedtimefrac;
		
		if (ramping)
		{
			chn->finalvoll = voll;
			chn->finalvolr = volr;	
		}
	}

};

template<bool ramping, mp_sint32 windowSize>
class ResamplerSinc : public ChannelMixer::ResamplerBase
{
private:
	
public:
	virtual bool isRamping() { return ramping; }
	virtual bool supportsFullChecking() { return false; }
	virtual bool supportsNoChecking() { return true; }

	virtual void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		if (chn->flags & 4)
			SincResamplerDummy<ramping, windowSize, mp_sword, 16>::addBlock(buffer, chn, count);		
		else
			SincResamplerDummy<ramping, windowSize, mp_sbyte, 8>::addBlock(buffer, chn, count);
	}
};

// fixed point sinc with almost hamming window

#define SINC(x) \
	((abs(x)>>16)>=(ResamplerSincTableBase<windowSize>::WIDTH-1) ? 0 : \
	(ResamplerSincTableBase<windowSize>::sinc_table[abs(x) >> (16-ResamplerSincTableBase<windowSize>::SAMPLES_PER_ZERO_CROSSING_SHIFT)] + \
	fpmul((ResamplerSincTableBase<windowSize>::sinc_table[(abs(x) >> (16-ResamplerSincTableBase<windowSize>::SAMPLES_PER_ZERO_CROSSING_SHIFT)) + 1] - \
	ResamplerSincTableBase<windowSize>::sinc_table[abs(x) >> (16-ResamplerSincTableBase<windowSize>::SAMPLES_PER_ZERO_CROSSING_SHIFT)]), \
	(abs(x) >> (16-ResamplerSincTableBase<windowSize>::SAMPLES_PER_ZERO_CROSSING_SHIFT)) & 65535)))
	
// share sinc lookup table
template<mp_sint32 windowSize>
class ResamplerSincTableBase : public ChannelMixer::ResamplerBase
{
protected:
	enum 
	{
		WINDOWSIZE = windowSize, // must be even
		WIDTH = (WINDOWSIZE / 2),
		SAMPLES_PER_ZERO_CROSSING_SHIFT = 10,
		SAMPLES_PER_ZERO_CROSSING = (1 << SAMPLES_PER_ZERO_CROSSING_SHIFT),
		TABLESIZE = SAMPLES_PER_ZERO_CROSSING*WIDTH,
	};

	static mp_sint32* sinc_table;

	void make_sinc()
	{
		mp_sint32 i;
		double temp,win_freq,win;
		win_freq = M_PI / WIDTH / SAMPLES_PER_ZERO_CROSSING;
		sinc_table[0] = 65536;
		for (i=1;i<WIDTH * SAMPLES_PER_ZERO_CROSSING;i++)   {
			temp = (double) i * M_PI / SAMPLES_PER_ZERO_CROSSING;
			win = 0.5 + 0.5 * cos(win_freq * i); // not quite true hamming window, but close
			sinc_table[i] = (mp_sint32)(((sin(temp) / temp) * win) * 65536.0);
		}
	}
	
	static bool tableInit;

	ResamplerSincTableBase()
	{
		if (!tableInit)
		{
			sinc_table = new mp_sint32[TABLESIZE];
			make_sinc();
			tableInit = true;
		}
	}
};

template<mp_sint32 windowSize>
bool ResamplerSincTableBase<windowSize>::tableInit = false;
template<mp_sint32 windowSize>
mp_sint32* ResamplerSincTableBase<windowSize>::sinc_table = NULL;

template<bool ramping, mp_sint32 windowSize, class bufferType, mp_uint32 shift>
class SincTableResamplerDummy : public ResamplerSincTableBase<windowSize>
{
public:
	static inline void addBlock(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		const bufferType* sample = (const bufferType*)chn->sample;

		mp_sint32 voll = chn->finalvoll;
		mp_sint32 volr = chn->finalvolr;
		
		const mp_sint32 rampFromVolStepL = ramping ? chn->rampFromVolStepL : 0;
		const mp_sint32 rampFromVolStepR = ramping ? chn->rampFromVolStepR : 0;		
		
		mp_sint32 smppos = chn->smppos;
		mp_sint32 smpposfrac = chn->smpposfrac;
		const mp_sint32 smpadd = (chn->flags&MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		const mp_sint32 rsmpadd = chn->rsmpadd;
		
		const mp_sint32 flags = chn->flags;
		const mp_sint32 loopstart = chn->loopstart;
		const mp_sint32 loopend = chn->loopend;
		const mp_sint32 loopendcopy = chn->loopendcopy;
		const mp_sint32 smplen = chn->smplen;
		
		mp_sint32 fixedtimefrac = chn->fixedtimefrac;
		const mp_sint32 timeadd = chn->smpadd;
	
		while (count--)
		{
			mp_sint32 result = 0;
			
			ChannelMixer::TMixerChannel pos;
		
			if (abs(smpadd) < 65536) 
			{
				pos.smppos = smppos; 
				pos.loopstart = loopstart;
				pos.loopend = loopend;
				pos.loopendcopy = loopendcopy;
				pos.flags = smpadd < 0 ? (flags & ~MP_SAMPLE_BACKWARD) : ((flags & ~MP_SAMPLE_BACKWARD) | MP_SAMPLE_BACKWARD); 
				if (!(((flags & 3) && pos.smppos > loopstart && pos.smppos< loopend)))
				{
					pos.loopstart = 0;
					pos.loopend = smplen;
					pos.flags &= ~3;
				}
																								
				mp_sint32 time = fixedtimefrac;
			
				mp_sint32 j;				
				for (j = 0; j<ResamplerSincTableBase<windowSize>::WIDTH; j++)
				{
					result += (sample[pos.smppos] * SINC(time)) >> shift;

					time+=65536;
					advancePos(pos);
					if (!(pos.flags & MP_SAMPLE_PLAY))
						break;
				}

				pos.smppos = smppos; 
				pos.flags = smpadd > 0 ? (flags & ~MP_SAMPLE_BACKWARD) : ((flags & ~MP_SAMPLE_BACKWARD) | MP_SAMPLE_BACKWARD); 

				time = fixedtimefrac;

				for (j = 1; j<ResamplerSincTableBase<windowSize>::WIDTH; j++)
				{							
					advancePos(pos);
					time-=65536;
					if (!(pos.flags & MP_SAMPLE_PLAY))
						break;

					result += (sample[pos.smppos] * SINC(time)) >> shift;
				}					
			}
			else 
			{					
				pos.smppos = smppos; 
				pos.loopstart = loopstart;
				pos.loopend = loopend+1;
				pos.loopendcopy = loopendcopy;
				pos.flags = smpadd < 0 ? (flags & ~MP_SAMPLE_BACKWARD) : ((flags & ~MP_SAMPLE_BACKWARD) | MP_SAMPLE_BACKWARD); 
				if (!(((flags & 3) && pos.smppos > loopstart && pos.smppos< loopend)))
				{
					pos.loopstart = 0;
					pos.loopend = smplen;
					pos.flags &= ~3;
				}
									
				mp_sint32 time = fpmul(fixedtimefrac, rsmpadd);
			
				mp_sint32 j;				
				for (j = 0; j<ResamplerSincTableBase<windowSize>::WIDTH; j++)
				{
					result += (sample[pos.smppos] * fpmul(SINC(time), rsmpadd)) >> shift;
				
					advancePos(pos);
					time+=rsmpadd;
					if (!(pos.flags & MP_SAMPLE_PLAY))
						break;
				}

				pos.smppos = smppos; 
				pos.flags = smpadd > 0 ? (flags & ~MP_SAMPLE_BACKWARD) : ((flags & ~MP_SAMPLE_BACKWARD) | MP_SAMPLE_BACKWARD);

				time = fpmul(fixedtimefrac, rsmpadd);

				for (j = 1; j<ResamplerSincTableBase<windowSize>::WIDTH; j++)
				{							
					advancePos(pos);
					time-=rsmpadd;
					if (!(pos.flags & MP_SAMPLE_PLAY))
						break;

					result += (sample[pos.smppos] * fpmul(SINC(time), rsmpadd)) >> shift;				
				}
			}
			
			
			(*buffer++)+=(((result)*(voll>>15))>>15); 
			(*buffer++)+=(((result)*(volr>>15))>>15); 

			if (ramping)
			{
				voll+=rampFromVolStepL; 
				volr+=rampFromVolStepR; 
			}
			
			MP_INCREASESMPPOS(smppos, smpposfrac, smpadd, 16);
			fixedtimefrac=(fixedtimefrac+timeadd) & 65535;
		}

		chn->smppos = smppos;
		chn->smpposfrac = smpposfrac;

		chn->fixedtimefrac = fixedtimefrac;
		
		if (ramping)
		{
			chn->finalvoll = voll;
			chn->finalvolr = volr;	
		}
	}
};

template<bool ramping, mp_sint32 windowSize>
class ResamplerSincTable : public ResamplerSincTableBase<windowSize>
{
public:
	ResamplerSincTable() :
		ResamplerSincTableBase<windowSize>()
	{
	}

	virtual bool isRamping() { return ramping; }
	virtual bool supportsFullChecking() { return false; }
	virtual bool supportsNoChecking() { return true; }

	virtual void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		if (chn->flags & 4)
			SincTableResamplerDummy<ramping, windowSize, mp_sword, 16>::addBlock(buffer, chn, count);		
		else
			SincTableResamplerDummy<ramping, windowSize, mp_sbyte, 8>::addBlock(buffer, chn, count);
	}
};

#undef fpmul
