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
 *  ChannelMixer.cpp
 *  MilkyPlay mixer class
 *
 *
 *  "- Be prepared! Are you sure you want to know? :-)"
 *
 *  This class is pretty much emulating a Gravis Ultrasound with a timer set to 250Hz
 *  I.e. the mixer() routine will call a user specified handler 250 times per second
 *  while mixing the audio stream in between.
 */
#include "ChannelMixer.h"
#include "ResamplerFactory.h"
#include "ResamplerMacros.h"
#include "AudioDriverManager.h"
#include <math.h>
 
// Ramp out will last (THEBEATLENGTH*RAMPDOWNFRACTION)>>8 samples
#define RAMPDOWNFRACTION 256

static inline mp_sint32 myMod(mp_sint32 a, mp_sint32 b)
{
	mp_sint32 r = a % b;
	return r < 0 ? b + r : r;
}

/*
  "I've found out that FT2 uses the square root pan law, which is
  close to the linear pan law, but it stresses samples that are played
  on the far left and right. Note that its indices range from 0 to
  256, while the internal panning ranges from 0 to 255, meaning that
  there is no true 100% right panning, only 100% left is possible."

  - Saga_Musix @ http://modarchive.org/forums/index.php?topic=3517.0
*/
mp_sint32 ChannelMixer::panLUT[257];
void ChannelMixer::panToVol (ChannelMixer::TMixerChannel *chn, mp_sint32 &volL, mp_sint32 &volR)
{
	mp_sint32 pan = (((chn->pan - 128)*panningSeparation) >> 8) + 128;
	if (pan < 0) pan = 0;
	if (pan > 255) pan = 255;

	volL = (chn->vol*panLUT[256-pan]*masterVolume);
	volR = (chn->vol*panLUT[pan]*masterVolume);

	if (chn->flags&MP_SAMPLE_MUTE)
		volL = volR = 0;
}

void ChannelMixer::ResamplerBase::addChannelsNormal(ChannelMixer* mixer, mp_uint32 numChannels, mp_sint32* buffer32,mp_sint32 beatNum, mp_sint32 beatlength)
{
	ChannelMixer::TMixerChannel* channel = mixer->channel;
	ChannelMixer::TMixerChannel* newChannel = mixer->newChannel;
	
	for (mp_uint32 c=0;c<numChannels;c++) 
	{
		ChannelMixer::TMixerChannel* chn = &channel[c];
		chn->index = c;		// For Amiga resampler

		if (!(chn->flags & MP_SAMPLE_PLAY))
			continue;
	
		switch (chn->flags&(MP_SAMPLE_FADEOUT|MP_SAMPLE_FADEIN|MP_SAMPLE_FADEOFF))
		{
			case MP_SAMPLE_FADEOFF:
			{
				chn->flags&=~(MP_SAMPLE_PLAY | MP_SAMPLE_FADEOFF);
				continue;
			}

			case MP_SAMPLE_FADEOUT:
			{
				chn->sample = newChannel[c].sample;
				chn->smplen = newChannel[c].smplen;
				chn->loopstart = newChannel[c].loopstart;
				chn->loopend = newChannel[c].loopend;
				chn->smppos = newChannel[c].smppos;
				chn->smpposfrac = newChannel[c].smpposfrac;
				chn->flags = newChannel[c].flags;
				chn->loopendcopy = newChannel[c].loopendcopy;
				chn->fixedtime = newChannel[c].fixedtimefrac;
				chn->fixedtimefrac = newChannel[c].fixedtimefrac;
				// break is missing here intentionally!!!
			}
			default:
			{
				mixer->panToVol(chn, chn->finalvoll, chn->finalvolr);
				break;
			}
		}
		
		// mix here
		addChannel(chn, buffer32, beatlength, beatlength);
		
	}
}

