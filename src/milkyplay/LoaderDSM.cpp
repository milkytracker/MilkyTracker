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
 *  LoaderDSM.cpp
 *  MilkyPlay Module Loader: Dynamic Studio & DSIK old and new Internal file formats
 */
#include "Loaders.h"


/////////////////////////////////////////////////
// Dynamic Studio DSM
/////////////////////////////////////////////////
const char* LoaderDSm::identifyModule(const mp_ubyte* buffer)
{
	// check for .DSM module
	if (!memcmp(buffer,"DSm\x1A\x20",5)) 
	{
		return "DSm";
	}

	return NULL;
}

mp_sint32 LoaderDSm::load(XMFileBase& f, XModule* module)
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
	
	strcpy(header->tracker,"Dynamic Studio");

	f.read(header->sig, 1, 3);
	f.readByte();

	if (f.readByte() != 0x20)
		return MP_LOADER_FAILED;
		
	f.read(header->name, 1, 20);
	
	f.readDword();
	f.readDword();
	f.readDword();
	f.readDword();
	f.readDword();

	header->channum = f.readByte();
	header->insnum = f.readByte();	
	header->ordnum = f.readByte();
	header->tempo = 6;
	header->speed = 125;
	
	f.readByte();
	
	header->mainvol = ((mp_sint32)f.readByte() * 255) / 100;

	f.readDword();
	f.readDword();
	f.readDword();
	f.readWord();
	
	mp_sint32 i,j;
	
	for (i = 0; i < header->channum; i++)
		header->pan[i] = (mp_ubyte)XModule::pan15to255(f.readByte());
			
	f.read(header->ord, 1, header->ordnum);

	header->patnum=0;
	for (i=0;i<header->ordnum;i++)
		if (header->ord[i]>header->patnum) header->patnum=header->ord[i];
	
	header->patnum++;	
	
	// skip trackinfo
	for (i = 0; i < header->patnum * header->channum * 2; i++)
		f.readDword();
	
	mp_sint32 s = 0;
	for (i = 0; i < header->insnum; i++)
	{
		f.read(instr[i].name, 1, 22);
		
		mp_ubyte type = f.readByte();
		mp_sint32 len = f.readWord();
		mp_ubyte ft = f.readByte();
		mp_ubyte vol = f.readByte();
		mp_sint32 repstart = f.readWord();
		mp_sint32 replen = f.readWord();
		f.readByte();

		if ((len>2) && type)
		{
			
			memcpy(smp[s].name, instr[i].name, 22);
			
			instr[i].samp=1;

			for (j = 0; j < 120; j++) 
				instr[i].snum[j] = s;
		
			module->convertc4spd(module->sfinetunes[ft],&smp[s].finetune,&smp[s].relnote);

			smp[s].flags = 1;
			smp[s].samplen = len;
			smp[s].loopstart = repstart;
			smp[s].looplen = replen;
			smp[s].vol = XModule::vol64to255(vol);

			if (replen > 2)
				smp[s].type = 1;
			
			if (type == 16)
				smp[s].type |= 16;
			
			s++;
		}
		
	}
	
	header->smpnum = s;
	
	// read patterns
	
	for (i = 0; i < header->patnum; i++)
	{
		mp_ubyte* pattern = new mp_ubyte[header->channum * 4 * 64];
		
		f.read(pattern, 1, header->channum * 4 * 64);
		
		phead[i].rows = 64;
		phead[i].effnum = 1;
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
		for (r = 0; r < phead[i].rows; r++) 
		{
			for (c = 0; c < header->channum;c++) 
			{
				mp_ubyte note = pattern[cnt+1];
				
				if (note)
				{
					note >>= 1;
					note+=12*2;
				}
					
				mp_ubyte ins = pattern[cnt];
				mp_ubyte eff = pattern[cnt+2];
				mp_ubyte op = pattern[cnt+3];
				
				phead[i].patternData[offs] = note;
				phead[i].patternData[offs+1] = ins;
				
				switch (eff)
				{
					case 0x00:
						if (op)
							eff = 0x20;
						break;
						
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

					case 0x08:
						{
							switch (op >> 4)
							{
								// set panning
								case 0x0:
									op=(mp_ubyte)XModule::pan15to255(op);
									break;
									
								// fine porta up
								case 0x3:
									eff = 0x31;
									op &= 0xf;
									break;

								// fine porta down
								case 0x4:
									eff = 0x32;
									op &= 0xf;
									break;
									
								default:
#ifdef VERBOSE
									printf("Unsupported command: %i/%i\n", eff, op);
#endif
									break;
							}
						}
						break;
						
					case 0x0C:
						op = XModule::vol64to255(op);
						break;
						
					case 0x0E:
						eff = 0x30 + (op >> 4);
						op &= 0xF;
						break;

					case 0x11:
						eff = 0x01;
						break;
						
					case 0x12:
						eff = 0x02;
						break;
					
					default:
#ifdef VERBOSE
						printf("Unsupported command: %i/%i\n", eff, op);
#endif
						eff = op = 0;
				}

				// old style modules don't support last effect for:
				// - portamento up/down
				// - volume slide
				if (eff==0x1&&(!op)) eff = 0;
				if (eff==0x2&&(!op)) eff = 0;
				if (eff==0xA&&(!op)) eff = 0;
				
				if (eff==0x5&&(!op)) eff=0x3;
				if (eff==0x6&&(!op)) eff=0x4;
				
				phead[i].patternData[offs+2] = eff;
				phead[i].patternData[offs+3] = op;
				
				offs+=(phead[i].effnum * 2 + 2);
				
				cnt+=4;
			}
						
		}
		
		delete[] pattern;
		
	}	
	
	mp_sint32 result = module->loadModuleSamples(f);
	if (result != MP_OK)
		return result;
	
	//module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;		
}

