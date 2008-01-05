/*
 *  SongLengthEstimator.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#include "SongLengthEstimator.h"
#include "MilkyPlay.h"
#include "AudioDriver_NULL.h"

SongLengthEstimator::SongLengthEstimator(XModule* theModule) :
	player(NULL),
	module(theModule)
{
}

SongLengthEstimator::SongLengthEstimator(const SongLengthEstimator& src) :
	player(NULL),
	module(src.module)
{
}

SongLengthEstimator::~SongLengthEstimator() 
{
	delete player;
}

const SongLengthEstimator& SongLengthEstimator::operator=(const SongLengthEstimator& src)
{
	if (&src != this)
	{
		module = src.module;
	}
	return *this;
}

mp_sint32 SongLengthEstimator::estimateSongLengthInSeconds()
{
	if (!player)
	{
		player = new PlayerGeneric(44100);
		
		if (!player)
			return -1;
		
		player->setBufferSize(1024);
		player->setResamplerType((ChannelMixer::ResamplerTypes)0);
		player->setMasterVolume(256);
		player->setDisableMixing(true);
	}
	
	AudioDriver_NULL* audioDriver = new AudioDriver_NULL;
	mp_sint32 res = player->exportToWAV(NULL, module, 0, -1, NULL, module->header.channum, NULL, audioDriver) / player->getMixFrequency();
	delete audioDriver;

	return res;
}

