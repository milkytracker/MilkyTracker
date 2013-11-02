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
 *  LoaderAMS.cpp
 *  MilkyPlay Module Loader: Velvet Studio and Extreme Tracker formats
 *
 *  Created by Peter Barth on 17.12.04.
 *
 */
#include "Loaders.h"
#ifdef VERBOSE
	#include <stdio.h>
#endif

// Hm well, AMS can handle up to 7 effects but my player is only configured to handle 4
#define MAXEFFECTS MP_NUMEFFECTS

static void convertAMSEffects(mp_ubyte& effect, mp_ubyte& operand)
{
	switch (effect)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0D:
		case 0x0F:
			break;
		
		case 0x0E:
			effect = 0x30 + (operand >> 4);
			operand &= 0xF;
			
			// break sample loop
			if (effect == 0x38 && !operand)
			{
				effect = 0x4F;
				operand = 0x3;
			}
			
			break;
		case 0x08:
			operand = (mp_ubyte)XModule::pan15to255(operand);
			break;
		case 0x10:
			effect = 0x4F;
			if (operand > 2)
				effect = operand = 0;
			break;
		case 0x11:
			effect = 0x41;
			break;
		case 0x12:
			effect = 0x42;
			break;
		case 0x13:
			effect = 0x1B;
			break;
		case 0x18:
			operand = ((operand & 0xf) << 4) | (operand>>4);
			break;
		case 0x1A:

			if (operand & 0xF)
			{
				effect = 0x46;
				operand = (operand&0xF)<<1;
			}
			else if (operand >> 4)
			{
				effect = 0x45;
				operand = (operand>>4)<<1;
			}

			break;
		case 0x1C:
			effect = 0x50;
			operand = XModule::vol127to255(operand);
			break;

		case 0x1E:
			switch (operand>>4)
			{
				case 0x01:
					effect = 0x31;
					operand &= 0xF;
					break;
				case 0x02:
					effect = 0x32;
					operand &= 0xF;
					break;
				// extra fine volume slide up
				case 0x0A:
					effect = 0x4B;
					operand = (operand&0xf)<<1;
					break;
				// extra fine volume slide down
				case 0x0B:
					effect = 0x4C;
					operand = (operand&0xf)<<1;
					break;
				default:
#ifdef VERBOSE
					printf("Missing: E%x\n", operand>>4);
#endif
					break;
			};
			break;
		case 0x20:
			effect = 0x51;
			break;
		case 0x21:
			effect = 1;
			break;
		case 0x22:
			effect = 2;
			break;
		case 0x2A:
			effect = 0x1E;
			
			if (operand & 0xF)
			{
				operand = 0xB0 + ((((operand & 0xF) << 1) < 0x10) ? ((operand & 0xF) << 1) : 0xF);
			}
			else if (operand >> 4)
			{
				operand = 0xA0 + ((((operand >> 4) << 1) < 0x10) ? ((operand >> 4) << 1) : 0xF);
			}
			
			break;
		case 0x2C:
			effect = 0x10;
			operand = XModule::vol127to255(operand);
			break;		
		case 0x0C:
			operand = XModule::vol127to255(operand);
			break;
			
		default:
#ifdef VERBOSE
			printf("Missing: %x/%x\n", effect, operand);
#endif
			effect = operand = 0;
			break;
	}
	
}

mp_uint32 UnpackMethod1(mp_ubyte* input, mp_ubyte* output, mp_uint32 inputsize);

static mp_sint32 loadAMSSamples(XModule* module, XMFileBase& f)
{
	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMSample*		smp	   = module->smp;

	mp_sint32 i,j;

	for (i = 0; i < header->smpnum; i++)
	{	
		if (!(smp[i].flags & 64))
		{
	
			if (smp[i].flags & 128)
			{
				mp_sint32 pos = f.posWithBaseOffset();
				
				j = f.readDword();
				
				mp_sint32 packedSize = f.readDword();
				
				mp_ubyte* packedData = new mp_ubyte[j>(packedSize+9)?j:(packedSize*2+9)];
				
				if (packedData == NULL)
				{
					return MP_OUT_OF_MEMORY;
				}
				
				f.seekWithBaseOffset(pos);
				
				f.read(packedData, 1, packedSize + 9);
				
				smp[i].sample = (mp_sbyte*)module->allocSampleMem(j);
				
				if (smp[i].sample == NULL)
				{
					delete[] packedData;				
					return MP_OUT_OF_MEMORY;
				}
				
				UnpackMethod1(packedData, (mp_ubyte*)smp[i].sample, packedSize);
				
				if (smp[i].type&16)
				{
					mp_sword* buffer = (mp_sword*)smp[i].sample;
					
					for (j = 0; j < (signed)smp[i].samplen; j++)
					{
						mp_sword v = LittleEndian::GET_WORD(buffer); 
						*buffer++ = v;
					}
				}
				
				delete[] packedData;
			}
			else
			{
				if (!(smp[i].type&16)) {
					
					smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen);
					
					if (smp[i].sample == NULL)
					{
						return MP_OUT_OF_MEMORY;
					}
					
					if (!module->loadSample(f,smp[i].sample,smp[i].samplen,smp[i].samplen))
					{
						return MP_OUT_OF_MEMORY;
					}					
				}
				else {
					
					smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen*2);
					
					if (smp[i].sample == NULL)
					{
						return MP_OUT_OF_MEMORY;
					}
					
					if (!module->loadSample(f,smp[i].sample, smp[i].samplen*2, smp[i].samplen, XModule::ST_16BIT))
					{
						return MP_OUT_OF_MEMORY;
					}									
				}
			}
			
		}
	}

	return MP_OK;
}

