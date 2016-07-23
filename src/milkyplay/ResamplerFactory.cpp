/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  Resampler.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 08.11.07.
 *
 */

#include "ResamplerFactory.h"
#include "ResamplerCubic.h"
#include "ResamplerFast.h"
#include "ResamplerSinc.h"
#include "ResamplerAmiga.h"

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

		case MIXER_AMIGA500:
			return new ResamplerAmiga<0>();

		case MIXER_AMIGA500_RAMPING:
			return new ResamplerAmiga<0>();

		case MIXER_AMIGA500LED:
			return new ResamplerAmiga<1>();

		case MIXER_AMIGA500LED_RAMPING:
			return new ResamplerAmiga<1>();

		case MIXER_AMIGA1200:
			return new ResamplerAmiga<2>();

		case MIXER_AMIGA1200_RAMPING:
			return new ResamplerAmiga<2>();

		case MIXER_AMIGA1200LED:
			return new ResamplerAmiga<3>();

		case MIXER_AMIGA1200LED_RAMPING:
			return new ResamplerAmiga<3>();

		/*	There is also ResamplerAmiga<5> which is a generic 22khz LP filter, it still
			emulates Paula's pulse chain but does not emulate any of the Amiga's internal
			filter circuitry. Already we have many options here so it's probably not worth
			including. */

		case MIXER_DUMMY:
			return new ResamplerDummy();
		case MIXER_INVALID:
			return NULL;
	}
}
