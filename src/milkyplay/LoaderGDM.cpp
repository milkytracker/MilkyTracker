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
 *  LoaderGDM.cpp
 *  MilkyPlay Module Loader: GDM General Digimusic
 */
#include "Loaders.h"

const char* LoaderGDM::identifyModule(const mp_ubyte* buffer)
{
	// check for .GDM module
	if (!memcmp(buffer,"GDM\xFE",4)) 
	{
		return "GDM";
	}

	return NULL;
}

struct tgdmheader {
	mp_ubyte sig[4];        // 'GDM˛'                                    000
	mp_ubyte name[32];      // name of the song                          004
	mp_ubyte composer[32];  // composer?                                 036
	mp_uword returnword;   // return word (0x0D0A)                      068
	mp_ubyte whythis1a;     // unknown                                   070
	mp_ubyte mainid[4];     // 'GFMS'                                    071
	mp_ubyte crap[6];       // maybe some time in the future             075
	mp_ubyte panset[32];    // Channel panning (0xFF=Unused channel)     081
	mp_ubyte mainvol;       // Main volume of the song (0-64)            113
	mp_ubyte tickspd;       // initial tickspeed                         114
	mp_ubyte bpmspd;        // initial bpmspeed                          115
	mp_uword unknown1;     // not known yet                             116
	mp_uint32 orderlistpos;  // adress for orderlist (in the file)        118
	mp_ubyte ordnum;        // length of the song-1                      122
	mp_uint32 patternpos;    // adress for first pattern (in the file)    123
	mp_ubyte patnum;        // number of patterns-1                      127
	mp_uint32 insinfopos;    // adress for first instrument (in the file) 128
	mp_uint32 samplepos;     // adress for first sample (in the file)     132
	mp_ubyte insnum;        // number of instruments-1                   136
	mp_ubyte channum;       // number of channels
	mp_ubyte ord[256];      // orderlist
};

struct tgdmsample {
	mp_ubyte name[32];      // name of instrument                        000
	mp_ubyte filename[13];  // filename with 0 terminator                032
	mp_uint32 samplen;       // size of sample                            045
	mp_uint32 loopstart;     // loop start                                049
	mp_uint32 loopend;       // loop end+1                                053
	mp_ubyte unknown1;      // not known yet                             057
	mp_uint32 c4speed;      // c4speed of sample                         058
	mp_ubyte vol;           // volume of sample                          060
	mp_ubyte unknown2;      // not known yet                             061
};

struct TXMNoteSlot {
	mp_uword Note,Ins,Eff[4],Op[4];
};

