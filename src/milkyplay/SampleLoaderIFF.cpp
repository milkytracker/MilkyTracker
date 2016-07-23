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
 *  SampleLoaderIFF.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 07.01.06.
 *
 */

#include "SampleLoaderIFF.h"
#include "XMFile.h"
#include "XModule.h"
#include "LittleEndian.h"

SampleLoaderIFF::SampleLoaderIFF(const SYSCHAR* fileName, XModule& theModule) :
	SampleLoaderAbstract(fileName, theModule)
{
}

bool SampleLoaderIFF::identifySample()
{
	return getNumChannels() != 0;
}

mp_sint32 SampleLoaderIFF::getNumChannels()
{
	mp_ubyte ID[4], buffer[4];
	mp_dword chunkLen;

	XMFile f(theFileName);
	
	f.read(ID, 4, 1);
	if (memcmp(ID, "FORM", 4) != 0)
		return 0;
		
	f.seek(0);
					
	bool hasFORM = false;
	bool hasVHDR = false;
	bool hasBODY = false;
		
	while (!(hasFORM && hasVHDR && hasBODY))
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
				if (memcmp(buffer, "8SVX", 4) == 0)
					hasFORM = true;
				else if (memcmp(buffer, "16SV", 4) == 0)
					hasFORM = true;
				
				break;
			}

			/*case 0x4E414D45:	// 'NAME'
			{
				break;
			}*/

			case 0x56484452:	// 'VHDR'
			{
				hasVHDR = true;
				mp_uint32 pos = f.pos();
				f.seek(pos + chunkLen);
				break;
			}

			case 0x424F4459:	// 'BODY'
			{
				hasBODY = true;
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

		// odd chunks are padded with zero
		if (chunkLen&1)
			f.readByte();
	}

	return (hasFORM && hasVHDR && hasBODY) ? 1 : 0;
}

/* 8SVX Voice8Header Structure Definition */
struct SVX8_Vhdr
{
	mp_dword Oneshothi;						/* Num Samples in high octave 1-shot part */
	mp_dword Repeathi;						/* Num Samples in high octave repeat part */
	mp_dword Samplescycle;					/* Num Samples/cycle in high octave, else 0 */
	mp_uword Samplessec;					/* Samples Per Second - Sampling Rate */
	mp_ubyte Octaves;						/* Number of Octaves */
	mp_ubyte Compression;					/* Compression algorithm used - see below */
	mp_sint32 Volume;						/* Volume from 0 to Unity - see below */
};

