/*
 *  milkyplay/SampleLoaderAbstract.cpp
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
 *  SampleLoaderAbstract.cpp
 MilkyPlay
 *
 *  Created by Peter Barth on 14.09.05.
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

