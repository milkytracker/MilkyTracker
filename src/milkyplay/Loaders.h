/*
 *  milkyplay/Loaders.h
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
 *  Loaders.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on Tue Oct 19 2004.
 *
 */
#ifndef __LOADERS_H__
#define __LOADERS_H__

#include "XModule.h"
#include "LittleEndian.h"

// i'm a lazy guy
#define DECLARE_LOADERCLASS(LOADERNAME) \
	class LOADERNAME : public XModule::LoaderInterface \
	{ \
	public: \
		const char*	identifyModule(const mp_ubyte* buffer); \
		mp_sint32   load(XMFileBase& f, XModule* module); \
	}; \

DECLARE_LOADERCLASS(Loader669)		// 669 Composer (Uses special FAR Player)
DECLARE_LOADERCLASS(LoaderAMF_1)	// Asylum Music Format 1.0
DECLARE_LOADERCLASS(LoaderAMF_2)	// Digital Sound And Music Interface (DMP module format)
DECLARE_LOADERCLASS(LoaderAMSv1)	// Extreme Tracker AMS
DECLARE_LOADERCLASS(LoaderAMSv2)	// Velvet Studio AMS 
DECLARE_LOADERCLASS(LoaderDBM)		// Digibooster Pro
DECLARE_LOADERCLASS(LoaderCBA)		// Chuck Biscuits / Black Artist special format (only one Music Disk)
DECLARE_LOADERCLASS(LoaderDIGI)		// Digibooster 1.x
DECLARE_LOADERCLASS(LoaderDSMv1)	// old digisound interface kit
DECLARE_LOADERCLASS(LoaderDSMv2)	// new digisound interface kit
DECLARE_LOADERCLASS(LoaderDSm)		// dynamic studio
DECLARE_LOADERCLASS(LoaderDTM_1)	// Digitrekker 3.0 (NOT DigiTrakker!!!)
DECLARE_LOADERCLASS(LoaderDTM_2)	// Digital Tracker
DECLARE_LOADERCLASS(LoaderFAR)		// Farandole Composer (Uses special Player)
//DECLARE_LOADERCLASS(LoaderFNK)
DECLARE_LOADERCLASS(LoaderGDM)		// General Digimusic
DECLARE_LOADERCLASS(LoaderGMC)		// Game Music Creator (very similiar to SoundTracker, 15 ins, kept in LoaderMOD.cpp)
DECLARE_LOADERCLASS(LoaderIMF)		// Imago Orpheus
DECLARE_LOADERCLASS(LoaderIT)		// Impulse Tracker
DECLARE_LOADERCLASS(LoaderMDL)		// Digitrakker 2.0/3.0
DECLARE_LOADERCLASS(LoaderMOD)		// Protracker and compatible (1..32 channels)
DECLARE_LOADERCLASS(LoaderMTM)		// MultiTracker
DECLARE_LOADERCLASS(LoaderMXM)		// Cubic Tiny XM
DECLARE_LOADERCLASS(LoaderOKT)		// Oktalyzer
DECLARE_LOADERCLASS(LoaderPLM)		// DisorderTracker II
DECLARE_LOADERCLASS(LoaderPSMv1)	// old Epic MegaGames MASI (Epic Pinball, ...)
DECLARE_LOADERCLASS(LoaderPSMv2)	// new Epic MegaGames MASI (Jazz Jack Rabbit, ...)
DECLARE_LOADERCLASS(LoaderPTM)		// PolyTracker
DECLARE_LOADERCLASS(LoaderS3M)		// ScreamTracker III
DECLARE_LOADERCLASS(LoaderSTM)		// ScreamTracker II
DECLARE_LOADERCLASS(LoaderSFX)		// SoundFX module (very similiar to SoundTracker, 15 ins, kept in LoaderMOD.cpp) 
DECLARE_LOADERCLASS(LoaderULT)		// Ultratracker
DECLARE_LOADERCLASS(LoaderUNI)		// MikMod internal module
DECLARE_LOADERCLASS(LoaderXM)		// FT2 Extended Module

#endif
