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
 *  SampleLoaderWAV.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 14.09.05.
 *
 */

#include "SampleLoaderWAV.h"
#include "XMFile.h"
#include "XModule.h"

const char* SampleLoaderWAV::channelNames[] = {"Left","Right"};

SampleLoaderWAV::SampleLoaderWAV(const SYSCHAR* fileName, XModule& theModule) :
	SampleLoaderAbstract(fileName, theModule)
{
}

bool SampleLoaderWAV::identifySample()
{
	return getNumChannels() != 0;
}

mp_sint32 SampleLoaderWAV::parseFMTChunk(XMFile& f, TWAVHeader& hdr)
{
	if (hdr.fmtDataLength < 16)
		return MP_LOADER_FAILED;
	
	hdr.encodingTag = f.readWord();
	
	if (hdr.encodingTag != 0x1 &&
		hdr.encodingTag != 0x3)
		return MP_LOADER_FAILED;
	
	hdr.numChannels = f.readWord();
	
	if (hdr.numChannels < 1 || hdr.numChannels > 2)
		return MP_LOADER_FAILED;
	
	hdr.sampleRate = f.readDword();
	hdr.bytesPerSecond = f.readDword();
	hdr.blockAlign = f.readWord();
	hdr.numBits = f.readWord();
	
	if (hdr.numBits != 8 && 
		hdr.numBits != 16 && 
		hdr.numBits != 24 &&
		hdr.numBits != 32)
		return MP_LOADER_FAILED;
	
	// skip rest of format structure if greater than 16
	for (mp_uint32 i = 0; i < hdr.fmtDataLength - 16; i++)
		f.readByte();
	
	return MP_OK;
}

