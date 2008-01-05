/*
 *  SampleLoaderGeneric.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.09.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SAMPLELOADERGENERIC__H
#define SAMPLELOADERGENERIC__H

#include "SampleLoaderAbstract.h"

class SampleLoaderGeneric : public SampleLoaderAbstract
{
public:
	enum OutputFiletypes
	{
		OutputFiletypeWAV,
		OutputFiletypeIFF,
		OutputFiletypeAIFF,
		OutputFiletypeRAW
	};

	SampleLoaderGeneric(const SYSCHAR* fileName, XModule& module);

	virtual bool identifySample();

	virtual mp_sint32 getNumChannels();
	
	virtual const char* getChannelName(mp_sint32 channelIndex);

	virtual mp_sint32 loadSample(mp_sint32 index, mp_sint32 channelIndex);

	mp_sint32 saveSample(const SYSCHAR* fileName, mp_sint32 index, OutputFiletypes type);

private:
	SampleLoaderAbstract* getSuitableLoader();
}; 

#endif