void ChannelMixer::ResamplerBase::addChannelsRamping(ChannelMixer* mixer, mp_uint32 numChannels, mp_sint32* buffer32,mp_sint32 beatNum, mp_sint32 beatlength)
{
	ChannelMixer::TMixerChannel* channel = mixer->channel;
	ChannelMixer::TMixerChannel* newChannel = mixer->newChannel;
	
	for (mp_uint32 c=0;c<numChannels;c++) 
	{	
		ChannelMixer::TMixerChannel* chn = &channel[c];
		chn->index = c;		// For Amiga resampler
		
		if (!(chn->flags & MP_SAMPLE_PLAY))
			continue;
		
		switch (chn->flags&(MP_SAMPLE_FADEOUT|MP_SAMPLE_FADEIN|MP_SAMPLE_FADEOFF))
		{
			case MP_SAMPLE_FADEOFF:
			{
				mp_sint32 maxramp = (beatlength*RAMPDOWNFRACTION)>>8;
				mp_sint32 beatl = (!(chn->flags & 3)) ? (ChannelMixer::fixedmul(chn->loopend,chn->rsmpadd) >> 1) : maxramp; 

				if (beatl > maxramp || beatl <= 0)
					beatl = maxramp;
				
				chn->rampFromVolStepL = (-chn->finalvoll)/beatl; 
				chn->rampFromVolStepR = (-chn->finalvolr)/beatl; 
				
				if (beatl)
					addChannel(chn, buffer32, beatl, beatlength);
				chn->flags&=~(MP_SAMPLE_PLAY | MP_SAMPLE_FADEOFF);
				continue;
			}
		
			case MP_SAMPLE_FADEIN:
			{
				chn->flags = (chn->flags&~(MP_SAMPLE_FADEOUT|MP_SAMPLE_FADEIN))/*|MP_SAMPLE_FADEIN*/;

				//mp_sint32 beatl = (beatlength*RAMPDOWNFRACTION)>>8;
				
				mp_sint32 maxramp = (beatlength*RAMPDOWNFRACTION)>>8;
				mp_sint32 beatl = (!(chn->flags & 3)) ? (ChannelMixer::fixedmul(chn->loopend,chn->rsmpadd) >> 1) : maxramp; 

				if (beatl > maxramp || beatl <= 0)
					beatl = maxramp;

				mp_sint32 volL, volR;
				mixer->panToVol(chn, volL, volR);
				
				chn->rampFromVolStepL = (volL-chn->finalvoll)/beatl;				
				chn->rampFromVolStepR = (volR-chn->finalvolr)/beatl;
				
				// mix here
				if (beatl)
					addChannel(chn, buffer32, beatl, beatlength);

				//chn->finalvoll = volL;
				//chn->finalvolr = volR;

				chn->rampFromVolStepL = 0;				
				chn->rampFromVolStepR = 0;
				
				mp_sint32 offset = beatl;
				
				beatl = beatlength - beatl;
				
				if (beatl)
					addChannel(chn, buffer32+offset*MP_NUMCHANNELS, beatl, beatlength);
				break;
			}
			
			case MP_SAMPLE_FADEOUT:
			{
				mp_sint32 maxramp = (beatlength*RAMPDOWNFRACTION)>>8;
				mp_sint32 beatl = (!(chn->flags & 3)) ? (ChannelMixer::fixedmul(chn->loopend,chn->rsmpadd) >> 1) : maxramp; 

				if (beatl > maxramp || beatl <= 0)
					beatl = maxramp;
				
				chn->rampFromVolStepL = (0-chn->finalvoll)/beatl;				
				chn->rampFromVolStepR = (0-chn->finalvolr)/beatl;
				
				chn->flags = (chn->flags&~(MP_SAMPLE_FADEOUT|MP_SAMPLE_FADEIN))/*|MP_SAMPLE_FADEIN*/;
				
				// for the last active sample we need to retrieve the last active sample rate
				// which is temporarly stored in newChannel[c]
				// get it, fill it in and restore the original value later
				mp_sint32 tmpsmpadd = chn->smpadd;
				mp_sint32 tmprsmpadd = chn->rsmpadd;
				mp_sint32 tmpcurrsample = chn->currsample; 
				mp_sint32 tmpprevsample = chn->prevsample;					
				mp_sint32 tmpa = chn->a;
				mp_sint32 tmpb = chn->b;
				mp_sint32 tmpc = chn->c;
				chn->smpadd = newChannel[c].smpadd;
				chn->rsmpadd = newChannel[c].rsmpadd;
				chn->currsample = newChannel[c].currsample;
				chn->prevsample = newChannel[c].prevsample;
				chn->a = newChannel[c].a;
				chn->b = newChannel[c].b;
				chn->c = newChannel[c].c;				
				if (beatl)
					addChannel(chn, buffer32, beatl, beatlength);
				chn->smpadd = tmpsmpadd;
				chn->rsmpadd = tmprsmpadd;
				chn->currsample = tmpcurrsample; 
				chn->prevsample = tmpprevsample;					
				chn->a = tmpa;
				chn->b = tmpb;
				chn->c = tmpc;

				// fade in new sample
				chn->sample = newChannel[c].sample;
				chn->smplen = newChannel[c].smplen;
				chn->loopstart = newChannel[c].loopstart;
				chn->loopend = newChannel[c].loopend;
				chn->smppos = newChannel[c].smppos;				
				chn->smpposfrac = newChannel[c].smpposfrac;
				chn->flags = newChannel[c].flags;
				chn->loopendcopy = newChannel[c].loopendcopy;
				chn->fixedtime = newChannel[c].fixedtimefrac;
				chn->fixedtimefrac = newChannel[c].fixedtimefrac;

				beatl = (!(chn->flags & 3)) ? (ChannelMixer::fixedmul(chn->loopend,chn->rsmpadd) >> 1) : maxramp; 

				if (beatl > maxramp || beatl <= 0)
					beatl = maxramp;
				
				mp_sint32 volL, volR;
				mixer->panToVol(chn, volL, volR);

				chn->rampFromVolStepL = volL/beatl;				
				chn->rampFromVolStepR = volR/beatl;

				chn->finalvoll = chn->finalvolr = 0;

				if (beatl)
					addChannel(chn, buffer32, beatl, beatlength);

				chn->rampFromVolStepL = 0;				
				chn->rampFromVolStepR = 0;
				
				mp_sint32 offset = beatl;
				
				beatl = beatlength - beatl;
				
				if (beatl)
					addChannel(chn, buffer32+offset*MP_NUMCHANNELS, beatl, beatlength);
				
				continue;
			}
			default:
			{
				mp_sint32 volL, volR;
				mixer->panToVol(chn, volL, volR);
				
				chn->rampFromVolStepL = (volL-chn->finalvoll)/beatlength;				
				chn->rampFromVolStepR = (volR-chn->finalvolr)/beatlength;
				
				// mix here
				addChannel(chn, buffer32, beatlength, beatlength);
	
				//chn->finalvoll = volL;
				//chn->finalvolr = volR;	
				break;
			}
		}
		
	}
}

void ChannelMixer::ResamplerBase::addChannels(ChannelMixer* mixer, mp_uint32 numChannels, mp_sint32* buffer32,mp_sint32 beatNum, mp_sint32 beatlength)
{
	if (beatNum >= (signed)mixer->getNumBeatPackets())
		beatNum = mixer->getNumBeatPackets();

	if (isRamping())
		addChannelsRamping(mixer, numChannels, buffer32, beatNum, beatlength);
	else
		addChannelsNormal(mixer, numChannels, buffer32, beatNum, beatlength);
}

