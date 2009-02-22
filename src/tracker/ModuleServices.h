/*
 *  tracker/ModuleServices.h
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
 *  ModuleServices.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.12.07.
 *
 */

#ifndef __MODULESERVICES_H__
#define __MODULESERVICES_H__

#include "BasicTypes.h"
#include "MilkyPlayCommon.h"

class ModuleServices
{
private:
	class XModule& module;

	pp_int32 estimatedSongLength;

public:
	ModuleServices(XModule& module) :
		module(module),
		estimatedSongLength(-1)
	{
	}
	
	void estimateSongLength();
	pp_int32 getEstimatedSongLength() const { return estimatedSongLength; }
	void resetEstimatedSongLength() { estimatedSongLength = -1; }
	
	struct WAVWriterParameters
	{
		pp_uint32 sampleRate;
		pp_uint32 resamplerType;
		pp_uint32 playMode;
		pp_uint32 mixerShift;
		pp_uint32 mixerVolume;
		
		pp_uint32 fromOrder;
		pp_uint32 toOrder;
		const pp_uint8* muting;
		const pp_uint8* panning;
		
		bool multiTrack;
		
		WAVWriterParameters() :
			sampleRate(0),
			resamplerType(0),
			playMode(0),
			mixerShift(0),	
			mixerVolume(0),	
			fromOrder(0),
			toOrder(0),
			muting(NULL),
			panning(NULL),
			multiTrack(false)
		{
		}
	};
	
	pp_int32 estimateMixerVolume(WAVWriterParameters& parameters, 
								 pp_int32* numSamplesProgressed = NULL);
								 
	pp_int32 estimateWaveLengthInSamples(WAVWriterParameters& parameters);

	pp_int32 exportToWAV(const PPSystemString& fileName, WAVWriterParameters& parameters);
	
	pp_int32 exportToBuffer16Bit(WAVWriterParameters& parameters, pp_int16* buffer, 
								 pp_uint32 bufferSize, bool mono = true);
};

#endif
