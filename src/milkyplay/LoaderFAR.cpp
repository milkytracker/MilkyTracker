/*
 *  milkyplay/LoaderFAR.cpp
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
 *  LoaderFAR.cpp
 *  MilkyPlay Module Loader: Farandole Composer
 *
 */
#include "Loaders.h"

const char* LoaderFAR::identifyModule(const mp_ubyte* buffer)
{
	// check for .FAR module
	if (!memcmp(buffer,"FAR\xFE",4))
	{
		return "FAR";
	}

	return NULL;
}

mp_sint32 LoaderFAR::load(XMFileBase& f, XModule* module)
{
	mp_ubyte buffer[2048];
	
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	
	
	// we're already out of memory here
	if (!phead || !instr || !smp)
		return -7;
	
	mp_sint32 i,j;
	
	// read most of the header
	f.read(buffer, 1, 98);
	
	memcpy(header->sig, buffer, 3);
	
	memcpy(header->name, buffer + 4, 32);

	header->freqtab = 1;

	header->mainvol = 255;
	header->tempo = buffer[75];
	header->speed = 80;
	header->tempo = 4;

	header->channum = 16;
	
	char* unpackedSongMessage = new char[LittleEndian::GET_WORD(buffer + 96)];
										
	f.read(unpackedSongMessage, 1, LittleEndian::GET_WORD(buffer + 96));

	mp_sint32 size = LittleEndian::GET_WORD(buffer + 96);
	
	// song message isn't null terminated
	for (i = 0; i < size; i++)
		if (unpackedSongMessage[i] == '\0') unpackedSongMessage[i] = ' ';

	for (i = 0; i < size; i++)
	{
		char line[140];
		memset(line, 0, sizeof(line));
		
		if (size - i >= 131)
		{
			XModule::convertStr(line, unpackedSongMessage+i, 132, false);
			i+=131;
		}
		else
		{
			XModule::convertStr(line, unpackedSongMessage+i, size-i, false);
			i+=size-i;
		}
		module->addSongMessageLine(line);
	}			
	
	delete[] unpackedSongMessage;
	
	f.read(header->ord, 1, 256);
	
	header->patnum = f.readByte();
	header->ordnum = f.readByte();
	header->restart = f.readByte();
	
	mp_uword patSizes[256];
	
	f.readWords(patSizes, 256);
	
	j = LittleEndian::GET_WORD(buffer + 47) - (869 + LittleEndian::GET_WORD(buffer + 96));
	
	for (i = 0; i < j; i++)
		f.readByte();
	
	j = 0;
	for (i = 0; i < 256; i++)
		if (patSizes[i])
			j = i;
			
	header->patnum = j+1;
	
	for (i = 0; i < header->patnum; i++)
	{
		
		if (patSizes[i])
		{
		
			mp_ubyte* pattern = new mp_ubyte[patSizes[i]];
			
			f.read(pattern, 1, patSizes[i]);
			
			// brauch ich des?
			
			mp_sint32 pNumRows = (patSizes[i] - 2) / (16*4);
			
			mp_sint32 numRows = *pattern + 2; // huh?
			
			if (numRows > pNumRows) 
				numRows = pNumRows;
			
			phead[i].rows = numRows;
			phead[i].effnum = 2;
			phead[i].channum = (mp_ubyte)header->channum;
			
			phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum * (phead[i].effnum * 2 + 2)];
			
			// out of memory?
			if (phead[i].patternData == NULL)
			{
				return -7;
			}
			
			memset(phead[i].patternData,0,phead[i].rows*header->channum * (phead[i].effnum * 2 + 2));

			mp_sint32 r,c,cnt = 2;
			mp_sint32 offs = 0;
			for (r=0;r < phead[i].rows; r++) {
				for (c=0;c < header->channum;c++) 
				{
					mp_ubyte note = pattern[cnt];
					mp_ubyte ins = pattern[cnt+1];
					mp_ubyte vol = pattern[cnt+2];
					mp_ubyte eff = pattern[cnt+3] >> 4;
					mp_ubyte op = pattern[cnt+3] & 0x0F;

					if (vol)
					{
						vol = XModule::vol64to255(vol << 2);
					
						phead[i].patternData[offs+2] = 0x0C;
						phead[i].patternData[offs+3] = vol;
					}
					
					if (note)
					{
						note+=12*3;
						ins++;
					}

					phead[i].patternData[offs]=note;
					phead[i].patternData[offs+1]=ins;
					
					eff=0x70;
					op = pattern[cnt+3];
					
					/*switch (eff)
					{
						// porta to note
						case 0x03:
							op <<= 4;
							break;
						
						// retrigger
						case 0x04:
							eff = 0x39;
							break;

						// vibrato depth
						case 0x5: 
							vibDepth[c] = op;
							break;

						// vibrato
						case 0x6: 
							eff = 0x04;
							op = (op<<4)|vibDepth[c];
							break;
							
						// volslide up
						case 0x07:
							eff = 0x0A;
							op<<=4;
							break;
							
						// volslide down
						case 0x08:
							eff = 0x0A;
							break;
							
						// break
						case 0x0B:
							eff = 0x08;
							op <<= 4;
							break;
						
						case 0x00:
							break;
							
						case 0x0f:
							eff = 0x7f;
							break;
							
						default:
							printf("Missing effects %i,%i\n",eff,op);
							eff = op = 0;
					}*/
					
					phead[i].patternData[offs+4]=eff;
					phead[i].patternData[offs+5]=op;
					
					offs+=(phead[i].effnum * 2 + 2);
					
					cnt+=4;
				}
			}
			
			
			delete[] pattern;
		}
		
	}
		
	char sampleMap[8];
	
	f.read(sampleMap, 1, 8);
	
	j = 0;
	for (i = 0; i < 64; i++)
		if (sampleMap[i>>3]&(1<<(i&7))) 
			j = i+1;
			
	header->insnum = j;       
		
	mp_sint32 s = 0;
	for (i = 0; i < header->insnum; i++)
	{
		if (sampleMap[i>>3]&(1<<(i&7))) 
		{
			instr[i].samp = 1;
			for (j = 0; j < 120; j++) 
				instr[i].snum[j] = s;

			f.read(instr[i].name, 1, 32);
			
			memcpy(smp[s].name, instr[i].name, 32);
			
			mp_sint32 size = f.readDword();
			mp_ubyte finetune = f.readByte();
			mp_sint32 volume = f.readByte() << 4;
			if (volume>255) volume = 255;
			mp_sint32 repstart = f.readDword();
			mp_sint32 repend = f.readDword();
			mp_ubyte type = f.readByte();
			mp_ubyte flags = f.readByte();

			smp[s].vol = volume;
			smp[s].flags = 1;
			smp[s].samplen = size;
			smp[s].loopstart = repstart;
			mp_sint32 looplen = (repend - repstart);
			if (looplen < 0) 
				looplen = 0;
			smp[s].looplen = looplen;
			
			if (flags & 8)
				smp[s].type = 1;
			
			if (type & 1)
			{
				smp[s].type |= 16;

				smp[s].samplen >>= 1;
				smp[s].loopstart >>=1;
				smp[s].looplen >>=1;
			}
			
			if (module->loadModuleSample(f, s) != 0)
				return -7;
			
			s++;
		}
	}
									
	header->smpnum = s;

	strcpy(header->tracker,"Farandole Composer");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return 0;
}
