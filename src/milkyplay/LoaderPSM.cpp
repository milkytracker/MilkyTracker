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
 *  LoaderPSM.cpp
 *  MilkyPlay Module Loader: Version 1/2 of Epic MegaGames MASI
 */
#include "Loaders.h"
#include <stdio.h>

static bool PATTTest(mp_ubyte* p, mp_sint32& size)
{
	if (p[0] != 'P') return false;
	
	// type Pxxx
	if (p[1] != 'A')
	{
		if (p[1] < '0' || p[1] > '9') return false;
		if ((p[2] < '0' || p[2] > '9') && p[2] != ' ') return false;
		if ((p[3] < '0' || p[3] > '9') && p[3] != ' ') return false;
		size = 4;
		return true;
	}
	// type PATTxxxx
	else
	{
		if (p[2] != 'T' || p[3] != 'T')
			return false;

		if (p[4] < '0' || p[4] > '9') return false;
		if ((p[5] < '0' || p[5] > '9') && p[5] != ' ') return false;
		if ((p[6] < '0' || p[6] > '9') && p[6] != ' ') return false;
		if ((p[7] < '0' || p[7] > '9') && p[7] != ' ') return false;	
		size = 8;
		return true;
	}
}

#define RELEASE_PATTERNS { \
	for (mp_sint32 i = 0; i < 1024; i++) \
		if (patterns[i]) \
		{ \
			delete[] patterns[i]; \
				patterns[i] = NULL; \
		} \
		delete[] patterns; \
} 

const char* LoaderPSMv2::identifyModule(const mp_ubyte* buffer)
{
	// check for .PSM (new) module
	if (!memcmp(buffer,"PSM\x20",4)) 
	{
		return "PSMv2";
	}

	// this is not an .PSM
	return NULL;
}

