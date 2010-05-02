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
 *  LoaderDIGI.cpp
 *  MilkyPlay Module Loader: Digibooster 1.0 - 1.7
 */
 
#include "Loaders.h"

const char* LoaderDIGI::identifyModule(const mp_ubyte* buffer)
{
	if (strcmp((const char*)buffer, "DIGI Booster module") == 0)
	{
		return "DIGI";
	}

	return NULL;
}

static void convertEvent(mp_ubyte* dst, mp_ubyte* src)
{
	mp_ubyte b1 = src[0];
	mp_ubyte b2 = src[1];
	mp_ubyte b3 = src[2];
	mp_ubyte b4 = src[3];
	
	mp_sint32 note,ins,eff,notenum = 0;
	note = ((b1&0xf)<<8)+b2;
	ins = (b1&0xf0)+(b3>>4);
	eff = b3&0xf;
	
	if (eff==0xE) {
		eff=(b4>>4)+0x30;
		b4&=0xf;
		
		if (eff == 0x33)
		{
			eff = 0x4F;
			b4 = 1;
		}
		if (eff == 0x34)
		{
			eff = 0x51; // key off at tick 0
			b4 = 0;
		}
		if (eff == 0x35)
		{
			if (b4 == 0)
			{
				eff = 0x50; // AMS set channel mastervolume to zero
				b4 = 0;
			}
			else if (b4 == 1)
			{
				eff = 0x50; // AMS set channel mastervolume to 255
				b4 = 255;
			}
		}
		if (eff == 0x38)
		{
#ifdef VERBOSE
			printf("Unsupported Digibooster effect: extended offset\n");
#endif
			eff = b4 = 0;
		}
		if (eff == 0x39)
		{
#ifdef VERBOSE
			printf("Unsupported Digibooster effect: retrace\n");
#endif
			eff = b4 = 0;
		}
	}
	
	if ((!eff)&&b4) 
		eff=0x20;
	
	if (eff==0x8)
		eff = b4 = 0;
	
	// old style modules don't support last effect for:
	// - portamento up/down
	// - volume slide
	if (eff==0x1&&(!b4)) eff = 0;
	if (eff==0x2&&(!b4)) eff = 0;
	if (eff==0xA&&(!b4)) eff = 0;
	
	if (eff==0x5&&(!b4)) eff = 0x3;
	if (eff==0x6&&(!b4)) eff = 0x4;
	
	if (eff==0xC) {
		b4 = XModule::vol64to255(b4);
	}
	
	if (note) 
		notenum = XModule::amigaPeriodToNote(note);
	
	dst[0] = notenum;
	dst[1] = ins;
	dst[2] = eff;
	dst[3] = b4;
}

