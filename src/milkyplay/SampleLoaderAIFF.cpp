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
 *  SampleLoaderAIFF.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 07.01.06.
 *
 */

#include "SampleLoaderAIFF.h"
#include "XMFile.h"
#include "XModule.h"
#include "LittleEndian.h"

const char* SampleLoaderAIFF::channelNames[] = {"Left","Right"};

SampleLoaderAIFF::SampleLoaderAIFF(const SYSCHAR* fileName, XModule& theModule) :
	SampleLoaderAbstract(fileName, theModule)
{
}

bool SampleLoaderAIFF::identifySample()
{
	return getNumChannels() != 0;
}

mp_sint32 SampleLoaderAIFF::getNumChannels()
{
	mp_ubyte ID[4], buffer[4];
	mp_dword chunkLen;

	XMFile f(theFileName);
	
	f.read(ID, 4, 1);
	if (memcmp(ID, "FORM", 4) != 0)
		return 0;
		
	f.seek(0);
			
	bool hasFORM = false;
	bool hasFVER = false;
	bool hasCOMM = false;
	bool hasSSND = false;
	
	mp_sint32 numChannels = 0;
	
	while (!(hasFORM && hasFVER && hasCOMM && hasSSND))
	{
		
		mp_uint32 bytesRead = f.read(ID, 4, 1);
		if (bytesRead != 4)
			break;		

		bytesRead = f.read(buffer, 4, 1);
		if (bytesRead != 4)
			break;		
		
		chunkLen = BigEndian::GET_DWORD(buffer);
		
		switch (BigEndian::GET_DWORD(ID))
		{
			case 0x464F524D:	// 'FORM'
			{
				f.read(buffer, 4, 1);
				if (memcmp(buffer, "AIFC", 4) == 0)
					hasFORM = true;
				else if (memcmp(buffer, "AIFF", 4) == 0)
					hasFORM = hasFVER = true;
				break;
			}

			case 0x46564552 :	// 'FVER'
			{
				hasFVER = true;
				mp_uint32 pos = f.pos();
				f.seek(pos + chunkLen);
				break;
			}

			case 0x434F4D4D:	// 'COMM'
			{
				hasCOMM = true;
				mp_uint32 pos = f.pos();
				
				f.read(buffer, 2, 1);
				numChannels = BigEndian::GET_WORD(buffer);
				
				f.seek(pos + chunkLen);
				break;
			}

			case 0x53534E44 :	// 'SSND'
			{
				hasSSND = true;
				mp_uint32 pos = f.pos();
				f.seek(pos + chunkLen);
				break;
			}
			
			default:
			{
				mp_uint32 pos = f.pos();
				f.seek(pos + chunkLen);
			}
		}
	}

	return (hasFORM && hasFVER && hasCOMM && hasSSND) ? numChannels : 0;
}

const char* SampleLoaderAIFF::getChannelName(mp_sint32 channelIndex)
{
	if (channelIndex < 2)
		return channelNames[channelIndex];
	else
		return SampleLoaderAbstract::getChannelName(channelIndex);
}

struct AIFC_CommChunk
{
	mp_uword numChannels;		//number of channels
	mp_uint32 numSampleFrames;	//number of sample frames
	mp_uword sampleSize;		//number of bits per sample
	mp_uint32 sampleRate;		//number of frames per second
	mp_uint32 compressionType;	//compression type ID

	AIFC_CommChunk() :
		numChannels(0),
		numSampleFrames(0),
		sampleSize(0),
		sampleRate(0),
		compressionType(0)
	{
	}
};

