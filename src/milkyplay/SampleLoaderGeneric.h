/*
 *  milkyplay/SampleLoaderGeneric.h
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
 *  SampleLoaderGeneric.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 16.09.05.
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

