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
 *  ResamplerMacros.h
 *  MilkyPlay mixer macros & "templates"
 *
 *
 *  "- Be prepared! Are you sure you want to know? :-)"
 *
 */
#ifndef __RESAMPLERMACROS_H__
#define __RESAMPLERMACROS_H__

#define VALIDATE \
	/*ASSERT((void*)(sample+(posfixed>>16)) >= (void*)chn->sample);*/

#define PROCESS_BLOCK(MIXER) \
	mp_sint32 blockCount = count>>3; \
	mp_sint32 remainCount = count & 7; \
	while (blockCount) \
	{ \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		blockCount--; \
	} \
	while (remainCount) \
	{ \
		VALIDATE \
		MIXER \
		posfixed+=smpadd; \
		remainCount--; \
	} 

#define NOCHECKMIXER_TEMPLATE(MIXER_8BIT,MIXER_16BIT) \
	if (!(chn->flags&4)) \
	{ \
		const mp_sbyte* sample = chn->sample + basepos; \
		PROCESS_BLOCK(MIXER_8BIT) \
	} \
	else \
	{ \
		const mp_sword* sample = (const mp_sword*)chn->sample + basepos; \
		PROCESS_BLOCK(MIXER_16BIT) \
	} 
	
/////////////////////////////////////////////////////////
//		NO INTERPOLATION AND NO VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
#define NOCHECKMIXER_8BIT_NORMAL \
	sd1 = sample[posfixed>>16]<<8; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); 

#define NOCHECKMIXER_16BIT_NORMAL \
	sd1 = sample[posfixed>>16]; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); 

/////////////////////////////////////////////////////////
//			NO INTERPOLATION BUT VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
#define NOCHECKMIXER_8BIT_NORMAL_RAMP(_RAMP_) \
	sd1 = sample[posfixed>>16]<<8; \
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	}

#define NOCHECKMIXER_16BIT_NORMAL_RAMP(_RAMP_) \
	sd1 = sample[posfixed>>16]; \
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	}

/////////////////////////////////////////////////////////
//			INTERPOLATION AND NO VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
#define NOCHECKMIXER_8BIT_LERP \
	sd1 = sample[posfixed>>16]<<8; \
	sd2 = sample[(posfixed>>16)+1]<<8; \
	\
	sd1 =((sd1<<12)+((posfixed>>4)&0xfff)*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); 

#define NOCHECKMIXER_16BIT_LERP \
	sd1 = sample[posfixed>>16]; \
	sd2 = sample[(posfixed>>16)+1]; \
	\
	sd1 =((sd1<<12)+((posfixed>>4)&0xfff)*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); 

/////////////////////////////////////////////////////////
//			INTERPOLATION AND VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
#define NOCHECKMIXER_8BIT_LERP_RAMP(_RAMP_) \
	sd1 = sample[posfixed>>16]<<8; \
	sd2 = sample[(posfixed>>16)+1]<<8; \
	\
	sd1 =((sd1<<12)+((posfixed>>4)&0xfff)*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	}

#define NOCHECKMIXER_16BIT_LERP_RAMP(_RAMP_) \
	sd1 = sample[posfixed>>16]; \
	sd2 = sample[(posfixed>>16)+1]; \
	\
	sd1 =((sd1<<12)+((posfixed>>4)&0xfff)*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	}

/////////////////////////////////////////////////////////
//	    INTERPOLATION/VOLUME RAMPING and FILTERING     //
/////////////////////////////////////////////////////////
#define NOCHECKMIXER_8BIT_LERP_RAMP_FILTER(_RAMP_) \
	sd1 = sample[posfixed>>16]<<8; \
	sd2 = sample[(posfixed>>16)+1]<<8; \
	\
	sd1 =((sd1<<12)+((posfixed>>4)&0xfff)*(sd2-sd1))>>12; \
	\
	sd1 = (MP_FP_MUL(sd1, a) + MP_FP_MUL(currsample, b) + MP_FP_MUL(prevsample, c)) >> ChannelMixer::MP_FILTERPRECISION; \
	prevsample = currsample; \
	currsample = sd1; \
	\
	/* adjust volume for left/right channels & mix */ \
	(*buffer++)+=MP_FP_MUL(sd1, voll>>14); \
	(*buffer++)+=MP_FP_MUL(sd1, volr>>14); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	}