void ChannelMixer::ResamplerBase::addChannel(TMixerChannel* chn, mp_sint32* buffer32, const mp_sint32 beatlength, const mp_sint32 beatSize)
{
	if ((chn->flags&MP_SAMPLE_PLAY)) 
	{ 
		/* check for computational work */ 
		mp_sint32 d = ChannelMixer::fixedmul((chn->loopend - chn->loopstart)<<4,chn->rsmpadd); 
		/* just a threshold value: */ 
		/* if distance between loopend and loopstart is very small and sample */ 
		/* step is "big" we need to many divisions to compute remaining sample */ 
		/* packets so add a full checked channel */ 
		if ((d<128 && supportsFullChecking()) || (supportsFullChecking() && !supportsNoChecking())) 
		{ 
			addBlockFull((buffer32), chn, (beatlength));
		} 
		else 
		{ 
			mp_sint32* tempBuffer32 = (buffer32); 
			mp_sint32 todo = (beatlength); 
			bool limit = false;
			while (todo>0) 
			{ 
				if (chn->flags&MP_SAMPLE_BACKWARD) 
				{ 
					mp_sint32 pos = ((todo*-chn->smpadd - chn->smpposfrac)>>16)+chn->smppos; 
					if (pos>chn->loopstart) 
					{ 
						addBlockNoCheck(tempBuffer32,chn,todo); 
						break; 
					} 
					else 
					{ 
						mp_sint32 length = MP_FP_CEIL(ChannelMixer::fixedmul((((chn->smppos-chn->loopstart)<<16)+chn->smpposfrac),chn->rsmpadd)); 
						
						if (!length) length++; 
						if (length>todo) 
						{
							// Final mixing length is limited by the remaining buffer size
							length = todo; 
							// Mark that we're limited
							limit = true;
						}
						/* sample is going to stop => fade out because of ending clicks*/ 
						else if ((chn->flags & 3) == 0) 
						{ 
							mp_sint32 maxramp = (beatSize*RAMPDOWNFRACTION)>>8;
							mp_sint32 rampl = ChannelMixer::fixedmul(chn->loopend,chn->rsmpadd) >> 1; 

							if (rampl > maxramp || rampl <= 0)
								rampl = maxramp;
								
							if (rampl < length)
							{
								length = length-rampl;
								addBlockNoCheck(tempBuffer32,chn,length); 
								tempBuffer32+=length*MP_NUMCHANNELS; 
								length = rampl;
							}

							chn->rampFromVolStepL = (-chn->finalvoll)/length; 
							chn->rampFromVolStepR = (-chn->finalvolr)/length; 
						} 
						addBlockNoCheck(tempBuffer32, chn, length);
						// Only stop when we're not limited, otherwise the sample will continue playing  
						if ((chn->flags & 3) == 0 && !limit) 
						{ 
							if (chn->flags & MP_SAMPLE_ONESHOT) 
							{ 
								chn->flags &= ~MP_SAMPLE_ONESHOT; 
								chn->flags |= 1; 
								chn->loopstart = chn->loopendcopy;
								chn->smppos = chn->smplen - myMod(chn->smplen - chn->smppos, chn->loopend-chn->loopstart); 
							} 
							else 
								chn->flags&=~MP_SAMPLE_PLAY; 
							break; 
						} 
						else if ((chn->flags & 3) == 1) 
						{ 
							// Is this correct looping??
							chn->smppos = chn->loopend - myMod(chn->loopend - chn->smppos, chn->loopend-chn->loopstart); 
						} 
						// Check if we went out of ping-pong loop bounds
						else if (chn->smppos < chn->loopstart)
						{ 
							// Invert
							chn->flags&=~MP_SAMPLE_BACKWARD; 
							BIDIR_REPOSITION(16, chn->smppos, chn->smpposfrac, chn->loopstart, chn->loopend);
						} 
						tempBuffer32+=length*MP_NUMCHANNELS; 
						todo-=length; 
					} 
				} 
				else 
				{ 
					mp_sint32 pos = ((todo*chn->smpadd + chn->smpposfrac)>>16)+chn->smppos; 
					if (pos<chn->loopend) 
					{ 
						addBlockNoCheck(tempBuffer32,chn,todo); 
						break; 
					} 
					else 
					{ 
						mp_sint32 length = MP_FP_CEIL(ChannelMixer::fixedmul((((chn->loopend-chn->smppos)<<16)-chn->smpposfrac),chn->rsmpadd)); 
						if (!length) length++; 
						if (length>todo) 
						{
							length = todo; 
							limit = true;
						}
						/* sample is going to stop => fade out because of ending clicks */ 
						else if ((chn->flags & 3) == 0) 
						{ 
							mp_sint32 maxramp = (beatSize*RAMPDOWNFRACTION)>>8;
							mp_sint32 rampl = ChannelMixer::fixedmul(chn->loopend,chn->rsmpadd) >> 1; 

							if (rampl > maxramp || rampl <= 0)
								rampl = maxramp;
								
							if (rampl < length)
							{
								length = length-rampl;
								addBlockNoCheck(tempBuffer32,chn,length); 
								tempBuffer32+=length*MP_NUMCHANNELS; 
								length = rampl;
							}
							
							chn->rampFromVolStepL = (-chn->finalvoll)/length; 
							chn->rampFromVolStepR = (-chn->finalvolr)/length; 
						} 
						addBlockNoCheck(tempBuffer32,chn,length); 
						if ((chn->flags & 3) == 0 && !limit) 
						{ 
							if (chn->flags & MP_SAMPLE_ONESHOT)
							{ 
								chn->flags &= ~MP_SAMPLE_ONESHOT; 
								chn->flags |= 1; 
								chn->loopend = chn->loopendcopy; 
								/*ASSERT(chn->loopend-chn->loopstart > 0);*/
								chn->smppos = ((chn->smppos - chn->smplen)%(chn->loopend-chn->loopstart))+chn->loopstart; 
							} 
							else 
								chn->flags&=~MP_SAMPLE_PLAY; 
							break; 
						} 
						else if ((chn->flags & 3) == 1) 
						{ 
							/*ASSERT(chn->loopend-chn->loopstart > 0);*/
							chn->smppos = ((chn->smppos - chn->loopstart)%(chn->loopend-chn->loopstart))+chn->loopstart; 
							/* correct if pre-calculation was a little bit incorrect*/ 
							if (chn->smppos < 0) 
								chn->smppos = 0; 
						} 
						else if (chn->smppos >= chn->loopend)
						{ 						
							// Invert
							chn->flags|=MP_SAMPLE_BACKWARD;
							BIDIR_REPOSITION(16, chn->smppos, chn->smpposfrac, chn->loopstart, chn->loopend);
						} 
						tempBuffer32+=length*MP_NUMCHANNELS; 
						todo-=length; 
					} 
				} 
			} 
		} 
	} 
}

