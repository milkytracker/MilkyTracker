/*
 *  milkyplay/SampleLoaderAIFF.h
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
 *  SampleLoaderAIFF.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 07.01.06.
 *
 */

#ifndef SAMPLELOADERAIFF__H
#define SAMPLELOADERAIFF__H

#include "SampleLoaderAbstract.h"

class SampleLoaderAIFF : public SampleLoaderAbstract
{
private:
	 static const char* channelNames[];

public:
	SampleLoaderAIFF(const SYSCHAR* fileName, XModule& theModule);

	virtual bool identifySample();

	virtual mp_sint32 getNumChannels();
	
	virtual const char* getChannelName(mp_sint32 channelIndex);

	virtual mp_sint32 loadSample(mp_sint32 index, mp_sint32 channelIndex);

	virtual mp_sint32 saveSample(const SYSCHAR* fileName, mp_sint32);
};

#endif
