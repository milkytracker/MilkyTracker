/*
 *  SampleLoaderIFF.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 07.01.06.
 *  Copyright 2006 milkytracker.net, All rights reserved.
 *
 */

#ifndef SAMPLELOADERIFF__H
#define SAMPLELOADERIFF__H

#include "SampleLoaderAbstract.h"

class SampleLoaderIFF : public SampleLoaderAbstract
{
/*private:
	 static const char* channelNames[];*/

public:
	SampleLoaderIFF(const SYSCHAR* fileName, XModule& theModule);

	virtual bool identifySample();

	virtual mp_sint32 getNumChannels();
	
	//virtual const char* getChannelName(mp_sint32 channelIndex);

	virtual mp_sint32 loadSample(mp_sint32 index, mp_sint32 channelIndex);

	virtual mp_sint32 saveSample(const SYSCHAR* fileName, mp_sint32);
};

#endif
