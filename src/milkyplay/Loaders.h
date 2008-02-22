/*
 *  milkyplay/Loaders.h
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
#define DEFINE_LOADERCLASS(LOADERNAME) \
	class LOADERNAME : public XModule::LoaderInterface \
	{ \
	public: \
		const char*	identifyModule(const mp_ubyte* buffer); \
		mp_sint32   load(XMFileBase& f, XModule* module); \
	}; \

DEFINE_LOADERCLASS(Loader669)	// 669 Composer (Uses special FAR Player)
DEFINE_LOADERCLASS(LoaderAMF_1)	// Asylum Music Format 1.0
DEFINE_LOADERCLASS(LoaderAMF_2)	// Digital Sound And Music Interface (DMP module format)
DEFINE_LOADERCLASS(LoaderAMSv1) // Extreme Tracker AMS
DEFINE_LOADERCLASS(LoaderAMSv2)	// Velvet Studio AMS 
DEFINE_LOADERCLASS(LoaderDBM)	// Digibooster Pro
DEFINE_LOADERCLASS(LoaderCBA)	// Chuck Biscuits / Black Artist special format (only one Music Disk)
DEFINE_LOADERCLASS(LoaderDIGI)  // Digibooster 1.x
DEFINE_LOADERCLASS(LoaderDSMv1) // old digisound interface kit
DEFINE_LOADERCLASS(LoaderDSMv2) // new digisound interface kit
DEFINE_LOADERCLASS(LoaderDSm)   // dynamic studio
DEFINE_LOADERCLASS(LoaderDTM_1) // Digitrekker 3.0 (NOT DigiTrakker!!!)
DEFINE_LOADERCLASS(LoaderDTM_2) // Digital Tracker
DEFINE_LOADERCLASS(LoaderFAR)	// Farandole Composer (Uses special Player)
//DEFINE_LOADERCLASS(LoaderFNK)
DEFINE_LOADERCLASS(LoaderGDM)	// General Digimusic
DEFINE_LOADERCLASS(LoaderGMC)	// Game Music Creator (very similiar to SoundTracker, 15 ins, kept in LoaderMOD.cpp)
DEFINE_LOADERCLASS(LoaderIMF)	// Imago Orpheus
DEFINE_LOADERCLASS(LoaderIT)	// Impulse Tracker
DEFINE_LOADERCLASS(LoaderMDL)	// Digitrakker 2.0/3.0
DEFINE_LOADERCLASS(LoaderMOD)	// Protracker and compatible (1..32 channels)
DEFINE_LOADERCLASS(LoaderMTM)	// MultiTracker
DEFINE_LOADERCLASS(LoaderMXM)	// Cubic Tiny XM
DEFINE_LOADERCLASS(LoaderOKT)	// Oktalyzer
DEFINE_LOADERCLASS(LoaderPLM)	// DisorderTracker II
DEFINE_LOADERCLASS(LoaderPSMv1)	// old Epic MegaGames MASI (Epic Pinball, ...)
DEFINE_LOADERCLASS(LoaderPSMv2)	// new Epic MegaGames MASI (Jazz Jack Rabbit, ...)
DEFINE_LOADERCLASS(LoaderPTM)	// PolyTracker
DEFINE_LOADERCLASS(LoaderS3M)	// ScreamTracker III
DEFINE_LOADERCLASS(LoaderSTM)	// ScreamTracker II
DEFINE_LOADERCLASS(LoaderSFX)	// SoundFX module (very similiar to SoundTracker, 15 ins, kept in LoaderMOD.cpp) 
DEFINE_LOADERCLASS(LoaderULT)	// Ultratracker
DEFINE_LOADERCLASS(LoaderUNI)	// MikMod internal module
DEFINE_LOADERCLASS(LoaderXM)	// FT2 Extended Module

#endif