mp_sint32 LoaderGDM::load(XMFileBase& f, XModule* module)
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

	tgdmheader GDMHeader;
	tgdmsample GDMSmp;
	
	f.read(GDMHeader.sig, 1, 4);
	f.read(header->name, 1, 32);
	f.read(GDMHeader.composer, 1, 32);
	GDMHeader.returnword = f.readWord();
	GDMHeader.whythis1a = f.readByte();
	f.read(GDMHeader.mainid, 1, 4);
	f.read(GDMHeader.crap, 1, 6);
	f.read(GDMHeader.panset, 1, 32);
	GDMHeader.mainvol = f.readByte();
	GDMHeader.tickspd = f.readByte();
	GDMHeader.bpmspd = f.readByte();
	GDMHeader.unknown1 = f.readWord();
	GDMHeader.orderlistpos = f.readDword();
	GDMHeader.ordnum = f.readByte();
	GDMHeader.patternpos = f.readDword();
	GDMHeader.patnum = f.readByte();
	GDMHeader.insinfopos = f.readDword();
	GDMHeader.samplepos = f.readDword();
	GDMHeader.insnum = f.readByte();

	memcpy(&header->sig, &GDMHeader.sig, 3);

	strcpy(header->tracker,"..converted..");
	
	header->ordnum = GDMHeader.ordnum + 1;
	header->patnum = GDMHeader.patnum + 1;
	header->insnum = GDMHeader.insnum + 1;
	header->tempo = GDMHeader.tickspd;
	header->speed = GDMHeader.bpmspd;
	header->mainvol = XModule::vol64to255(GDMHeader.mainvol);

	f.seekWithBaseOffset(GDMHeader.orderlistpos);
	f.read(&header->ord, 1, header->ordnum);
	i = 0;
	do {
		i++;
	} while (i < sizeof(GDMHeader.panset) && GDMHeader.panset[i]!=0xFF);
	header->channum = i;
	
	f.seekWithBaseOffset(GDMHeader.insinfopos);
	
	mp_sint32 s = 0;
	for (i = 0; i < header->insnum; i++)
	{
		
		f.read(GDMSmp.name, 1, 32);
		
#ifdef VERBOSE
		printf("%s\n",GDMSmp.name);
#endif
	
		f.read(GDMSmp.filename, 1, 13);
		GDMSmp.samplen = f.readDword();
		GDMSmp.loopstart = f.readDword();
		GDMSmp.loopend = f.readDword() - 1;
		GDMSmp.unknown1 = f.readByte();
		GDMSmp.c4speed = f.readWord();
		GDMSmp.vol = f.readByte();
		GDMSmp.unknown2 = f.readByte();

		if (GDMSmp.samplen>2) 
		{
			
			memcpy(instr[i].name, GDMSmp.name, 30);
			memcpy(smp[s].name, GDMSmp.filename, 12);
			
			smp[s].flags = 1;
			smp[s].pan = 0x80;
			
			smp[s].samplen = GDMSmp.samplen;
			smp[s].loopstart = GDMSmp.loopstart;
			mp_sint32 looplen = (GDMSmp.loopend - smp[s].loopstart);
			if (looplen < 0) 
				looplen = 0;
			smp[s].looplen = looplen;
			
			XModule::convertc4spd(GDMSmp.c4speed,&smp[s].finetune,&smp[s].relnote);
			
			smp[s].vol = XModule::vol64to255(GDMSmp.vol);
			
			if (looplen>2)
				smp[s].type=1; // looped sample
			
			instr[i].samp=1;
			for (j=0;j<120;j++) 
				instr[i].snum[j] = s;
			
			s++;
		}
		
	}

	header->smpnum = s;

	f.seekWithBaseOffset(GDMHeader.patternpos);
	
	for (i=0;i<header->patnum;i++) 
	{
		//cprintf("converting pattern %x/%x\r",PatCnt+1,GDMHeader.patnum);
		mp_sint32 patsize = f.readWord();
	
		patsize-=2;

		mp_sint32 slotSize = 2 + 4*2;

		phead[i].rows = 64;
		phead[i].effnum = 4;
		phead[i].channum= (mp_ubyte)header->channum;
		
		//phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*slotSize];

		mp_ubyte* tempPattern = new mp_ubyte[phead[i].rows*header->channum*slotSize];

		if (tempPattern == NULL)
			return MP_OUT_OF_MEMORY;

		// out of memory?
		//if (phead[i].patternData == NULL)
		//	return MP_OUT_OF_MEMORY;
		
		//memset(phead[i].patternData,0,phead[i].rows*header->channum*slotSize);

		memset(tempPattern,0,phead[i].rows*header->channum*slotSize);
		
		if (patsize) {
			mp_ubyte* packed=new mp_ubyte[patsize*2];

			if (packed==NULL) 
				return MP_OUT_OF_MEMORY;

			memset(packed, 0, patsize*2);
			
			f.read(packed, 1, patsize);

			mp_sint32 offs	= 0;
			mp_sint32 pos	= 0;
			mp_uword b1		= 0;
			mp_uword b2		= 0;

			mp_sint32 maxEffects = 0;

			for (mp_sint32 RowCnt=0;RowCnt<64;RowCnt++) 
			{
				do 
				{
					b1=packed[offs++];
					if (b1) {
						pos=(RowCnt*header->channum)+(b1&31);
						
						TXMNoteSlot XMSlot;
						memset(&XMSlot, 0, sizeof(XMSlot));
						
						if ((b1&32)) {
							b2=packed[offs++]&127;
							XMSlot.Note=(b2>>4)*12+(b2&0xf);
							XMSlot.Ins=packed[offs++];
						}
						if ((b1&64)) {
							
							mp_ubyte eff = 0;
							do
							{
								eff = packed[offs++];
								
								XMSlot.Eff[eff>>6] = eff&0x1f;
								XMSlot.Op[eff>>6] = packed[offs++];
								
								if ((eff>>6) + 1 > maxEffects)
									maxEffects = (eff>>6) + 1;

								/*if (b1==0x2C) {
									XMSlot.Vol=b2+0x10;
									XMSlot.Eff=packed[offs++]&0x1f;
									XMSlot.Op=packed[offs++];
								}
								else {
									if (b1==0xC) 
										XMSlot.Vol=b2+0x10;
									else {
										XMSlot.Eff=b1&0x1f;
										XMSlot.Op=b2;
									}
								}*/
							} while (eff&32);
						}
						
						mp_sint32 dstOffs = pos * slotSize;
						
						tempPattern[dstOffs]=(mp_ubyte)XMSlot.Note;
						tempPattern[dstOffs+1]=(mp_ubyte)XMSlot.Ins;
						
						for (mp_sint32 j = 0; j < phead[i].effnum; j++)
						{

							switch (XMSlot.Eff[j])
							{
								case 0x00:
									if (XMSlot.Op[j]) XMSlot.Eff[j]=0x20;
									break;
								case 0x01:
								case 0x02:
								case 0x03:
								case 0x04:
								case 0x05:
								case 0x06:
								case 0x07:
								case 0x09:
								case 0x0a:
								case 0x0b:
								case 0x0d:
								case 0x0e:
									break;
								case 0x0c:
									XMSlot.Op[j] = XModule::vol64to255(XMSlot.Op[j]);	
									break;
								// tremor
								case 0x08:
									XMSlot.Eff[j] = XMSlot.Op[j] = 0;
									break;
								case 0x0f:
									XMSlot.Eff[j] = 0x1c;
									break;
								// arpeggio
								case 0x10:
									XMSlot.Eff[j] = 0x20;
									break;
								case 0x12:
									XMSlot.Eff[j] = 0x39;
									break;
								case 0x13:
									XMSlot.Eff[j] = 0x10;
									XMSlot.Op[j] = XModule::vol64to255(XMSlot.Op[j]);	
									break;								
								// fine vibrato
								case 0x14:
									XMSlot.Eff[j] = 0x4a;
									break;
								// set bpm
								case 0x1f:
									XMSlot.Eff[j] = 0x16;
									break;								
								default:
									XMSlot.Eff[j] = XMSlot.Op[j] = 0;
							}
							
							//if ((!XMSlot.Eff[j])&&(XMSlot.Op[j])) XMSlot.Eff[j]=0x20;
							
							if (XMSlot.Eff[j]==0xE) {
								XMSlot.Eff[j]=(XMSlot.Op[j]>>4)+0x30;
								XMSlot.Op[j]=XMSlot.Op[j]&0xf;
							}
							
							tempPattern[dstOffs+2+j*2]=(mp_ubyte)XMSlot.Eff[j];
							tempPattern[dstOffs+2+j*2+1]=(mp_ubyte)XMSlot.Op[j];
						}

						
					}
				} while (b1);
			}
			
			delete[] packed;

			// sort out unused effects
			mp_sint32 newSlotSize = 2 + maxEffects*2;

			phead[i].rows = 64;
			phead[i].effnum = maxEffects;
			phead[i].channum= (mp_ubyte)header->channum;
		
			phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*newSlotSize];

			// out of memory?
			if (phead[i].patternData == NULL)
				return MP_OUT_OF_MEMORY;

			memset(phead[i].patternData,0,phead[i].rows*header->channum*newSlotSize);

			for (mp_sint32 r = 0; r < phead[i].rows; r++)
				for (mp_sint32 c = 0; c < phead[i].channum; c++)
				{
					mp_sint32 srcOffs = (r * phead[i].channum + c) * slotSize;
					mp_sint32 dstOffs = (r * phead[i].channum + c) * newSlotSize;
				
					// lazyness here
					memcpy(phead[i].patternData + dstOffs, tempPattern + srcOffs, newSlotSize);
				}

			delete[] tempPattern;

		}
			
	}
				
	f.seekWithBaseOffset(GDMHeader.samplepos);

	mp_sint32 result = module->loadModuleSamples(f, XModule::ST_UNSIGNED);
	if (result != MP_OK)
		return result;	

	strcpy(header->tracker,"General Digimusic");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;	
}	