mp_sint32 SampleLoaderIFF::loadSample(mp_sint32 index, mp_sint32 channelIndex)
{
	mp_ubyte ID[4], buffer[4];
	mp_dword chunkLen;
	SVX8_Vhdr vhdr;

	XMFile f(theFileName);
	
	f.read(ID, 4, 1);
	if (memcmp(ID, "FORM", 4) != 0)
		return MP_LOADER_FAILED;
		
	f.seek(0);
					
	bool hasFORM = false;
	bool hires = false;
	bool hasVHDR = false;
	bool hasBODY = false;
		
	mp_ubyte* name = NULL;
	mp_ubyte* anno = NULL;
	
	mp_sbyte* sampleData = NULL;
	mp_sint32 sampleDataLen = 0;
	
	mp_dword iffSize = 0;
				
	while (!(hasFORM && hasVHDR && hasBODY))
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
				if (memcmp(buffer, "8SVX", 4) == 0)
				{
					hasFORM = true;
					hires = false;
				}
				else if (memcmp(buffer, "16SV", 4) == 0)
				{
					hasFORM = true;
					hires = true;
				}
				
				iffSize = chunkLen;
				
				break;
			}

			case 0x4E414D45:	// 'NAME'
			{
				if (name)
					delete[] name;
					
				name = new mp_ubyte[chunkLen+1];
				if (name == NULL)
				{
					if (sampleData)
						delete[] sampleData;
					if (name)
						delete[] name;
					if (anno)
						delete[] anno;
					return MP_OUT_OF_MEMORY;
				}
				
				f.read(name, 1, chunkLen);
				name[chunkLen] = '\0';
				break;
			}

			case 0x414E4E4F :	// 'ANNO'
			{
				if (anno)
					delete[] anno;
					
				anno = new mp_ubyte[chunkLen+1];
				if (anno == NULL)
				{
					if (sampleData)
						delete[] sampleData;
					if (name)
						delete[] name;
					if (anno)
						delete[] anno;
					return MP_OUT_OF_MEMORY;
				}
				
				f.read(anno, 1, chunkLen);
				anno[chunkLen] = '\0';
				break;
			}

			case 0x56484452:	// 'VHDR'
			{
				hasVHDR = true;
				mp_uint32 pos = f.pos();
				
				if (chunkLen < 20)
				{
					if (sampleData)
						delete[] sampleData;
					if (name)
						delete[] name;
					if (anno)
						delete[] anno;
					return MP_LOADER_FAILED;
				}
				
				f.read(buffer, 1, 4);
				vhdr.Oneshothi = BigEndian::GET_DWORD(buffer);		// sample len
				f.read(buffer, 1, 4);
				vhdr.Repeathi = BigEndian::GET_DWORD(buffer);		// loop start
				f.read(buffer, 1, 4);
				vhdr.Samplescycle = BigEndian::GET_DWORD(buffer);	// loop len
				f.read(buffer, 1, 2);
				vhdr.Samplessec = BigEndian::GET_WORD(buffer);		// c4 speed 
				vhdr.Octaves = f.readByte();		
				vhdr.Compression = f.readByte();
				f.read(buffer, 1, 4);
				vhdr.Volume = BigEndian::GET_DWORD(buffer);			// volume
				
				f.seek(pos + chunkLen);
				break;
			}

			case 0x424F4459:	// 'BODY'
			{
				if (!hasVHDR)
					goto bail;
				
				mp_uint32 pos = f.pos();
			
				switch (vhdr.Compression)
				{
					// uncompressed IFF
					case 0:
					{
						hasBODY = true;
				
						// check for some nasty invalid TFMX IFF
						// (contains zero length body but file contains actually some data)
						if (chunkLen == 0)
						{
							sampleDataLen = iffSize - f.pos();
						}
						else
						{
							sampleDataLen = chunkLen;
						}
						
						if (vhdr.Oneshothi < chunkLen)
							vhdr.Oneshothi = chunkLen;
						
						if (sampleDataLen > (signed)vhdr.Oneshothi)
							sampleDataLen = vhdr.Oneshothi;
						else if (sampleDataLen < (signed)vhdr.Oneshothi)
							sampleDataLen = vhdr.Oneshothi;
						
						sampleData = new mp_sbyte[sampleDataLen];
						if (sampleData == NULL)
						{
							if (sampleData)
								delete[] sampleData;
							if (name)
								delete[] name;
							if (anno)
								delete[] anno;
							return MP_OUT_OF_MEMORY;
						}
						memset(sampleData, 0, sampleDataLen);
						
						f.read(sampleData, 1, (signed)chunkLen > sampleDataLen ? sampleDataLen : chunkLen);
						break;
					}
					
					case 1:
					case 2:
					{
						if (hires)
							goto bail;

						hasBODY = true;
					
		            	mp_sbyte *Body,*src,*dst;
						mp_ubyte d;
						mp_sbyte x;
						mp_sint32 i,n,lim;
						mp_sbyte fibtab[16] = { -34,-21,-13,-8,-5,-3,-2,-1,0,1,2,3,5,8,13,21 };
						mp_sbyte exptab[16] = {-128,-64,-32,-16,-8,-4,-2,-1,0,1,2,4,8,16,32,64};
						
						Body = new mp_sbyte[chunkLen];
						if (!Body)
						{
							if (sampleData)
								delete[] sampleData;
							if (name)
								delete[] name;
							if (anno)
								delete[] anno;
							return MP_OUT_OF_MEMORY;
						}
						
						f.read(Body, 1, chunkLen);
						
						sampleDataLen = chunkLen<<1;
						sampleData = new mp_sbyte[sampleDataLen];
						
						if (!sampleData)
						{
							if (sampleData)
								delete[] sampleData;
							if (name)
								delete[] name;
							if (anno)
								delete[] anno;
							if (Body)
								delete[] Body;
							return MP_OUT_OF_MEMORY;
						}
						
						/* Fibonacci Delta Decompression */
						mp_sbyte* CodeToDelta = NULL;
						if (vhdr.Compression == 1)
							 CodeToDelta = fibtab;
						/* Exponential Delta Decompression */
						else
							 CodeToDelta = exptab;
							
						src = Body+2;
						dst = sampleData;
						n = chunkLen-2;
						x = Body[1];
						lim = n << 1;
						for(i = 0; i < lim; ++i)
						{
							d = src[i >> 1];
							if(i & 1) d &= 0xF;
							else d >>= 4;
							x += CodeToDelta[d];
							dst[i] = x;
						}
						delete[] Body;
						
						if ((signed)vhdr.Oneshothi < sampleDataLen)
							vhdr.Oneshothi = sampleDataLen;
						
						break;
					}
					
					default:
						hasBODY = false;
						goto bail;
					
					
				}
				
				f.seek(pos + chunkLen);
				break;
			}
			
			default:
			{
bail:
				mp_uint32 pos = f.pos();
				f.seek(pos + chunkLen);
			}
		}
		
		// odd chunks are padded with zero
		if (chunkLen&1)
			f.readByte();
		
	}

	if (hasFORM && hasVHDR && hasBODY)
	{
		TXMSample* smp = &theModule.smp[index];

		if (smp->sample)
		{
			theModule.freeSampleMem((mp_ubyte*)smp->sample);
			smp->sample = NULL;
		}					
		
		smp->samplen = hires ? sampleDataLen >> 1 : sampleDataLen;
		
		smp->sample = (mp_sbyte*)theModule.allocSampleMem(hires ? (smp->samplen<<1) : smp->samplen);						
		if (smp->sample == NULL)
		{
			if (sampleData)
				delete[] sampleData;
			if (name)
				delete[] name;
			if (anno)
				delete[] anno;
			return MP_OUT_OF_MEMORY;						
		}

		memcpy(smp->sample, sampleData, sampleDataLen);
		
		if (hires)
		{
			// huuuu? 16 bit IFF samples are little endian? how stupid is that?
			mp_uword* ptr = (mp_uword*)smp->sample; 
			for (mp_uint32 i = 0; i < smp->samplen; i++)
			{
				*ptr = LittleEndian::GET_WORD(ptr);
				ptr++;
			}
		}
		
		delete[] sampleData;

		smp->loopstart = hires ? vhdr.Repeathi >> 1 : vhdr.Repeathi;
		smp->looplen = hires ? vhdr.Samplescycle >> 1 : vhdr.Samplescycle;
		mp_sint32 vol = (vhdr.Volume*255)>>16;
		smp->vol = (mp_ubyte)(vol > 255 ? 255 : vol); 
		smp->pan = 0x80;
		smp->flags = 3;
		
		memset(smp->name, 0, sizeof(smp->name));
		
		if (name || anno)
		{
			const char* buff = name ? (const char*)name : (const char*)anno;

			nameToSample(buff, smp);
			
			if (name)
				delete[] name;
			if (anno)
				delete[] anno;
		}
		else
		{
			nameToSample(preferredDefaultName, smp);
		}
		
		smp->type = 0;
		
		if (smp->looplen)
			smp->type |= 1;
		if (hires)
			smp->type |= 16;		
		
		XModule::convertc4spd(vhdr.Samplessec, &smp->finetune, &smp->relnote);
		
		if (smp->samplen > vhdr.Oneshothi)
			smp->samplen = vhdr.Oneshothi;
								
		return MP_OK;
	}

	delete[] name;
	
	delete[] sampleData;

	return MP_LOADER_FAILED;
}

