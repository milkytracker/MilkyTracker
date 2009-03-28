/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  SampleLoaderWAV.h
 *  MilkyPlay
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