void ChannelMixer::muteChannel(mp_sint32 c, bool m) 
{ 
	channel[c].flags&=~MP_SAMPLE_MUTE;
	if (m) channel[c].flags|=MP_SAMPLE_MUTE;
}

bool ChannelMixer::isChannelMuted(mp_sint32 c)
{
	return (channel[c].flags&MP_SAMPLE_MUTE) == MP_SAMPLE_MUTE;
}

void ChannelMixer::setFrequency(mp_sint32 frequency)
{
	if (frequency == (signed)mixFrequency)
		return;

	mixFrequency = frequency;
	rMixFrequency = 0x7FFFFFFF / frequency;

	beatPacketSize = (MP_BEATLENGTH*frequency)/MP_BASEFREQ;	
	
	if (mixbuffBeatPacket)
	{
		delete[] mixbuffBeatPacket;
		mixbuffBeatPacket = NULL;
	}
	
	mixbuffBeatPacket = new mp_sint32[beatPacketSize*MP_NUMCHANNELS];
	
	// channels contain information based on beatPacketSize so this might
	// have been changed
	reallocChannels();
	
	if (resamplerType != MIXER_INVALID && resamplerTable[resamplerType])
		resamplerTable[resamplerType]->setFrequency(frequency);
}

void ChannelMixer::reallocChannels()
{
	// optimization in case we already have the allocated number of channels
	if (mixerNumAllocatedChannels != mixerLastNumAllocatedChannels)
	{
		delete[] channel;
		channel = new TMixerChannel[mixerNumAllocatedChannels];	
		
		delete[] newChannel;
		newChannel = new TMixerChannel[mixerNumAllocatedChannels];
		
		clearChannels();
	}
	
#if defined(MILKYTRACKER) || defined (__MPTIMETRACKING__)
	for (mp_uint32 i = 0; i < mixerNumAllocatedChannels; i++)
		channel[i].reallocTimeRecord(getNumBeatPackets()+1);
#endif	
	
	mixerLastNumAllocatedChannels = mixerNumAllocatedChannels;

	if (resamplerType != MIXER_INVALID && resamplerTable[resamplerType])
		resamplerTable[resamplerType]->setNumChannels(mixerNumAllocatedChannels);
}

void ChannelMixer::clearChannels()
{
	for (mp_uint32 i = 0; i < mixerNumAllocatedChannels; i++)
	{
		channel[i].clear();
		newChannel[i].clear();
	}
}

ChannelMixer::ChannelMixer(mp_uint32 numChannels,
						   mp_uint32 frequency) :
	mixerNumAllocatedChannels(numChannels),
	mixerNumActiveChannels(numChannels),
	mixerLastNumAllocatedChannels(0),
	mixFrequency(0),
	mixbuffBeatPacket(NULL),
	mixBufferSize(0),
	channel(NULL),
	newChannel(NULL),
	resamplerType(MIXER_INVALID),
	paused(false),
	disableMixing(false),
	allowFilters(false),
	initialized(false),
	sampleCounter(0)
{	
	memset(resamplerTable, 0, sizeof(resamplerTable));

	setFrequency(frequency);

	// full volume
	masterVolume = 256;
	panningSeparation = 256;
	
	for (mp_sint32 i = 0; i < NUMRESAMPLERTYPES; i++)
	{
		setFreqFuncTable[i]	= &ChannelMixer::setChannelFrequency;
		resamplerTable[i] = NULL;
	}

	setResamplerType(MIXER_NORMAL);

	setBufferSize(BUFFERSIZE_DEFAULT);

	// FT2 panning law
	if (panLUT[1] == 0)
		for (int i = 0; i <= 256; i++)
			panLUT[i] = static_cast<mp_sint32> (8192.0 * sqrt(i/256.0) + 0.5);
}

ChannelMixer::~ChannelMixer()
{
	if (initialized)
	{
		closeDevice();
	}

	if (mixbuffBeatPacket)
		delete[] mixbuffBeatPacket;

	if (channel) 
		delete[] channel;
	
	if (newChannel) 
		delete[] newChannel;
	
	for (mp_uint32 i = 0; i < sizeof(resamplerTable) / sizeof(ResamplerBase*); i++)
		delete resamplerTable[i];
}

void ChannelMixer::resetChannelsWithoutMuting()
{	
	mp_ubyte* isMuted = new mp_ubyte[mixerNumAllocatedChannels];
	
	// save muting
	mp_uint32 i;
	for (i = 0; i < mixerNumAllocatedChannels; i++)
		isMuted[i] = (mp_ubyte)isChannelMuted(i);
	
	clearChannels();

	for (i = 0; i < mixerNumAllocatedChannels; i++)
		muteChannel(i, isMuted[i] == 1);
	
	delete[] isMuted;
	
	lastBeatRemainder = 0;
}

void ChannelMixer::resetChannelsFull()
{	
	clearChannels();

	lastBeatRemainder = 0;
}

void ChannelMixer::setResamplerType(ResamplerTypes type) 
{
	if (resamplerTable[type] == NULL)
		resamplerTable[type] = ResamplerFactory::createResampler(type);
	
	resamplerType = type; 

	if (resamplerType != MIXER_INVALID && resamplerTable[resamplerType])
	{
		resamplerTable[resamplerType]->setFrequency(mixFrequency);				
		resamplerTable[resamplerType]->setNumChannels(mixerNumAllocatedChannels);
	}
}

void ChannelMixer::setNumChannels(mp_uint32 num)
{
	if (num > mixerNumAllocatedChannels)
	{
		mixerNumAllocatedChannels = num;

		reallocChannels();
		resetChannelsFull();
	}
	else
	{
		mixerNumAllocatedChannels = num;
		resetChannelsWithoutMuting();
	}
}

void ChannelMixer::setActiveChannels(mp_uint32 num)
{ 
	if (num > mixerNumAllocatedChannels)
	{
		mixerNumAllocatedChannels = num;
		
		reallocChannels();
		resetChannelsFull();
	}

	mixerNumActiveChannels = num; 
}

