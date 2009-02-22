/*
 *  tracker/ResamplerHelper.h
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
 *  ResamplerHelper.h
 *  milkytracker
 *
 *  Created by Peter Barth on 15.01.08.
 *
 */

#ifndef __RESAMPLERHELPER_H__
#define __RESAMPLERHELPER_H__

#include "BasicTypes.h"
#include "ResamplerFactory.h"

class ResamplerHelper
{
private:
	static const char* resamplerNames[];
	static const char* resamplerNamesShort[];
	
public:
	pp_uint32 getNumResamplers();
	
	const char* getResamplerName(pp_uint32 index, bool shortName = false);
	
	ChannelMixer::ResamplerBase* createResamplerFromIndex(pp_uint32 index);
	
	ChannelMixer::ResamplerTypes getResamplerType(pp_uint32 index, bool ramping);
};

#endif