/////////////////////////////////////////////////
// Old DSIK
/////////////////////////////////////////////////
const char* LoaderDSMv1::identifyModule(const mp_ubyte* buffer)
{
	// check for .DSM module
	if (!memcmp(buffer,"DSM\x10",4)) 
	{
		return "DSMv1";
	}

	return NULL;
}

static mp_sint32 convertDSMPattern(TXMPattern* XMPattern,
								   const mp_ubyte* srcPattern,
								   mp_uint32 maxChannels,
								   mp_sint32 patCnt,
								   mp_sint32 octaveAdjust = 24)
{
	
	XMPattern->channum = maxChannels;
	XMPattern->effnum = 2;
	XMPattern->rows = 64;
	XMPattern->patternData = new mp_ubyte[maxChannels*6*64];
	
	if (XMPattern->patternData == NULL)
	{
		return MP_OUT_OF_MEMORY;
	}
	
	memset(XMPattern->patternData,0,maxChannels*6*64);
	
	mp_ubyte* dstSlot = XMPattern->patternData;
	for (mp_sint32 row = 0; row < 64; row++)
		for (mp_uint32 c = 0; c < maxChannels; c++)
		{
			const mp_ubyte* srcSlot = srcPattern+row*32*5+c*5;
			
			mp_ubyte note = srcSlot[0];
			
			if (note) 
				note+=octaveAdjust;
			
			dstSlot[0] = note;
			
			dstSlot[1] = srcSlot[1];
			
			if (srcSlot[2]<=64)
			{
				
				dstSlot[2] = 0xC;
				dstSlot[3] = XModule::vol64to255(srcSlot[2]);
				
			}
			
			mp_ubyte eff = srcSlot[3];
			mp_ubyte op = srcSlot[4];
			
			if (eff <= 0xF)
			{				
				if (eff==0xE) {
					eff=(op>>4)+0x30;
					op&=0xf;
				}
				
				if ((!eff)&&op) 
					eff=0x20;
				
				if (eff==0x5&&(!op)) eff=0x3;
				if (eff==0x6&&(!op)) eff=0x4;
				
				if (eff==0xC) 
				{
					op = XModule::vol64to255(op);
				}
				
				if (eff == 0x38)
				{
					eff = 0x8;
					op = XModule::pan15to255(op);
				}
				
				dstSlot[4] = eff;
				dstSlot[5] = op;
			}
			
			dstSlot+=6;
		}
			
	return MP_OK;	
}