// Default lo precision calculations
void ChannelMixer::setChannelFrequency(mp_sint32 c, mp_sint32 f)
{
	channel[c].smpadd = ((mp_sint32)(((mp_int64)((mp_int64)f*(mp_int64)rMixFrequency))>>15))<<0; 
	
	if (channel[c].smpadd)
		channel[c].rsmpadd = 0xFFFFFFFF/channel[c].smpadd;
	else
		channel[c].rsmpadd = 0;			
}

void ChannelMixer::setFilterAttributes(mp_sint32 chn, mp_sint32 cutoff, mp_sint32 resonance)
{
	if (!allowFilters ||
		(channel[chn].cutoff == cutoff &&
		 channel[chn].resonance == resonance))
		return;
	
	channel[chn].cutoff = cutoff;
	channel[chn].resonance = resonance;
	
	if (cutoff == MP_INVALID_VALUE || resonance == MP_INVALID_VALUE)
		return;

	// Thanks to DUMB for the filter coefficient computations	
	const float LOG10 = 2.30258509299f;
	const mp_sint32 IT_ENVELOPE_SHIFT = 8;
	
	float a, b, c;
	{
		float sampfreq = this->mixFrequency;
	
		float inv_angle = (float)(sampfreq * pow(0.5, 0.25 + cutoff*(1.0/(24<<IT_ENVELOPE_SHIFT))) * (1.0/(2*3.14159265358979323846*110.0)));
		float loss = (float)exp(resonance*(-LOG10*1.2/128.0));
		float d, e;
#if 0
		loss *= 2; // This is the mistake most players seem to make!
#endif

#if 1
		d = (1.0f - loss) / inv_angle;
		if (d > 2.0f) d = 2.0f;
		d = (loss - d) * inv_angle;
		e = inv_angle * inv_angle;
		a = 1.0f / (1.0f + d + e);
		c = -e * a;
		b = 1.0f - a - c;
#else
		a = 1.0f / (inv_angle*inv_angle + inv_angle*loss + loss);
		c = -(inv_angle*inv_angle) * a;
		b = 1.0f - a - c;
#endif
	}
	
	channel[chn].a = (mp_sint32)(a * (1 << (MP_FILTERPRECISION+16)));
	channel[chn].b = (mp_sint32)(b * (1 << (MP_FILTERPRECISION+16)));
	channel[chn].c = (mp_sint32)(c * (1 << (MP_FILTERPRECISION+16)));
}

void ChannelMixer::playSample(mp_sint32 c, // channel
							  mp_sbyte* smp, // sample buffer
							  mp_sint32 smplen, // sample size
							  mp_sint32 smpoffs, // sample offset 
							  mp_sint32 smpoffsfrac,
							  bool smpOffsetWrap,
							  mp_sint32 lstart, // loop start
							  mp_sint32 len, // loop end
							  mp_sint32 flags,
							  bool ramp/* = true*/) 
{
	// doesn't play
	if (smp == NULL)
		return;
	
	// disable looping when loopstart = loopend
	if (lstart == len)
	{
		flags &= ~(3+32);
		lstart = 0;
		len = smplen;
	}
	
	// this is not allowed, assume bidir loop when both forward and biloop settings are made
	if ((flags & 3) == 3) flags &= ~1;
	
	// stupid check if artists are to stupid to use a valid sampleoffset
	// seems to be correct
	// treat bidir looped samples as normal samples
	if (((flags & 3) == 2) || !(flags&3))
	{
		if (smpoffs >= len) 
		{
			if (smpOffsetWrap)
				return;
			else
			{
				stopSample(c);
				return;
			}
		}
	}
	// below offset correction code only works for forward played samples
	// to-do: add offset correction code for bi-dir loops
	else if ((flags & 3) == 1)
	{
		if (smpOffsetWrap)
		{
			if (smpoffs > len)
			{
				smpoffs = ((smpoffs - lstart)%(len-lstart))+lstart;
			}
			else if (smpoffs == len && smpoffsfrac)
			{
				smpoffs = lstart;
			}
		}
		else
		{
			if (smpoffs >= len) 
			{
				stopSample(c);
				return;
			}
		}
	}
	
	// play sample but don't ramp volume
	if (!ramp)
	{
		channel[c].sample=(mp_sbyte*)smp;
		channel[c].smplen = smplen;
		channel[c].loopstart=lstart;
		channel[c].loopend=len;
		
		if (flags & MP_SAMPLE_BACKWARD)
			channel[c].smppos = smplen - smpoffs;
		else
			channel[c].smppos = smpoffs;
		
		channel[c].smpposfrac = smpoffsfrac;						
		channel[c].flags&=~MP_SAMPLE_FADEOFF;
		channel[c].flags=flags|MP_SAMPLE_PLAY|(channel[c].flags&MP_SAMPLE_MUTE);

		channel[c].currsample = channel[c].prevsample = 0;
		
		channel[c].fixedtime = 0;
		channel[c].fixedtimefrac = smpoffsfrac;
	}
	// currently no sample playing on that channel
	else if (!(channel[c].flags&MP_SAMPLE_PLAY))
	{
		channel[c].sample=(mp_sbyte*)smp;
		channel[c].smplen = smplen;
		channel[c].loopstart=lstart;
		channel[c].loopend=len;
		
		if (flags & MP_SAMPLE_BACKWARD)
			channel[c].smppos = smplen - smpoffs;
		else
			channel[c].smppos = smpoffs;
		
		channel[c].smpposfrac = smpoffsfrac;
		channel[c].flags&=~MP_SAMPLE_FADEOFF;
		channel[c].flags=flags|MP_SAMPLE_PLAY|(channel[c].flags&MP_SAMPLE_MUTE)|MP_SAMPLE_FADEIN;
		// if a new sample is played, its volume is ramped from zero to current volume
		channel[c].finalvoll = 0;
		channel[c].finalvolr = 0;
		
		// one shot looping sample?
		if (flags & 32)
		{
			// Not looping yet
			channel[c].flags &= ~(3+32);
			channel[c].flags |= MP_SAMPLE_ONESHOT;
			if (flags & MP_SAMPLE_BACKWARD)
			{
				channel[c].loopendcopy = channel[c].loopstart;
				channel[c].loopstart = 0;
			}
			else
			{
				channel[c].loopendcopy = channel[c].loopend;
				channel[c].loopend = channel[c].smplen;
			}
		}
	
		channel[c].currsample = channel[c].prevsample = 0;
		
		channel[c].fixedtime = 0;
		channel[c].fixedtimefrac = smpoffsfrac;
	}
	// there is a sample playing on that channel, ramp volume of current sample down
	// then play new sample and ramp volume up
	else
	{
		newChannel[c].sample=(mp_sbyte*)smp;
		newChannel[c].smplen = smplen;
		newChannel[c].loopstart = lstart;
		newChannel[c].loopend = len;
		
		if (flags & MP_SAMPLE_BACKWARD)
			newChannel[c].smppos = smplen - smpoffs;
		else
			newChannel[c].smppos = smpoffs;
		
		newChannel[c].smpposfrac = smpoffsfrac;
		newChannel[c].flags=flags|MP_SAMPLE_PLAY|(channel[c].flags&MP_SAMPLE_MUTE);
		// if a new sample is played, its volume is ramped from zero to current volume
		newChannel[c].finalvoll = newChannel[c].finalvolr = 0;
		newChannel[c].currsample = newChannel[c].prevsample = 0;

		newChannel[c].fixedtime = 0;
		newChannel[c].fixedtimefrac = smpoffsfrac;
		
		// "fade off" current sample
		channel[c].flags = (channel[c].flags&~(MP_SAMPLE_FADEOUT|MP_SAMPLE_FADEIN|MP_SAMPLE_FADEOFF))|MP_SAMPLE_FADEOUT;
		
		// one shot looping sample?
		if (flags & 32)
		{
			// Not looping yet
			newChannel[c].flags &= ~(3+32);
			newChannel[c].flags |= MP_SAMPLE_ONESHOT;

			if (flags & MP_SAMPLE_BACKWARD)
			{
				newChannel[c].loopendcopy = newChannel[c].loopstart;
				newChannel[c].loopstart = 0;
			}
			else
			{
				newChannel[c].loopendcopy = newChannel[c].loopend;
				newChannel[c].loopend = newChannel[c].smplen;
			}						
		}
	}
	
}

