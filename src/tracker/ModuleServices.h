/*
 *  ModuleServices.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 08.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
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
		
		WAVWriterParameters() :
			sampleRate(0),
			resamplerType(0),
			playMode(0),
			mixerShift(0),	
			mixerVolume(0),	
			fromOrder(0),
			toOrder(0),
			muting(NULL),
			panning(NULL)
		{
		}
	};
	
	pp_int32 estimateMixerVolume(WAVWriterParameters& parameters, 
								 pp_int32* numSamplesProgressed = NULL);
								 
	pp_int32 estimateWaveLengthInSamples(WAVWriterParameters& parameters);

	pp_int32 exportToWAV(const SYSCHAR* fileName, WAVWriterParameters& parameters);
	
	pp_int32 exportToBuffer16Bit(WAVWriterParameters& parameters, pp_int16* buffer, 
								 pp_uint32 bufferSize, bool mono = true);
};

#endif
