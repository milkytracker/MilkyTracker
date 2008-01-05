/*
 *  SampleLoaderALL.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.09.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "SampleLoaderALL.h"
#include "XMFile.h"
#include "XModule.h"

SampleLoaderALL::SampleLoaderALL(const SYSCHAR* fileName, XModule& theModule) :
	SampleLoaderAbstract(fileName, theModule)
{
}

bool SampleLoaderALL::identifySample()
{
	return getNumChannels() != 0;
}

mp_sint32 SampleLoaderALL::getNumChannels()
{
	return 1;
}
	
mp_sint32 SampleLoaderALL::loadSample(mp_sint32 index, mp_sint32 channelIndex)
{
	XMFile f(theFileName);
				
	TXMSample* smp = &theModule.smp[index];

	if (smp->sample)
	{
		theModule.freeSampleMem((mp_ubyte*)smp->sample);
		smp->sample = NULL;
	}
	
	smp->samplen = f.size();
	
	// Bigger than 100mb? Probably someone's kidding here...
	if ((unsigned)smp->samplen > 100*1024*1024)
		smp->samplen = 100*1024*1024;
	
	smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen);
	
	if (smp->sample == NULL)
		return -7;

	smp->type = 0;
	smp->flags = 3;
	smp->looplen = smp->loopstart = 0;

	nameToSample(preferredDefaultName, smp);
	
	// assuming signed data
	theModule.loadSample(f, smp->sample, smp->samplen, smp->samplen);
	return 0;
}

mp_sint32 SampleLoaderALL::saveSample(const SYSCHAR* fileName, mp_sint32 index)
{
	TXMSample* smp = &theModule.smp[index];

	XMFile f(fileName, true);

	if (smp->type & 16)
	{
		for (mp_uint32 i = 0; i < smp->samplen; i++)
			f.writeWord(smp->getSampleValue(i));
	}
	else
	{
		for (mp_uint32 i = 0; i < smp->samplen; i++)
			f.writeByte(smp->getSampleValue(i));
	}
	return 0;
}