mp_sint32 LoaderDIGI::load(XMFileBase& f, XModule* module)
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

	mp_sint32 i,j;
	mp_ubyte buffer[31*4];
		
	// read header
	f.read(header->sig, 1, 16);	
	// skip 4 bytes (ule\0)
	f.readDword();	
	// skip version string
	f.readDword();	
	// check version
	mp_ubyte ver = f.readByte();
	if (ver < 0x10 ||
		ver > 0x17)
		return MP_LOADER_FAILED;
	// read numchannels
	header->channum = f.readByte();
	// packenable
	mp_ubyte pack = f.readByte(); 
	f.read(buffer, 1, 19);
	// read numpatterns
	header->patnum = (mp_sword)f.readByte() + 1;
	// read songlength
	header->ordnum = (mp_sword)f.readByte() + 1;
	f.read(header->ord, 1, 128);
	
	header->insnum = 31;
	
	f.read(buffer, 4, 31);
	
	mp_sint32 s = 0;
	for (i = 0; i < header->insnum; i++)
	{
		mp_sint32 slen = BigEndian::GET_DWORD(buffer+i*4);
		if (slen)
		{
			instr[i].samp = 1;
			
			for (j = 0; j < 120; j++) 
				instr[i].snum[j] = s;
		
			smp[s].samplen = slen;
			s++;
		}
	}
	header->smpnum = s;
	
	// loop starts
	f.read(buffer, 4, 31);
	for (i = 0; i < header->insnum; i++)
	{
		if (instr[i].samp)
		{
			s = instr[i].snum[0];
			smp[s].loopstart = BigEndian::GET_DWORD(buffer+i*4);
		}
	}

	// loop lengths
	f.read(buffer, 4, 31);
	for (i = 0; i < header->insnum; i++)
	{
		if (instr[i].samp)
		{
			s = instr[i].snum[0];
			smp[s].looplen = BigEndian::GET_DWORD(buffer+i*4);
			if (smp[s].looplen)
				smp[s].type = 1;
		}
	}

	// volumes & finetunes
	f.read(buffer, 1, 31);
	f.read(buffer+31, 1, 31);
	for (i = 0; i < header->insnum; i++)
	{
		if (instr[i].samp)
		{
			s = instr[i].snum[0];
			smp[s].vol = XModule::vol64to255(buffer[i]);
			smp[s].pan = 0x80;
			smp[s].flags = 1;
			smp[s].finetune = ((mp_sbyte)buffer[i+31])*16;
		}
	}
	
	f.read(header->name, 1, 32);
	
	for (i = 0; i < header->insnum; i++)
	{
		f.read(instr[i].name, 1, 30);		
		if (instr[i].samp)
		{
			s = instr[i].snum[0];
			memcpy(smp[s].name, instr[i].name, 30);
		}
	}
	
	// read patterns
	for (i = 0; i < header->patnum; i++)
	{
		phead[i].rows = 64;
		phead[i].effnum = 1;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*4];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum*4);
		
		mp_sint32 pSize = 64*header->channum*4 + 64;
		if (pack)
		{
			f.read(buffer, 1, 2);
			pSize = BigEndian::GET_WORD(buffer);
			if (pSize && pSize < 64)
				return MP_LOADER_FAILED;
			// read packing mask
			f.read(buffer, 1, 64);
		}
		else
		{
			memset(buffer, 0xFF, 64);
		}
		
		if (!pSize)
			continue;
		
		mp_ubyte* pattern = new mp_ubyte[pSize-64];
		
		if (pattern == NULL)
			return MP_OUT_OF_MEMORY;
		
		f.read(pattern, 1, pSize-64);
		
		mp_ubyte* patPtr = pattern;
		
		mp_sint32 r,c,cnt=0;
		
		if (pack)
		{
			for (r=0;r<64;r++) {
				for (c=0;c<header->channum;c++) {
					if (buffer[r] & (1 << (7-c)))
					{			
						convertEvent(phead[i].patternData + cnt, patPtr);
						
						patPtr+=4;				
					}
					cnt+=4;
				}
			}
		}
		else
		{
			for (c=0;c<header->channum;c++) {
				for (r=0;r<64;r++) {
					cnt = r*header->channum*4+c*4;
				
					convertEvent(phead[i].patternData + cnt, patPtr);
									
					patPtr+=4;				
				}
			}
		}
		
		
		delete[] pattern;
	}

	// No for something really stupid: Pattern loop correction
	for (mp_sint32 p = 0; p < header->patnum; p++)
	{
		struct Position
		{
			mp_sint32 row, channel;
		};
		
		mp_ubyte* pattern = phead[p].patternData;
	
		Position loopStart = {-1, -1}, loopEnd = {-1, -1};
		
		for (i = 0; i < 64; i++) {
			for (j = 0; j < header->channum; j++) 
			{
				mp_ubyte* slot = pattern + i*header->channum*4 + j*4;
				
				// Loop start
				if (slot[2] == 0x36 && !slot[3])
				{
					loopStart.row = i;
					loopStart.channel = j;
				}
				else if (slot[2] == 0x36 && slot[3])
				{
					loopEnd.row = i;
					loopEnd.channel = j;
				}
			}
			
			if (loopEnd.row != -1 && loopEnd.channel != -1 &&
				loopStart.row != -1 && loopStart.channel != -1 &&
				loopStart.channel != loopEnd.channel)
			{
			
				// sanity check
				if (loopStart.row == loopEnd.row)
				{
					mp_ubyte* slot = pattern + loopStart.row*header->channum*4 + loopStart.channel*4;
					slot[2] = slot[3] = 0;
					slot = pattern + loopEnd.row*header->channum*4 + loopEnd.channel*4;
					slot[2] = slot[3] = 0;
				}
				else
				{
					
					for (j = 0; j < header->channum; j++) 
					{
						mp_ubyte* slotStart = pattern + loopStart.row*header->channum*4 + j*4;
						mp_ubyte* slotEnd = pattern + loopEnd.row*header->channum*4 + j*4;
					
						if (!slotStart[2] && !slotEnd[2] &&
							!slotStart[3] && !slotEnd[3])
						{
							mp_ubyte* slot = pattern + loopStart.row*header->channum*4 + loopStart.channel*4;
							slotStart[2] = slot[2];
							slotStart[3] = slot[3];
							slot[2] = slot[3] = 0;
							slot = pattern + loopEnd.row*header->channum*4 + loopEnd.channel*4;
							slotEnd[2] = slot[2];
							slotEnd[3] = slot[3];
							slot[2] = slot[3] = 0;
							break;
						}
					}
					
				}
				
				loopStart.row = loopStart.channel = loopEnd.row = loopEnd.channel = -1;
				
			}
		}
		
	} 

	mp_sint32 result = module->loadModuleSamples(f);
	if (result != MP_OK)
		return result;

	header->speed = 125;
	header->tempo = 6;
	header->mainvol = 255;
	
	strcpy(header->tracker,"Digibooster");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;
}