#define NOCHECKMIXER_16BIT_LERP_RAMP_FILTER(_RAMP_) \
	sd1 = sample[posfixed>>16]; \
	sd2 = sample[(posfixed>>16)+1]; \
	\
	sd1 =((sd1<<12)+((posfixed>>4)&0xfff)*(sd2-sd1))>>12; \
	\
	sd1 = (MP_FP_MUL(sd1, a) + MP_FP_MUL(currsample, b) + MP_FP_MUL(prevsample, c)) >> ChannelMixer::MP_FILTERPRECISION; \
	prevsample = currsample; \
	currsample = sd1; \
	\
	/* adjust volume for left/right channels & mix */ \
	(*buffer++)+=MP_FP_MUL(sd1, voll>>14); \
	(*buffer++)+=MP_FP_MUL(sd1, volr>>14); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	}

#define BIDIR_REPOSITION(FRACBITS, SMPPOS, SMPPOSFRAC, LOOPSTART, LOOPEND) \
	if (!((SMPPOS == LOOPEND) && (SMPPOSFRAC == 0) ||\
		(SMPPOS == LOOPSTART) && (SMPPOSFRAC == 0))) \
	{ \
		do \
		{ \
			if (SMPPOS >= LOOPEND) \
				SMPPOS = LOOPEND-(SMPPOS-LOOPEND+1); \
			else if (SMPPOS < LOOPSTART) \
				SMPPOS = LOOPSTART + (LOOPSTART-SMPPOS - 1); \
			SMPPOSFRAC = ((1<<FRACBITS)-SMPPOSFRAC) & ((1<<FRACBITS)-1); \
		} while (!(SMPPOS >= LOOPSTART && SMPPOS <= LOOPEND)); \
	}

						
