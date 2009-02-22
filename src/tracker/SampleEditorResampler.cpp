/*
 *  tracker/SampleEditorResampler.cpp
 *
 *  Copyright 2009 Peter Barth
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
#include "ResamplerHelper.h"
#include <math.h>

SampleEditorResampler::SampleEditorResampler(XModule& module, TXMSample& sample, pp_uint32 type) :
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
	
	ResamplerHelper resamplerHelper;
	ChannelMixer::ResamplerBase* resampler = resamplerHelper.createResamplerFromIndex(type);

	if (resampler == NULL)
	{
		TXMSample::freePaddedMem(buffer);
		delete[] dst;
		return false;
	}
	
	resampler->setNumChannels(1);
	resampler->setFrequency((mp_sint32)newRate);
	
	// the resampler is used to process small blocks
	// so we feed in small blocks as well
	mp_sint32 numBlocks = (finalSize+1) >> 6;
	mp_sint32 lastBlock = (finalSize+1) & 63;
	
	mp_sint32* dsttmp = dst;
	for (mp_sint32 k = 0; k < numBlocks; k++)
	{
		resampler->addChannel(&channel, dsttmp, 64, 1);
		dsttmp+=64*2;
	}
	if (lastBlock)
		resampler->addChannel(&channel, dsttmp, lastBlock, 1);
					
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
