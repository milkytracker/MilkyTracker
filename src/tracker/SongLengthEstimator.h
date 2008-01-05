/*
 *  SongLengthEstimator.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.11.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef SONGLENGTHESTIMATOR__H
#define SONGLENGTHESTIMATOR__H

#include "MilkyPlayTypes.h"

class PlayerGeneric;
class XModule;

class SongLengthEstimator
{
private:
	PlayerGeneric* player;
	XModule* module;
	
public:
	SongLengthEstimator(XModule* theModule);
	SongLengthEstimator(const SongLengthEstimator& src);
	~SongLengthEstimator();
	
	const SongLengthEstimator& operator=(const SongLengthEstimator& src);
	
	mp_sint32 estimateSongLengthInSeconds();
};

#endif
