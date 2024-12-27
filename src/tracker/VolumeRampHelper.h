/*
 *  tracker/VolumeRampHelper.h
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
 *  VolumeRampHelper.h
 *  milkytracker
 *
 *  Created by Peter Barth on 15.01.08.
 *
 */

#ifndef __RAMPHELPER_H__
#define __RAMPHELPER_H__

#include "BasicTypes.h"
#include "ChannelMixer.h"

class VolumeRampHelper
{
private:
	static const char* rampNames[];
	static const char* rampNamesShort[];
	
public:
	pp_uint32 getNumVolumeRamps();
	
	const char* getVolumeRampName(pp_uint32 index, bool shortName = false);
	
	ChannelMixer::RampTypes getVolumeRampType(pp_uint32 index);
};

#endif
