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
 *  AudioDriver_WAVWriter.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 29.07.05.
 *
 */

#include "AudioDriver_WAVWriter.h"

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

static void writeWAVHeader(XMFile* f, const TWAVHeader& hdr)
{
	f->write(hdr.RIFF, 1, 4);
	
	f->writeDword(hdr.length);
	f->write(hdr.WAVE, 1, 4);	

	f->write(hdr.FMT, 1, 4);	

	f->writeDword(hdr.fmtDataLength);
	f->writeWord(hdr.encodingTag);
	
	f->writeWord(hdr.numChannels);
	
	f->writeDword(hdr.sampleRate);
	f->writeDword(hdr.bytesPerSecond);
	f->writeWord(hdr.blockAlign);
	f->writeWord(hdr.numBits);
	
	f->write(hdr.DATA, 1, 4);	
	f->writeDword(hdr.dataLength);
}

WAVWriter::WAVWriter(const SYSCHAR* fileName) :
	AudioDriver_NULL(),
	f(NULL),
	mixFreq(44100)
{
	TWAVHeader hdr;
	
	f = new XMFile(fileName, true);

	if (!f->isOpenForWriting())
	{
		delete f;
		f = NULL;
	}
	else
	{
		// build wav header
		memcpy(hdr.RIFF, "RIFF", 4);
		hdr.length = 44 - 8;
		memcpy(hdr.WAVE, "WAVE", 4);
		memcpy(hdr.FMT, "fmt ", 4);
		hdr.fmtDataLength = 16;
		hdr.encodingTag = 1;
		hdr.numChannels = 2;
		hdr.sampleRate = mixFreq;
		hdr.numBits = 16;
		hdr.blockAlign = (hdr.numChannels*hdr.numBits) / 8;
		hdr.bytesPerSecond = hdr.sampleRate*hdr.blockAlign;
		memcpy(hdr.DATA, "data", 4);
		hdr.dataLength = 0;	
		
		writeWAVHeader(f, hdr);
	}
}

WAVWriter::~WAVWriter() 
{
	if (f)
		delete f;
}

mp_sint32 WAVWriter::initDevice(mp_sint32 bufferSizeInWords, mp_uint32 mixFrequency, MasterMixer* mixer)
{
	mp_sint32 res = AudioDriver_NULL::initDevice(bufferSizeInWords, mixFrequency, mixer);
	if (res < 0)
		return res;

	mixFreq = mixFrequency;
	return MP_OK;
}

mp_sint32 WAVWriter::closeDevice()
{
	if (!f)
		return MP_DEVICE_ERROR;
		
	TWAVHeader hdr;
	
	// build wav header
	memcpy(hdr.RIFF, "RIFF", 4);
	hdr.length = 44 + numSamplesWritten*4 - 8;
	memcpy(hdr.WAVE, "WAVE", 4);
	memcpy(hdr.FMT, "fmt ", 4);
	hdr.fmtDataLength = 16;
	hdr.encodingTag = 1;
	hdr.numChannels = 2;
	hdr.sampleRate = mixFreq;
	hdr.numBits = 16;
	hdr.blockAlign = (hdr.numChannels*hdr.numBits) / 8;
	hdr.bytesPerSecond = hdr.sampleRate*hdr.blockAlign;
	memcpy(hdr.DATA, "data", 4);
	hdr.dataLength = (numSamplesWritten*2*hdr.numBits)/8;	
		
	f->seek(0);

	writeWAVHeader(f, hdr);
	
	return MP_OK;
}

void WAVWriter::advance()
{
	AudioDriver_NULL::advance();

	if (!f)
		return;
	
	f->writeWords((mp_uword*)compensateBuffer, bufferSize);
}