const char* LoaderAMSv1::identifyModule(const mp_ubyte* buffer)
{
	// check for .AMS module
	if (!memcmp(buffer,"Extreme\x30\x1",9)) 
	{
		return "AMSv1";
	}

	return NULL;
}

mp_sint32 LoaderAMSv1::load(XMFileBase& f, XModule* module)
{
	mp_sint32 i,j;

	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	
	
	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;
	
	f.read(header->sig, 1, 7);

	if (f.readWord() != 0x130)
		return MP_LOADER_FAILED;

	header->mainvol = 0xff;
	
	memcpy(header->tracker, "Extreme Tracker", 15);
	
	i = f.readByte();
	
	header->channum = (i & 31) + 1;
	mp_ubyte effnum = i>>5;
	header->insnum = f.readByte();	
	header->patnum = f.readWord();
	header->ordnum = f.readWord();
	header->speed = 125;
	header->tempo = 6;

	f.readByte();

	i = f.readWord();
	f.seekWithBaseOffset(f.posWithBaseOffset() + i);

	mp_sint32 s = 0;
	for (i = 0; i < header->insnum; i++)
	{

		mp_sint32 smpLen = f.readDword();

		if (smpLen)
		{
			instr[i].samp = 1;
			for (j = 0; j < 120; j++)
				instr[i].snum[j] = s;
		}

		mp_sint32 loopStart = f.readDword();
		mp_sint32 loopEnd = f.readDword();
		mp_ubyte panPosFinetune = f.readByte();
		mp_sint32 c4spd = f.readWord();
		mp_ubyte volume = f.readByte();
		mp_ubyte infoByte = f.readByte();

		if (instr[i].samp)
		{
		
			smp[s].samplen = smpLen;
			smp[s].loopstart = loopStart;
			smp[s].looplen = loopEnd - loopStart;
			
			mp_sint32 newC4spd = XModule::getc4spd(0, XModule::modfinetunes[panPosFinetune & 0xF]);			
			newC4spd = (c4spd*newC4spd) / 8363;			
			XModule::convertc4spd(newC4spd, &smp[s].finetune, &smp[s].relnote);
			
			smp[s].vol = XModule::vol127to255(volume);
			
			if (loopEnd)
				smp[s].type = 1;

			// 16 bit sample
			smp[s].type |= (infoByte&128)>>3;
			
			// mark as packed sample (for now)
			smp[s].flags = ((infoByte&1) << 7) | 1;
			
			if (panPosFinetune >> 4)
			{
				smp[s].flags |= 2;
				smp[s].pan = (((panPosFinetune >> 4)-1)*1259226) >> 16;
			}
			else
			{
				smp[s].pan = 0x80;
			}
			
			s++;
		}
	}
	
	header->smpnum = s;

	i = f.readByte();
	
	f.read(header->name, 1, i); 
	
	for (j = 0; j < header->insnum; j++)
	{
		i = f.readByte();
		f.read(instr[j].name, 1, i);
	}

	for (j = 0; j < header->channum; j++)
	{
		i = f.readByte();
		f.seekWithBaseOffset(f.posWithBaseOffset()+i);
	}

	for (j = 0; j < header->patnum; j++)
	{
		i = f.readByte();
		f.seekWithBaseOffset(f.posWithBaseOffset()+i);
	}
	
	// read song message
	mp_sint32 size = f.readWord();
	
	mp_ubyte* packedSongMessage = new mp_ubyte[size];
	
	f.read(packedSongMessage, 1, size);
	
	j = 0;
	for (i = 0; i < size; i++)
	{
		if (packedSongMessage[i] & 0x80)
			j+=packedSongMessage[i] & 127;
		else
			j++;
	}

	char* unpackedSongMessage = new char[j+1];
	
	j = 0;
	for (i = 0; i < size; i++)
	{
		if (packedSongMessage[i] & 0x80)
		{
			mp_sint32 len = packedSongMessage[i] & 0x7f;
			for (mp_sint32 k = 0; k < len; k++)
				unpackedSongMessage[j++] = ' ';
		}
		else
			unpackedSongMessage[j++] = packedSongMessage[i];
	}
	
	unpackedSongMessage[j] = '\0';
				
	delete[] packedSongMessage;
	
	for (i = 0; i < j; i++)
	{
		char line[80];
		memset(line, 0, sizeof(line));
		
		if (j - i >= 75)
		{
			XModule::convertStr(line, unpackedSongMessage+i, 76, false);
			i+=75;
		}
		else
		{
			XModule::convertStr(line, unpackedSongMessage+i, j-i, false);
			i+=j-i;
		}
		
		module->addSongMessageLine(line);
	}	

	delete[] unpackedSongMessage;

#ifdef VERBOSE
	printf("%s",module->message);
#endif
		
	mp_uword* orders = new mp_uword[header->ordnum];
	
	f.readWords(orders, header->ordnum);
	
	if (header->ordnum > 255)
		header->ordnum = 255;
	
	for (i = 0; i < header->ordnum; i++)
		header->ord[i] = (mp_ubyte)orders[i];
	
	delete[] orders;
	
	for (i = 0; i < header->patnum; i++)
	{
		
		mp_sint32 patSize = f.readDword();
		
		phead[i].rows = 64;		
		phead[i].channum = (mp_ubyte)header->channum;		
		phead[i].effnum = effnum;
		
		if (phead[i].effnum > MAXEFFECTS)
			phead[i].effnum = MAXEFFECTS;
		
		mp_sint32 slotSize = phead[i].effnum*2 + 2;

		phead[i].patternData = new mp_ubyte[slotSize * phead[i].channum * phead[i].rows];
		
		if (phead[i].patternData == NULL)
		{
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData, 0, slotSize * phead[i].channum * phead[i].rows);
		
		if (patSize)
		{		
			mp_ubyte* pattern = new mp_ubyte[patSize*2];
		
			if (pattern == NULL)
			{
				return MP_OUT_OF_MEMORY;
			}
			
			memset(pattern, 0xff, patSize*2);
		
			f.read(pattern, 1, patSize);

			mp_ubyte fullRow[(7*2+2)*32];
			
			mp_sint32 patOfs = 0;
			mp_sint32 rowOfs = 0;
				
			memset(fullRow, 0, sizeof(fullRow));

			mp_sint32 rowCnt = 0;

			while (patOfs < patSize)
			{
				mp_ubyte ins = 0;
				mp_ubyte note = 0;
				mp_ubyte b = 0;
				mp_ubyte flag = 0;
			
				if ((b = pattern[patOfs++]) != 0xFF)
				{
					// channel
					rowOfs = (b & 31)*slotSize;
					
					flag = 1;
					
					// if bit 6 not set, read note and instrument
					if (!(b & 64))
					{
						note = pattern[patOfs++];
						ins = pattern[patOfs++];					
						
						flag = note>>7;
						note = note&127;
						
						if (note)
							note += 12;
						
						fullRow[rowOfs++] = note;
						fullRow[rowOfs++] = ins;						
					}
					else rowOfs+=2;
					
					if (flag)
					{
						
						mp_sint32 numfx = 0;
						while (true)
						{
							
							mp_ubyte command = pattern[patOfs++];
							// volume
							if (command & 64)
							{
								// add volume
								if (numfx < phead[i].effnum)
								{
									fullRow[rowOfs++] = 0x0C;
									fullRow[rowOfs++] = XModule::vol127to255((command&63)<<1);
								}
								else
								{
#ifdef VERBOSE
									printf("Skipping");
#endif
								}
							
								numfx++;
							}
							else
							{
								mp_ubyte operand = pattern[patOfs++];
								
								mp_ubyte effect = command & 63;
								
								convertAMSEffects(effect, operand);
								
								if (numfx < phead[i].effnum)
								{
									fullRow[rowOfs++] = effect;
									fullRow[rowOfs++] = operand;
								}
								else
								{
#ifdef VERBOSE
									printf("Skipping");
#endif
								}
								
								numfx++;
							}
							
							if (!(command & 128))
								break;
						}
						
					}
					
				}

				// row finished?

				if ((b & 128))
				{
					ASSERT(rowCnt < phead[i].rows);
					
					memcpy(phead[i].patternData + rowCnt * phead[i].channum * slotSize, fullRow, slotSize * phead[i].channum);
				
					memset(fullRow, 0, sizeof(fullRow));
					rowCnt++;
				}

			}
			
#ifdef VERBOSE
			printf("Pattern (%i) finished %i/%i, %i/%i\n", i, patOfs, patSize, rowCnt, phead[i].rows);				
#endif
			
			delete[] pattern;
		}
	}

	mp_sint32 result = loadAMSSamples(module, f);
	if (result != MP_OK)
		return result;

	module->setDefaultPanning();
	
	module->postProcessSamples();	
	
	return MP_OK;
}