mp_sint32 LoaderPSMv2::load(XMFileBase& f, XModule* module)
{
	module->cleanUp();
	
	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	

	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;
	
	// size for 1024 patterns
	// should be enough, really
	mp_ubyte** patterns = new mp_ubyte*[1024];
	if (patterns == NULL)
		return MP_OUT_OF_MEMORY;
	
	mp_uint32* patternSizes = new mp_uint32[1024];
	if (patternSizes == NULL)
	{
		delete[] patterns;
		return MP_OUT_OF_MEMORY;
	}
	
	for (mp_sint32 j = 0; j < 1024; j++)
		patterns[j] = NULL;
	
	memset(patternSizes,0,1024*sizeof(mp_uint32));
	
	//mp_uint32 songSize = 0;
	mp_uint32 patIndex = 0;
	mp_uint32 insIndex = 0;
	mp_uint32 smpIndex = 0;
	
	mp_ubyte* songSignature = NULL;
	mp_uint32 signatureSize = 0;
	
	bool sinaria = false;
	
	while (true)
	{
		
		mp_ubyte ID[4];
		
		mp_uint32 bytesRead = f.read(&ID,4,1);
		
		if (bytesRead != 4)
			break;
		
		switch (LittleEndian::GET_DWORD(ID))
		{
			// 'PSM '
			case 0x204d5350:
				
				memcpy(header->sig,"PSM",3);
				
				f.readDword();	// consume chunk size (song size)
				break;
				
			// 'FILE'
			case 0x454c4946:	// just eat ID
				break;
				
			// 'SDFT'
			case 0x54464453:	// whatever this is
				signatureSize = f.readDword();	// get chunk size
				
				delete[] songSignature;
				songSignature = new mp_ubyte[signatureSize];
				// typically size is 8 and buffer will contain "MAINSONG"
				f.read(songSignature,1,signatureSize);
				break;
				
			// 'TITL'
			case 0x4c544954:	// title chunk... song contains title
			{
				mp_uint32 size = f.readDword();	// get chunk size
				
				mp_ubyte* buffer = new mp_ubyte[size];
				
				f.read(buffer,1,size);
				
				// just make sure title fits into our name field
				if (size > sizeof(header->name))
					size = sizeof(header->name);
				
				memcpy(header->name,buffer,size);
				
				delete[] buffer;
				
				break;
			}
				
			// 'PBOD'
			case 0x444f4250:	// pattern chunk
			{
				mp_uint32 size = f.readDword();	// get chunk size
				
				patterns[patIndex] = new mp_ubyte[size];
				
				patternSizes[patIndex] = size;
				
				f.read(patterns[patIndex++],1,size);
				
				break;
			}
				
			// 'SONG'
			case 0x474E4F53:	// song chunk
			{
				f.readDword();	// consume chunk size
				
				mp_ubyte buffer[9]; 
				f.read(buffer,1,8); buffer[8] = '\0';
				/*if (memcmp(buffer,songSignature,signatureSize)!=0)
				{
					printf("Something went wrong here!");
					exit(0);
				}
				delete buffer;*/
				
#ifdef VERBOSE
				printf("Song (pos: %i): %s\n",header->ordnum,buffer);
#endif
				
				f.readByte();	// what's this?
				f.readByte();	// what's this?
				
				mp_ubyte numchannels = f.readByte(); 
				
				if (numchannels > header->channum)
					header->channum = numchannels;
				
				break;
			}
				
			// 'OPLH'
			case 0x484c504f:
			{
				mp_uint32 size = f.readDword();
				
				// the following is a little bit messy, but i dunno all the fields
				// so the data i'll need is looked up by a brute force search
				mp_ubyte* buffer = new mp_ubyte[size]; 
				
				f.read(buffer,1,size);
				
				bool helperPattern = false;
				
				mp_uint32 i = 0;
				for (;;) 
				{
					if (buffer[i] == 0x07 && buffer[i+2] == 0x08)
					{
						i++;
						if (header->ordnum == 0)
						{
							header->tempo = buffer[i];	// tickspeed
							header->speed = buffer[i+2]; // BPM
						}
						// if there are more then 1 songs contained in the file we need to create a little
						// helper pattern to set speed and tempo for that particular song
						else
						{
							char patSig[10];
							sprintf(patSig, "P%i  ", patIndex % 999);
						
							patterns[patIndex] = new mp_ubyte[20];
							memset(patterns[patIndex],0,20);
							memcpy(patterns[patIndex]+4, patSig, 4);
							patternSizes[patIndex] = 20;
							
							mp_ubyte* pattern = patterns[patIndex];

							pattern[0] = 0xFF;
							pattern[1] = 0xFF;
							pattern[2] = 0xFF;
							pattern[3] = 0xFF;

							pattern[8] = 0x01;	// just one row
							pattern[10] = 0x0A;	// 10 bytes in size
							pattern[12] = 0x10; // read effect + operand 
							pattern[13] = 0x00;	// channel 0 and 
							pattern[14] = 0x3E; // set BPM
							pattern[15] = buffer[i+2];
							pattern[16] = 0x10; // read effect + operand 
							pattern[17] = 0x01; // channel 1 
							pattern[18] = 0x3D; // set speed
							pattern[19] = buffer[i];
							
							helperPattern = true;
							
							patIndex++;
						}
						break;
					}
					i++;
					
					if (i >= size)
						break;
					
				}
				
				mp_sint32 patIDSize = 0;
				while (!PATTTest(buffer+i+1, patIDSize) && i < size)
					i++;
				
				if (patIDSize == 8)
					sinaria = true;
				
				// hier lief was schief
				if (i >= size || (patIDSize != 4 && patIDSize != 8))
				{
					delete[] buffer;
					RELEASE_PATTERNS;
					return MP_LOADER_FAILED;
				}
				
				//header->ordnum = 0;
				// place helper pattern right before the song
				if (header->ordnum != 0 && helperPattern)
					header->ord[header->ordnum++] = patIndex-1; 
				
				while (buffer[i+1]=='P' && i < (size-5))
				{					
					for (mp_uint32 j = 0; j < patIndex; j++)
					{
						if (memcmp(buffer+i+1,patterns[j]+4,patIDSize) == 0)
						{
							header->ord[header->ordnum] = j;
							break;
						}						
					}
					i+=patIDSize+1;
					header->ordnum++;
				}
				
				delete[] buffer;
				
				break;
			}
				
			// 'DATE'
			case 0x45544144:	// date chunk (ASCII string)
			// 'PPAN'
			case 0x4E415050:	// panning found in sinaria PSM modules
			// 'PATT'
			case 0x54544150:	// list of used patterns? pretty useless to me...
			// 'DSAM'
			case 0x4d415344:	// another useless chunk...
			{
				mp_uint32 size = f.readDword();				
				// read and discard
				mp_ubyte* buffer = new mp_ubyte[size]; 
				f.read(buffer,1,size);
				delete[] buffer;				
				break;
			}
				
			// 'DSMP'
			case 0x504d5344:
			{
				mp_uint32 size = f.readDword();
				
				mp_uint32 infoSize = size>96 ? 96:size;
				
				// read sample info first
				mp_ubyte* buffer = new mp_ubyte[infoSize]; 	
				f.read(buffer,1,infoSize);
				
				if (infoSize == 96)
				{
					mp_sint32 offsetName = 13;
					mp_sint32 offsetSamplen = 54;
					mp_sint32 offsetLoopstart = 58;
					mp_sint32 offsetLoopEnd = 62;
					mp_sint32 offsetVolume = 68;
					mp_sint32 offsetC4spd = 73;
					
					if (sinaria)
					{
						offsetName += 4;
						offsetSamplen += 4;
						offsetLoopstart += 4;
						offsetLoopEnd += 4;
						offsetVolume += 5;
						offsetC4spd += 5;
					}
					
					memcpy(instr[insIndex].name, buffer+offsetName, 24);
					
					if (size > 96)
					{
						
						instr[insIndex].samp = 1;				// one sample used
						for (mp_sint32 j = 0; j < 120; j++) 
							instr[insIndex].snum[j] = smpIndex;	// build sample table
						
						smp[smpIndex].samplen = (mp_sint32)LittleEndian::GET_DWORD(buffer+offsetSamplen);		// sample size
						smp[smpIndex].loopstart = (mp_sint32)LittleEndian::GET_DWORD(buffer+offsetLoopstart);		// loop start
						smp[smpIndex].looplen = ((mp_sint32)LittleEndian::GET_DWORD(buffer+offsetLoopEnd)) - 
							((mp_sint32)LittleEndian::GET_DWORD(buffer+offsetLoopstart))+1;	// loop length
						
						mp_sint32 finetune = 0;
						if (sinaria)
							finetune = buffer[offsetVolume-1] & 0xf;
						
						smp[smpIndex].vol = module->vol127to255(buffer[offsetVolume]);		// volume
						
						smp[smpIndex].flags = 1;						// set volume flag
						
						if (buffer[0] & 0x80)
						{
							smp[smpIndex].type = 1;					// looped sample
																	// correct looplen if necessary
							if ((smp[smpIndex].loopstart+smp[smpIndex].looplen)>smp[smpIndex].samplen)
								smp[smpIndex].looplen-=(smp[smpIndex].loopstart+smp[smpIndex].looplen)-smp[smpIndex].samplen;
						}
						
						mp_uint32 c4speed = (mp_uword)LittleEndian::GET_WORD(buffer+offsetC4spd); 
						mp_uint32 ftC4Speed = module->sfinetunes[finetune];
			
						module->convertc4spd(c4speed*ftC4Speed/8448,
											 &smp[smpIndex].finetune,&smp[smpIndex].relnote);						

						//module->convertc4spd((mp_uword)LittleEndian::GET_WORD(buffer+offsetC4spd),&smp[smpIndex].finetune,&smp[smpIndex].relnote);
						
						ASSERT(smp[smpIndex].samplen+96 == size);
						
						if (module->loadModuleSample(f, smpIndex, XModule::ST_DELTA) != 0)
						{
							delete[] buffer;
							RELEASE_PATTERNS;
							return MP_OUT_OF_MEMORY;
						}
						
						//if (sinaria)
						//{
						//	if (smp[smpIndex].samplen > 2)
						//		smp[smpIndex].samplen-=2;
						//	if (smp[smpIndex].looplen > 2)
						//		smp[smpIndex].looplen-=2;
						//}
						
						smpIndex++;
						
					}
					
					insIndex++;
					
				}
				
				delete[] buffer;
				
				break;
			}
				
			default:
#ifdef VERBOSE
				printf("Unknown chunk: %X\n",ID);
#endif
				return MP_LOADER_FAILED;
				break;
				
		}
		
	}
	
	if (songSignature)
		delete[] songSignature;
	
	mp_ubyte* pattern = new mp_ubyte[32*256*5];
	
	if (pattern == NULL)
	{
		RELEASE_PATTERNS;
		return MP_OUT_OF_MEMORY;
	}
	
	for (mp_uint32 i = 0; i < patIndex; i++)
	{
		
		for (mp_sint32 j = 0; j < 32*256; j++)
		{
			pattern[j*5] = 0xFF;
			pattern[j*5+1] = 0xFF;
			pattern[j*5+2] = 0xFF;
			pattern[j*5+3] = 0;
			pattern[j*5+4] = 0;
		}
		
		mp_sint32 patIDSize = 0;
		if (!PATTTest(patterns[i]+4, patIDSize))
		{
			RELEASE_PATTERNS;
			return MP_LOADER_FAILED;
		}
		
		mp_sint32 offset = 6+patIDSize;
		mp_ubyte* packed = patterns[i]+offset;
		
		mp_uint32 index = 0;
		
		mp_uint32 row = 0;
		
		mp_uint32 maxChannels = 0;
		
		while (index<(patternSizes[i]-offset))
		{
			
			mp_uint32 size = ((mp_uword)LittleEndian::GET_WORD(packed+index))-2;
			index+=2;	// advance pointer
			
			mp_uint32 dstIndex = index+size;
			
			if (size)
			{
				
				do {
					
					mp_ubyte pi = packed[index++];
					
					mp_uint32 chn = packed[index++];
					
					if (chn>maxChannels)
						maxChannels = chn;
					
					mp_ubyte* slot = pattern+(row*32*5)+chn*5;
					
					// note 
					if (pi & 0x80)
					{
						mp_ubyte note = packed[index++];
						
						// key off note
						if (note == 255) 
							note = 254;
						else if (patIDSize == 8)
						{
							note = ((note+23) / 12) * 16 + (((note+23)) % 12);
						}
						
						slot[0] = note;
					}
					// instrument
					if (pi & 0x40)
					{
						slot[1] = packed[index++];
					}
					// volume
					if (pi & 0x20)
					{
						slot[2] = packed[index++];
					}
					// effect & operand
					if (pi & 0x10)
					{
						slot[3] = packed[index++];
						slot[4] = packed[index++];
						
						if (slot[3] == 0x29)
						{
							slot[4] = packed[index+=2];
						}
						else if (slot[3] == 0x2D)
						{
							slot[4] = packed[index+=3];
						}
						
					}
					
				} while (index < dstIndex);
				
#ifdef VERBOSE
				if (index!=dstIndex)
					printf("nagnag ");
#endif
				
			}
			
			row++;			
		}
		delete[] patternSizes;

		maxChannels++;
		
		if (maxChannels > header->channum)
			maxChannels = header->channum;
		
		// convert pattern here:
		phead[i].rows = (mp_uword)LittleEndian::GET_WORD(patterns[i]+offset-2);
		phead[i].effnum = 3;
		phead[i].channum = maxChannels;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*maxChannels*8];
		
		if (phead[i].patternData == NULL)
		{
			delete[] pattern;
			RELEASE_PATTERNS;
			return MP_OUT_OF_MEMORY;							
		}
		
		memset(phead[i].patternData,0,maxChannels*8*phead[i].rows);
		
		mp_ubyte* dstSlot = phead[i].patternData;
		for (row = 0; row < phead[i].rows; row++)
		{
			for (mp_uint32 c = 0; c < maxChannels; c++)
			{
				const mp_ubyte* srcSlot = pattern+row*32*5+c*5;
				
				mp_ubyte note = srcSlot[0];
				
				mp_ubyte finalNote = 0;
				
				if (note<254)
				{
					finalNote = ((note>>4)*12+(note&0x0f))+1;
					
					if (finalNote>120)
					{
#ifdef VERBOSE
						printf("Wrong note: %i",finalNote);
#endif
						finalNote = 0;
					}
				}
				else if (note==254)
				{
					finalNote = 122; // key off, s3m style
				}
				
				dstSlot[0] = finalNote;
				
				if (srcSlot[1] != 0xFF)
					dstSlot[1] = srcSlot[1]+1;
				
				if (srcSlot[2]<=128)
				{
					
					dstSlot[2] = 0xC;
					dstSlot[3] = XModule::vol127to255(srcSlot[2]);
					
				}
				
				mp_ubyte eff = srcSlot[3];
				mp_ubyte op = srcSlot[4];
				
				switch (eff)
				{
					// set sample offset
					case 0x29:
						eff = 0x09;
						break;
						
						// set BPM
					case 0x3D:
						eff = 0x1C;
						break;
						
						// set tickspeed
					case 0x3E:
						eff = 0x16;
						break;
						
						// pattern break
					case 0x34:
						eff = 0x0D;
						break;
						
						// vibrato
					case 0x15:
						eff = 0x04;
						break;
						
						// tone porta
					case 0x0F:
						eff = 0x03;
						break;
						
						// arpeggio
					case 0x47:
						eff = 0x20;
						break;
						
						// portamento up
					case 0x0C:
						eff = 0x4D;
						break;
						
						// portamento down
					case 0x0E:
						eff = 0x4E;
						break;
						
						// volume slide up = MDL volslide up
					case 02:
						eff = 0x45;
						op<<=1;
						break;
						
						// volume slide down = MDL volslide down
					case 04:
						eff = 0x46;
						op<<=1;
						break;
						
						// extra fine volume slide up
					case 01:
						eff = 0x4B;
						op<<=1;
						break;
						
						// extra fine volume slide down
					case 03:
						eff = 0x4C;
						op<<=1;
						break;
						
						// retrig note
					case 0x2A:
						eff = 0x1B;
						break;
						
						// fine portamento up
					case 11:
						eff = 0x31;
						break;
						
						// fine portamento down
					case 13:
						eff = 0x32;
						break;
						
						// note-cut
					case 0x2B:
						eff = 0x3C;
						break;
						
						// note-delay
					case 0x2C:
						eff = 0x3D;
						break;
						
						// jump loop
					case 0x35:
						eff = 0x36;
						break;
						
						// pattern delay
					case 0x36:
						eff = 0x3E;
						break;
						
						// position jump
					case 50:
						eff = 0x0B;
						break;
						
						/*// tone porta + volslide
						case 17:
							eff = 0x05;
							if (op) op >>= 1;
								break;*/
									
						// tone porta + volslide up
					case 16:
						dstSlot[6] = 0x45;
						dstSlot[7] = op<<1;
						
						eff = 0x03;
						op = 0;
						break;
						
						// tone porta + volslide down
					case 17:
						dstSlot[6] = 0x46;
						dstSlot[7] = op<<1;
						
						eff = 0x03;
						op = 0;
						break;
						
						// vibrato + volslide up
					case 0x17:
						dstSlot[6] = 0x45;
						dstSlot[7] = op<<1;
						
						eff = 0x04;
						op = 0;
						break;
						
						// vibrato + volslide down
					case 0x18:
						dstSlot[6] = 0x46;
						dstSlot[7] = op<<1;
						
						eff = 0x04;
						op = 0;
						break;
						
						// tremolo
					case 0x1F:
						eff = 0x07;
						break;
						
						// set panning
					case 0x49:
						eff = 0x08;
						break;
						
					default:
#ifdef VERBOSE
						if (eff != 0)
						{
							printf("%i, %x, %x\n",i,eff,op);
						}
#endif
						//eff = op = 0;
						break;
				}
				
				//eff = op = 0;
				
				/*if (i == 13 && c == 0)
				{
					printf("%x, %x, %x, %x\n",finalNote,dstSlot[1],eff,op);
				}*/
				
				dstSlot[4] = eff;
				dstSlot[5] = op;
				
				dstSlot+=8;
			}
			
		}
		
		bool dummyPattern = (*(patterns[i]) == 0xFF 
							 && *(patterns[i]+1) == 0xFF  
							 && *(patterns[i]+2) == 0xFF  
							 && *(patterns[i]+3) == 0xFF);
		
		// mark song start with effect 0x1E + operand 0xFF
		if (dummyPattern && phead[i].rows == 1)
		{
			phead[i].patternData[2] = XModule::SubSongMarkEffect;
			phead[i].patternData[3] = XModule::SubSongMarkOperand;
		}
			
	}
		
	delete[] pattern;
	
	RELEASE_PATTERNS;
	
	header->smpnum = smpIndex;
	header->insnum = insIndex;
	header->patnum = patIndex;
	
	header->mainvol = 255;
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	strcpy(header->tracker,"Epic Megagames MASI <new>");
	
	return MP_OK;
}

