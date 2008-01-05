/*
 *  MilkyPlay.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Thu Jan 20 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */
#ifndef __MILKYPLAY_H__
#define __MILKYPLAY_H__

#include "AudioDriverManager.h"
#include "MasterMixer.h"
#include "PlayerSTD.h"
#include "PlayerGeneric.h"
#include "XModule.h"

#ifdef MILKYTRACKER
	#include "XIInstrument.h"
	#include "SampleLoaderAbstract.h"
	#include "SampleLoaderGeneric.h"
#else
	#include "PlayerFAR.h"
	#include "PlayerIT.h"
#endif

#define MILKYPLAY_HIVER (mp_uint32)((0<<16) + (0))
#define MILKYPLAY_LOVER (mp_uint32)((2<<16) + (0))

#endif