static inline mp_uword swapW(mp_uword x)
{
	return (x>>8)+((x&255)<<8);
}

static inline mp_dword swapDW(mp_dword x)
{
	return (mp_dword)swapW(x>>16) | (mp_dword)swapW(x&0xFFFF) << 16;
}

mp_sint32 SampleLoaderIFF::saveSample(const SYSCHAR* fileName, mp_sint32 index)
{
	TXMSample* smp = &theModule.smp[index];

	XMFile f(fileName, true);

	f.write("FORM", 1, 4);
	
	bool hires = (smp->type & 16) != 0;
	
	mp_dword chunkLen = 70 + (hires ? smp-> samplen << 1 : smp-> samplen);
	f.writeDword(swapDW(chunkLen));
	
	f.write(hires ? "16SV" : "8SVX", 1, 4);
	
	f.write("NAME", 1, 4);
	f.writeDword(swapDW(22));
	f.write(smp->name, 1, 22);
	
	SVX8_Vhdr vhdr;	
	
	vhdr.Oneshothi = hires ? smp->samplen << 1 : smp->samplen;
	vhdr.Repeathi = (smp->type & 3) ? (hires ? smp->loopstart << 1 : smp->loopstart) : 0;
	vhdr.Samplescycle = (smp->type & 3) ? (hires ? smp->looplen << 1 : smp->looplen) : 0;	
	vhdr.Samplessec = XModule::getc4spd(smp->relnote,smp->finetune);
	vhdr.Octaves = 1;		
	vhdr.Compression = 0;
	vhdr.Volume = (smp->vol*65536)/255;
	
	f.write("VHDR", 1, 4);
	f.writeDword(swapDW(0x14));
	f.writeDword(swapDW(vhdr.Oneshothi));
	f.writeDword(swapDW(vhdr.Repeathi));
	f.writeDword(swapDW(vhdr.Samplescycle));
	f.writeWord(swapW(vhdr.Samplessec));
	f.writeByte(vhdr.Octaves);
	f.writeByte(vhdr.Compression);
	f.writeDword(swapDW(vhdr.Volume));
	
	f.write("BODY", 1, 4);
	f.writeDword(swapDW(hires ? smp->samplen << 16 : smp->samplen));	

	if (smp->type & 16)
	{
		for (mp_uint32 i = 0; i < smp->samplen; i++)
			f.writeWord(smp->getSampleValue(i));
	}
	else
	{
		for (mp_uint32 i = 0; i < smp->samplen; i++)
			f.writeByte(smp->getSampleValue(i));
	}
	
	return MP_OK;
}
