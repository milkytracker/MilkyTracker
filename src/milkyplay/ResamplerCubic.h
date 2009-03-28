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
 *  ResamplerCubic.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 03.01.08.
 *
 */

/*
 * Cubic 4 Point 3rd order polynomial interpolation resampler                 
 *
 * Formulas from:
 * Polynomial Interpolators for High-Quality Resampling of Oversampled Audio by 
 * Olli Niemitalo ollinie@freenet.hut.fi 
 *
 * Website: http://www.student.oulu.fi/~oniemita/DSP/INDEX.HTM
 *
 */
 
#define __DEIP__
#define fpmul MP_FP_MUL

enum CubicResamplers
{
	CubicResamplerLagrange,
	CubicResamplerSpline
};

// here is some code to work around my VS6.1 backward compability
template<bool ramping, CubicResamplers type, class bufferType, mp_uint32 shift>
class CubicResamplerDummy
{
public:
	static inline mp_sint32 interpolate_lagrange4Point(const bufferType* sample,
													   const mp_sint32 smppos)
	{
		// note that the sample buffer is padded with leading and
		// trailing space which is filled with samples from the 
		// beginning/end, so even if it SEEMS we are crossing sample
		// boundaries by -1, +1 and +2, it doesn't happen
#ifdef __DEIP__		
		const mp_sint32 v1 = (mp_sint32)sample[(smppos>>16)] << (16-shift);
		const mp_sint32 v2 = (mp_sint32)sample[(smppos>>16) + 1] << (16-shift);							
		const mp_sint32 v0 = (mp_sint32)sample[(smppos>>16) - 1] << (16-shift);
		const mp_sint32 v3 = (mp_sint32)sample[(smppos>>16) + 2] << (16-shift);
		
		const mp_sint32 x = smppos & 65535;
		
		mp_sint32 c0 = v1; 
		mp_sint32 c1 = v2 - ((v0*(65536/3))>>16) - ((v3*(65536/6))>>16) - (v1>>1); 
		mp_sint32 c2 = ((v0+v2)>>1) - v1; 
		mp_sint32 c3 = (((65536/6)*(v3-v0))>>16) + ((v1-v2)>>1); 
		
		return fpmul(fpmul(fpmul(c3,x)+c2,x)+c1, x)+c0;
#else
		const mp_sint32 v1 = sample[(smppos>>16)] << (16-shift);
		const mp_sint32 v2 = sample[(smppos>>16) + 1] << (16-shift);							
		const mp_sint32 v0 = sample[(smppos>>16) - 1] << (16-shift);
		mp_sint32 v3 = sample[(smppos>>16) + 2] << 8;
		const mp_sint32 ofsf = (smppos&65535) + 65536;
		v3 += -3*v2 + 3*v1 - v0;
		v3 = fpmul(v3, ((ofsf - 2*65536) * (65536/6)) >> 16);
		v3 += v2 - v1 - v1 + v0;
		v3 = fpmul(v3, (ofsf - 65536) >> 1);
		v3 += v1 - v0;
		v3 = fpmul(v3, ofsf);
		v3 += v0;	
		return v3;
#endif
	}
	
	static inline mp_sint32 interpolate_spline4Point(const bufferType* sample,
													 const mp_sint32 smppos)
	{
#ifdef __DEIP__
		const mp_sint32 v1 = sample[(smppos>>16)] << (16-shift);
		const mp_sint32 v2 = sample[(smppos>>16) + 1] << (16-shift);							
		const mp_sint32 v0 = sample[(smppos>>16) - 1] << (16-shift);
		const mp_sint32 v3 = sample[(smppos>>16) + 2] << (16-shift);
		
		const mp_sint32 x = smppos & 65535;
		
		mp_sint32 ym1py1 = v0+v2; 
		mp_sint32 c0 = (((65536/6)*ym1py1) + (65536*2/3)*v1) >> 16; 
		mp_sint32 c1 = (v2-v0)>>1; 
		mp_sint32 c2 = (ym1py1>>1) - v1; 
		mp_sint32 c3 = ((v1-v2)>>1) + (((65536/6)*(v3-v0)) >> 16); 
		
		return fpmul(fpmul(fpmul(c3,x)+c2,x)+c1, x)+c0;
#else
		// this the stuff timidity uses, I have no clue what it does, but
		// it causes heavy gibbs phenomena and doesn't look nearly as the above
		const mp_sint32 v1 = sample[(smppos>>16)] << 8;
		const mp_sint32 v2 = sample[(smppos>>16) + 1] << 8;							
		const mp_sint32 v0 = sample[(smppos>>16) - 1] << 8;
		const mp_sint32 v3 = sample[(smppos>>16) + 2] << 8;
		
		const mp_sint32 ofsf = smppos & 65535;
		const mp_sint32 temp = v2;
		const mp_sint32 temp2 = (5 * v3 - 11 * v2 + 7 * v1 - v0) >> 2;
		
		const mp_sint32 hres = fpmul(fpmul(fpmul(temp2, ofsf + 65536), (ofsf - 65536)) + 6*v2, ofsf);
		
		const mp_sint32 temp3 = (5 * v0 - 11 * v1 + 7 * temp - v3) >> 2;
		
		const mp_sint32 temp4 = fpmul(temp3, ofsf) >> 16;
		const mp_sint32 temp5 = fpmul(temp4, ofsf - 65536*2) >> 16;
		const mp_sint32 res = fpmul(6*v1+temp5, 65536 - ofsf);
		
		return fpmul(res + hres, 65536 / 6);
#endif
	}
	
	static inline void addBlock(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
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
		
		const bufferType* sample = ((const bufferType*)chn->sample) + smppos;
		smppos = smpposfrac;
		
		while (count--)
		{
			mp_sint32 s;
			
			switch (type)
			{
				case CubicResamplerLagrange:
					s = interpolate_lagrange4Point(sample, smppos);
					break;
				case CubicResamplerSpline:
					s = interpolate_spline4Point(sample, smppos);
					break;
			}
			
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
		
		if (ramping)
		{
			chn->finalvoll = voll;
			chn->finalvolr = volr;	
		}
	}
};

template<bool ramping, CubicResamplers type>
class ResamplerLagrange : public ChannelMixer::ResamplerBase
{
public:
	virtual bool isRamping() { return ramping; }
	virtual bool supportsFullChecking() { return false; }
	virtual bool supportsNoChecking() { return true; }

	virtual void addBlockNoCheck(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		if (chn->flags & 4)
			CubicResamplerDummy<ramping, type, mp_sword, 16>::addBlock(buffer, chn, count);		
		else
			CubicResamplerDummy<ramping, type, mp_sbyte, 8>::addBlock(buffer, chn, count);
	}
};

#undef __DEIP__
#undef fpmul