const char* LoaderAMSv2::identifyModule(const mp_ubyte* buffer)
{
	// check for .AMS module
	if (!memcmp(buffer,"AMShdr\x1a",7)) 
	{
		return "AMSv2";
	}

	return NULL;
}

mp_sint32 LoaderAMSv2::load(XMFileBase& f, XModule* module)
{
	mp_sint32 i,j,k;

	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	
	
	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;
	
	f.read(header->sig, 1, 6);
	f.readByte();
	
	i = f.readByte();
	
	f.read(header->name, 1, i);
	
	mp_sint32 ver = f.readWord();
	if ((ver != 0x202) && (ver != 0x201))
		return MP_LOADER_FAILED;
	
	header->mainvol = 0xff;	
	memcpy(header->tracker, "Velvet Studio", 13);
	header->insnum = f.readByte();	
	header->patnum = f.readWord();	
	header->ordnum = f.readWord();
	
	if (ver == 0x202)
		header->speed = f.readWord() >> 8;
	else
		header->speed = f.readByte();		

	header->tempo = f.readByte();
	
	mp_sint32 defaultChannels, defaultCommands, defaultRows, flags;
	if (ver == 0x202)
	{
		defaultChannels = f.readByte();
		defaultCommands = f.readByte();
		defaultRows = f.readByte();
	
		flags = f.readWord();
	}
	else flags = f.readByte();
	
	header->freqtab = (flags>>6)&1;
	
	mp_ubyte* shadowInsLut = new mp_ubyte[header->insnum*16];
	
	mp_sint32 s = 0;
	for (i = 0; i < header->insnum; i++)
	{
		
		j = f.readByte();
		f.read(instr[i].name, 1, j);
		
		instr[i].samp = f.readByte();
		
		if (instr[i].samp)
		{
		
			mp_ubyte nbu[120];
			
			f.read(nbu, 1, 120);
			
			for (j = 0; j < 120; j++)
				instr[i].snum[j] = nbu[j] + s;

			mp_sint32 envSpeed;
			mp_sint32 susPt;
			mp_sint32 loopStart;
			mp_sint32 loopEnd;
			mp_sint32 numPts;
			mp_ubyte envelope[64*3];
			
			TEnvelope env;
			
			// read volume envelope
			envSpeed = f.readByte();
			susPt = f.readByte();
			loopStart = f.readByte();
			loopEnd = f.readByte();
			numPts = f.readByte();
			
			f.read(envelope, 3, numPts);

			if (numPts)
			{
				memset(&env,0,sizeof(env));
				mp_uword x = 0;
				for (j = 0; j < numPts; j++)
				{
					mp_uword dx = (mp_uword)envelope[j*3] + (((mp_uword)envelope[j*3+1] << 8) & 256);
					x+=dx;
					
					env.env[j][0] = x;
					env.env[j][1] = XModule::vol127to255(envelope[j*3+2]);
				}
				
				env.num = numPts;
				env.sustain = susPt;
				env.loops = loopStart;
				env.loope = loopEnd;
				env.speed = envSpeed;

				if (!module->addVolumeEnvelope(env)) 
				{
					delete[] shadowInsLut;
					return MP_OUT_OF_MEMORY;							
				}
			}
			
			// read panning envelope
			envSpeed = f.readByte();
			susPt = f.readByte();
			loopStart = f.readByte();
			loopEnd = f.readByte();
			numPts = f.readByte();
			
			f.read(envelope, 3, numPts);
			
			if (numPts)
			{
				memset(&env,0,sizeof(env));
				mp_uword x = 0;
				for (j = 0; j < numPts; j++)
				{
					mp_uword dx = (mp_uword)envelope[j*3] + (((mp_uword)envelope[j*3+1] << 8) & 256);
					x+=dx;
					
					env.env[j][0] = x;
					env.env[j][1] = envelope[j*3+2];
				}
				env.num = numPts;
				env.sustain = susPt;
				env.loops = loopStart;
				env.loope = loopEnd;
				env.speed = envSpeed;
				
				if (!module->addPanningEnvelope(env)) 
				{
					delete[] shadowInsLut;
					return MP_OUT_OF_MEMORY;			
				}
			}

			// read vibrato envelope
			envSpeed = f.readByte();
			susPt = f.readByte();
			loopStart = f.readByte();
			loopEnd = f.readByte();
			numPts = f.readByte();
			
			f.read(envelope, 3, numPts);

			if (numPts)
			{
				memset(&env,0,sizeof(env));
				mp_uword x = 0;
				for (j = 0; j < numPts; j++)
				{
					mp_uword dx = (mp_uword)envelope[j*3] + (((mp_uword)envelope[j*3+1] << 8) & 256);
					x+=dx;
					
					env.env[j][0] = x;
					env.env[j][1] = envelope[j*3+2];
				}
				env.num = numPts;
				env.sustain = susPt;
				env.loops = loopStart;
				env.loope = loopEnd;
				env.speed = envSpeed;
				
				if (!module->addVibratoEnvelope(env)) 
				{
					delete[] shadowInsLut;				
					return MP_OUT_OF_MEMORY;			
				}
			}			

			mp_sint32 shadowIns = f.readByte();
			instr[i].res = shadowIns;
			
			mp_sint32 vibAmpVolFade = f.readWord();
			
			mp_sint32 envFlags = f.readWord();
			
			// convert volume envelope flags
			if ((envFlags & 4) && module->numVEnvs)
			{
				module->venvs[module->numVEnvs-1].type |= 1;
			}
			if ((envFlags & 2) && module->numVEnvs)
			{
				module->venvs[module->numVEnvs-1].type |= 2;
			}
			if ((envFlags & 1) && module->numVEnvs)
			{
				module->venvs[module->numVEnvs-1].type |= 4;
			}
			if ((envFlags & 512) && module->numVEnvs)
			{
				module->venvs[module->numVEnvs-1].type |= 8;
			}

			// convert panning envelope flags
			if ((envFlags & 32) && module->numPEnvs)
			{
				module->penvs[module->numPEnvs-1].type |= 1;
			}
			if ((envFlags & 16) && module->numPEnvs)
			{
				module->penvs[module->numPEnvs-1].type |= 2;
			}
			if ((envFlags & 8) && module->numPEnvs)
			{
				module->penvs[module->numPEnvs-1].type |= 4;
			}
			if ((envFlags & 1024) && module->numPEnvs)
			{
				module->penvs[module->numPEnvs-1].type |= 8;
			}

			// convert vibrato envelope flags
			if ((envFlags & 256) && module->numVibEnvs)
			{
				module->vibenvs[module->numVibEnvs-1].type |= 1;
				module->vibenvs[module->numVibEnvs-1].type |= (vibAmpVolFade>>12)<<(8-2);
			}
			if ((envFlags & 128) && module->numVibEnvs)
			{
				module->vibenvs[module->numVibEnvs-1].type |= 2;
			}
			if ((envFlags & 64) && module->numVibEnvs)
			{
				module->vibenvs[module->numVibEnvs-1].type |= 4;
			}
			if ((envFlags & 2048) && module->numVibEnvs)
			{
				module->vibenvs[module->numVibEnvs-1].type |= 8;
			}
			
			for (j = 0; j < instr[i].samp; j++)
			{
				shadowInsLut[i*16+j] = s;
				//instr[i].extra[j] = s;
			
				k = f.readByte();
				
				f.read(smp[s].name, 1, k);
				
				mp_sint32 smpLen = f.readDword();
				mp_sint32 loopStart = f.readDword();
				mp_sint32 loopEnd = f.readDword();
				mp_sint32 smpRate = f.readWord();
				mp_ubyte panPosFinetune = f.readByte();
				mp_sint32 c4spd = f.readWord();
				// achtung signed byte
				mp_sbyte relNote = f.readByte();
				mp_ubyte volume = f.readByte();
				mp_ubyte infoByte = f.readByte();
				
				if (envFlags & 4)
					smp[s].venvnum = module->numVEnvs;

				if (envFlags & 32)
					smp[s].penvnum = module->numPEnvs;
				
				if (envFlags & 256)
					smp[s].vibenvnum = module->numVibEnvs;
				
				smp[s].volfade = (vibAmpVolFade & 4095) << 1;				
				
				smp[s].samplen = smpLen;
				smp[s].loopstart = loopStart;
				smp[s].looplen = loopEnd - loopStart;
				
				mp_sint32 newC4spd = XModule::getc4spd(relNote, XModule::modfinetunes[panPosFinetune & 0xF]);
				
				newC4spd = (c4spd*newC4spd) / 8363;
				
				XModule::convertc4spd(newC4spd, &smp[s].finetune, &smp[s].relnote);
				
				smp[s].vol = XModule::vol127to255(volume);
				
				// looping
				if ((infoByte&8) && !(infoByte&16))
					smp[s].type = 1;
				else if (infoByte&16)
					smp[s].type = 2;
					
				// 16 bit sample
				smp[s].type |= (infoByte&4)<<2;
				
				// reversed
				smp[s].type |= (infoByte&64)<<1;
				
				// mark as packed sample (for now)
				smp[s].flags = ((infoByte&1) << 7) | 1;
				smp[s].flags |= (shadowIns?64:0);
				
				if (panPosFinetune >> 4)
				{
					smp[s].flags |= 2;
					smp[s].pan = (((panPosFinetune >> 4)-1)*1259226) >> 16;
				}
				else
				{
					smp[s].pan = 0x80;
				}
				
				s++;
			}
		}
		
	}
	
	header->smpnum = s;

	header->volenvnum = module->numVEnvs;
	header->panenvnum = module->numPEnvs;
	header->vibenvnum = module->numVibEnvs;

	// skip composer, who needs to know the composer? :D
	i = f.readByte();
	
	f.seekWithBaseOffset(f.posWithBaseOffset() + i);
	
	// skip channel names
	for (j = 0; j < 32; j++)
	{
		i = f.readByte();
	
		f.seekWithBaseOffset(f.posWithBaseOffset() + i);
	}
	
	// read song message
	i = f.readDword() - 4;
	
	mp_ubyte* packedSongMessage = new mp_ubyte[i];
	
	f.read(packedSongMessage, 1, i);
	
	mp_sint32 size = LittleEndian::GET_DWORD(packedSongMessage);
	
	char* unpackedSongMessage = new char[size+1];
	
	mp_ubyte* srcPtr = packedSongMessage + 7;
	
	// preferred because of bounds check
	mp_sint32 dstPtr = 0;
	
	j = i-7;
	while (j>0)
	{
		if (*srcPtr == 255)
		{
			char c = *(++srcPtr);
			srcPtr++;
			for (k = 0; k < *srcPtr; k++)
				if (dstPtr < size)
					unpackedSongMessage[dstPtr++] = c;
			srcPtr++;
			j-=3;
		}
		else
		{
			if (dstPtr < size)
				unpackedSongMessage[dstPtr++] = *srcPtr++;
			j--;
		}
	}
	
	unpackedSongMessage[size] = '\0';

	delete[] packedSongMessage;	

	/*for (i = 0; i < size; i++)
	{
		printf("%c", unpackedSongMessage[i]);
		if ((i % 74) == 73)
			printf("\n");
	}*/
	
	for (i = 0; i < size; i++)
	{
		char line[80];
		memset(line, 0, sizeof(line));
		
		if (size - i >= 73)
		{
			XModule::convertStr(line, unpackedSongMessage+i, 74, false);
			i+=73;
		}
		else
		{
			XModule::convertStr(line, unpackedSongMessage+i, size-i, false);
			i+=size-i;
		}
		
		module->addSongMessageLine(line);
	}		
		
	delete[] unpackedSongMessage;
	
#ifdef VERBOSE
	printf("%s",module->message);
#endif

	mp_uword* orders = new mp_uword[header->ordnum];
	
	f.readWords(orders, header->ordnum);
	
	if (header->ordnum > 255)
		header->ordnum = 255;
	
	for (i = 0; i < header->ordnum; i++)
		header->ord[i] = (mp_ubyte)orders[i];
	
	delete[] orders;
	
	header->channum = 0;
	
	for (i = 0; i < header->patnum; i++)
	{
		
		mp_sint32 patSize = f.readDword();
		
		phead[i].rows = f.readByte()+1;
		
		mp_sint32 numEffectsChannels = f.readByte();
		
		phead[i].channum = (numEffectsChannels & 31) + 1;
		
		if (phead[i].channum > header->channum)
			header->channum = phead[i].channum;
		
		phead[i].effnum = numEffectsChannels >> 5;
		
		if (phead[i].effnum > MAXEFFECTS)
			phead[i].effnum = MAXEFFECTS;
		
		mp_sint32 slotSize = phead[i].effnum*2 + 2;

		phead[i].patternData = new mp_ubyte[slotSize * phead[i].channum * phead[i].rows];
		
		if (phead[i].patternData == NULL)
		{
			delete[] shadowInsLut;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData, 0, slotSize * phead[i].channum * phead[i].rows);
		
		j = f.readByte();
		
		f.seekWithBaseOffset(f.posWithBaseOffset() + j);
		
		patSize-=(3+j);
		
		if (patSize)
		{		
			mp_ubyte* pattern = new mp_ubyte[patSize*2];
		
			if (pattern == NULL)
			{
				delete[] shadowInsLut;
				return MP_OUT_OF_MEMORY;
			}
			
			memset(pattern, 0xff, patSize*2);
		
			f.read(pattern, 1, patSize);

			mp_ubyte fullRow[(7*2+2)*32];
			
			mp_sint32 patOfs = 0;
			mp_sint32 rowOfs = 0;
				
			memset(fullRow, 0, sizeof(fullRow));

			mp_sint32 rowCnt = 0;

			while (patOfs < patSize)
			{
				mp_ubyte ins = 0;
				mp_ubyte note = 0;
				mp_ubyte b = 0;
				mp_ubyte flag = 0;
			
				if ((b = pattern[patOfs++]) != 0xFF)
				{
					// channel
					rowOfs = (b & 31)*slotSize;
					
					flag = 1;
					
					// if bit 6 not set, read note and instrument
					if (!(b & 64))
					{
						note = pattern[patOfs++];
						ins = pattern[patOfs++];					
						
						/*if (ins == 0x1B)
						{
							printf("%i: %i\n",i,b&31);
						}*/

						flag = note>>7;
						note = note&127;
						
						if (note)
							note = (note == 1)?XModule::NOTE_OFF:(note-1);
						
						fullRow[rowOfs++] = note;
						fullRow[rowOfs++] = ins;						
					}
					else rowOfs+=2;
					
					if (flag)
					{
						
						mp_sint32 numfx = 0;
						while (true)
						{
							
							mp_ubyte command = pattern[patOfs++];
							// volume
							if (command & 64)
							{
								// add volume
								if (numfx < phead[i].effnum)
								{
									fullRow[rowOfs++] = 0x0C;
									fullRow[rowOfs++] = XModule::vol127to255((command&63)<<1);
								}
								else
								{
#ifdef VERBOSE
									printf("Skipping");
#endif
								}
							
								numfx++;
							}
							else
							{
								mp_ubyte operand = pattern[patOfs++];
								
								mp_ubyte effect = command & 63;
								
								convertAMSEffects(effect, operand);
								
								if (numfx < phead[i].effnum)
								{
									fullRow[rowOfs++] = effect;
									fullRow[rowOfs++] = operand;
								}
								else
								{
#ifdef VERBOSE
									printf("Skipping");
#endif
								}
								
								numfx++;
							}
							
							if (!(command & 128))
								break;
						}
						
					}
					
				}

				// row finished?

				if ((b & 128))
				{
					ASSERT(rowCnt < phead[i].rows);
					
					memcpy(phead[i].patternData + rowCnt * phead[i].channum * slotSize, fullRow, slotSize * phead[i].channum);
				
					memset(fullRow, 0, sizeof(fullRow));
					rowCnt++;
				}

			}
			delete[] pattern;
		}
	}
	
	if (loadAMSSamples(module, f) != 0)
	{
		delete[] shadowInsLut;
		return MP_OUT_OF_MEMORY;
	}

	// duplicate shadowed samples
	for (i = 0; i < header->smpnum; i++)
	{
		if ((smp[i].flags & 64))
		{
			for (j = 0; j < header->insnum; j++)
			{
				
				for (k = 0; k < instr[j].samp; k++)
					// that's where i am
					//if (instr[j].extra[k] == i)
					if (shadowInsLut[j*16+k] == i)
					{
						mp_sint32 shadowIns = instr[j].res-1;
						
						// copy sample
						mp_sint32 size = (smp[i].type & 16)?(smp[i].samplen<<1):smp[i].samplen;
						
						smp[i].sample = (mp_sbyte*)module->allocSampleMem(size);
						
						//memcpy(smp[i].sample, smp[instr[shadowIns].extra[k]].sample, size);
						memcpy(smp[i].sample, smp[shadowInsLut[shadowIns*16+k]].sample, size);
					}
				
			}		
		}
	}
	
	delete[] shadowInsLut;
	
	header->flags |= module->MODULE_AMSENVELOPES;
	
	module->setDefaultPanning();
	
	module->postProcessSamples();	
	
	return MP_OK;
	
}

