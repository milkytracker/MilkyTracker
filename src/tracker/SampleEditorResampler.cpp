/*
 *  SampleEditorResampler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 06.01.08.
 *  Copyright 2008 milkytracker.net. All rights reserved.
 *
 */

#include "SampleEditorResampler.h"
#include "XModule.h"
#include "ChannelMixer.h"
#include "ResamplerFactory.h"

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
bool SampleEditorResampler::resample(float factor)
{
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
	channel.flags = MP_SAMPLE_PLAY | ((sample.type & 16) ? 4 : 0);
	channel.smppos = 0;
	channel.smpposfrac = 0;
	channel.fixedtimefrac = 0;
	channel.smpadd = (mp_sint32)(factor*65536.0f);
	channel.loopend = channel.smplen;
	channel.loopstart = 0;
	channel.vol = 512;
	channel.pan = 128;
	channel.cutoff = MP_INVALID_VALUE;
	channel.resonance = MP_INVALID_VALUE;
	
	channel.rsmpadd = (mp_sint32)((1.0f / factor) * 65536.0f);
	
	// we only need the left channel as no panning is involved
	channel.finalvoll = (channel.vol*128*256)<<6; 
	channel.finalvolr = 0;
	channel.rampFromVolStepL = channel.rampFromVolStepR = 0;		
	
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
	}

	if (resampler == NULL)
	{
		TXMSample::freePaddedMem(buffer);
		delete[] dst;
		return false;
	}
					
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
