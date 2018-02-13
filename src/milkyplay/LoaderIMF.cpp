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
 *  LoaderIMF.cpp
 *  MilkyPlay Module Loader: Imago Orpheus
 *
 *  Much thanks to the MikMod team for providing lots of information about the format
 *
 *  --------------------------------
 *			Version History:
 *  --------------------------------
 *  01/24/05: Started...
 */
#include "Loaders.h"

//#define VERBOSE

const char* LoaderIMF::identifyModule(const mp_ubyte* buffer)
{
	// check for .IMF module first
	if (!memcmp(buffer+0x3c,"IM10",4))
	{
		return "IMF";
	}

	// this is not an .IMF
	return NULL;
}

struct IMFChannel
{
	char name[12];
	mp_ubyte chorus, reverb, panning, status;
};

struct IMFInstrument 
{
	char name[32];
	mp_ubyte snum[120];
	mp_uword volenv[16*2];
	mp_uword panenv[16*2];
	mp_uword pitenv[16*2];
	mp_ubyte volpts;
	mp_ubyte volsus;
	mp_ubyte volbeg;
	mp_ubyte volend;
	mp_ubyte volflg;
	mp_ubyte panpts;
	mp_ubyte pansus;
	mp_ubyte panbeg;
	mp_ubyte panend;
	mp_ubyte panflg;
	mp_ubyte pitpts;
	mp_ubyte pitsus;
	mp_ubyte pitbeg;
	mp_ubyte pitend;
	mp_ubyte pitflg;
	mp_uword volfade;
	mp_uword numsmp;
	char signature[4];
};

struct IMFSample 
{
	char name[13];
	mp_uint32 length;
	mp_uint32 loopstart;
	mp_uint32 loopend;
	mp_uint32 samplerate;
	mp_ubyte volume;
	mp_ubyte pan;
	mp_ubyte flags;
};

static void convertEffect(mp_ubyte& eff, mp_ubyte& op)
{
	switch (eff)
	{
		case 0x01: // set speed
			eff = 0x1C;
			break;
			
		case 0x02: // set tempo
			eff = 0x16;
			break;
			
		case 0x03: // portamento to note
			break;
			
		case 0x04: // porta to note + volslide									
			eff = 0x05;
			break;
			
		case 0x05: // vibrato									
			eff = 0x04;
			break;
			
		case 0x06: // vibrato + volslide
			break;
			
		case 0x07: // fine vibrato
			eff = 0x4A;
			break;
			
		case 0x08: // tremolo									
			eff = 0x07;
			break;
			
		case 0x09: // argpeggio									
			eff = 0x20;
			break;
			
		case 0x0A: // set panning									
			eff = 0x08;
			break;
			
		case 0x0B: // panning slide
			eff = 0x19;
			break;
			
		case 0x0C: // set volume
			op = XModule::vol64to255(op);
			break;
			
		case 0x0D: // volume slide
			eff = 0x0A;
			break;
			
		case 0x0E: // fine volume slide
			if (op) 
			{
				if (op>>4)
				{
					eff = 0x49;
					op = 0x0f | (op & 0xF0);
				}
				else
				{
					eff = 0x49;
					op = 0xf0 | (op & 0xF);
				}
				
			} 
			else
				eff = 0x49;
			break;
			
		case 0x0F: // set finetune
			eff = 0x35;
			break;
			
		case 0x12: // porta up
			eff = 0x01;
			break;
			
		case 0x13: // porta down
			eff = 0x02;
			break;
			
		case 0x14: // fine porta up
			eff = 0x41;
			break;
			
		case 0x15: // fine porta down
			eff = 0x42;
			break;
			
		case 0x18: // set sample offset
			eff = 0x09;
			break;
			
		case 0x1A: // set keyoff
			eff = 0x51;
			break;
			
		case 0x1B: // multi retrig
			break;
			
		case 0x1D: // pos jump
			eff = 0x0B;
			break;
			
		case 0x1E: // pattern break
			eff = 0x0D;
			break;
			
		case 0x1F: // set mainvol
			eff = 0x10;
			op = XModule::vol64to255(op);
			break;
			
		case 0x20: // mainvolslide
			eff = 0x11;
			break;
		
		case 0x21:
			switch (op >> 4)
			{
				case 0x0A: // loop
					eff = 0x36;
					op&=0x0f;
					break;
				
				case 0x0B: // pattern delay
					eff = 0x3e;
					op&=0x0f;
					break;

				case 0x0C: // note cut
					eff = 0x3c;
					op&=0x0f;
					break;

				case 0x0D: // note cut
					eff = 0x3d;
					op&=0x0f;
					break;
				
				default:
#ifdef VERBOSE
					printf("Missing IMF Sub-Effect: %x, %x\n", op>>4, op&0xf);
#endif
					eff = op = 0;
					break;
			}
		
		case 0:
			op = 0;
			break;
		
		default:
#ifdef VERBOSE
			printf("Missing IMF Effect: %x, %x\n", eff, op);
#endif
			eff = op = 0;
	}
}

