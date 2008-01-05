/*
 *  SampleLoaderAbstract.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.09.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SAMPLELOADERABSTRACT__H
#define SAMPLELOADERABSTRACT__H

#include "XMFile.h"

class XModule;
struct TXMSample;

class SampleLoaderAbstract
{
private:
	 static const char* emptyChannelName;
	
protected:
	XModule& theModule;
	const SYSCHAR* theFileName;
	const char* preferredDefaultName;
	
	void nameToSample(const char* name, TXMSample* smp); 
	
public:
	SampleLoaderAbstract(const SYSCHAR* fileName, XModule& module);

	virtual ~SampleLoaderAbstract() {}

	virtual bool identifySample() = 0;
	
	virtual mp_sint32 getNumChannels() { return 1; }
	
	virtual const char* getChannelName(mp_sint32 channelIndex) { return emptyChannelName; }

	virtual mp_sint32 loadSample(mp_sint32 index, mp_sint32 channelIndex) = 0;

	virtual mp_sint32 saveSample(const SYSCHAR* fileName, mp_sint32) { return 0; }
	
	void setPreferredDefaultName(const char* preferredDefaultName) { this->preferredDefaultName = preferredDefaultName; }
};

#endif

