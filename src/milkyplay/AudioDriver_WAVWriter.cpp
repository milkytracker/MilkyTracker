/*
 *  milkyplay/AudioDriver_WAVWriter.cpp
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
	return 0;
}

mp_sint32 WAVWriter::closeDevice()
{
	if (!f)
		return -1;
		
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
	
	return 0;
}

void WAVWriter::advance()
{
	AudioDriver_NULL::advance();

	if (!f)
		return;
	
	f->writeWords((mp_uword*)compensateBuffer, bufferSize);
}