///////////////////////////////////////////////////////////////////////////////////////////
// You better don't look at what's coming next:
// I was so lazy and i had to find a way to way to decompress AMS samples packed samples
// and all i had was this assembler source for unpacking so guess what? I converted
// the assembler stuff right into C++ using a little helper class for representing
// an x86 32bit register
// No more comments...
///////////////////////////////////////////////////////////////////////////////////////////
struct Reg
{
	mp_ubyte hh,hl,lh,ll;

	Reg() 
	{
		hh = hl = lh = ll = 0;
	}
	
	mp_uint32 ex()
	{
		return (mp_uint32)ll +
			   (mp_uint32)(lh << 8) +
			   (mp_uint32)(hl << 16) +
			   (mp_uint32)(hh << 24);		
	}
	
	mp_uword x()
	{
		return (mp_uword)ll +
			   (mp_uword)(lh << 8);
	}

	void setEx(mp_uint32 v)
	{
		hh = (mp_ubyte)(v>>24);
		hl = (mp_ubyte)(v>>16);
		lh = (mp_ubyte)(v>>8);		
		ll = (mp_ubyte)(v);
	}

	void setX(mp_uword v)
	{
		lh = (mp_ubyte)(v>>8);		
		ll = (mp_ubyte)(v);
	}
	