mp_sint32 LoaderIMF::load(XMFileBase& f, XModule* module)
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
	
	memcpy(header->sig, "IM10", 4);
	
	f.read(header->name, 1, 32);
	
	header->ordnum = f.readWord();
	header->patnum = f.readWord();
	header->insnum = f.readWord();
	header->freqtab = f.readWord() & 1;

	if(header->ordnum > MP_MAXORDERS || header->patnum > 256 || header->insnum > 256)
		return MP_LOADER_FAILED;

	f.readDword();
	f.readDword();

	header->tempo = f.readByte();
	header->speed = f.readByte();
	header->mainvol = XModule::vol64to255(f.readByte());
	header->flags = XModule::MODULE_ST3DUALCOMMANDS;
	
	f.seekWithBaseOffset(64);
		
	IMFChannel channels[32];
	for (i = 0; i < 32; i++)
	{
		f.read(channels[i].name, 1, 12);
		channels[i].chorus = f.readByte();
		channels[i].reverb = f.readByte();
		channels[i].panning = f.readByte();
		channels[i].status = f.readByte();
	}
			
	// ripped from MikMod loader :) 
	if (!channels[0].status) {
		for (i = 1; i < 16; i++) 
			if (channels[i].status!=1) 
				break;
		if (i==16) 
			for (i=1;i<16;i++) 
				channels[i].status=0;
	}
	
	header->channum = 32;
	
	while (channels[header->channum-1].status == 2 && header->channum > 1)
		header->channum--;
	
#ifdef VERBOSE
	for (i = 0; i < header->channum; i++)
	{
		printf("Channel: %i\n", channels[i].status);
	}
#endif

	f.read(header->ord, 1, 256);
	
	for (i = 0; i < 256; i++)
		if (header->ord[i] == 255) 
			header->ord[i]--;
							
	// we want patterns
	for (i = 0; i < header->patnum; i++)
	{
		
		if (i == header->ord[2])
		{
			i = header->ord[2];
		}
		
		mp_sint32 size = f.readWord() - 4;
		mp_sint32 rows = f.readWord();
	
		if (size < 0 || rows > 256)
			return MP_LOADER_FAILED;
		
		phead[i].rows = rows;
		phead[i].effnum = 2;
		phead[i].channum = (mp_ubyte)header->channum;
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum * (phead[i].effnum * 2 + 2)];
		
#ifdef VERBOSE
		printf("%i, %x, %i\n", i, f.posWithBaseOffset(), phead[i].rows);
