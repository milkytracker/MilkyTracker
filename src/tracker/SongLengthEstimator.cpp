/*
 *  tracker/SongLengthEstimator.cpp
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
 *  SongLengthEstimator.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.11.05.
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

