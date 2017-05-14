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
 *  LoaderOKT.cpp
 *  MilkyPlay Module Loader: Oktalyzer
 *
 */
#include "Loaders.h"

const char* LoaderOKT::identifyModule(const mp_ubyte* buffer)
{
	// check for .OKT module
	if (!memcmp(buffer,"OKTASONG",8)) 
	{
		return "OKT";
	}

	return NULL;
}

static void convertOKTEffects(mp_ubyte& eff, mp_ubyte& op)
{
	switch (eff)
	{
		case 00:
			op = 0;
			break;
			
		case 10:	// arpeggio I
		case 11:	// arpeggio II
		case 12:	// arpeggio III
			eff = (eff-10)+0x56;
			break;
		
		case 13:	// note slide down
			eff = 0x54;
			break;

		case 17:	// note slide up
			eff = 0x55;
			break;

		case 21:	// fine note slide down
			eff = 0x54;
			break;

		case 25:	// position jump
			eff = 0x0B;
			break;

		case 30:	// fine note slide up
			eff = 0x55;
			break;
		
		case 27:	// play release part
			eff = 0x4F;
			op = 3;
			break;
		
		case 28:	// set speed
			eff = 0x0f;
			break;

		case 31:	// volume stuff
		{
			if (op <= 0x40)
			{
				eff = 0x0c;
				op = XModule::vol64to255(op);
			}
			else if (op > 0x40 && op <= 0x50) // volslide down
			{
				eff = 0x0A;
				op = (op-0x40);
			}
			else if (op > 0x50 && op <= 0x60) // volslide up
			{
				eff = 0x0A;
				op = (op-0x50) << 4;
			}
			else if (op > 0x60 && op <= 0x70) // fine volslide down
			{
				eff = 0x3B;
				op = (op-0x60);
			}
			else if (op > 0x70 && op <= 0x80) // fine volslide up
			{
				eff = 0x3A;
				op = (op-0x70);
			}
			else
			{
#ifdef VERBOSE
				printf("Unsupported Oktalyzer effect: %i/%i\n", eff, op);
#endif
				eff = op = 0;
			}
		
			break;
		}
		
		default:
#ifdef VERBOSE
			printf("Unsupported Oktalyzer effect: %i/%i\n", eff, op);
#endif
			eff = op = 0;
	}
}

