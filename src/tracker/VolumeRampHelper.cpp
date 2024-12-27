/*
 *  tracker/VolumeRampHelper.cpp
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
 *  VolumeRampHelper.cpp
 *  milkytracker
 *
 *  Created by Peter Barth on 15.01.08.
 *
 */

#include "VolumeRampHelper.h"

const char* VolumeRampHelper::rampNames[] =
{
	"None",
	"Sample End (keep transients)",
	"Sample End+Start (FT2)"
	/*FUTURE: "Custom volume ramping"*/
};

const char* VolumeRampHelper::rampNamesShort[] =
{
	"None",
	"Out",
	"FT2",
	/*FUTURE: "CUSTOM"*/
};

pp_uint32 VolumeRampHelper::getNumVolumeRamps()
{
	return sizeof(rampNames) / sizeof(const char*);
}
	
const char* VolumeRampHelper::getVolumeRampName(pp_uint32 index, bool shortName/* = false*/)
{
	if (index >= getNumVolumeRamps())
		return NULL;
	
	return shortName ? rampNamesShort[index] : rampNames[index];	
}

ChannelMixer::RampTypes VolumeRampHelper::getVolumeRampType(pp_uint32 index)
{
	return (ChannelMixer::RampTypes)index; 
}

