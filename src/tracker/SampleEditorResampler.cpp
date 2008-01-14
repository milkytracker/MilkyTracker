/*
 *  tracker/SampleEditorResampler.cpp
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
 *  SampleEditorResampler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 06.01.08.
 *
 */

#include "SampleEditorResampler.h"
#include "XModule.h"
#include "ChannelMixer.h"
#include "ResamplerFactory.h"
#include <math.h>

SampleEditorResampler::SampleEditorResampler(XModule& module, TXMSample& sample, ResamplerTypes type) :
	module(module),
	sample(sample),
	type(type)
{
}

SampleEditorResampler::~SampleEditorResampler()
{
}

// we're going to abuse the resampler of the ChannelMixer class
// Problem here is, we need to build up some temporary channel structure 
// PLUS the resampler only deals with stereo channels, so basically we're 
// resampling stereo data (left channel = full, right channel = empty)
bool SampleEditorResampler::resample(float oldRate, float newRate)
{
	float factor = oldRate / newRate;

	mp_ubyte* buffer = TXMSample::allocPaddedMem(sample.samplen * ((sample.type & 16) ? 2 : 1));

	if (buffer == NULL)
		return false;

	// retrieve original sample without loop modifications
	if (sample.type & 16)
	{
		mp_sword* bu = (mp_sword*)buffer;

		for (mp_uint32 i = 0; i < sample.samplen; i++)
			bu[i] = sample.getSampleValue(i);
		
		bu[-1]=bu[0];
		bu[-2]=bu[0];
		bu[-3]=bu[0];
		bu[-4]=bu[0];
		
		bu[sample.samplen]=bu[sample.samplen-1];
		bu[sample.samplen+1]=bu[sample.samplen-1];
		bu[sample.samplen+3]=bu[sample.samplen-1];
		bu[sample.samplen+4]=bu[sample.samplen-1];
	}
	else
	{
		mp_sbyte* bu = (mp_sbyte*)buffer;

		for (mp_uint32 i = 0; i < sample.samplen; i++)
			bu[i] = sample.getSampleValue(i);

		bu[-1]=bu[0];
		bu[-2]=bu[0];
		bu[-3]=bu[0];
		bu[-4]=bu[0];
		
		bu[sample.samplen]=bu[sample.samplen-1];
		bu[sample.samplen+1]=bu[sample.samplen-1];
		bu[sample.samplen+3]=bu[sample.samplen-1];
		bu[sample.samplen+4]=bu[sample.samplen-1];
	}
	
	// get space for resampled data*2 as the resampler only processes stereo samples
	mp_sint32 finalSize = (mp_sint32)ceil(sample.samplen/factor);
	
	mp_sint32* dst = new mp_sint32[(finalSize+1)*2];
	
	if (dst == NULL)
	{
		TXMSample::freePaddedMem(buffer);
		return false;
	}
	
	memset(dst, 0, sizeof(mp_sint32)*(finalSize+1)*2);

	ChannelMixer::TMixerChannel channel;	
	

	channel.sample = (mp_sbyte*)buffer;
	channel.smplen = sample.samplen;
	channel.flags = ChannelMixer::MP_SAMPLE_PLAY | ((sample.type & 16) ? 4 : 0);
	channel.smppos = 0;
	channel.smpposfrac = 0;
	channel.fixedtimefrac = 0;
	channel.smpadd = (mp_sint32)(factor*65536.0f);
	channel.loopend = channel.smplen;
	channel.loopstart = 0;
	channel.vol = 512;
	channel.pan = 128;
	channel.cutoff = ChannelMixer::MP_INVALID_VALUE;
	channel.resonance = ChannelMixer::MP_INVALID_VALUE;
	
	channel.rsmpadd = (mp_sint32)((1.0f / factor) * 65536.0f);
	
	// we only need the left channel as no panning is involved
	channel.finalvoll = (channel.vol*128*256)<<6; 
	channel.finalvolr = 0;
	channel.rampFromVolStepL = channel.rampFromVolStepR = 0;	
	channel.index = 0;
	
	ChannelMixer::ResamplerBase* resampler = NULL;
	
	switch (type)
	{
		case ResamplerTypeNone:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_NORMAL);
			break;
		case ResamplerTypeLinear:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_LERPING);
			break;
		case ResamplerTypeLagrange:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_LAGRANGE);
			break;
		case ResamplerTypeSpline:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_SPLINE);
			break;
		case ResamplerTypeFastSinc:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_SINCTABLE);
			break;
		case ResamplerTypePreciseSinc:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_SINC);
			break;
		case ResamplerTypeAmiga500:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_AMIGA500);
			break;
		case ResamplerTypeAmiga500LED:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_AMIGA500LED);
			break;
		case ResamplerTypeAmiga1200:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_AMIGA1200);
			break;
		case ResamplerTypeAmiga1200LED:
			resampler = ResamplerFactory::createResampler(ResamplerFactory::MIXER_AMIGA1200LED);
			break;
	}

	if (resampler == NULL)
	{
		TXMSample::freePaddedMem(buffer);
		delete[] dst;
		return false;
	}
	
	resampler->setNumChannels(1);
	resampler->setFrequency((mp_sint32)newRate);
	resampler->addChannel(&channel, dst, finalSize+1, 1);
					
	delete resampler;
	TXMSample::freePaddedMem(buffer);

	module.freeSampleMem((mp_ubyte*)sample.sample);

	sample.sample = (mp_sbyte*)module.allocSampleMem((sample.type & 16) ? finalSize*2 : finalSize);
	
	for (mp_sint32 i = 0; i < finalSize; i++)
	{
		mp_sint32 s = dst[i*2];
		if (s > 32767) s = 32767;
		if (s < -32768) s = -32768;
		if (sample.type & 16)
			sample.setSampleValue(i, s);
		else
			sample.setSampleValue(i, s >> 8);
	}
	
	sample.samplen = finalSize;
	
	delete[] dst;
	return true;
}