#endif

		// out of memory?
		if (phead[i].patternData == NULL)
		{
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum * (phead[i].effnum * 2 + 2));		
		
		if (size)
		{
			mp_ubyte* packed = new mp_ubyte[size];
		
			f.read(packed, 1, size);
		
			mp_sint32 index = 0;
			mp_uint32 row = 0;
			
			while (index<size)
			{
				
				mp_ubyte pi = packed[index++];
				
				if (pi == 0) {
					row++;
					continue;
				}
				
				mp_uint32 chn = pi&31;
				
				mp_ubyte slot[6];
				memset(slot, 0, sizeof(slot));
				slot[0] = 255;
				
				if (pi & 32)
				{
					slot[0] = packed[index++];
					slot[1] = packed[index++];
				}
				if (pi & 64)
				{
					slot[2] = packed[index++];
					slot[3] = packed[index++];
				}
				if (pi & 128)
				{
					slot[4] = packed[index++];
					slot[5] = packed[index++];
				}
				
				if (chn < phead[i].channum && row < phead[i].rows)
				{
					mp_sint32 o = chn*(phead[i].effnum * 2 + 2) + row*(phead[i].channum*(phead[i].effnum * 2 + 2));
					
					mp_ubyte note = 0;
					if (slot[0] != 255)
					{
						if (note >= 0xa0)
						{
#ifdef VERBOSE
							printf("blabla");
#endif
							note = XModule::NOTE_OFF;
						}
						else
						{
							note = ((slot[0]>>4)*12+(slot[0]&0x0f))+1;
						}
					}
					slot[0] = note;

					convertEffect(slot[2], slot[3]);
					convertEffect(slot[4], slot[5]);
					
					memcpy(phead[i].patternData + o, slot, sizeof(slot));
				}
			}

			delete[] packed;
					
		}
	}
	
	mp_sint32 vEnvIdx = 0;
	mp_sint32 pEnvIdx = 0;
	// i think pitch envelope is pretty much AMS vibrato envelope => no it's not :(
	//mp_sint32 vibEnvIdx = 0;
	
	mp_sint32 s = 0;
	for (i = 0; i < header->insnum; i++)
	{
		IMFInstrument ins;
		
		f.read(ins.name, 1, 32);
		f.read(ins.snum, 1, 120);
		f.readDword();
		f.readDword();
		f.readWords(ins.volenv, 16*2);
		f.readWords(ins.panenv, 16*2);
		f.readWords(ins.pitenv, 16*2);
		ins.volpts = f.readByte();
		ins.volsus = f.readByte();
		ins.volbeg = f.readByte();
		ins.volend = f.readByte();
		ins.volflg = f.readByte();		
		f.readByte();
		f.readByte();
		f.readByte();
		ins.panpts = f.readByte();
		ins.pansus = f.readByte();
		ins.panbeg = f.readByte();
		ins.panend = f.readByte();
		ins.panflg = f.readByte();
		f.readByte();
		f.readByte();
		f.readByte();
		ins.pitpts = f.readByte();
		ins.pitsus = f.readByte();
		ins.pitbeg = f.readByte();
		ins.pitend = f.readByte();
		ins.pitflg = f.readByte();
		f.readByte();
		f.readByte();
		f.readByte();
		ins.volfade = f.readWord();
		ins.numsmp = f.readWord();
		f.read(ins.signature, 1, 4);
		
		instr[i].samp = ins.numsmp;
		
		for (j = 0; j < 120; j++)
			instr[i].snum[j] = ins.snum[j]+s;
		
		memcpy(instr[i].name, ins.name, 32);
		
		if (ins.volflg)
		{
			vEnvIdx++;
			
			TEnvelope venv;
			memset(&venv,0,sizeof(venv));
				
			for (k = 0; k < 16; k++)
			{
				venv.env[k][0] = ins.volenv[k*2];
				venv.env[k][1] = ins.volenv[k*2+1] << 2;
			}
			
			venv.num = ins.volpts;	
			venv.sustain = ins.volsus;
			venv.loops = ins.volbeg;
			venv.loope = ins.volend;
			venv.type = ins.volflg;
			
			if (!module->addVolumeEnvelope(venv)) 
				return MP_OUT_OF_MEMORY;
		}

		if (ins.panflg)
		{
			pEnvIdx++;
			
			TEnvelope penv;
			memset(&penv,0,sizeof(penv));
				
			for (k = 0; k < 16; k++)
			{
				penv.env[k][0] = ins.panenv[k*2];
				penv.env[k][1] = ins.panenv[k*2+1];
			}
			
			penv.num = ins.panpts;	
			penv.sustain = ins.pansus;
			penv.loops = ins.panbeg;
			penv.loope = ins.panend;
			penv.type = ins.panflg;
			
			if (!module->addPanningEnvelope(penv)) 
				return MP_OUT_OF_MEMORY;
		}

		/*if (ins.pitflg)
		{
			vibEnvIdx++;
			
			TEnvelope vibenv;
			memset(&vibenv,0,sizeof(vibenv));
				
			for (k = 0; k < 16; k++)
			{
				vibenv.env[k][0] = ins.pitenv[k*2];
				vibenv.env[k][1] = 256-ins.pitenv[k*2+1];
			}
			
			vibenv.num = ins.pitpts;	
			vibenv.sustain = ins.pitsus;
			vibenv.loops = ins.pitbeg;
			vibenv.loope = ins.pitend;
			vibenv.type = ins.pitflg + (2<<6);
			
			if (!module->addVibratoEnvelope(vibenv)) 
				return MP_OUT_OF_MEMORY;
		}
		
		printf("%i\n",ins.pitflg);*/
		
		for (j = 0; j < ins.numsmp; j++)
		{
		
			IMFSample samp;
			
			f.read(samp.name, 1, 13);
			f.readByte();
			f.readByte();
			f.readByte();
			samp.length = f.readDword();
			samp.loopstart = f.readDword();
			samp.loopend = f.readDword();
			samp.samplerate = f.readDword();
			samp.volume = f.readByte();
			samp.pan = f.readByte();
			
			mp_ubyte buffer[14];
			
			f.read(buffer, 1, 14);
			
			samp.flags = f.readByte();

			f.read(buffer, 1, 15);

			memcpy(smp[s].name, samp.name, 13);

			smp[s].samplen = samp.length;
			smp[s].loopstart = samp.loopstart;
			smp[s].looplen = samp.loopend - samp.loopstart;
			smp[s].vol = XModule::vol64to255(samp.volume);
			smp[s].flags = 1;
			smp[s].pan = samp.pan;
			smp[s].volfade = ins.volfade<<1;
			
			if (ins.volflg)
				smp[s].venvnum = vEnvIdx;
			if (ins.panflg)
				smp[s].penvnum = pEnvIdx;
			/*if (ins.pitflg)
				smp[s].vibenvnum = vibEnvIdx;*/

			XModule::convertc4spd(samp.samplerate, &smp[s].finetune, &smp[s].relnote);

			if (samp.flags&0x1) smp[s].type = 1;
			if (samp.flags&0x2) smp[s].type = 2;
			if (samp.flags&0x8) smp[s].flags |= 2;
			if (samp.flags&0x4) smp[s].type |= 16;

			if (smp[s].type & 16)
			{
				smp[s].samplen>>=1;
				smp[s].loopstart>>=1;
				smp[s].looplen>>=1;
			}
			
			mp_sint32 result = module->loadModuleSample(f, s);
			if (result != MP_OK)
				return result;
			
			s++;
		
		}
		
	}
	
	strcpy(header->tracker,"Imago Orpheus");
	
	header->volenvnum = vEnvIdx;
	header->panenvnum = pEnvIdx;
	//header->vibenvnum = vibEnvIdx;
	
	header->smpnum = s;

	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;
}
