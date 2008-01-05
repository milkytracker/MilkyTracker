/*
 *  ResamplerFactory.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.11.07.
 *  Copyright 2007 milkytracker.net. All rights reserved.
 *
 */

#ifndef __RESAMPLERFACTORY_H__
#define __RESAMPLERFACTORY_H__

#include "ChannelMixer.h"

class ResamplerFactory : public MixerSettings
{
public:
	static ChannelMixer::ResamplerBase* createResampler(ResamplerTypes type);
};

#endif
