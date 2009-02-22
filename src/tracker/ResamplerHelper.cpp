/*
 *  tracker/ResamplerHelper.cpp
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
 *  ResamplerHelper.cpp
 *  milkytracker
 *
 *  Created by Peter Barth on 15.01.08.
 *
 */

#include "ResamplerHelper.h"

const char* ResamplerHelper::resamplerNames[] =
{
	"No interpolation",
	"Linear interpolation",
	"Cubic Lagrange",
	"Cubic Spline",
	"Fast Sinc",
	"Precise Sinc",
	"Amiga 500",
	"Amiga 500 LED",
	"Amiga 1200",
	"Amiga 1200 LED"
};

const char* ResamplerHelper::resamplerNamesShort[] =
{
	"None",
	"Linear",
	"Lagrange",
	"Spline",
	"Fast Sinc",
	"Precise Sinc",
	"A500",
	"A500LED",
	"A1200",
	"A1200LED"
};

pp_uint32 ResamplerHelper::getNumResamplers()
{
	return sizeof(resamplerNames) / sizeof(const char*);
}
	
const char* ResamplerHelper::getResamplerName(pp_uint32 index, bool shortName/* = false*/)
{
	if (index >= getNumResamplers())
		return NULL;
	
	return shortName ? resamplerNamesShort[index] : resamplerNames[index];	
}

ChannelMixer::ResamplerBase* ResamplerHelper::createResamplerFromIndex(pp_uint32 index)
{
	return ResamplerFactory::createResampler((MixerSettings::ResamplerTypes)(index << 1));
}

ChannelMixer::ResamplerTypes ResamplerHelper::getResamplerType(pp_uint32 index, bool ramping)
{
	return (ChannelMixer::ResamplerTypes)((index << 1) | (ramping ? 1 : 0));
}