#define FULLMIXER_TEMPLATE(MIXER_8BIT, MIXER_16BIT, FRACBITS, LABELNO) \
	mp_sint32 smppos = chn->smppos; \
	mp_sint32 smpposfrac = chn->smpposfrac; \
	mp_sint32 smpadd = chn->smpadd; \
	mp_sint32 loopstart = chn->loopstart; \
	mp_sint32 loopend = chn->loopend; \
	mp_sint32 flags = chn->flags; \
	const mp_sbyte* sample = chn->sample; \
	mp_sint32 sd1,sd2; \
	\
	/* 8 bit version */ \
	if (!(flags&4)) \
	{ \
		\
		if (((((flags&3) == 0 || (flags&3) == 1)) && !(flags&ChannelMixer::MP_SAMPLE_BACKWARD)) || \
			((flags&3) == 2 && (flags&ChannelMixer::MP_SAMPLE_BACKWARD) == 0)) \
		{ \
			\
			while (count) \
			{ \
				MIXER_8BIT \
				MP_INCREASESMPPOS(smppos,smpposfrac,smpadd,FRACBITS); \
				/* stop playing if necessary */ \
				if (smppos>=loopend) \
				{ \
					if ((flags & 3) == 0) \
					{ \
						if (flags & ChannelMixer::MP_SAMPLE_ONESHOT) \
						{ \
							flags &= ~ChannelMixer::MP_SAMPLE_ONESHOT; \
							flags |= 1; \
							chn->loopend = chn->loopendcopy; \
							loopend = chn->loopend; \
							/*ASSERT(loopend-loopstart > 0);*/ \
							smppos = ((smppos - loopstart)%(loopend-loopstart))+loopstart; \
						} \
						else \
						{ \
							flags&=~ChannelMixer::MP_SAMPLE_PLAY; \
							break; \
						} \
					} \
					else if ((flags & 3) == 1) \
					{ \
						/*if (smppos>=((loopend<<1)-loopstart))*/ \
						/*	smppos=loopstart;*/ \
						/*else */\
						/*	smppos=loopstart+(smppos-loopend); */\
						/*ASSERT(loopend-loopstart > 0);*/ \
						smppos = ((smppos - loopstart)%(loopend-loopstart))+loopstart; \
					} \
					else \
					{ \
						flags|=ChannelMixer::MP_SAMPLE_BACKWARD; \
						BIDIR_REPOSITION(FRACBITS, smppos, smpposfrac, loopstart, loopend) \
						/*ASSERT(smppos >= loopstart && smppos <= loopend);*/ \
						goto continueWithBiDir8_## LABELNO; \
					} \
				} \
				\
continueWithNormalDir8_## LABELNO: \
					\
				count--; \
				\
			}	\
			\
		} \
		/* bi-dir loop */ \
		else \
		{ \
			while (count) \
			{ \
				MIXER_8BIT \
				MP_INCREASESMPPOS(smppos,smpposfrac,-smpadd,FRACBITS); \
				\
				if (loopstart>smppos) \
				{ \
					if ((flags & 3) == 0) \
					{ \
						flags&=~ChannelMixer::MP_SAMPLE_PLAY; \
						break; \
					} \
					else if ((flags & 3) == 1) \
					{ \
						smppos = loopend-((loopstart-smppos)%(loopend-loopstart)); \
					} \
					else \
					{ \
						flags&=~ChannelMixer::MP_SAMPLE_BACKWARD; \
						BIDIR_REPOSITION(FRACBITS, smppos, smpposfrac, loopstart, loopend) \
						/*ASSERT(smppos >= loopstart && smppos <= loopend);*/ \
						goto continueWithNormalDir8_## LABELNO; \
					} \
				} \
				\
continueWithBiDir8_## LABELNO: \
					\
				count--; \
				\
			} \
			\
		} \
		\
	} \
	else \
	{ \
		\
		if (((((flags&3) == 0 || (flags&3) == 1)) && !(flags&ChannelMixer::MP_SAMPLE_BACKWARD)) || \
			((flags&3) == 2 && (flags&ChannelMixer::MP_SAMPLE_BACKWARD) == 0)) \
		{ \
		\
			while (count) \
			{ \
				MIXER_16BIT \
				MP_INCREASESMPPOS(smppos,smpposfrac,smpadd,FRACBITS); \
				/* stop playing if necessary */ \
				if (smppos>=loopend) \
				{ \
					if ((flags & 3) == 0) \
					{ \
						if (flags & ChannelMixer::MP_SAMPLE_ONESHOT) \
						{ \
							flags &= ~ChannelMixer::MP_SAMPLE_ONESHOT; \
							flags |= 1; \
							chn->loopend = chn->loopendcopy; \
							loopend = chn->loopend; \
							smppos = ((smppos - loopstart)%(loopend-loopstart))+loopstart; \
						} \
						else \
						{ \
							flags&=~ChannelMixer::MP_SAMPLE_PLAY; \
							break; \
						} \
					} \
					else if ((flags & 3) == 1) \
					{ \
						/*if (smppos>=((loopend<<1)-loopstart))*/ \
						/*	smppos=loopstart;*/ \
						/*else */\
						/*	smppos=loopstart+(smppos-loopend); */\
						smppos = ((smppos - loopstart)%(loopend-loopstart))+loopstart; \
					} \
					else \
					{ \
						flags|=ChannelMixer::MP_SAMPLE_BACKWARD; \
						BIDIR_REPOSITION(FRACBITS, smppos, smpposfrac, loopstart, loopend) \
						/*ASSERT(smppos >= loopstart && smppos <= loopend);*/ \
						goto continueWithBiDir16_## LABELNO; \
					} \
				} \
				\
continueWithNormalDir16_## LABELNO: \
					\
				count--; \
				\
			}	\
			\
		} \
		/* bi-dir loop */ \
		else \
		{ \
			while (count) \
			{ \
				MIXER_16BIT \
				MP_INCREASESMPPOS(smppos,smpposfrac,-smpadd,FRACBITS); \
				\
				if (loopstart>smppos) \
				{ \
					if ((flags & 3) == 0) \
					{ \
						flags&=~ChannelMixer::MP_SAMPLE_PLAY; \
						break; \
					} \
					else if ((flags & 3) == 1) \
					{ \
						smppos = loopend-((loopstart-smppos)%(loopend-loopstart)); \
					} \
					else \
					{ \
						flags&=~ChannelMixer::MP_SAMPLE_BACKWARD; \
						BIDIR_REPOSITION(FRACBITS, smppos, smpposfrac, loopstart, loopend) \
						/*ASSERT(smppos >= loopstart && smppos <= loopend);*/ \
						goto continueWithNormalDir16_## LABELNO; \
					} \
				} \
				\
continueWithBiDir16_## LABELNO: \
					\
				count--; \
				\
			} \
			\
		} \
		\
	} \
	chn->smppos = smppos; \
	chn->smpposfrac = smpposfrac; \
	/*chn->smpadd = smpadd;*/ \
	chn->flags = flags;

/////////////////////////////////////////////////////////
//		NO INTERPOLATION AND NO VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
#define FULLMIXER_8BIT_NORMAL \
	/* 8 bit sample */ \
	sd1 = ((mp_sbyte)sample[smppos])<<8; \
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15);

#define FULLMIXER_16BIT_NORMAL \
	/* 16 bit sample */ \
	sd1 = ((mp_sword*)(sample))[smppos]; \
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); 

/////////////////////////////////////////////////////////
//			NO INTERPOLATION BUT VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
#define FULLMIXER_8BIT_NORMAL_RAMP(_RAMP_) \
	/* 8 bit sample */ \
	sd1 = ((mp_sbyte)sample[smppos])<<8; \
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	} 

#define FULLMIXER_16BIT_NORMAL_RAMP(_RAMP_) \
	/* 16 bit sample */ \
	sd1 = ((mp_sword*)(sample))[smppos]; \
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	} 

