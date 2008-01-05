/*
 *  SampleLoaderAbstract.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.09.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "SampleLoaderAbstract.h"
#include "XModule.h"

const char* SampleLoaderAbstract::emptyChannelName = "";

SampleLoaderAbstract::SampleLoaderAbstract(const SYSCHAR* fileName, XModule& module) :
	theModule(module),
	theFileName(fileName),
	preferredDefaultName(emptyChannelName)
{
}

void SampleLoaderAbstract::nameToSample(const char* name, TXMSample* smp)
{
	memset(smp->name, 0, sizeof(smp->name));
	
	if (strlen(name) <= sizeof(smp->name))
	{
		memcpy(smp->name, name, strlen(name));
	}
	else
	{
		memcpy(smp->name, name, sizeof(smp->name));
	}
}

