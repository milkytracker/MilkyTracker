/*
 *  milkyplay/ResamplerFactory.cpp
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
 *  Resampler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.11.07.
 *
 */

#include "ResamplerFactory.h"
#include "ResamplerCubic.h"
#include "ResamplerFast.h"
#include "ResamplerSinc.h"

ChannelMixer::ResamplerBase* ResamplerFactory::createResampler(ResamplerTypes type)
{
	switch (type)
	{
		case MIXER_NORMAL:
			return new ResamplerSimple();
			
		case MIXER_NORMAL_RAMPING:
			return new ResamplerSimpleRamp();

		case MIXER_LERPING:
			return new ResamplerLerp();

		case MIXER_LERPING_RAMPING:
			return new ResamplerLerpRampFilter();

		case MIXER_LAGRANGE:
			return new ResamplerLagrange<false, CubicResamplerLagrange>();

		case MIXER_LAGRANGE_RAMPING:
			return new ResamplerLagrange<true, CubicResamplerLagrange>();

		case MIXER_SPLINE:
			return new ResamplerLagrange<false, CubicResamplerSpline>();

		case MIXER_SPLINE_RAMPING:
			return new ResamplerLagrange<true, CubicResamplerSpline>();

		case MIXER_SINCTABLE:
			return new ResamplerSincTable<false, 16>();

		case MIXER_SINCTABLE_RAMPING:
			return new ResamplerSincTable<true, 16>();

		case MIXER_SINC:
			return new ResamplerSinc<false, 128>();

		case MIXER_SINC_RAMPING:
			return new ResamplerSinc<true, 128>();

		case MIXER_DUMMY:
			return new ResamplerDummy();
	}
	
	return NULL;
}
