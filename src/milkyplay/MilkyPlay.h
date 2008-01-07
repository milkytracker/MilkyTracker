/*
 *  milkyplay/MilkyPlay.h
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
 *  MilkyPlay.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Thu Jan 20 2005.
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
