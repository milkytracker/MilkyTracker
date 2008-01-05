/*
 *  SampleLoaderGeneric.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.09.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "SampleLoaderGeneric.h"
#include "SampleLoaderWAV.h"
#include "SampleLoaderIFF.h"
#include "SampleLoaderAIFF.h"
#include "SampleLoaderALL.h"

SampleLoaderGeneric::SampleLoaderGeneric(const SYSCHAR* fileName, XModule& module) :
	SampleLoaderAbstract(fileName, module)
{
}

mp_sint32 SampleLoaderGeneric::getNumChannels()
{
	SampleLoaderAbstract* loader = getSuitableLoader();
	
	if (loader)
	{
		mp_sint32 res = loader->getNumChannels();
		delete loader;
		return res;
	}
	
	return 0;
}
	
const char* SampleLoaderGeneric::getChannelName(mp_sint32 channelIndex)
{
	SampleLoaderAbstract* loader = getSuitableLoader();
	
	if (loader)
	{
		const char* res = loader->getChannelName(channelIndex);
		delete loader;
		return res;
	}
	
	return SampleLoaderAbstract::getChannelName(channelIndex);
}

bool SampleLoaderGeneric::identifySample()
{
	SampleLoaderAbstract* loader = getSuitableLoader();
	
	if (loader)
	{
		delete loader;
		return true;
	}
	
	return false;
}

mp_sint32 SampleLoaderGeneric::loadSample(mp_sint32 index, mp_sint32 channelIndex)
{
	SampleLoaderAbstract* loader = getSuitableLoader();

	loader->setPreferredDefaultName(this->preferredDefaultName);

	if (loader)
	{
		mp_sint32 res = loader->loadSample(index, channelIndex);
		delete loader;
		return res;
	}
	
	return -9999;
}

mp_sint32 SampleLoaderGeneric::saveSample(const SYSCHAR* fileName, mp_sint32 index, OutputFiletypes type)
{
	SampleLoaderAbstract* loader = NULL;	

	switch (type)
	{
		case OutputFiletypeWAV:
			loader = new SampleLoaderWAV(theFileName, theModule);
			break;

		case OutputFiletypeIFF:
			loader = new SampleLoaderIFF(theFileName, theModule);
			break;

		case OutputFiletypeAIFF:
			loader = new SampleLoaderAIFF(theFileName, theModule);
			break;

		case OutputFiletypeRAW:
			loader = new SampleLoaderALL(theFileName, theModule);
			break;
	}
	
	if (loader)
	{
		mp_sint32 res = loader->saveSample(fileName, index);
		delete loader;
		return res;
	}
	
	return -9999;
}

SampleLoaderAbstract* SampleLoaderGeneric::getSuitableLoader()
{
	// Try to find WAV first
	SampleLoaderAbstract* loader = new SampleLoaderWAV(theFileName, theModule);
	if (loader && loader->identifySample())
		return loader;
	
	delete loader;

	// Try to find IFF then
	loader = new SampleLoaderIFF(theFileName, theModule);
	if (loader && loader->identifySample())
		return loader;
	
	delete loader;

	// Try to find AIFF then
	loader = new SampleLoaderAIFF(theFileName, theModule);
	if (loader && loader->identifySample())
		return loader;
	
	delete loader;
	
	// Try to find something else
	loader = new SampleLoaderALL(theFileName, theModule);
	if (loader && loader->identifySample())
		return loader;

	delete loader;
	return NULL;
}