//////////////////////////////////////////////////////////////////////////
// Old epic pinball PSM loader. Not much tested yet					    //
// Warning: This is an one-by-one conversion of an assembler version ;) //
//////////////////////////////////////////////////////////////////////////
#define MEMREAD(dst,size,count) \
memcpy(dst,buffer+memPos,size*count); \
memPos+=size*count

#define MEMSEEK(position) \
memPos=position;

#define MEMADVANCE(amount) \
memPos+=amount;

const char* LoaderPSMv1::identifyModule(const mp_ubyte* buffer)
{
	// check for .PSM (old) module
	if (!memcmp(buffer,"PSM\xFE",4)) 
	{
		return "PSMv1";
	}

	// this is not an .PSM
	return NULL;
}

mp_sint32 LoaderPSMv1::load(XMFileBase& f, XModule* module)
{
	mp_uint32 memPos = 0;
	
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	

	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;	
	
	mp_ubyte* buffer = new mp_ubyte[f.sizeWithBaseOffset()];
	
	if (buffer == NULL)
		return MP_OUT_OF_MEMORY;
	
	f.read(buffer,1,f.sizeWithBaseOffset());
	
	MEMSEEK(0);
	
	MEMREAD(header->sig, 1, 3);	// PSM signature
	
	MEMADVANCE(1);
	
	MEMREAD(header->name,1,20);	// Don't know how long this is actually
	
	header->tempo = buffer[67];	// Tickspeed
	
	header->speed = buffer[68];  // BPM
	
	header->mainvol = 255;
	
	header->ordnum = buffer[72];
	
	header->insnum = header->smpnum = buffer[76];
	
	header->patnum = buffer[74];
	
	MEMSEEK((mp_sint32)LittleEndian::GET_DWORD(buffer+82));
	
	MEMREAD(header->ord,1,header->ordnum);
	
	MEMSEEK((mp_sint32)LittleEndian::GET_DWORD(buffer+94));
	
	mp_uint32 samplePtrs[256];
	
	mp_uint32 maxInstruments = 0;
	
	mp_sint32 i;
	
	for (i = 0; i < header->insnum; i++)
	{
		
		mp_ubyte ins[64];
		MEMREAD(ins,1,64);
		
		// sample size
		if ((mp_sint32)LittleEndian::GET_DWORD(ins+48) > 0)
		{
			mp_uint32 insIndex = ins[45]-1;
			
			if (insIndex > maxInstruments)
				maxInstruments = insIndex;
			
			memcpy(smp[i].name,ins,12);
			
			memcpy(instr[insIndex].name,ins+13,22);
			
			samplePtrs[i] = (mp_sint32)LittleEndian::GET_DWORD(ins+37);	// store sample offset
			
			instr[insIndex].samp = 1;			// one sample used
			for (mp_sint32 j = 0; j < 120; j++) 
				instr[insIndex].snum[j] = i;	// build sample table
			
			smp[i].samplen = (mp_sint32)LittleEndian::GET_DWORD(ins+48);	// sample size
			smp[i].loopstart = (mp_sint32)LittleEndian::GET_DWORD(ins+52); // loop start
			smp[i].looplen = ((mp_sint32)LittleEndian::GET_DWORD(ins+56)) - ((mp_sint32)LittleEndian::GET_DWORD(ins+52));	// loop length
			
			smp[i].vol = module->vol64to255(ins[61]);	// volume
			
			smp[i].flags = 1;					// set volume flag
			
			if ((mp_sint32)LittleEndian::GET_DWORD(ins+56) > 0 && (ins[47]&0x80))
			{
				smp[i].type = 1;				// looped sample
			}
			
			mp_uint32 c4speed = ((mp_uword)LittleEndian::GET_WORD(ins+62)); 
			mp_sint32 finetune = ins[60]&0xf;
			mp_uint32 ftC4Speed = module->sfinetunes[finetune];
			
			module->convertc4spd(c4speed*ftC4Speed/8363,
								  &smp[i].finetune,&smp[i].relnote);						
		}
		
	}
	
	maxInstruments++;
	
	header->insnum = maxInstruments;
	
	MEMSEEK((mp_sint32)LittleEndian::GET_DWORD(buffer+90));
	
	//////////////////////
	// read patterns	//
	//////////////////////
	mp_ubyte* pattern = new mp_ubyte[256*32*5];
	
	if (pattern == NULL)
	{
		delete[] buffer;
		return MP_OUT_OF_MEMORY;
	}
	
	for (i = 0; i < header->patnum; i++)
	{
		for (mp_sint32 j = 0; j < 32*256; j++)
		{
			pattern[j*5] = 0x00;
			pattern[j*5+1] = 0x00;
			pattern[j*5+2] = 0xFF;
			pattern[j*5+3] = 0x00;
			pattern[j*5+4] = 0x00;
		}
		
		mp_uword size;
		mp_ubyte numRows, numChannels;
		
		MEMREAD(&size,2,1);
		
		size = LittleEndian::GET_WORD((mp_ubyte*)&size);
		
		MEMREAD(&numRows,1,1);
		MEMREAD(&numChannels,1,1);
		
		mp_ubyte* packed = new mp_ubyte[size-4+5];
		if (packed == NULL)
		{
			delete[] buffer;
			delete[] pattern;
			return MP_OUT_OF_MEMORY;				
		}
		memset(packed,0,size-4+5);
		
		MEMREAD(packed,1,size-4);
		
		mp_uint32 index = 0;
		mp_uint32 row = 0;
		
		mp_uint32 maxChannels = 0;
		
		while (index<size)
		{
			
			mp_ubyte pi = packed[index++];
			
			if (pi == 0) {
				row++;
			}
			
			mp_uint32 chn = pi&31;
			
			if (chn>maxChannels)
				maxChannels = chn;
			
			mp_ubyte* slot = pattern+(row*32*5)+chn*5;
			
			if (pi & 128)
			{
				slot[0] = packed[index++];
				slot[1] = packed[index++];
			}
			if (pi & 64)
			{
				slot[2] = packed[index++];
			}
			if (pi & 32)
			{
				slot[3] = packed[index++];
				slot[4] = packed[index++];
			}
			
		}
		
		maxChannels++;
		
		if (maxChannels > numChannels)
			maxChannels = numChannels;
		
		if (maxChannels > header->channum)
			header->channum = maxChannels;
		
		delete[] packed;
		
		phead[i].rows = numRows;
		phead[i].effnum = 3;
		phead[i].channum = maxChannels;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*maxChannels*8];
		
		if (phead[i].patternData == NULL)
		{
			delete[] buffer;
			delete[] pattern;
			return MP_OUT_OF_MEMORY;							
		}
		
		memset(phead[i].patternData,0,maxChannels*8*phead[i].rows);
		
		mp_ubyte* dstSlot = phead[i].patternData;
		for (row = 0; row < phead[i].rows; row++)
			for (mp_uint32 c = 0; c < maxChannels; c++)
			{
				const mp_ubyte* srcSlot = pattern+row*32*5+c*5;
				
				mp_ubyte note = srcSlot[0];
				
				if (note && (note!=254)) 
					note+=24;
				else if (note == 254)
					note = 122;
				
				dstSlot[0] = note;
				
				dstSlot[1] = srcSlot[1];
				
				if (srcSlot[2]<=64)
				{
					
					dstSlot[2] = 0xC;
					dstSlot[3] = XModule::vol64to255(srcSlot[2]);
					
				}
				
				mp_ubyte eff = srcSlot[3];
				mp_ubyte op = srcSlot[4];
				
				switch (eff)
				{
					// fine volume slide up
					case 01:
						eff = 0x3A;
						break;
						
						// volume slide up = MDL volslide up
					case 02:
						eff = 0x45;
						op<<=2;
						break;
						
						// fine volume slide down
					case 03:
						eff = 0x3B;
						break;
						
						// volume slide down = MDL volslide down
					case 04:
						eff = 0x46;
						op<<=2;
						break;
						
						// fine portamento up
					case 10:
						eff = 0x31;
						break;
						
						// portamento up
					case 11:
						eff = 0x01;
						break;
						
						// fine portamento down
					case 12:
						eff = 0x32;
						break;
						
						// portamento down
					case 13:
						eff = 0x02;
						break;
						
						// tone porta
					case 14:
						eff = 0x03;
						break;
						
						// glissando control
					case 15:
						eff = 0x33;
						break;
						
						// tone porta + volslide up
					case 16:
						dstSlot[6] = 0x45;
						dstSlot[7] = op<<2;
						
						eff = 0x03;
						op = 0;
						break;
						
						// tone porta + volslide down
					case 17:
						dstSlot[6] = 0x46;
						dstSlot[7] = op<<2;
						
						eff = 0x03;
						op = 0;
						break;
						
						// vibrato
					case 20:
						eff = 0x04;
						break;
						
						// set vibrato waveform
					case 21:
						eff = 0x34;
						break;
						
						// vibrato + volslide up
					case 22:
						dstSlot[6] = 0x45;
						dstSlot[7] = op<<2;
						
						eff = 0x04;
						op = 0;
						break;
						
						// vibrato + volslide down
					case 23:
						dstSlot[6] = 0x46;
						dstSlot[7] = op<<2;
						
						eff = 0x04;
						op = 0;
						break;
						
						// tremolo
					case 30:
						eff = 0x07;
						break;
						
						// set tremolo waveform
					case 31:
						eff = 0x37;
						break;
						
						// set sample offset
					case 40:
						eff = 0x09;
						break;
						
						// retrig note
					case 41:
						eff = 0x1B;
						break;
						
						// note-cut
					case 42:
						eff = 0x3C;
						break;
						
						// note-delay
					case 43:
						eff = 0x3D;
						break;
						
						// position jump
					case 50:
						eff = 0x0B;
						break;
						
						// pattern break
					case 51:
						eff = 0x0D;
						break;
						
						// jump loop
					case 52:
						eff = 0x36;
						break;
						
						// pattern delay
					case 53:
						eff = 0x3E;
						break;
						
						// set tickspeed
					case 60:
						eff = 0x1C;
						break;
						
						// set BPM
					case 61:
						eff = 0x16;
						break;
						
						// arpeggio
					case 70:
						eff = 0x20;
						break;
						
						// set finetune
					case 71:
						eff = 0x35;
						break;
						
						// set panning
					case 72:
						eff = 0x08;
						op*=17;
						break;
						
					default:
#ifdef VERBOSE
						if (eff != 0)
						{
							printf("%i, %x, %x\n",i,eff,op);
						}
#endif
						//eff = op = 0;
						break;
				}
				
				dstSlot[4] = eff;
				dstSlot[5] = op;
				
				dstSlot+=8;
			}
				
				
	}
		
	/*for (i = 0; i < header->ordnum; i++)
		printf("%i ",header->ord[i]);
	
	header->ord[0] = 0;
	header->ord[1] = 0;
	header->ord[2] = 6;*/
	
	delete[] pattern;
	
	for (i=0;i<header->smpnum;i++) {
		
		MEMSEEK(samplePtrs[i]);
		
		smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen);
		
		if (smp[i].sample == NULL)
		{
			delete[] buffer;
			return MP_OUT_OF_MEMORY;
		}
		
		MEMREAD(smp[i].sample,1,smp[i].samplen);
		
		mp_sbyte b1=0;
		for (mp_uint32 j = 0; j < smp[i].samplen; j++) 
			smp[i].sample[j] = b1+=smp[i].sample[j];
		
	}
	
	delete[] buffer;
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	strcpy(header->tracker,"Epic Megagames MASI <old>");
	
	return MP_OK;
}