static inline void storeTimeRecordData(mp_sint32 nb, ChannelMixer::TMixerChannel* chn)
{
	if (!(chn->flags & ChannelMixer::MP_SAMPLE_PLAY))
	{
		if (chn->timeRecord)
		{
			chn->timeRecord[nb].flags = chn->flags;
			chn->timeRecord[nb].sample = NULL;
			chn->timeRecord[nb].volPan = 128 << 16;
			chn->timeRecord[nb].smppos = -1;
		}
	}
	else
	{
		if (chn->timeRecord)
		{
			chn->timeRecord[nb].flags = chn->flags;
			chn->timeRecord[nb].sample = chn->sample;
			chn->timeRecord[nb].smppos = chn->smppos;
			chn->timeRecord[nb].volPan = chn->vol + (chn->pan << 16);
			chn->timeRecord[nb].smpposfrac = chn->smpposfrac;
			chn->timeRecord[nb].smpadd = chn->smpadd;
			chn->timeRecord[nb].smplen = chn->smplen;
			if (chn->flags & ChannelMixer::MP_SAMPLE_ONESHOT)
				chn->timeRecord[nb].loopend = chn->loopendcopy;
			else
				chn->timeRecord[nb].loopend = chn->loopend;
			chn->timeRecord[nb].loopstart = chn->loopstart;
			chn->timeRecord[nb].fixedtime = chn->fixedtime;			
			chn->timeRecord[nb].fixedtimefrac = chn->fixedtimefrac;
		}
	}
}

