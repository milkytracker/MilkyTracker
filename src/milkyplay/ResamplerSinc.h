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
 *  ResamplerSinc.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 03.01.08.
 *
 */

#include <math.h>

/*
 * Sinc resamplers based on:                                                        
 * http://www.cs.princeton.edu/courses/archive/spr07/cos325/src/TimeStuf/srconvrt.c  
 *
 */
 
#ifndef M_PI 
#define M_PI 3.14159265358979323846 
#endif

#define fpmul MP_FP_MUL

#define advancePos(CHNsmppos, CHNflags, CHNloopstart, CHNloopend, CHNloopendcopy) \
	if (((((CHNflags&3) == 0 || (CHNflags&3) == 1)) && !(CHNflags&ChannelMixer::MP_SAMPLE_BACKWARD)) || \
		((CHNflags&3) == 2 && (CHNflags&ChannelMixer::MP_SAMPLE_BACKWARD) == 0)) \
	{ \
		CHNsmppos++; \
		/* stop playing if necessary */ \
		if (CHNsmppos>=CHNloopend) \
		{ \
			if ((CHNflags & 3) == 0) \
			{ \
				if (CHNflags & ChannelMixer::MP_SAMPLE_ONESHOT) \
				{ \
					CHNflags &= ~ChannelMixer::MP_SAMPLE_ONESHOT; \
					CHNflags |= 1; \
					CHNloopend = CHNloopendcopy; \
					CHNsmppos = CHNloopstart; \
				} \
				else \
				{ \
					CHNflags&=~ChannelMixer::MP_SAMPLE_PLAY; \
				} \
			} \
			else if ((CHNflags & 3) == 1) \
			{ \
				CHNsmppos = CHNloopstart; \
			} \
			else \
			{ \
				CHNflags|=ChannelMixer::MP_SAMPLE_BACKWARD; \
				CHNsmppos = CHNloopend-1; \
			} \
		}\
	} \
	/* bi-dir loop */ \
	else \
	{ \
		CHNsmppos--; \
		if (CHNloopstart>CHNsmppos) \
		{ \
			if ((CHNflags & 3) == 0) \
			{ \
				CHNflags&=~ChannelMixer::MP_SAMPLE_PLAY; \
			} \
			else if ((CHNflags & 3) == 1) \
			{ \
				CHNsmppos = CHNloopend-1; \
			} \
			else \
			{ \
				CHNflags&=~ChannelMixer::MP_SAMPLE_BACKWARD; \
				CHNsmppos = CHNloopstart; \
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
		if (x==0.0) 
			return 1.0;
		else 
		{
			double temp = M_PI * x;
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
		const mp_sint32 smpadd = (chn->flags&ChannelMixer::MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		
		const mp_sint32 flags = chn->flags;
		const mp_sint32 loopstart = chn->loopstart;
		const mp_sint32 loopend = chn->loopend;
		const mp_sint32 loopendcopy = chn->loopendcopy;
		const mp_sint32 smplen = chn->smplen;
		
		mp_sint32 fixedtimefrac = chn->fixedtimefrac;
		const mp_sint32 timeadd = chn->smpadd;
	
		ChannelMixer::TMixerChannel pos(true);

		while (count--)
		{
			double result = 0;
			
			const double time_now = fixedtimefrac * (1.0 / 65536.0);
		
			if (abs(smpadd)<65536) 
			{
				pos.smppos = smppos; 
				pos.loopstart = loopstart;
				pos.loopend = loopend;
				pos.loopendcopy = loopendcopy;
				pos.flags = smpadd < 0 ? (flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) : ((flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) | ChannelMixer::MP_SAMPLE_BACKWARD); 
				// check whether we are outside loop points
				// if that's the case we're treating the sample as a normal finite signal
				// note that this is still not totally correct treatment
				const bool outSideLoop = !(((flags & 3) && pos.smppos >= loopstart && pos.smppos < loopend));
				if (outSideLoop)
				{
					pos.loopstart = 0;
					pos.loopend = smplen;
					pos.flags &= ~3;
				}
																								
				double time = time_now;
				if (!fixedtimefrac && (flags & ChannelMixer::MP_SAMPLE_BACKWARD)) 
					time = 1.0;
			
				mp_sint32 j;				
				for (j = 0; j<WIDTH; j++)
				{
					//double w = 0.42 - 0.5 * cos(2.0*M_PI*(WIDTH-1-j)/(WIDTH*2)) + 0.08*cos(4.0*M_PI*(WIDTH-1-j)/(WIDTH*2));
				
					result += (sample[pos.smppos]) * sinc(time);											

					time++;
					advancePos(pos.smppos, pos.flags, pos.loopstart, pos.loopend, pos.loopendcopy);
					if (!(pos.flags & ChannelMixer::MP_SAMPLE_PLAY))
						break;
				}

				pos.smppos = smppos; 
				pos.flags = smpadd > 0 ? (flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) : ((flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) | ChannelMixer::MP_SAMPLE_BACKWARD); 
				if (outSideLoop)
					pos.flags &= ~3;

				time = time_now;
				if (!fixedtimefrac && (flags & ChannelMixer::MP_SAMPLE_BACKWARD)) 
					time = 1.0;

				for (j = 1; j<WIDTH; j++)
				{							
					//double w = 0.42 - 0.5 * cos(2.0*M_PI*(j-1+WIDTH)/(WIDTH*2)) + 0.08*cos(4.0*M_PI*(j-1+WIDTH)/(WIDTH*2));

					advancePos(pos.smppos, pos.flags, pos.loopstart, pos.loopend, pos.loopendcopy);
					time--;
					if (!(pos.flags & ChannelMixer::MP_SAMPLE_PLAY))
						break;

					result += (sample[pos.smppos]) * sinc(time);				
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
				pos.flags = smpadd < 0 ? (flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) : ((flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) | ChannelMixer::MP_SAMPLE_BACKWARD); 
				// check whether we are outside loop points
				// if that's the case we're treating the sample as a normal finite signal
				// note that this is still not totally correct treatment
				const bool outSideLoop = !(((flags & 3) && pos.smppos >= loopstart && pos.smppos < loopend));
				if (outSideLoop)
				{
					pos.loopstart = 0;
					pos.loopend = smplen;
					pos.flags &= ~3;
				}
									
				double time = time_now;
				if (!fixedtimefrac && (flags & ChannelMixer::MP_SAMPLE_BACKWARD)) 
					time = 1.0;
			
				mp_sint32 j;				
				for (j = 0; j<WIDTH; j++)
				{
					//double w = 0.42 - 0.5 * cos(2.0*M_PI*(WIDTH-1-j)/(WIDTH*2)) + 0.08*cos(4.0*M_PI*(WIDTH-1-j)/(WIDTH*2));
				
					result += (sample[pos.smppos]) * one_over_factor * sinc(one_over_factor * time);											

					advancePos(pos.smppos, pos.flags, pos.loopstart, pos.loopend, pos.loopendcopy);
					time++;
					if (!(pos.flags & ChannelMixer::MP_SAMPLE_PLAY))
						break;
				}

				pos.smppos = smppos; 
				pos.flags = smpadd > 0 ? (flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) : ((flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) | ChannelMixer::MP_SAMPLE_BACKWARD);
				if (outSideLoop)
					pos.flags &= ~3;

				time = time_now;
				if (!fixedtimefrac && (flags & ChannelMixer::MP_SAMPLE_BACKWARD)) 
					time = 1.0;

				for (j = 1; j<WIDTH; j++)
				{							
					//double w = 0.42 - 0.5 * cos(2.0*M_PI*(j-1+WIDTH)/(WIDTH*2)) + 0.08*cos(4.0*M_PI*(j-1+WIDTH)/(WIDTH*2));

					advancePos(pos.smppos, pos.flags, pos.loopstart, pos.loopend, pos.loopendcopy);
					time--;
					if (!(pos.flags & ChannelMixer::MP_SAMPLE_PLAY))
						break;

					result += (sample[pos.smppos]) * one_over_factor * sinc(one_over_factor * time);					
				}
			}
			
			
			mp_sint32 final = (mp_sint32)(result*(1 << (16-shift)));
			
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

// you like that, eh?
#define SINCTAB ResamplerSincTableBase<windowSize>::sinc_table
#define WSIZE ResamplerSincTableBase<windowSize>::WIDTH
#define SPZCSHIFT ResamplerSincTableBase<windowSize>::SAMPLES_PER_ZERO_CROSSING_SHIFT

#define SINC(x) \
	((abs(x)>>16)>=(WSIZE-1) ? 0 : \
	(SINCTAB[abs(x) >> (16-SPZCSHIFT)] + \
	fpmul((SINCTAB[(abs(x) >> (16-SPZCSHIFT)) + 1] - \
	SINCTAB[abs(x) >> (16-SPZCSHIFT)]), \
	(abs(x) >> (16-SPZCSHIFT)) & 65535)))
	
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
		const mp_sint32 smpadd = (chn->flags&ChannelMixer::MP_SAMPLE_BACKWARD) ? -chn->smpadd : chn->smpadd;
		const mp_sint32 rsmpadd = chn->rsmpadd;
		
		const mp_sint32 flags = chn->flags;
		const mp_sint32 loopstart = chn->loopstart;
		const mp_sint32 loopend = chn->loopend;
		const mp_sint32 loopendcopy = chn->loopendcopy;
		const mp_sint32 smplen = chn->smplen;
		
		mp_sint32 fixedtimefrac = chn->fixedtimefrac;
		const mp_sint32 timeadd = chn->smpadd;
	
		const mp_sint32 negflags = smpadd < 0 ? (flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) : ((flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) | ChannelMixer::MP_SAMPLE_BACKWARD);
		const mp_sint32 posflags = smpadd > 0 ? (flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) : ((flags & ~ChannelMixer::MP_SAMPLE_BACKWARD) | ChannelMixer::MP_SAMPLE_BACKWARD);
		
		mp_sint32 tmpsmppos;
		mp_sint32 tmpflags;
		mp_sint32 tmploopstart;
		mp_sint32 tmploopend;
		
		if (timeadd < 65536)
		{
			while (count--)
			{
				mp_sint32 result = 0;
				
				tmpsmppos = smppos; 
				tmploopstart = loopstart;
				tmploopend = loopend;
				tmpflags = negflags; 
				// check whether we are outside loop points
				// if that's the case we're treating the sample as a normal finite signal
				// note that this is still not totally correct treatment
				const bool outSideLoop = !(((flags & 3) && tmpsmppos >= loopstart && tmpsmppos < loopend));
				if (outSideLoop)
				{
					tmploopstart = 0;
					tmploopend = smplen;
					tmpflags &= ~3;
				}
				
				mp_sint32 time = fixedtimefrac;
				if (!time && (flags & ChannelMixer::MP_SAMPLE_BACKWARD)) 
					time = 65536;
				
				mp_sint32 j;				
				for (j = 0; j<ResamplerSincTableBase<windowSize>::WIDTH; j++)
				{
					result += (sample[tmpsmppos] * SINC(time)) >> shift;
					
					time+=65536;
					advancePos(tmpsmppos, tmpflags, tmploopstart, tmploopend, loopendcopy);
					if (!(tmpflags & ChannelMixer::MP_SAMPLE_PLAY))
						break;
				}
				
				tmpsmppos = smppos; 
				tmpflags = posflags; 
				if (outSideLoop)
					tmpflags &= ~3;
				
				time = fixedtimefrac;
				if (!time && (flags & ChannelMixer::MP_SAMPLE_BACKWARD)) 
					time = 65536;
				
				for (j = 1; j<ResamplerSincTableBase<windowSize>::WIDTH; j++)
				{							
					advancePos(tmpsmppos, tmpflags, tmploopstart, tmploopend, loopendcopy);
					time-=65536;
					if (!(tmpflags & ChannelMixer::MP_SAMPLE_PLAY))
						break;
					
					result += (sample[tmpsmppos] * SINC(time)) >> shift;
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
		}
		else
		{
			while (count--)
			{
				mp_sint32 result = 0;
				
				tmpsmppos = smppos; 
				tmploopstart = loopstart;
				tmploopend = loopend;
				tmpflags = negflags; 
				// check whether we are outside loop points
				// if that's the case we're treating the sample as a normal finite signal
				// note that this is still not totally correct treatment
				const bool outSideLoop = !(((flags & 3) && tmpsmppos >= loopstart && tmpsmppos < loopend));
				if (outSideLoop)
				{
					tmploopstart = 0;
					tmploopend = smplen;
					tmpflags &= ~3;
				}
				
				mp_sint32 time = fpmul(fixedtimefrac, rsmpadd);
				if (!time && (flags & ChannelMixer::MP_SAMPLE_BACKWARD)) 
					time = 65536;
				
				mp_sint32 j;				
				for (j = 0; j<ResamplerSincTableBase<windowSize>::WIDTH; j++)
				{
					result += (sample[tmpsmppos] * fpmul(SINC(time), rsmpadd)) >> shift;
					
					advancePos(tmpsmppos, tmpflags, tmploopstart, tmploopend, loopendcopy);
					time+=rsmpadd;
					if (!(tmpflags & ChannelMixer::MP_SAMPLE_PLAY))
						break;
				}
				
				tmpsmppos = smppos; 
				tmpflags = posflags;
				if (outSideLoop)
					tmpflags &= ~3;
				
				time = fpmul(fixedtimefrac, rsmpadd);
				if (!time && (flags & ChannelMixer::MP_SAMPLE_BACKWARD)) 
					time = 65536;
				
				for (j = 1; j<ResamplerSincTableBase<windowSize>::WIDTH; j++)
				{							
					advancePos(tmpsmppos, tmpflags, tmploopstart, tmploopend, loopendcopy);
					time-=rsmpadd;
					if (!(tmpflags & ChannelMixer::MP_SAMPLE_PLAY))
						break;
					
					result += (sample[tmpsmppos] * fpmul(SINC(time), rsmpadd)) >> shift;				
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

#undef SINC

#undef SPZCSHIFT
#undef WSIZE
#undef SINCTAB

#undef fpmul