mp_sint32 LoaderOKT::load(XMFileBase& f, XModule* module)
{
	// max pattern size
	mp_ubyte buffer[8192];
	
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	
	
	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;
	
	f.read(header->sig, 1, 8);

	header->speed = 125;
	header->tempo = 6;
	header->mainvol = 255;
	
	mp_sint32 pc = 0;
	mp_sint32 sc = 0;
	
	while (true)
	{
		
		mp_ubyte ID[4];
		
		mp_uint32 bytesRead = f.read(&ID, 4, 1);

		if (bytesRead != 4)
			break;		
		
		switch (BigEndian::GET_DWORD(ID))
		{
			case 0x434D4F44:	// 'CMOD'
			{
				f.read(buffer, 4, 1);
				
				if (BigEndian::GET_DWORD(buffer) != 8)
					return MP_LOADER_FAILED;
					
				for (mp_sint32 i = 0; i < 4; i++)
				{
					f.read(buffer, 2, 1);
					if (!BigEndian::GET_WORD(buffer))
						header->channum++;
					else
						header->channum+=2;
				}
				
				break;
			}
			
			case 0x53414D50:	// 'SAMP'
			{
				f.read(buffer, 4, 1);
				
				header->insnum = BigEndian::GET_DWORD(buffer) / 32;
				
				mp_sint32 s = 0;
				for (mp_sint32 i = 0; i < header->insnum; i++)
				{
					f.read(buffer, 1, 32);
					
					memcpy(instr[i].name, buffer, 20);
					
					if (BigEndian::GET_DWORD(buffer+20))
					{
						instr[i].samp = 1;
						
						memcpy(smp[s].name, buffer, 20);
						
						smp[s].samplen = BigEndian::GET_DWORD(buffer+20);
						
						smp[s].loopstart = BigEndian::GET_WORD(buffer+24) << 1;
						smp[s].looplen = BigEndian::GET_WORD(buffer+26) << 1;
						smp[s].vol = XModule::vol64to255(BigEndian::GET_WORD(buffer+29));
						smp[s].flags = 1;
						
						if (smp[s].looplen > 2)
							smp[s].type = 1;
						
						for (mp_sint32 j = 0; j < 120; j++) 
							instr[i].snum[j] = s;
						
						s++;
					}
					
					header->smpnum = s;
					
				}
				
				break;
			}

			case 0x53504545:	// 'SPEE'
			{
				f.read(buffer, 4, 1);
				
				if (BigEndian::GET_DWORD(buffer) != 2)
					return MP_LOADER_FAILED;
				
				f.read(buffer, 2, 1);				
				
				header->tempo = BigEndian::GET_WORD(buffer);
				
				break;
			}
			
			case 0x534C454E:	// 'SLEN'
			{
				f.read(buffer, 4, 1);			

				if (BigEndian::GET_DWORD(buffer) != 2)
					return MP_LOADER_FAILED;
					
				f.read(buffer, 2, 1);				
				
				header->patnum = BigEndian::GET_WORD(buffer);
			
				break;
			}

			case 0x504C454E:	// 'PLEN'
			{
				f.read(buffer, 4, 1);			

				if (BigEndian::GET_DWORD(buffer) != 2)
					return MP_LOADER_FAILED;
					
				f.read(buffer, 2, 1);				
				
				header->ordnum = BigEndian::GET_WORD(buffer);
			
				break;
			}

			case 0x50415454:	// 'PATT'
			{
				f.read(buffer, 4, 1);			

				if (BigEndian::GET_DWORD(buffer) > 256)
					return MP_LOADER_FAILED;
					
				f.read(header->ord, 1, BigEndian::GET_DWORD(buffer));				
				
				break;
			}

			case 0x50424F44 :	// 'PBOD'
			{
				f.read(buffer, 4, 1);			

				mp_sint32 chunkLen = BigEndian::GET_DWORD(buffer);

				mp_sint32 i = pc;
				
				if (chunkLen)
				{
					f.read(buffer, 1, chunkLen);
					
					phead[i].rows = BigEndian::GET_WORD(buffer);					
					phead[i].effnum = 1;
					phead[i].channum = (mp_ubyte)header->channum;
					
					phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum * (phead[i].effnum * 2 + 2)];
					
					// out of memory?
					if (phead[i].patternData == NULL)
					{
						return MP_OUT_OF_MEMORY;
					}
					
					memset(phead[i].patternData,0,phead[i].rows*header->channum * (phead[i].effnum * 2 + 2));
					
					mp_ubyte* pattern = buffer+2;
					
					mp_sint32 r,c,cnt = 0;
					mp_sint32 offs = 0;
					for (r = 0; r < phead[i].rows; r++) 
					{
						for (c = 0; c < header->channum;c++) 
						{
							mp_ubyte note = pattern[cnt];
							
							if (note)
								note+=12*3;
							
							mp_ubyte ins = pattern[cnt+1];
							
							if (note)
								ins++;
							else ins = 0;
							
							mp_ubyte eff = pattern[cnt+2];
							mp_ubyte op = pattern[cnt+3];
							
							convertOKTEffects(eff, op);
#ifdef VERBOSE
							if (pattern[cnt+2] && !eff)
							{
								printf("Pattern: %i, %i, %i\n", i, c, r);
							}
#endif
							
							phead[i].patternData[offs] = note;
							phead[i].patternData[offs+1] = ins;
							phead[i].patternData[offs+2] = eff;
							phead[i].patternData[offs+3] = op;
							
							offs+=(phead[i].effnum * 2 + 2);
							
							cnt+=4;
						}
						
					}
					
				}
				else // empty pattern
				{
					phead[i].rows = 64;
					phead[i].effnum = 1;
					phead[i].channum = (mp_ubyte)header->channum;
					
					phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum * (phead[i].effnum * 2 + 2)];
					
					// out of memory?
					if (phead[i].patternData == NULL)
					{
						return MP_OUT_OF_MEMORY;
					}
					
					memset(phead[i].patternData,0,phead[i].rows*header->channum * (phead[i].effnum * 2 + 2));
				}
				
				pc++;
				
				break;
			}
			
			case 0x53424F44  :	// 'SBOD'
			{
				f.read(buffer, 4, 1);
				
				mp_sint32 sampLen = BigEndian::GET_DWORD(buffer);
			
				mp_uint32 allocMem = sampLen;
				if (smp[sc].samplen > allocMem)
					allocMem = smp[sc].samplen;
			
				smp[sc].sample = (mp_sbyte*)module->allocSampleMem(allocMem);
				memset(smp[sc].sample, 0, allocMem);
				
				if (smp[sc].sample == NULL)
				{
					return MP_OUT_OF_MEMORY;
				}
				
				if (!module->loadSample(f,smp[sc].sample,sampLen,sampLen))
				{
					return MP_OUT_OF_MEMORY;
				}
			
				sc++;
			
			}
			
		}
			
	}
	
	strcpy(header->tracker,"Oktalyzer");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;
}