mp_sint32 SampleLoaderWAV::parseDATAChunk(XMFile& f, TWAVHeader& hdr, mp_sint32 index, mp_sint32 channelIndex)
{
	TXMSample* smp = &theModule.smp[index];
	
	if (hdr.dataLength)
	{
		
		if (hdr.numBits == 8)
		{
			if (smp->sample)
			{
				theModule.freeSampleMem((mp_ubyte*)smp->sample);
				smp->sample = NULL;
			}					
			smp->samplen = hdr.dataLength;					
			if (hdr.numChannels == 2)
			{
				mp_sbyte* buffer = new mp_sbyte[smp->samplen];						
				if (buffer == NULL)
					return MP_OUT_OF_MEMORY;						
				theModule.loadSample(f, buffer, smp->samplen, smp->samplen, XModule::ST_UNSIGNED);						
				smp->samplen>>=1;						
				smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen);						
				if (smp->sample == NULL)
				{
					delete[] buffer;
					return MP_OUT_OF_MEMORY;
				}						
				// Downmix channels
				if (channelIndex < 0)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
					{
						mp_sint32 s1 = buffer[i*2];
						mp_sint32 s2 = buffer[i*2+1];
						smp->sample[i] = (mp_sbyte)((s1+s2)>>1);
					}
				}
				// take left channel
				else if (channelIndex == 0)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
						smp->sample[i] = buffer[i*2];
				}
				// take right channel
				else if (channelIndex == 1)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
						smp->sample[i] = buffer[i*2+1];
				}
				else 
					ASSERT(false);						
				delete[] buffer;
			}
			else
			{
				smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen);						
				if (smp->sample == NULL)
					return MP_OUT_OF_MEMORY;						
				theModule.loadSample(f, smp->sample, smp->samplen, smp->samplen, XModule::ST_UNSIGNED);
			}					
			smp->type = 0;
		}
		else if (hdr.numBits == 16)
		{
			if (smp->sample)
			{
				theModule.freeSampleMem((mp_ubyte*)smp->sample);
				smp->sample = NULL;
			}					
			smp->samplen = hdr.dataLength>>1;					
			if (hdr.numChannels == 2)
			{
				mp_sword* buffer = new mp_sword[smp->samplen];						
				if (buffer == NULL)
					return MP_OUT_OF_MEMORY;						
				theModule.loadSample(f, buffer, hdr.dataLength, smp->samplen, XModule::ST_16BIT);						
				smp->samplen>>=1;						
				smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						
				if (smp->sample == NULL)
				{
					delete[] buffer;
					return MP_OUT_OF_MEMORY;
				}						
				mp_sword* sample = (mp_sword*)smp->sample;
				// Downmix channels
				if (channelIndex < 0)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
					{
						mp_sint32 s1 = buffer[i*2];
						mp_sint32 s2 = buffer[i*2+1];
						sample[i] = (mp_sword)((s1+s2)>>1);
					}
				}
				// take left channel
				else if (channelIndex == 0)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
						sample[i] = buffer[i*2];
				}
				// take right channel
				else if (channelIndex == 1)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
						sample[i] = buffer[i*2+1];
				}
				else 
					ASSERT(false);							
				delete[] buffer;
			}
			else
			{
				smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						
				if (smp->sample == NULL)
					return MP_OUT_OF_MEMORY;						
				theModule.loadSample(f, smp->sample, hdr.dataLength, smp->samplen, XModule::ST_16BIT);
			}					
			smp->type = 16;
		}
		else if (hdr.numBits == 24)
		{
			if (smp->sample)
			{
				theModule.freeSampleMem((mp_ubyte*)smp->sample);
				smp->sample = NULL;
			}					
			smp->samplen = hdr.dataLength/3;					
			if (hdr.numChannels == 2)
			{
				mp_ubyte* buffer = new mp_ubyte[smp->samplen*3];						
				if (buffer == NULL)
					return MP_OUT_OF_MEMORY;												
				f.read(buffer, 3, smp->samplen);
				smp->samplen>>=1;						
				smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						
				if (smp->sample == NULL)
				{
					delete[] buffer;
					return MP_OUT_OF_MEMORY;
				}						
				mp_sword* sample = (mp_sword*)smp->sample;
				// Downmix channels
				if (channelIndex < 0)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
					{
						mp_sword s1 = ((mp_sint32)buffer[i*6+1] + (((mp_sint32)buffer[i*6+2])<<8));
						mp_sword s2 = ((mp_sint32)buffer[i*6+4] + (((mp_sint32)buffer[i*6+5])<<8));
						sample[i] = ((mp_sint32)s1 + (mp_sint32)s2) >> 1;
					}
				}
				// take left channel
				else if (channelIndex == 0)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
						sample[i] = ((mp_sint32)buffer[i*6+1] + (((mp_sint32)buffer[i*6+2])<<8));
				}
				// take right channel
				else if (channelIndex == 1)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
						sample[i] = ((mp_sint32)buffer[i*6+4] + (((mp_sint32)buffer[i*6+5])<<8));
				}
				else 
					ASSERT(false);							
				delete[] buffer;
			}
			else
			{
				smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						
				if (smp->sample == NULL)
					return MP_OUT_OF_MEMORY;
				mp_ubyte* buffer = new mp_ubyte[smp->samplen*3];						
				if (buffer == NULL)
					return MP_OUT_OF_MEMORY;												
				f.read(buffer, 3, smp->samplen);
				mp_sword* sample = (mp_sword*)smp->sample;
				for (mp_uint32 i = 0; i < smp->samplen; i++)
					sample[i] = ((mp_sint32)buffer[i*3+1] + (((mp_sint32)buffer[i*3+2])<<8));
				delete[] buffer;
			}					
			smp->type = 16;
		}
		else if (hdr.numBits == 32)
		{
			// 32 bit DWORD sample data?
			if (hdr.encodingTag == 0x01)
			{
				if (smp->sample)
				{
					theModule.freeSampleMem((mp_ubyte*)smp->sample);
					smp->sample = NULL;
				}					
				smp->samplen = hdr.dataLength>>2;					
				
				if (hdr.numChannels == 2)
				{
					mp_sint32* buffer = new mp_sint32[smp->samplen];						
					if (buffer == NULL)
						return MP_OUT_OF_MEMORY;												
					f.readDwords((mp_dword*)buffer, smp->samplen);
					smp->samplen>>=1;						
					smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						
					if (smp->sample == NULL)
					{
						delete[] buffer;
						return MP_OUT_OF_MEMORY;
					}						
					mp_sword* sample = (mp_sword*)smp->sample;
					// Downmix channels
					if (channelIndex < 0)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
						{
							mp_sint32 s1 = buffer[i*2]>>16;
							mp_sint32 s2 = buffer[i*2+1]>>16;
							sample[i] = (mp_sword)((s1+s2)>>1);
						}
					}
					// take left channel
					else if (channelIndex == 0)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
							sample[i] = buffer[i*2]>>16;
					}
					// take right channel
					else if (channelIndex == 1)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
							sample[i] = buffer[i*2+1]>>16;
					}
					else 
						ASSERT(false);							
					delete[] buffer;
				}
				else
				{
					smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						
					if (smp->sample == NULL)
						return MP_OUT_OF_MEMORY;
					mp_sint32* buffer = new mp_sint32[smp->samplen];						
					if (buffer == NULL)
						return MP_OUT_OF_MEMORY;												
					f.readDwords((mp_dword*)buffer, smp->samplen);
					mp_sword* sample = (mp_sword*)smp->sample;
					for (mp_uint32 i = 0; i < smp->samplen; i++)
						sample[i] = buffer[i]>>16;
					delete[] buffer;
				}
			}
			else if (hdr.encodingTag == 0x03)
			{
				if (smp->sample)
				{
					theModule.freeSampleMem((mp_ubyte*)smp->sample);
					smp->sample = NULL;
				}					
				smp->samplen = hdr.dataLength>>2;					
				
				if (hdr.numChannels == 2)
				{
					float* buffer = new float[smp->samplen];						
					if (buffer == NULL)
						return MP_OUT_OF_MEMORY;												
					f.readDwords(reinterpret_cast<mp_dword*>(buffer), smp->samplen);
					smp->samplen>>=1;						
					smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						
					if (smp->sample == NULL)
					{
						delete[] buffer;
						return MP_OUT_OF_MEMORY;
					}						
					mp_sword* sample = (mp_sword*)smp->sample;
					// Downmix channels
					if (channelIndex < 0)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
						{
							mp_sint32 s1 = (mp_sint32)(buffer[i*2]*32767.0f);
							mp_sint32 s2 = (mp_sint32)(buffer[i*2+1]*32767.0f);
							sample[i] = (mp_sword)((s1+s2)>>1);
						}
					}
					// take left channel
					else if (channelIndex == 0)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
							sample[i] = (mp_sint32)(buffer[i*2]*32767.0f);
					}
					// take right channel
					else if (channelIndex == 1)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
							sample[i] = (mp_sint32)(buffer[i*2+1]*32767.0f);
					}
					else 
						ASSERT(false);							
					delete[] buffer;
				}
				else
				{
					smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen*2);						
					if (smp->sample == NULL)
						return MP_OUT_OF_MEMORY;
					float* buffer = new float[smp->samplen];						
					if (buffer == NULL)
						return MP_OUT_OF_MEMORY;												
					f.readDwords(reinterpret_cast<mp_dword*>(buffer), smp->samplen);
					mp_sword* sample = (mp_sword*)smp->sample;
					for (mp_uint32 i = 0; i < smp->samplen; i++)
						sample[i] = (mp_sint32)(buffer[i]*32767.0f);
					delete[] buffer;
				}
			}
			smp->type = 16;
		}								
		smp->loopstart = 0;
		smp->looplen = 0; 
		
		nameToSample(preferredDefaultName, smp);
		
		XModule::convertc4spd(hdr.sampleRate, &smp->finetune, &smp->relnote);
		smp->vol = 255;
		smp->pan = 0x80;
		smp->flags = 3;
	}			
	
	return MP_OK;
}