void ChannelMixer::mix(mp_sint32* mixbuff32, mp_uint32 bufferSize)
{
	updateSampleCounter(bufferSize);
	
	if (!isPlaying())
		return;

	if (!paused)
	{
		mp_sint32* buffer = mixbuff32;
		
		mp_sint32 beatLength = beatPacketSize;
		mp_sint32 mixSize = mixBufferSize;

		mp_sint32 done = 0;

		if (lastBeatRemainder)
		{
			mp_sint32 todo = lastBeatRemainder;
			if (lastBeatRemainder > mixBufferSize)
			{
				todo = mixBufferSize;
				mp_uint32 pos = beatLength - lastBeatRemainder;
				//memcpy(buffer, mixbuffBeatPacket + pos*MP_NUMCHANNELS, todo*MP_NUMCHANNELS*sizeof(mp_sint32));				
				const mp_sint32* src = mixbuffBeatPacket + pos*MP_NUMCHANNELS;
				mp_sint32* dst = buffer;
				for (mp_sint32 i = 0; i < todo*MP_NUMCHANNELS; i++, src++, dst++)
					*dst += *src;
				done = mixBufferSize;
				lastBeatRemainder-=done;
			}
			else
			{
				mp_uint32 pos = beatLength - lastBeatRemainder;
				//memcpy(buffer, mixbuffBeatPacket + pos*MP_NUMCHANNELS, todo*MP_NUMCHANNELS*sizeof(mp_sint32));
				const mp_sint32* src = mixbuffBeatPacket + pos*MP_NUMCHANNELS;
				mp_sint32* dst = buffer;
				for (mp_sint32 i = 0; i < todo*MP_NUMCHANNELS; i++, src++, dst++)
					*dst += *src;
				buffer+=lastBeatRemainder*MP_NUMCHANNELS;
				mixSize-=lastBeatRemainder;
				done = lastBeatRemainder;
				lastBeatRemainder = 0;
			}
		}

		if (done < (mp_sint32)mixBufferSize)
		{
			const mp_sint32 numbeats = /*numBeatPackets*/mixSize / beatLength;

			done+=numbeats*beatLength;

			mp_sint32 nb;

			const bool isRamping = this->isRamping();

			for (nb=0;nb<numbeats;nb++) 
			{
				if (isRamping)
				{
					const TMixerChannel* src = channel;
					TMixerChannel* dst = newChannel;
					const mp_uint32 mixerNumActiveChannels = this->mixerNumActiveChannels;
					if (allowFilters)
					{
						// this is crucial for volume ramping, store current
						// active sample rate (stored in the step values for each channel)
						// and also filter coefficients	and last samples			
						for (mp_uint32 c = 0; c < mixerNumActiveChannels; c++, src++, dst++)
						{
							dst->smpadd = src->smpadd;
							dst->rsmpadd = src->rsmpadd; 

							dst->a = src->a; 
							dst->b = src->b; 
							dst->c = src->c; 
							dst->currsample = src->currsample;
							dst->prevsample = src->prevsample;
						}
					}
					else
					{
						// this is crucial for volume ramping, store current
						// active sample rate (stored in the step values for each channel)
						// and also filter coefficients	and last samples			
						for (mp_uint32 c = 0; c < mixerNumActiveChannels; c++, src++, dst++)
						{
							dst->smpadd = src->smpadd;
							dst->rsmpadd = src->rsmpadd; 
						}
					}
				}

				timer(nb);

				if (!disableMixing)
				{
					// do some in between state recording 
					// to be able to show smooth updates even if the buffer is large
					for (mp_uint32 c=0;c<mixerNumActiveChannels;c++) 
						storeTimeRecordData(nb, &channel[c]);

					mixBeatPacket(mixerNumActiveChannels, buffer+nb*beatLength*MP_NUMCHANNELS, nb, beatLength);	
				}
			}		

			buffer+=numbeats*beatLength*MP_NUMCHANNELS;

			if (done < (mp_sint32)mixBufferSize)
			{
				memset(mixbuffBeatPacket, 0, beatLength*MP_NUMCHANNELS*sizeof(mp_sint32));

				if (isRamping)
				{
					const TMixerChannel* src = channel;
					TMixerChannel* dst = newChannel;
					const mp_uint32 mixerNumActiveChannels = this->mixerNumActiveChannels;
					if (allowFilters)
					{
						// this is crucial for volume ramping, store current
						// active sample rate (stored in the step values for each channel)
						// and also filter coefficients	and last samples			
						for (mp_uint32 c = 0; c < mixerNumActiveChannels; c++, src++, dst++)
						{
							dst->smpadd = src->smpadd;
							dst->rsmpadd = src->rsmpadd; 

							dst->a = src->a; 
							dst->b = src->b; 
							dst->c = src->c; 
							dst->currsample = src->currsample;
							dst->prevsample = src->prevsample;
						}
					}
					else
					{
						// this is crucial for volume ramping, store current
						// active sample rate (stored in the step values for each channel)
						// and also filter coefficients	and last samples			
						for (mp_uint32 c = 0; c < mixerNumActiveChannels; c++, src++, dst++)
						{
							dst->smpadd = src->smpadd;
							dst->rsmpadd = src->rsmpadd; 
						}
					}
				}

				timer(numbeats);

				if (!disableMixing)
				{
					// do some in between state recording 
					// to be able to show smooth updates even if the buffer is large
					for (mp_uint32 c=0;c<mixerNumActiveChannels;c++) 
						storeTimeRecordData(nb, &channel[c]);

					mixBeatPacket(mixerNumActiveChannels, mixbuffBeatPacket, numbeats, beatLength);	
				}

				mp_sint32 todo = mixBufferSize - done;

				if (todo)
				{
					//memcpy(buffer, mixbuffBeatPacket, todo*MP_NUMCHANNELS*sizeof(mp_sint32));
					const mp_sint32* src = mixbuffBeatPacket;
					mp_sint32* dst = buffer;
					for (mp_sint32 i = 0; i < todo*MP_NUMCHANNELS; i++, src++, dst++)
						*dst += *src;
					lastBeatRemainder = beatLength - todo;
				}
			}
		}
	}
	
}

mp_sint32 ChannelMixer::initDevice()
{	
	resetChannelsWithoutMuting();

	initialized = true;

	return MP_OK;
}

void ChannelMixer::stop()
{
	// stop playing
	startPlay = false;
	
	paused = false;
}

mp_sint32 ChannelMixer::closeDevice()
{
	if (initialized)
	{
		paused = false; 
	
		// finish playing
		stop();

		resetChannelsWithoutMuting();
		initialized = false;
	}

	return MP_OK;
}

mp_sint32 ChannelMixer::pause()
{
	paused = true;
	return MP_OK;
}

mp_sint32 ChannelMixer::resume()
{
	paused = false;
	return MP_OK;
}

mp_sint32 ChannelMixer::adjustFrequency(mp_uint32 frequency)
{
	if (frequency == mixFrequency)
		return MP_OK;

	mp_sint32 err = MP_OK;
	if (initialized)
	{
		err = closeDevice();
	}

	// adjust sample counter to keep on the current time
	sampleCounter = (mp_sint32)(((double)sampleCounter/(double)mixFrequency)*(double)frequency);

	setFrequency(frequency);

	return err;
}

mp_sint32 ChannelMixer::beatPacketsToBufferSize(mp_uint32 mixFrequency, mp_uint32 numBeats)
{
	mp_uint32 beatPacketSize = (MP_BEATLENGTH*mixFrequency)/MP_BASEFREQ;
	return numBeats * beatPacketSize;
}

mp_sint32 ChannelMixer::setBufferSize(mp_uint32 bufferSize)
{
	if (this->mixBufferSize == bufferSize)
		return MP_OK;

	if (initialized)
	{
		mp_sint32 err = closeDevice();
		if (err != MP_OK)
			return err;
	}
	
	this->mixBufferSize = bufferSize;
	
	// channels contain information depending up the buffer size
	// update those too
	reallocChannels();
	
	return MP_OK;
}

mp_sint32 ChannelMixer::getNumActiveChannels()
{	
	mp_sint32 i = 0;

	for (mp_uint32 j = 0; j < mixerNumActiveChannels; j++)
		if (channel[j].flags & 256)
			i++;

	return i;
}

