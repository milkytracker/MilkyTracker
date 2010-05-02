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
 *  LoaderCBA.cpp
 *  MilkyPlay Module Loader: Chuck Biscuits Music whatever
 *
 */
#include "Loaders.h"

const char* LoaderCBA::identifyModule(const mp_ubyte* buffer)
{
	// check for .AMF module
	if (!memcmp(buffer,"CBA\xF9",4)) 
	{
		return "CBA";
	}

	return NULL;
}

static void convertEffect(mp_ubyte& eff, mp_ubyte& op)
{
	switch (eff)
	{
		case 0x01:  // arpeggio
			eff = 0x20;
			break;

		case 0x02:  // porta up
			eff = 0x47;
			break;

		case 0x03:  // porta down
			eff = 0x48;
			break;
		
		case 0x04:  // porta to note
			eff = 0x03;
			break;

		case 0x05:  // vibrato
			eff = 0x04;
			break;

		case 0x06:  // porta to note + volslide
			eff = 0x05;
			break;

		case 0x07:  // vibrato + volslide
			eff = 0x06;
			break;
		
		case 0x08:  // tremolo
			eff = 0x07;
			break;

		case 0x09:  // set panning
			eff = 0x08;
			break;
		
		case 0x0a:  // sample offset
			eff = 0x9;
			break;
	
		case 0x0B:  // volslide
			eff = 0x49;
			break;		
	
		case 0x0C:  // position jump
			eff = 0x0B;
			break;
	
		case 0x0e:  // pattern break
			eff = 0x0D;
			break;

		/*case 0x0f:  
			eff = 0x4A;
			break;*/

		case 0x10:  // fine porta up
			eff = 0x31;
			break;

		case 0x11:  // fine porta down
			eff = 0x32;
			break;

		case 0x12:  
			eff = 0x33;
			break;

		case 0x13:  
			eff = 0x34;
			break;

		case 0x14:  
			eff = 0x35;
			break;

		case 0x15:  // loop
			eff = 0x36;
			break;

		case 0x16:  
			eff = 0x37;
			break;

		case 0x17:  // set panning
			eff = 0x8;
			op<<=4;
			break;
	
		case 0x18:  // retrig with volslide
			eff = 0x1B;
			break;

		case 0x19:  // fine volslide up
			eff = 0x3a;
			break;

		case 0x1a:  // fine volslide down
			eff = 0x3b;
			break;

		case 0x1b:  // note cut
			eff = 0x3c;
			break;

		case 0x1c:  // note delay
			eff = 0x3d;
			break;

		case 0x1d:  // pattern delay
			eff = 0x3e;
			break;
	
		case 0x1f:  // set speed
			eff = 0x1C;
			break;
			
		case 0x20:  // set tempo
			eff = 0x16;
			break;

		case 0:
			op = 0;
			break;
		
		default:
#ifdef VERBOSE
			printf("Missing CBA Effect: %x, %x\n", eff, op);
#endif
			eff = op = 0;
	}
}

mp_sint32 LoaderCBA::load(XMFileBase& f, XModule* module)
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
	
	f.read(header->sig, 1, 3);
	f.readByte();   // skip 0xF9
	
	f.read(header->name, 1, 32);
	
	f.readByte();   // skip 0x1A
	
	mp_sint32 songMsgLen = f.readWord();

	header->channum = f.readByte();
	header->patnum = f.readByte()+1; // number of patterns
	header->ordnum = f.readByte(); // song length
	header->insnum = f.readByte(); // number of instruments
	header->tempo = f.readByte();	// default tickspeed
	header->speed = f.readByte();	// default tempo
	header->flags = XModule::MODULE_ST3NEWINSTRUMENT;
	header->mainvol = 255;
	
	f.read(header->pan, 1, 32);

	f.read(header->ord, 1, 255);
	
	mp_sint32 i, s = 0;
	for (i = 0; i < header->insnum; i++) 
	{		
		mp_ubyte name[32];
		mp_ubyte flags;
		mp_ubyte vol;
		mp_uword c4spd;
		mp_sint32 len;
		mp_sint32 loopstart;
		mp_sint32 loopend;
		f.read(name, 1, 32);
		flags = f.readByte();
		vol = f.readByte();
		c4spd = f.readWord();
		
		len = f.readDword();
		loopstart = f.readDword();
		loopend = f.readDword();

		memcpy(instr[i].name, name, 32);
		
		if ((flags & 0x02) && len)
		{
			instr[i].samp = 1;
		
			memcpy(smp[s].name, name, 32);
		
			for (mp_sint32 j = 0; j < 120; j++) 
				instr[i].snum[j] = s;			
		
			smp[s].flags = 1;
			smp[s].samplen = len;
			
			mp_sint32 looplen = (loopend - loopstart);
			if (looplen < 0) 
				looplen = 0;
			smp[s].looplen = looplen;
			
			smp[s].loopstart = loopstart;
			smp[s].vol = XModule::vol64to255(vol);
			
			if (flags & 0x08)
				smp[s].type = 1;
			
			XModule::convertc4spd(c4spd, &smp[s].finetune, &smp[s].relnote);			
			
			s++;
		}
	}
	
	header->smpnum = s;
	
	mp_ubyte* pattern = new mp_ubyte[header->channum*64*5];
	if (pattern == NULL)
		return MP_OUT_OF_MEMORY;
	
	for (i = 0; i < header->patnum;i++) 
	{

		f.read(pattern, 1, header->channum*64*5);
		
		phead[i].rows = 64;
		phead[i].effnum = 2;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum * (phead[i].effnum * 2 + 2)];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			delete[] pattern;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum * (phead[i].effnum * 2 + 2));
		
		mp_sint32 r,c,cnt = 0;
		mp_sint32 offs = 0;
		for (r=0; r < 64; r++) {
			for (c = 0; c < header->channum; c++) {
				mp_ubyte ins = pattern[cnt];
				mp_ubyte note = pattern[cnt+1];
				mp_ubyte vol = pattern[cnt+2];
				mp_ubyte eff = pattern[cnt+3];
				mp_ubyte op = pattern[cnt+4];
				
				if (note == 255)
					note = 122;
				
				phead[i].patternData[offs] = note;
				phead[i].patternData[offs+1] = ins;
				
				if (vol)
				{
					phead[i].patternData[offs+2] = 0x0C;
					phead[i].patternData[offs+3] = XModule::vol64to255(vol-1);
				}

				convertEffect(eff, op);
#ifdef VERBOSE
				if (eff == 0 && pattern[cnt+3])
				{
					printf("pattern: %i, row: %i, channel %i: %x, %x\n", i, r, c, pattern[cnt+3], pattern[cnt+4]);
				}
#endif								
				
				phead[i].patternData[offs+4] = eff;
				phead[i].patternData[offs+5] = op;
				
				cnt+=5;
				offs+=(phead[i].effnum * 2 + 2);
			}
		}
		
	}
	
	delete[] pattern;
	
	mp_sint32 result = module->loadModuleSamples(f, XModule::ST_DELTA);
	if (result != MP_OK)
		return result;

	module->allocateSongMessage(songMsgLen+1);
			
	if (module->message)	
	{
		memset(module->message, 0, songMsgLen+1);
		f.read(module->message, 1, songMsgLen);
	}
	
	strcpy(header->tracker,"..converted..");
	
	//module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;
}