mp_sint32 SampleLoaderWAV::parseSMPLChunk(XMFile& f, TWAVHeader& hdr, 
										  TSamplerChunk& samplerChunk, TSampleLoop& sampleLoop)
{
	mp_dword pos = (unsigned)f.pos() + (unsigned)hdr.dataLength;			
	
	f.readDwords((mp_dword*)&samplerChunk, sizeof(TSamplerChunk) / sizeof(mp_dword));
	
	if (samplerChunk.cSampleLoops)
	{
		f.readDwords((mp_dword*)&sampleLoop, sizeof(TSampleLoop) / sizeof(mp_dword));
	}

	f.seek(pos);
	return MP_OK;
}

mp_sint32 SampleLoaderWAV::getNumChannels()
{
	XMFile f(theFileName);
		
	TWAVHeader hdr;
	
	f.read(hdr.RIFF, 1, 4);
	
	if (memcmp(hdr.RIFF, "RIFF", 4))
		return 0;
	
	hdr.length = f.readDword();
	f.read(hdr.WAVE, 1, 4);	

	if (memcmp(hdr.WAVE, "WAVE", 4))
		return 0;

	hdr.numChannels = 0;

	do
	{
		f.read(hdr.FMT, 1, 4);	
		hdr.fmtDataLength = f.readDword();
		
		// found a "fmt " chunk?
		if (memcmp(hdr.FMT, "fmt ", 4) == 0)
		{
			mp_sint32 res = parseFMTChunk(f, hdr);
			if (res < 0)
				return 0;
			break;
		}
		else
		{
			mp_dword pos = (unsigned)f.pos() + (unsigned)hdr.fmtDataLength;
			
			if (pos >= f.size())
				break;
			
			f.seek(pos);
		}
		
	} while (f.pos() < f.size());
	
	return hdr.numChannels;
}
	