mp_sint32 LoaderDSMv1::load(XMFileBase& f, XModule* module)
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
	
	f.read(header->sig,3,1);
	
	f.readByte();
	
	f.read(&header->name,1,28);
	
	f.readDword(); // skip next 5 bytes
	f.readByte();
	
	header->channum = f.readByte();	// number of channels
	header->ordnum = f.readByte();	// songlength
	header->insnum = f.readByte();
	header->patnum = f.readByte();
	header->tempo = f.readByte();
	header->speed = f.readByte();
	header->mainvol = XModule::vol64to255(f.readByte()); // initial main volume
	
	f.readDword(); // skip next 4 bytes
	
	f.read(header->ord,1,128);
	
	mp_uword* insParaPtrs = new mp_uword[header->insnum];
	
	if (insParaPtrs == NULL)
		return MP_OUT_OF_MEMORY;
	
	f.readWords(insParaPtrs,header->insnum);
	
	mp_uword* patParaPtrs = new mp_uword[header->patnum];
	
	if (patParaPtrs == NULL)
	{
		delete[] insParaPtrs;
		return MP_OUT_OF_MEMORY;
	}
	
	f.readWords(patParaPtrs,header->patnum);
	
	//for (i = 0; i < header->insnum; i++)
	//{
	//	printf("%x\n",insParaPtrs[i]*16);
	//}
		
	//////////////////////
	// read instruments //
	//////////////////////
	mp_uint32* samplePtrs = new mp_uint32[header->insnum];
	if (samplePtrs == NULL)
	{
		delete[] insParaPtrs;
		delete[] patParaPtrs;
		return MP_OUT_OF_MEMORY;
	}
	
	memset(samplePtrs,0,sizeof(mp_uint32)*header->insnum);
	
	mp_sint32 s = 0;
	
	for (i = 0; i < header->insnum; i++)
	{
		mp_uint32 insOffs = insParaPtrs[i]*16;
		
		if (insOffs)
		{
			f.seekWithBaseOffset(insOffs);
			
			//f.read(buffer,1,64);
			
			if (f.readDword() == 0x10495344) // "DSI"+0x10
			{
				f.read(instr[i].name,1,30); // instrument name				
				memcpy(smp[s].name,instr[i].name,30);
				
				f.readByte(); // zero terminator maybe?
				
				mp_ubyte bOffs = f.readByte();
				mp_uword wOffs = f.readWord();
				
				// stupid fileoffsets
				samplePtrs[s] = (((mp_uint32)bOffs<<16)+(mp_uint32)wOffs)*16;
				
				smp[s].flags = 1;
				smp[s].pan = 0x80;
				
				smp[s].samplen = f.readWord();
				smp[s].loopstart = f.readWord();
				mp_sint32 looplen = ((mp_uint32)f.readWord() - (mp_uint32)smp[s].loopstart);
				if (looplen < 0) 
					looplen = 0;
				smp[s].looplen = looplen;
				
				XModule::convertc4spd(f.readWord(),&smp[s].finetune,&smp[s].relnote);
				
				smp[s].vol = XModule::vol64to255(f.readByte());
				
				if (looplen>0)
					smp[s].type=1; // looped sample
				
				if (smp[s].samplen)
				{
					instr[i].samp=1;
					for (j=0;j<120;j++) 
						instr[i].snum[j] = s;
					s++;
				}
				
				//f.readDword();
				//f.readDword();
				//f.readDword();
				//f.readDword();
				//f.readByte();
				
			}			
		}
	}
	
	header->smpnum = s;
	
	//////////////////////
	// read patterns	//
	//////////////////////
	mp_ubyte* pattern = new mp_ubyte[64*32*5];
	if (pattern == NULL)
	{
		delete[] insParaPtrs;
		delete[] patParaPtrs;
		delete[] samplePtrs;
		return MP_OUT_OF_MEMORY;
	}
	
	for (i = 0; i < header->patnum; i++)
	{
		for (j = 0; j < 32*64; j++)
		{
			pattern[j*5] = 0;
			pattern[j*5+1] = 0;
			pattern[j*5+2] = 0xFF;
			pattern[j*5+3] = 0;
			pattern[j*5+4] = 0;
		}

		mp_uint32 maxChannels = 1;		
		
		mp_uint32 patOffs = patParaPtrs[i]*16;

		if (patOffs)
		{
			f.seekWithBaseOffset(patOffs);
			
			mp_uint32 size = f.readWord()-2;
			
			mp_ubyte* packed = new mp_ubyte[size];
			if (packed == NULL)
			{
				delete[] insParaPtrs;
				delete[] patParaPtrs;
				delete[] samplePtrs;
				delete[] pattern;
				return MP_OUT_OF_MEMORY;				
			}
			
			f.read(packed,1,size);
			
			mp_uint32 index = 0;
			mp_uint32 row = 0;
			
			mp_ubyte obfuscate = 3;
			
			while (index<size)
			{
				
				mp_ubyte pi = packed[index++]^obfuscate++;
				
				if (pi == 0) {
					row++;
					continue;
				}
				
				mp_uint32 chn = pi&15;
				
				if (chn>maxChannels)
					maxChannels = chn;
				
				mp_ubyte* slot = pattern+(row*32*5)+chn*5;
				
				if (pi & 128)
				{
					slot[0] = packed[index++];
					obfuscate++;
				}
				if (pi & 64)
				{
					slot[1] = packed[index++];
					obfuscate++;
				}
				if (pi & 32)
				{
					slot[2] = packed[index++];
					obfuscate++;
				}
				if (pi & 16)
				{
					slot[3] = packed[index++];
					slot[4] = packed[index++];
					obfuscate++;
					obfuscate++;
				}
				
			}
			
			maxChannels++;
			
			if (maxChannels > header->channum)
				maxChannels = header->channum;
			
			delete[] packed;
		}
		
		convertDSMPattern(&phead[i],pattern, maxChannels, i);			
		
	}
	
	delete[] pattern;
	delete[] insParaPtrs;
	delete[] patParaPtrs;
	
	for (i = 0; i < header->smpnum; i++)
	{
		f.seekWithBaseOffset(samplePtrs[i]);
		if (module->loadModuleSample(f,i) != 0)
		{
			delete[] samplePtrs;
			return MP_OUT_OF_MEMORY;
		}			
	}
	
	strcpy(header->tracker,"Digisound Interface Kit");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;	
}

