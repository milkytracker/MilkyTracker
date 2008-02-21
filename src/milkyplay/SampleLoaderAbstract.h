/*
 *  milkyplay/SampleLoaderAbstract.h
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
 *  SampleLoaderAbstract.h
 MilkyPlay
 *
 *  Created by Peter Barth on 14.09.05.
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

