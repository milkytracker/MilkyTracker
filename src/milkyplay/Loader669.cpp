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
 *  Loader669.cpp
 *  MilkyPlay Module Loader: 669
 *
 *  Warning: This is an one-by-one conversion of an assembler version ;)
 *
 *  --------------------------------
 *			Version History:
 *  --------------------------------
 *  01/24/05: 669 loader is now handled by the FAR replayer
 *
 */
#include "Loaders.h"

const char* Loader669::identifyModule(const mp_ubyte* buffer)
{
	mp_sint32 i;

	// check for .669 module
	if (!memcmp(buffer,"if",2) ||
		!memcmp(buffer,"JN",2))
	{
		// NOS
		if ((mp_ubyte)buffer[0x6e] > 64)
			return NULL;
		// NOP
		if ((mp_ubyte)buffer[0x6f] > 128)
			return NULL;
			
		if ((mp_ubyte)buffer[0x70] > 127)
			return NULL;
	
		for (i = 0; i < 0x80; i++)
		{
			if ((mp_ubyte)buffer[0x71 + i] > 128 && (mp_ubyte)buffer[0x71 + i] != 255)
				return NULL;
			if ((mp_ubyte)buffer[0xf1 + i] > 32)
				return NULL;
			if ((mp_ubyte)buffer[0x171 + i] > 63)
				return NULL;
		}
		
		return "669";
	}

	return NULL;
}

mp_sint32 Loader669::load(XMFileBase& f, XModule* module)
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
		return MP_OUT_OF_MEMORY;
	
	f.read(buffer,1,0x1f1);
	
	memcpy(header->sig, buffer, 2);
	memcpy(header->name, buffer+2, 30);
	
	mp_sint32 i;
	for (i = 0; i < 256; i++)
	{
		if (buffer[i + 0x71] == 0xFF)
			break;
		header->ord[i] = buffer[i + 0x71];
	}	
	header->ordnum = i;
		
	// song message
	char* unpackedSongMessage = reinterpret_cast<char*>(buffer+2);
	mp_sint32 size = 108;
	
	// song message isn't null terminated
	for (i = 0; i < size; i++)
		if (unpackedSongMessage[i] == '\0') unpackedSongMessage[i] = ' ';

	for (i = 0; i < size; i++)
	{
		char line[40];
		memset(line, 0, sizeof(line));
		
		if (size - i >= 35)
		{
			XModule::convertStr(line, unpackedSongMessage+i, 36, false);
			i+=35;
		}
		else
		{
			XModule::convertStr(line, unpackedSongMessage+i, size-i, false);
			i+=size-i;
		}
		module->addSongMessageLine(line);
	}			
	
#ifdef VERBOSE
	printf("%s\n",module->message);
#endif

	header->restart = LittleEndian::GET_WORD(buffer + 0x70);

	header->insnum = buffer[0x6e]; // number of instruments
	header->patnum = buffer[0x6f]; // number of patterns

	header->speed = 80;	// default tickspeed
	header->tempo = 4;	// default tempo
	header->mainvol = 255;
	header->channum = 9; // add one more channel for speed settings
	
	header->freqtab = 1;
	
	mp_uint32 s = 0;
	for (i = 0; i < header->insnum; i++) 
	{
		f.read(buffer + 0x1f1,1,25);
		
		memcpy(instr[i].name, buffer + 0x1f1, 12);
		
		if (LittleEndian::GET_DWORD(buffer + 13 + 0x1f1) != 0)
		{
		
			mp_uint32 size = LittleEndian::GET_DWORD(buffer + 13 + 0x1f1);
			
			mp_uint32 loopstart = LittleEndian::GET_DWORD(buffer + 17 + 0x1f1);
			
			mp_uint32 loopend = LittleEndian::GET_DWORD(buffer + 21 + 0x1f1);
			
			if (size > 0) 
			{
				instr[i].samp = 1;
				
				for (mp_sint32 j = 0; j < 120; j++) 
					instr[i].snum[j] = s;
				
				smp[s].samplen = size;
				smp[s].flags = 0;
				
				smp[s].finetune = 0;
				
				if (loopend != 1048575 && loopend <= size)
				{
					smp[s].loopstart = loopstart;
					smp[s].looplen = loopend - loopstart;
					smp[s].type = 1;
				}
			
				s++;
			}
			
		}
		
	}
	
	header->smpnum = s;
	
	for (i = 0; i < header->patnum;i++) 
	{
		f.read(buffer + 0x1f1, 1, 0x600);
		
		phead[i].rows = buffer[0x171 + i] + 1;

#ifdef VERBOSE
		printf("%i\n",phead[i].rows);
#endif

		phead[i].effnum = 2;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum * (phead[i].effnum * 2 + 2)];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum * (phead[i].effnum * 2 + 2));
		
		mp_sint32 r,c,cnt = 0x1f1;
		mp_sint32 offs = 0;
		for (r=0;r < phead[i].rows; r++) 
		{
			// add speed into first channel
			if (r == 0 && buffer[i+0xf1])
			{
				phead[i].patternData[offs+4] = 0x70;
				phead[i].patternData[offs+5] = 0xF0 + (buffer[i+0xf1]&0xF);					
			}
			
			offs+=(phead[i].effnum * 2 + 2);
			for (c=1;c < header->channum;c++) 
			{
			
				mp_ubyte note = (buffer[cnt] >> 2) + 25;
				mp_ubyte ins = ((((buffer[cnt]&3)<<4) + (buffer[cnt+1] >> 4))) + 1;
				
				mp_sint32 vol = (((mp_sint32)buffer[cnt+1] & 0xf) + 1) * 16;
				if (vol >= 256) vol = 255;

				mp_ubyte eff1 = 0xC;
				mp_ubyte op1 = vol;
				
				if (buffer[cnt+2] != 0xff)
				{
					mp_ubyte eff2 = buffer[cnt+2] >> 4;
					mp_ubyte op2 = buffer[cnt+2] & 0xf;
					switch (eff2)
					{
						case 0x00:
							eff2 = 0x70;
							op2 |= 0x10;
#ifdef VERBOSE
							printf("porta up\n");
#endif
							break;
						case 0x01:
							eff2 = 0x70;
							op2 |= 0x20;
#ifdef VERBOSE							
							printf("porta down\n");
#endif
							break;
						case 0x02:
							eff2 = 0x70;
							op2 |= 0x30;
							break;
						case 0x04:
							eff2 = 0x70;
							op2 |= 0x40;
							break;
						case 0x05:
							eff2 = 0x70;
							op2 |= 0xF0;
							break;
						default:
#ifdef VERBOSE							
							printf("%i, %i\n",eff2, op2);
#endif
							eff2 = op2 = 0;
					}
					
					phead[i].patternData[offs+4]=eff2;
					phead[i].patternData[offs+5]=op2;
				}
				
				if (buffer[cnt] == 0xfe)
				{
					note = ins = 0;
				}
				else if (buffer[cnt] == 0xff)
				{
					note = ins = eff1 = op1 = 0;
				}
				
				phead[i].patternData[offs]=note;
				phead[i].patternData[offs+1]=ins;
				
				phead[i].patternData[offs+2]=eff1;
				phead[i].patternData[offs+3]=op1;
				
				offs+=(phead[i].effnum * 2 + 2);
				
				cnt+=3;
			}
		}
		
	}
	
	mp_sint32 result = module->loadModuleSamples(f, XModule::ST_UNSIGNED);
	if (result != MP_OK)
		return result;

	strcpy(header->tracker,"Composer669");
	
	module->setDefaultPanning();
	
	module->postProcessSamples(true);
	
	return MP_OK;
}
