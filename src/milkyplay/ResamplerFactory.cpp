/*
 *  Resampler.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.11.07.
 *  Copyright 2007 milkytracker.net. All rights reserved.
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