/////////////////////////////////////////////////////////
//			INTERPOLATION AND NO VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
#define FULLMIXER_8BIT_LERP \
	/* 8 bit sample */ \
	sd1 = ((mp_sbyte)sample[smppos])<<8; \
	sd2 = ((mp_sbyte)sample[smppos+1])<<8; \
	\
	sd1 =((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); 

#define FULLMIXER_16BIT_LERP \
	/* 16 bit sample */ \
	sd1 = ((mp_sword*)(sample))[smppos]; \
	sd2 = ((mp_sword*)(sample))[smppos+1]; \
	\
	sd1 =((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); 
		
/////////////////////////////////////////////////////////
//			INTERPOLATION AND VOLUME RAMPING		   //
/////////////////////////////////////////////////////////
#define FULLMIXER_8BIT_LERP_RAMP(_RAMP_) \
	/* 8 bit sample */ \
	sd1 = ((mp_sbyte)sample[smppos])<<8; \
	sd2 = ((mp_sbyte)sample[smppos+1])<<8; \
	\
	sd1 =((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	} 

#define FULLMIXER_16BIT_LERP_RAMP(_RAMP_) \
	/* 16 bit sample */ \
	sd1 = ((mp_sword*)(sample))[smppos]; \
	sd2 = ((mp_sword*)(sample))[smppos+1]; \
	\
	sd1 =((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	} 

/////////////////////////////////////////////////////////
//	    INTERPOLATION/VOLUME RAMPING and FILTERING     //
/////////////////////////////////////////////////////////
#define FULLMIXER_8BIT_LERP_RAMP_FILTER(_RAMP_) \
	sd1 = ((mp_sbyte)sample[smppos])<<8; \
	sd2 = ((mp_sbyte)sample[smppos+1])<<8; \
	\
	sd1 =((sd1<<12)+((smpposfrac>>4)&0xfff)*(sd2-sd1))>>12; \
	\
	sd1 = (MP_FP_MUL(sd1, a) + MP_FP_MUL(currsample, b) + MP_FP_MUL(prevsample, c)) >> ChannelMixer::MP_FILTERPRECISION; \
	prevsample = currsample; \
	currsample = sd1; \
	\
	/* adjust volume for left/right channels & mix */ \
	(*buffer++)+=MP_FP_MUL(sd1, voll>>14); \
	(*buffer++)+=MP_FP_MUL(sd1, volr>>14); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	}

#define FULLMIXER_16BIT_LERP_RAMP_FILTER(_RAMP_) \
	sd1 = ((mp_sword*)(sample))[smppos]; \
	sd2 = ((mp_sword*)(sample))[smppos+1]; \
	\
	sd1 =((sd1<<12)+((smpposfrac>>4)&0xfff)*(sd2-sd1))>>12; \
	\
	sd1 = (MP_FP_MUL(sd1, a) + MP_FP_MUL(currsample, b) + MP_FP_MUL(prevsample, c)) >> ChannelMixer::MP_FILTERPRECISION; \
	prevsample = currsample; \
	currsample = sd1; \
	\
	/* adjust volume for left/right channels & mix */ \
	(*buffer++)+=MP_FP_MUL(sd1, voll>>14); \
	(*buffer++)+=MP_FP_MUL(sd1, volr>>14); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	}


/////////////////////////////////////////////////////////
//		 INTERPOLATION AND VOLUME RAMPING HIRES		   //
/////////////////////////////////////////////////////////
#define FULLMIXER_8BIT_LERP_RAMP_HIRES(_RAMP_) \
	/* 8 bit sample */ \
	sd1 = ((mp_sbyte)sample[smppos])<<8; \
	sd2 = ((mp_sbyte)sample[smppos+1])<<8; \
	\
	sd1 =((sd1<<12)+(smpposfrac>>(4+12))*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	} 

#define FULLMIXER_16BIT_LERP_RAMP_HIRES(_RAMP_) \
	/* 16 bit sample */ \
	sd1 = ((mp_sword*)(sample))[smppos]; \
	sd2 = ((mp_sword*)(sample))[smppos+1]; \
	\
	sd1 =((sd1<<12)+(smpposfrac>>(4+12))*(sd2-sd1))>>12; \
	\
	/* adjust volume for left channel & mix */ \
	(*buffer++)+=((sd1*(voll>>15))>>15); \
	/* adjust volume for right channel & mix */ \
	(*buffer++)+=((sd1*(volr>>15))>>15); \
	\
	if ((_RAMP_)) \
	{ \
		voll+=rampFromVolStepL; \
		volr+=rampFromVolStepR; \
	} 


#endif