const char* SampleLoaderWAV::getChannelName(mp_sint32 channelIndex)
{
	if (channelIndex < 2)
		return channelNames[channelIndex];
	else
		return SampleLoaderAbstract::getChannelName(channelIndex);
}

mp_sint32 SampleLoaderWAV::loadSample(mp_sint32 index, mp_sint32 channelIndex)
{
	XMFile f(theFileName);
		
	TWAVHeader hdr;
	
	f.read(hdr.RIFF, 1, 4);
	
	if (memcmp(hdr.RIFF, "RIFF", 4))
		return MP_LOADER_FAILED;
	
	hdr.length = f.readDword();
	f.read(hdr.WAVE, 1, 4);	

	if (memcmp(hdr.WAVE, "WAVE", 4))
		return MP_LOADER_FAILED;

	// Start looking for "fmt " chunk
	hdr.numChannels = 0;

	do
	{
		f.read(hdr.FMT, 1, 4);	
		hdr.fmtDataLength = f.readDword();
		
		// found a "fmt " chunk?
		if (memcmp(hdr.FMT, "fmt ", 4) == 0)
		{
			mp_sint32 res = parseFMTChunk(f, hdr);
			if (res < 0)
				return res;
			break;
		}
		else
		{
			mp_dword pos = (unsigned)f.pos() + (unsigned)hdr.fmtDataLength;
			
			if (pos >= f.size())
				break;
			
			f.seek(pos);
		}
		
	} while (f.pos() < f.size());

	// check if we found a "fmt " chunk, otherwise the number of channels will be 0
	if (hdr.numChannels == 0)
		return MP_LOADER_FAILED;
	
	// process remaining chunks
	
	TSamplerChunk samplerChunk;
	TSampleLoop sampleLoop;
	bool hasSamplerChunk = false;
	bool hasData = false;
	
	do
	{
		mp_sint32 res = f.read(hdr.DATA, 1, 4);	
		if (res < 4)
			break;
			
		hdr.dataLength = f.readDword();
	
		if (memcmp(hdr.DATA, "data", 4) == 0)
		{
			mp_sint32 res = parseDATAChunk(f, hdr, index, channelIndex);
			if (res < 0)
				return res;
			hasData = true;
		}
		else if (memcmp(hdr.DATA, "smpl", 4) == 0)
		{
			mp_sint32 res = parseSMPLChunk(f, hdr, samplerChunk, sampleLoop);
			if (res == 0)
				hasSamplerChunk = true;
		}
		else
		{
			mp_sint32 pos = f.pos() + hdr.dataLength;
			
			if (pos >= (signed)f.size())
				break;
			
			f.seek(pos);
		}
	
	} while (f.pos() < f.size());
	
	if (!hasData)
		return MP_LOADER_FAILED;
	
	if (hasSamplerChunk && samplerChunk.cSampleLoops)
	{
		TXMSample* smp = &theModule.smp[index];
		
		mp_dword start = sampleLoop.dwStart/* / (hdr.numBits*hdr.numChannels / 8)*/;
		mp_dword end = sampleLoop.dwEnd/* / (hdr.numBits*hdr.numChannels / 8)*/;
		
		if (end > smp->samplen)
			end = smp->samplen;
		
		if (start < end && end <= smp->samplen)
		{
			smp->loopstart = start;
			smp->looplen = end - start;
			switch (sampleLoop.dwType)
			{
				case 0:
					smp->type|=1;
					break;
				case 1:
					smp->type|=2;
					break;
			}
		}
	}
	
	return MP_OK;
}