mp_sint32 SampleLoaderAIFF::loadSample(mp_sint32 index, mp_sint32 channelIndex)
{
	mp_ubyte ID[4], buffer[4];
	mp_dword chunkLen;
	AIFC_CommChunk commChunk;

	XMFile f(theFileName);
	
	f.read(ID, 4, 1);
	if (memcmp(ID, "FORM", 4) != 0)
		return MP_LOADER_FAILED;
		
	f.seek(0);
					
	bool hasFORM = false;
	bool hasFVER = false;
	bool hasCOMM = false;
	bool hasSSND = false;
	
	bool aifc = false;
	bool sowt = false;
	
	mp_sbyte* sampleData = NULL;
	mp_sint32 sampleDataLen = 0;
				
	while (!(hasFORM && hasFVER && hasCOMM && hasSSND))
	{
		
		mp_uint32 bytesRead = f.read(ID, 4, 1);
		if (bytesRead != 4)
			break;		

		bytesRead = f.read(buffer, 4, 1);
		if (bytesRead != 4)
			break;		
		
		chunkLen = BigEndian::GET_DWORD(buffer);
		
		switch (BigEndian::GET_DWORD(ID))
		{
			case 0x464F524D:	// 'FORM'
			{
				f.read(buffer, 4, 1);
				if (memcmp(buffer, "AIFC", 4) == 0)
				{
					hasFORM = true;
					aifc = true;
				}
				else if (memcmp(buffer, "AIFF", 4) == 0)
				{
					hasFORM = hasFVER = true;
					aifc = false; 
				}
				break;
			}

			case 0x46564552 :	// 'FVER'
			{
				hasFVER = true;
				mp_uint32 pos = f.pos();
				f.seek(pos + chunkLen);
				break;
			}

			case 0x434F4D4D:	// 'COMM'
			{
				hasCOMM = true;
				//mp_uint32 pos = f.pos();
				
				mp_ubyte* temp = new mp_ubyte[chunkLen];
				
				f.read(temp, chunkLen, 1);

				commChunk.numChannels = BigEndian::GET_WORD(temp);
				commChunk.numSampleFrames = BigEndian::GET_DWORD(temp+2);
				commChunk.sampleSize = BigEndian::GET_WORD(temp+6);
				commChunk.sampleRate = BigEndian::GET_DWORD(temp+10) >> 16;				
				
				if (aifc)
				{
					commChunk.compressionType = BigEndian::GET_DWORD(temp+14);
					mp_dword compressionName = BigEndian::GET_DWORD(temp+18);
					sowt = (compressionName == 0x736F7774);
				}
				
				delete[] temp;
				
				if ((commChunk.compressionType != 0) &&
					(commChunk.compressionType != 0x4E4F4E45/*NONE*/))
				{
					return MP_LOADER_FAILED;
				}
				
				break;
			}

			case 0x53534E44 :	// 'SSND'
			{
				hasSSND = true;
				sampleDataLen = chunkLen;
				sampleData = new mp_sbyte[chunkLen];
				f.read(sampleData, chunkLen, 1);
				break;
			}
			
			default:
			{
				mp_uint32 pos = f.pos();
				f.seek(pos + chunkLen);
			}
		}
	}

	if (hasFORM && hasFVER && hasCOMM && hasSSND)
	{
		if ((commChunk.numChannels >= 1) && 
			(commChunk.numChannels <= 2) &&
			(commChunk.sampleSize == 8 ||
			 commChunk.sampleSize == 16))
		{
			sampleDataLen = (commChunk.numChannels * commChunk.numSampleFrames * commChunk.sampleSize) / 8;
		
			TXMSample* smp = &theModule.smp[index];
			
			if (smp->sample)
			{
				theModule.freeSampleMem((mp_ubyte*)smp->sample);
				smp->sample = NULL;
			}					
			
			smp->samplen = ((commChunk.sampleSize == 16) ? sampleDataLen >> 1 : sampleDataLen) / commChunk.numChannels;
			
			smp->sample = (mp_sbyte*)theModule.allocSampleMem((commChunk.sampleSize == 16) ? (smp->samplen<<1) : smp->samplen);						
			if (smp->sample == NULL)
			{
				if (sampleData)
					delete[] sampleData;
				return MP_OUT_OF_MEMORY;						
			}
			
			if (commChunk.sampleSize == 8)
			{
				mp_sbyte* ptr = (mp_sbyte*)smp->sample; 
				mp_sbyte* src = (mp_sbyte*)sampleData;

				if (commChunk.numChannels == 1)
				{
					memcpy(ptr, src, smp->samplen);
				}
				else if (commChunk.numChannels == 2)
				{
					if (channelIndex == 0)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
						{
							*ptr = *src;
							ptr++;
							src+=2;
						}
					}
					else if (channelIndex == 1)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
						{
							*ptr = *(src+1);
							ptr++;
							src+=2;
						}
					}
					else if (channelIndex == -1)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
						{
							*ptr = (mp_sbyte)(((mp_sword)(*src) + (mp_sword)(*(src+1))) >> 1);
							ptr++;
							src+=2;
						}
					}			
				}
			}
			else if (commChunk.sampleSize == 16)
			{
				mp_uword* ptr = (mp_uword*)smp->sample; 
				mp_ubyte* src = (mp_ubyte*)sampleData;
				
				if (commChunk.numChannels == 1)
				{
					for (mp_uint32 i = 0; i < smp->samplen; i++)
					{
						*ptr = sowt ? LittleEndian::GET_WORD(src) : BigEndian::GET_WORD(src);
						ptr++;
						src+=2;
					}
				}
				else if (commChunk.numChannels == 2)
				{
					if (channelIndex == 0)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
						{
							*ptr = sowt ? LittleEndian::GET_WORD(src) : BigEndian::GET_WORD(src);
							ptr++;
							src+=4;
						}
					}
					else if (channelIndex == 1)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
						{
							*ptr = sowt ? LittleEndian::GET_WORD(src+2) : BigEndian::GET_WORD(src+2);
							ptr++;
							src+=4;
						}
					}
					else if (channelIndex == -1)
					{
						for (mp_uint32 i = 0; i < smp->samplen; i++)
						{
							*ptr = sowt ? 
								(mp_sword)(((mp_sint32)((mp_sword)LittleEndian::GET_WORD(src)) + (mp_sint32)((mp_sword)LittleEndian::GET_WORD(src+2))) >> 1) :
								(mp_sword)(((mp_sint32)((mp_sword)BigEndian::GET_WORD(src)) + (mp_sint32)((mp_sword)BigEndian::GET_WORD(src+2))) >> 1);
							ptr++;
							src+=4;
						}
					}
				}
			}
			
			delete[] sampleData;
			
			smp->loopstart = 0;
			smp->looplen = 0;
			smp->type = 0;
			if (commChunk.sampleSize == 16)
				smp->type |= 16;		
						
			nameToSample(preferredDefaultName, smp);
			
			XModule::convertc4spd(commChunk.sampleRate, &smp->finetune, &smp->relnote);
					 
			return MP_OK;
		}
	}

	delete[] sampleData;

	return MP_LOADER_FAILED;
}

mp_sint32 SampleLoaderAIFF::saveSample(const SYSCHAR* fileName, mp_sint32 index)
{
	return MP_UNSUPPORTED;
}