mp_sint32 ChannelMixer::getBeatIndexFromSamplePos(mp_uint32 smpPos) const
{
	mp_sint32 maxLen = (mixBufferSize/beatPacketSize)-1;
	if (maxLen < 0)
		maxLen = 0;

	mp_sint32 maxSize = maxLen*(mp_sint32)beatPacketSize - 1;
	if (maxSize < 0)
		maxSize = 0;

	if ((signed)smpPos < 0)
		smpPos = 0;

	if (smpPos > (unsigned)maxSize)
		smpPos = maxSize;

	return smpPos / getBeatPacketSize();
}

/*mp_sint32 ChannelMixer::getCurrentSample(mp_sint32 position,mp_sint32 channel)
{
	if (position < 0)
	{
		position = abs(position);
	}
	if (position > (mp_sint32)mixBufferSize-1)
	{
		position %= mixBufferSize*2;
		position -= mixBufferSize;
		position = mixBufferSize-1-position;
	}
	
	mp_sint32 val = (mp_sword)mixbuff32[position*MP_NUMCHANNELS+channel];
	if (val < -32768)
		val = -32768;
	if (val > 32767)
		val = 32767;

	return val;
}

mp_sint32 ChannelMixer::getCurrentSamplePeak(mp_sint32 position,mp_sint32 channel)
{
	if (audioDriver->supportsTimeQuery())
	{
		mp_sword peak = 0;
		
		for (mp_sint32 p = position-mixBufferSize; p <= position; p++)
		{
			mp_sword s = getCurrentSample(p, channel);
			if (s > peak)
				peak = s;
			if (-s > peak)
				peak = -s;				
		}
		return peak;
	}	
	else
	{
		mp_sword peak = 0;
		for (mp_uint32 pos = 0; pos < mixBufferSize; pos++)
		{
			mp_sint32 s = mixbuff32[pos*MP_NUMCHANNELS+channel];
			if (s < -32768)
				s = -32768;
			if (s > 32767)
				s = 32767;
			if (s > peak)
				peak = s;
			if (-s > peak)
				peak = -s;
		}	
		
		return peak;
	}
}*/

mp_uint32 ChannelMixer::getSyncSampleCounter()
{
	/*return audioDriver->getNumPlayedSamples();*/
	
	return 0;
}

#ifdef __MPTIMETRACKING__

#define FULLMIXER_8BIT_NORMAL_TEMP \
	if (sample) { \
		sd1 = ((mp_sbyte)sample[smppos])<<8; \
		sd2 = ((mp_sbyte)sample[smppos+1])<<8; \
		sd1 =((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; \
		buffer[i] = (sd1*vol)>>9; \
	} \
	else buffer[i] = 0; \
	i++;

#define FULLMIXER_16BIT_NORMAL_TEMP \
	if (sample) { \
		sd1 = ((mp_sword*)(sample))[smppos]; \
		sd2 = ((mp_sword*)(sample))[smppos+1]; \
		sd1 =((sd1<<12)+(smpposfrac>>4)*(sd2-sd1))>>12; \
		buffer[i] = (sd1*vol)>>9; \
	} \
	else buffer[i] = 0; \
	i++;

#include "ResamplerMacros.h"

/////////////////////////////////////////////////////////
//		SIMPLE MIXER, NO INTERPOLATION, NO RAMPING     //
/////////////////////////////////////////////////////////
class ResamplerDummy : public ChannelMixer::ResamplerBase
{
private:
	mp_sint32 vol;
public:
	ResamplerDummy(mp_sint32 vol) :
		vol(vol)
	{
	}

	virtual bool isRamping() { return false; }
	virtual bool supportsFullChecking() { return true; }
	virtual bool supportsNoChecking() { return false; }

	virtual void addBlockFull(mp_sint32* buffer, ChannelMixer::TMixerChannel* chn, mp_uint32 count)
	{
		mp_sint32 vol = this->vol;
		mp_sint32 i = 0;
		FULLMIXER_TEMPLATE(FULLMIXER_8BIT_NORMAL_TEMP,FULLMIXER_16BIT_NORMAL_TEMP, 16, 0);
	}
};


void ChannelMixer::mixData(mp_sint32 c, 
					mp_sint32* buffer, 
					mp_sint32 count, 
					mp_sint32 sampleShift,
					mp_sint32 fMul/* = 0*/, 
					mp_sint32 bufferIndex/* = -1*/, 
					mp_sint32 packetIndex/* = -1*/) const
{
	if (fMul == 0)
		fMul = count;
	if (packetIndex < 0)
		packetIndex = 0;
	
	const ChannelMixer::TMixerChannel* tempchn = &channel[c];
	ChannelMixer::TMixerChannel channel;
	
	channel.flags = tempchn->timeRecord[packetIndex].flags;
	channel.sample = tempchn->timeRecord[packetIndex].sample;
	channel.smppos = tempchn->timeRecord[packetIndex].smppos;
	channel.smpposfrac = tempchn->timeRecord[packetIndex].smpposfrac;
	channel.smpadd = tempchn->timeRecord[packetIndex].smpadd;
	channel.smplen = tempchn->timeRecord[packetIndex].smplen;
	channel.loopend = channel.loopendcopy = tempchn->timeRecord[packetIndex].loopend;
	channel.loopstart = tempchn->timeRecord[packetIndex].loopstart;
	channel.vol = tempchn->timeRecord[packetIndex].volPan & 0xFFFF;
	channel.pan = tempchn->timeRecord[packetIndex].volPan >> 16;
	
	ChannelMixer::TMixerChannel* chn = &channel;
	
	if (startPlay && (chn->flags & MP_SAMPLE_PLAY))
	{
		chn->smpadd = (chn->smpadd*fMul) / (count ? count : 1);
		mp_sint32 vol = (chn->vol*masterVolume) >> (8 + sampleShift);	
		
		ResamplerDummy resampler(vol);
		resampler.addBlockFull(buffer, chn, count);
	}
	else
	{
		memset(buffer, 0, sizeof(mp_sint32)*count);
	}
}

#endif

