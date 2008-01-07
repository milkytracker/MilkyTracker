/*
 *  milkyplay/SampleLoaderWAV.h
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
 *  SampleLoaderWAV.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.09.05.
 *
 */

#ifndef SAMPLELOADERWAV__H
#define SAMPLELOADERWAV__H

#include "SampleLoaderAbstract.h"

class SampleLoaderWAV : public SampleLoaderAbstract
{
private:
	struct TWAVHeader
	{
		mp_ubyte RIFF[4];			// "RIFF"
		mp_dword length;			// filesize - 8
		mp_ubyte WAVE[4];			// "WAVE"
		mp_ubyte FMT[4];			// "fmt "
		mp_dword fmtDataLength;		// = 16
		mp_uword encodingTag;		
		mp_uword numChannels;		// Channels: 1 = mono, 2 = stereo
		mp_dword sampleRate;		// Samples per second: e.g., 44100
		mp_dword bytesPerSecond;	// sample rate * block align
		mp_uword blockAlign;		// channels * numBits / 8
		mp_uword numBits;			// 8 or 16
		mp_ubyte DATA[4];			// "data"
		mp_dword dataLength;		// sample data size
	};

	struct TSamplerChunk 
	{
		mp_dword dwManufacturer;
		mp_dword dwProduct;
		mp_dword dwSamplePeriod;
		mp_dword dwMIDIUnityNote;
		mp_dword dwMIDIPitchFraction;
		mp_dword dwSMPTEFormat;
		mp_dword dwSMPTEOffset;
		mp_dword cSampleLoops;
		mp_dword cbSamplerData;
		
		TSamplerChunk() :
			dwManufacturer(0),
			dwProduct(0),
			dwSamplePeriod(0),
			dwMIDIUnityNote(0),
			dwMIDIPitchFraction(0),
			dwSMPTEFormat(0),
			dwSMPTEOffset(0),
			cSampleLoops(0),
			cbSamplerData(0)
		{
		}
	};
	
	struct TSampleLoop
	{
		mp_dword dwIdentifier;
		mp_dword dwType;
		mp_dword dwStart;
		mp_dword dwEnd;
		mp_dword dwFraction;
		mp_dword dwPlayCount;
		
		TSampleLoop() :
			dwIdentifier(0),
			dwType(0),
			dwStart(0),
			dwEnd(0),
			dwFraction(0),
			dwPlayCount(0)
		{
		}
	};	
	
	static const char* channelNames[];

	mp_sint32 parseFMTChunk(XMFile& f, TWAVHeader& hdr);
	mp_sint32 parseDATAChunk(XMFile& f, TWAVHeader& hdr, mp_sint32 index, mp_sint32 channelIndex);
	mp_sint32 parseSMPLChunk(XMFile& f, TWAVHeader& hdr, TSamplerChunk& samplerChunk, TSampleLoop& sampleLoop);

public:
	SampleLoaderWAV(const SYSCHAR* fileName, XModule& theModule);

	virtual bool identifySample();

	virtual mp_sint32 getNumChannels();
	
	virtual const char* getChannelName(mp_sint32 channelIndex);

	virtual mp_sint32 loadSample(mp_sint32 index, mp_sint32 channelIndex);

	virtual mp_sint32 saveSample(const SYSCHAR* fileName, mp_sint32);
};

#endif