/////////////////////////////////////////////////
// New DSIK
/////////////////////////////////////////////////
const char* LoaderDSMv2::identifyModule(const mp_ubyte* buffer)
{
	// check for .DSM module
	if (!memcmp(buffer,"RIFF",4) && 
		!memcmp(buffer+8,"DSMFSONG",8)) 
	{
		return "DSMv2";
	}

	return NULL;
}

mp_sint32 LoaderDSMv2::load(XMFileBase& f, XModule* module)
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
	
	// Even though the new DSM format is chunk based we don't read it  
	// in a real chunk fashion i.e. we're assuming fixed structure
	
	// skip RIFF and chunk length
	f.seekWithBaseOffset(8);
	
	f.read(header->sig, 1, 4);
	
	// skip "SONG" & chunk length
	f.readDword();
	if (f.readDword() != 0xC0)
		return MP_LOADER_FAILED;
		
	f.read(header->name, 1, 28);
	
	mp_uword ver = f.readWord();
	mp_uword flags = f.readWord();
	f.readDword();
	header->ordnum = f.readWord();
	header->insnum = f.readWord();
	header->patnum = f.readWord();
	header->channum = f.readWord();
	header->mainvol = XModule::vol64to255(f.readByte());
	mp_ubyte mainvol = f.readByte();
	header->tempo = f.readByte();
	header->speed = f.readByte();
	mp_ubyte panpos[16];
	f.read(panpos, 1, 16);
	f.read(header->ord, 1, 128);
	
	mp_sint32 patcnt = 0;
	mp_sint32 inscnt = 0;
	mp_sint32 smpcnt = 0;
	
	mp_uint32 size = f.sizeWithBaseOffset();
	
	while (f.posWithBaseOffset() < size && (patcnt < header->patnum || inscnt < header->insnum))
	{
		mp_ubyte ID[4];
		f.read(ID, 1, 4);
		mp_dword chunkLen = f.readDword();
		
		switch (BigEndian::GET_DWORD(&ID))
		{
			// "PATT"
			case 0x50415454:
			{
				mp_sint32 j;
			
				mp_uint32 size = f.readWord()-2;
			
				mp_ubyte* packed = new mp_ubyte[size];
				if (packed == NULL)
				{
					return MP_OUT_OF_MEMORY;				
				}			
				
				f.read(packed, 1, size);
				
				mp_ubyte* pattern = new mp_ubyte[64*32*5];
				if (pattern == NULL)
				{
					delete[] packed;
					return MP_OUT_OF_MEMORY;
				}
	
				for (j = 0; j < 32*64; j++)
				{
					pattern[j*5] = 0;
					pattern[j*5+1] = 0;
					pattern[j*5+2] = 0xFF;
					pattern[j*5+3] = 0;
					pattern[j*5+4] = 0;
				}

				mp_uint32 maxChannels = 1;		
		
				mp_uint32 index = 0;
				mp_uint32 row = 0;
			
				while (index<size && row < 64)
				{
					mp_ubyte pi = packed[index++];
				
					if (pi == 0) 
					{
						row++;
						continue;
					}
				
					mp_uint32 chn = pi&15;
				
					if (chn>maxChannels)
						maxChannels = chn;
				
					mp_ubyte* slot = pattern+(row*32*5)+chn*5;
				
					if (pi & 128)
					{
						slot[0] = packed[index++];
					}
					if (pi & 64)
					{
						slot[1] = packed[index++];
					}
					if (pi & 32)
					{
						slot[2] = packed[index++];
					}
					if (pi & 16)
					{
						slot[3] = packed[index++];
						slot[4] = packed[index++];
					}
				}
			
				maxChannels++;
			
				if (maxChannels > header->channum)
					maxChannels = header->channum;
			
				delete[] packed;
		
				convertDSMPattern(&phead[patcnt], pattern, maxChannels, patcnt, 0);
		
				delete[] pattern;				
				
				patcnt++;
				
				break;
			}
				
			// "INST"
			case 0x494E5354:
			{
				mp_ubyte dosname[13];
				f.read(dosname, 1, 13);
				
				mp_uword flags = f.readWord();
				mp_ubyte vol = f.readByte();
				mp_dword length = f.readDword();
				mp_dword loopstart = f.readDword();
				mp_sint32 looplen = f.readDword() - loopstart;
				if (looplen < 0) 
					looplen = 0;
				f.readDword();
				
				mp_sint32 c2spd = f.readWord();		
				mp_sint32 period = f.readWord();		 		

				f.read(instr[inscnt].name, 1, 28);
				if (length)
				{
					instr[inscnt].samp = 1;
					for (mp_sint32 j = 0; j < 120; j++) 
						instr[inscnt].snum[j] = smpcnt;
						
					TXMSample* s = &smp[smpcnt];
					s->flags = 1;
					memcpy(s->name, dosname, sizeof(dosname));
					s->samplen = length;
					s->loopstart = loopstart;
					s->looplen = looplen;
					if (flags & 1)
						s->type = 1;
					s->vol = XModule::vol64to255(vol);
					s->pan = 0x80;
					XModule::convertc4spd(c2spd, &s->finetune, &s->relnote);
				
					s->sample = (mp_sbyte*)module->allocSampleMem(length);
				
					if (s->sample == NULL)
					{
						return MP_OUT_OF_MEMORY;
					}
				
					if (!module->loadSample(f, s->sample, length, length, (flags & 2) ? XModule::ST_DEFAULT : XModule::ST_UNSIGNED))
					{
						return MP_OUT_OF_MEMORY;
					}					
				
					smpcnt++;
				}

				inscnt++;
			
				break;
			}
		}
	}
	
	strcpy(header->tracker,"Digisound Interface Kit");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;	
}
