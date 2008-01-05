/*
 *  SampleLoaderALL.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.09.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SAMPLELOADERALL__H
#define SAMPLELOADERALL__H

#include "SampleLoaderAbstract.h"

class SampleLoaderALL : public SampleLoaderAbstract
{
private:
	 static const char* channelNames[];

public:
	SampleLoaderALL(const SYSCHAR* fileName, XModule& theModule);

	virtual bool identifySample();

	virtual mp_sint32 getNumChannels();
	
	virtual mp_sint32 loadSample(mp_sint32 index, mp_sint32 channelIndex);

	virtual mp_sint32 saveSample(const SYSCHAR* fileName, mp_sint32);
};

#endif