mp_sint32 SampleLoaderWAV::saveSample(const SYSCHAR* fileName, mp_sint32 index)
{
	TXMSample* smp = &theModule.smp[index];

	XMFile f(fileName, true);
	
	TWAVHeader hdr;
	TSamplerChunk samplerChunk;
	TSampleLoop sampleLoop;
	
	bool hasLoop = (smp->type & 3) != 0;
	
	// build wav header
	memcpy(hdr.RIFF, "RIFF", 4);
	hdr.length = 44 + ((smp->type&16) ? smp->samplen*2 : smp->samplen) - 8 +
		(hasLoop ? (sizeof(TSamplerChunk) + sizeof(TSampleLoop) + 8) : 0);
	memcpy(hdr.WAVE, "WAVE", 4);
	memcpy(hdr.FMT, "fmt ", 4);
	hdr.fmtDataLength = 16;
	hdr.encodingTag = 1;
	hdr.numChannels = 1;
	hdr.sampleRate = XModule::getc4spd(smp->relnote,smp->finetune);
	hdr.numBits = ((smp->type&16) ? 16 : 8);
	hdr.blockAlign = (1*hdr.numBits) / 8;
	hdr.bytesPerSecond = hdr.sampleRate*hdr.blockAlign;

	f.write(hdr.RIFF, 1, 4);
	
	f.writeDword(hdr.length);
	f.write(hdr.WAVE, 1, 4);	

	f.write(hdr.FMT, 1, 4);	

	f.writeDword(hdr.fmtDataLength);
	f.writeWord(hdr.encodingTag);
	
	f.writeWord(hdr.numChannels);
	
	f.writeDword(hdr.sampleRate);
	f.writeDword(hdr.bytesPerSecond);
	f.writeWord(hdr.blockAlign);
	f.writeWord(hdr.numBits);

	if (hasLoop)
	{
		memcpy(hdr.DATA, "smpl", 4);
		hdr.dataLength = sizeof(TSamplerChunk) + sizeof(TSampleLoop);	
		
		f.write(hdr.DATA, 1, 4);	
		f.writeDword(hdr.dataLength);
		
		samplerChunk.cSampleLoops = 1;
		samplerChunk.dwSamplePeriod = (mp_dword)(1000.0*1000.0*1000.0 / hdr.sampleRate);
		
		sampleLoop.dwType = ((smp->type & 3) == 2 ? 1 : 0);
		sampleLoop.dwStart = smp->loopstart;
		sampleLoop.dwEnd = smp->loopstart + smp->looplen;
		
		f.writeDwords((mp_dword*)&samplerChunk, sizeof(samplerChunk) / sizeof(mp_dword));
		f.writeDwords((mp_dword*)&sampleLoop, sizeof(sampleLoop) / sizeof(mp_dword));
	} 
	
	memcpy(hdr.DATA, "data", 4);
	hdr.dataLength = (smp->samplen*hdr.numBits)/8;	
	
	f.write(hdr.DATA, 1, 4);	
	f.writeDword(hdr.dataLength);
	
	if (smp->type & 16)
	{
		for (mp_uint32 i = 0; i < smp->samplen; i++)
			f.writeWord(smp->getSampleValue(i));
	}
	else
	{
		// WAV 8 bit is unsigned 
		mp_ubyte* dstPtr = new mp_ubyte[smp->samplen]; 
		mp_ubyte* dst = dstPtr;
		for (mp_uint32 i = 0; i < smp->samplen; i++)
			*dstPtr++ = ((mp_sbyte)smp->getSampleValue(i))^127;
		f.write(dst, 1, smp->samplen);
		delete[] dst;
	}
	
	return MP_OK;
}