	void add(mp_uint32 v)
	{
		setEx(ex()+v);
	}
	
	void sub(mp_uint32 v)
	{
		setEx(ex()-v);		
	}
	
	void shr(mp_uint32 v)
	{
		setEx(ex()>>v);
	}

	void shl(mp_uint32 v)
	{
		setEx(ex()<<v);
	}
};

mp_uint32 getDword(mp_ubyte* buffer)
{
	return (mp_uint32)(buffer[0]) +
		   (mp_uint32)(buffer[1] << 8) +
		   (mp_uint32)(buffer[2] << 16) +
		   (mp_uint32)(buffer[3] << 24);
}

// cheating
#define ror(b, c) (mp_ubyte)(((b<<8)>>c | (b>>c)));

/*;²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²
;                             UnPack sample method 1
;²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²
; Input:
;         esi = input offset
;         edi = dest offset
;         ecx = input size
; Output:
;         ecx = output size
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
mp_uint32 UnpackMethod1(mp_ubyte* input, mp_ubyte* output, mp_uint32 inputsize)
{
	Reg a, b, d;	
	mp_sint32 c, di, si, bp, cx2;
	mp_ubyte cl;
	
	mp_ubyte packcharacter;
	
	si = 0;									// mov esi,cs:inputoffset
	di = 0;									// mov edi,cs:outputoffset
	
	inputsize = getDword(input+si);			// mov ecx,fs:[esi]; mov cs:inputsize,ecx
	c = getDword(input+si+4);				// mov ecx,fs:[esi+4]
	
	packcharacter = a.ll = *(input+si+8);	// mov al,fs:[esi+8]; mov cs:packcharacter,al
	
	si+=9;									// add esi,9
	
unpackloop:

	a.ll = *(input+si);						// mov al,fs:[esi]
	
	si+=1;									// inc esi
	
	if (a.ll == packcharacter)				// cmp al,cs:packcharacter
		goto unpacka;						// jz unpacka

	*(output+di) = a.ll;					// mov fs:[edi],al
	
	di+=1;									// inc edi

	c--;
	
	if (c != 0)
		goto unpackloop;					// loop unpackloop

	goto endofunpack;						// endofunpack

unpacka:

	a.ll = *(input+si);						// mov al,fs:[esi]
	si+=1;									// inc esi

	c--;									// dec ecx

	if (a.ll == 0)							// or al,al
		goto putpackcharacter;				// jz putpackcharacter
		
	cx2 = a.ll;								// movzx ecx,al

	a.ll = *(input+si);						// mov al,fs:[esi]
	si+=1;									// inc esi
	
	if (cx2 == 0)							// or ecx,ecx
		goto stosbd1;						// jz @stosbd1
		
stosbl1: 

	*(output+di) = a.ll;					// mov fs:[edi],al
	
	di+=1;									// inc edi
	
	cx2--;									// dec ecx
	
	if (cx2 != 0) 
		goto stosbl1;						// jnz @stosbl1
	
stosbd1:

	c--;									// dec ecx
	
	c--;									// again because of the loop
	if (c != 0)
		goto unpackloop;					// loop unpackloop
	
	goto endofunpack;						// jmp endofunpack
	
putpackcharacter: 

	a.ll = packcharacter;					// mov al,cs:packcharacter

	*(output+di) = a.ll;					// mov fs:[edi],al
	
	di+=1;									// inc edi

	c--; 
	
	if (c != 0)
		goto unpackloop;					// loop unpackloop	

endofunpack:
	
	di = 0;									// mov edi,cs:inputoffset 
	
	c = inputsize;							// mov ecx,cs:inputsize
	
	a.setEx(0);								// xor eax,eax  
	
	d.setEx(c);								// mov edx,ecx
	
	c >>= 2;								// shr ecx,2
	
	if (c == 0)								// or ecx,ecx
		goto stosdd1;						// jz @stosdd1
		
stosdl1:
	*(input+di) = a.ll;						// mov fs:[edi],eax
	*(input+di+1) = a.lh; 
	*(input+di+2) = a.hl;
	*(input+di+3) = a.hh;
	
	di+=4;									// add edi,4
	
	c--;									// dec ecx
	
	if (c != 0)
		goto stosdl1;						// jnz @stosdl1
		
stosdd1:
	c = d.ex()&3;							// mov ecx,edx; and ecx,3
	
	if (c==0)								// or ecx,ecx
		goto stosbd2;						// jz @stosbd2

stosbl2:
	*(input+di) = a.ll;						// mov fs:[edi],al
	di+=1;									// inc edi
	
	c--;									// dec ecx
	
	if (c != 0) 
		goto stosbl2;						// jnz @stosbl2 
	
stosbd2:
	
	di = 0;									// mov edi,cs:inputoffset
	si = 0;									// mov esi,cs:outputoffset
	
	bp = di;								// mov ebp,edi
	
	c = inputsize;							// mov ecx,cs:inputsize
	
	bp+=c;									// add ebp,ecx
	
	d.ll = 128;								// mov dl,10000000b
bitunpackloop:
	d.lh = 0;								// xor dh,dh
	a.ll = *(output + si);					// mov al,fs:[esi]
	si+=1;									// inc esi
	
	cl = 8;									// mov cl,8
bitunpack2:
	b.ll = a.ll;							// mov bl,al
	b.ll &= d.ll;							// and bl,dl
	cl += d.lh;								// add cl,dh
	
	b.ll = ror(b.ll, cl);					// ror bl,cl
	cl -= d.lh;								// sub cl,dh
	d.ll = ror(d.ll, 1);					// ror dl,1
	
	*(input + di) |= b.ll;					// or fs:[edi],bl
	di+=1;									// inc edi
	if (di != bp)							// cmp edi,ebp
		goto notsettaback;					// jnz notsettaback
		
	di = 0;									// mov edi,cs:inputoffset
	d.lh++;									// inc dh
	
notsettaback:

	cl--;									// dec ecx

	if (cl != 0)
		goto bitunpack2;					// jnz bitunpack2
		
	cl = d.lh;								// mov cl,dh
	d.ll = ror(d.ll, cl);					// ror dl,cl
	
	c--;

	if (c != 0)
		goto bitunpackloop;					// loop bitunpackloop 
	
	si = 0;									// cs:inputoffset
	di = 0;									// cs:outputoffset
	
	c = inputsize;							// cs:inputsize
	
	b.ll = 0;								// xor bl,bl
deltaunpack:
	a.ll = *(input+si);						// mov al,fs:[esi]
	si+=1;									// inc esi
	
	if (a.ll == 128)						// cmp al,128
		goto notnegative;					// jz notnegative
	
	if (!(a.ll&128))						// test al,10000000b
		goto notnegative;					// jz notnegative
		
	a.ll&=127;								// and al,01111111b
	a.ll = -a.ll;							// neg al
	
notnegative:
	b.ll-=a.ll;								// sub bl,al
	
	*(output+di) = b.ll;					// mov fs:[edi],bl
	di+=1;									// inc edi
	
	c--;
	
	if (c != 0)
		goto deltaunpack;					// loop deltaunpack
		
	return di;
}
