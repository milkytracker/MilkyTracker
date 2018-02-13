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
 *  LoaderIT.cpp
 *  MilkyPlay Module Loader: Impulse Tracker
 */
#include "Loaders.h"

const char* LoaderIT::identifyModule(const mp_ubyte* buffer)
{
	// check for .IT module
	if (!memcmp(buffer,"IMPM",4)) 
	{
		return "IT";
	}

	// this is not an .IT
	return NULL;
}

struct ITEnvelope
{
	mp_ubyte Flg;
	mp_ubyte Num;
	mp_ubyte LpB;
	mp_ubyte LpE;
	mp_ubyte SLB;
	mp_ubyte SLE;
	mp_sbyte Nodes[75];
	mp_ubyte unused;
};

struct ITInstrument 
{
	char sig[4];
	char dosName[12];
	mp_ubyte unused1;

	mp_ubyte NNA;
	mp_ubyte DCT;
	mp_ubyte DCA;
	mp_uword FadeOut;
	mp_ubyte PPS;
	mp_ubyte PPCxx;
	mp_ubyte GbV;
	mp_ubyte DfP;
	mp_ubyte RV;
	mp_ubyte RP;
	mp_uword TrkVers;
	mp_ubyte NoS;

	mp_ubyte unused2;
	char name[26];
	
	mp_ubyte IFC;
	mp_ubyte IFR;
	mp_ubyte MCh;
	mp_ubyte MPr;
	mp_uword MIDIBnk;
		
	mp_ubyte snum[240]; 

	ITEnvelope volEnv;
	ITEnvelope panEnv;
	ITEnvelope pitchEnv;
};

struct ITSample 
{
	char sig[4];
	char dosName[12];
	mp_ubyte unused1;

	mp_ubyte GvL;
	mp_ubyte Flg;
	mp_ubyte Vol;

	char name[26];

	mp_ubyte Cvt;
	mp_ubyte DfP;

	mp_dword Length;
    mp_dword LoopBeg;
    mp_dword LoopEnd;
    mp_dword C5Speed;
    mp_dword SusLBeg;
    mp_dword SusLEnd;

    mp_dword SmpPoint;

    mp_ubyte ViS;
    mp_ubyte ViD;
    mp_ubyte ViR;
    mp_ubyte ViT;
};

static void readITEnvelope(XMFileBase& f, ITEnvelope& itEnv)
{
	itEnv.Flg = f.readByte();
	itEnv.Num = f.readByte();
	itEnv.LpB = f.readByte();
	itEnv.LpE = f.readByte();
	itEnv.SLB = f.readByte();
	itEnv.SLE = f.readByte();
	f.read(itEnv.Nodes, 1, 75);
	itEnv.unused = f.readByte();
}

static void convertITEnvelope(TEnvelope& outEnv, const ITEnvelope& itEnv, mp_sword center = 0)
{
	memset(&outEnv, 0, sizeof(TEnvelope));
	outEnv.type = (itEnv.Flg & 1) | (((itEnv.Flg>>1) & 1)<<2) | ((itEnv.SLB == itEnv.SLE) ? (((itEnv.Flg>>2) & 1)<<1) : (((itEnv.Flg>>2) & 1)<<4));
	
	outEnv.num = itEnv.Num;
	outEnv.sustain = itEnv.SLB;
	outEnv.susloope = itEnv.SLE;
	outEnv.loops = itEnv.LpB;
	outEnv.loope = itEnv.LpE;
 
	for (mp_sint32 i = 0; i < outEnv.num; i++)
	{
		outEnv.env[i][0] = LittleEndian::GET_WORD(itEnv.Nodes+(i*3+1));
		outEnv.env[i][1] = ((mp_sword)itEnv.Nodes[i*3] + center) << 2;
	}
}

static void replace(char* str, mp_uint32 size, char from, char to)
{
	for (mp_uint32 i = 0; i < size; i++)
		if (str[i] == from)
			str[i] = to;
}

mp_sint32 LoaderIT::load(XMFileBase& f, XModule* module)
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
	
	f.read(header->sig, 1, 4);
	f.read(header->name, 1, 26);
	f.readWord();
	mp_uword ordnum = f.readWord();
	if(ordnum > MP_MAXORDERS)
		return MP_LOADER_FAILED;
	header->ordnum = ordnum;
	header->insnum = f.readWord();
	header->smpnum = f.readWord();

	header->patnum = f.readWord();

	mp_uword cwt = f.readWord();
	mp_uword cmwt = f.readWord();

	mp_uword flags = f.readWord();
	mp_uword special = f.readWord();

	header->freqtab = (flags >> 3) & 1;

	header->mainvol = XModule::vol128to255(f.readByte());
	
	header->flags = XModule::MODULE_ITNOTEOFF |
		XModule::MODULE_ST3DUALCOMMANDS |
		XModule::MODULE_ST3NEWINSTRUMENT |
		XModule::MODULE_ITTEMPOSLIDE |
		XModule::MODULE_XMPORTANOTEBUFFER;
	
	if (!(flags & 16))
		header->flags |= XModule::MODULE_ITNEWEFFECTS;
	if (!(flags & 32))
		header->flags |= XModule::MODULE_ITNEWGXX;
	else
		header->flags |= XModule::MODULE_ITLINKPORTAMEM;
													
	header->relnote = -12;

	// skip mixer volume
	f.readByte();

	header->tempo = f.readByte();
	header->speed = f.readByte();

	// skip Sep
	f.readByte();
	// skip PWD
	f.readByte();

	mp_uword messageLength = f.readWord();
	mp_dword messageOffset = f.readDword();

	// skip reserved
	f.readDword();

	mp_ubyte chnPan[64];
	f.read(chnPan, 1, 64);
	
	mp_ubyte chnVol[64];
	f.read(chnVol, 1, 64);

	// just assume the order list is smaller than 256 entries
	mp_ubyte orders[256];
	f.read(orders, 1, header->ordnum);
	
	j = 0;
	for (i = 0; i < header->ordnum; i++)
	{
		if (orders[i] == 255) 
			break;
		
		header->ord[j++] = orders[i];		
	}
	
	header->ordnum = j; // final songlength

	mp_dword insOffs[256];
	mp_dword smpOffs[256];
	mp_dword patOffs[256];

	// creating some nice potential buffer overflows here
	f.readDwords(insOffs, header->insnum);
	f.readDwords(smpOffs, header->smpnum);
	f.readDwords(patOffs, header->patnum);

	// read instruments
	for (i = 0; i < header->insnum; i++)
	{
		f.seekWithBaseOffset(insOffs[i]);
		
		ITInstrument ins;

		f.read(ins.sig, 1, 4);

		// should match "IMPI"
		if (LittleEndian::GET_DWORD(ins.sig) != 0x49504D49)
			return MP_LOADER_FAILED;

		f.read(ins.dosName, 1, 12);

		// 0x00?
		ins.unused1 = f.readByte();
		
		// enable note remapping
		instr[i].flags = TXMInstrument::IF_ITNOTEREMAPPING | 
						 TXMInstrument::IF_ITFADEOUT |
						 TXMInstrument::IF_ITENVELOPES |
						 TXMInstrument::IF_ITGOBALINSVOL;

		if (cmwt >= 0x200)
		{			
			ins.NNA = f.readByte();
			ins.DCT = f.readByte();
			ins.DCA = f.readByte();
			
			// sanity checks
			if (ins.NNA > 3) ins.NNA = 0;
			if (ins.DCT > 3) ins.DCT = 0;
			if (ins.DCA > 3) ins.DCA = 0;
			// fill in NNA stuff
			instr[i].flags |= (((mp_uword)ins.NNA) << 4);
			instr[i].flags |= (((mp_uword)ins.DCT) << 6);
			instr[i].flags |= (((mp_uword)ins.DCA) << 8);
			
			ins.FadeOut = f.readWord();
			
			mp_sint32 fade = (mp_sint32)ins.FadeOut*64;
			if (fade > 65535) fade = 65535;
			instr[i].volfade = fade;
			
			ins.PPS = f.readByte();
			ins.PPCxx = f.readByte();
			ins.GbV = f.readByte();
			
			instr[i].res = XModule::vol128to255(ins.GbV);
			
			ins.DfP = f.readByte();
			ins.RV = f.readByte();
			ins.RP = f.readByte();
			ins.TrkVers = f.readWord();
			ins.NoS = f.readByte();
			
			// x?
			ins.unused2 = f.readByte();
			
			f.read(ins.name, 1, 26);
			replace(ins.name, 26, '\0', ' ');
			memcpy(instr[i].name, ins.name, 26);
			
			ins.IFC = f.readByte();
			ins.IFR = f.readByte();
			ins.MCh = f.readByte();
			ins.MPr = f.readByte();
			ins.MIDIBnk = f.readWord();
			
			instr[i].ifc = ins.IFC;
			instr[i].ifr = ins.IFR;
			
			//printf("%i, %i\n", ins.IFC, ins.IFR);
			
			f.read(ins.snum, 1, 240);
			for (j = 0; j < 120; j++)
			{
				instr[i].snum[j] = -1;
				instr[i].notemap[j] = 0xff;
			}
			
			mp_ubyte sampleTable[256];
			memset(sampleTable, 0, sizeof(sampleTable));
			
			for (j = 0; j < 120; j++)
			{
				if (ins.snum[j*2] != 0xff)
				{
					mp_sint32 index = (mp_sword)ins.snum[j*2+1]-1;
					
					instr[i].snum[j] = index;
					instr[i].notemap[j] = ins.snum[j*2];

					//if ((j - 12) >= 0)
					//	instr[i].snum[j-12] = index;
					
					//if ((j - 12) >= 0 && (ins.snum[j*2] >= 12))		
					//	instr[i].notemap[j-12] = ins.snum[j*2] - 12;

					if (index >= 0 && index < 256)
						sampleTable[index] = 1;
				}
			}
			
			mp_sint32 smpNum = 0;
			for (j = 0; j < 256; j++)
				if (sampleTable[j]) smpNum++;
			
			instr[i].samp = smpNum;
			
			readITEnvelope(f, ins.volEnv);
			readITEnvelope(f, ins.panEnv);
			readITEnvelope(f, ins.pitchEnv);
			
			// convert envelope
			TEnvelope venv, penv, pitchenv;
			convertITEnvelope(venv, ins.volEnv);
			
			if (!module->addVolumeEnvelope(venv)) 
			{
				return MP_OUT_OF_MEMORY;
			}
			
			instr[i].venvnum = ++header->volenvnum;
			
			convertITEnvelope(penv, ins.panEnv, 32);
			
			if (!module->addPanningEnvelope(penv)) 
			{
				return MP_OUT_OF_MEMORY;
			}
			
			instr[i].penvnum = ++header->panenvnum;

			convertITEnvelope(pitchenv, ins.pitchEnv, 32);
			
			if (ins.pitchEnv.Flg & 128)
				pitchenv.type |= 128;
			
			if (!module->addPitchEnvelope(pitchenv)) 
			{
				return MP_OUT_OF_MEMORY;
			}
			
			instr[i].pitchenvnum = ++header->pitchenvnum;
			
		}
		else
		{
			// Deal with old format
			ins.volEnv.Flg = f.readByte();
			ins.volEnv.LpB = f.readByte();
			ins.volEnv.LpE = f.readByte();
			ins.volEnv.SLB = f.readByte();
			ins.volEnv.SLE = f.readByte();

			f.readWord();

			ins.FadeOut = f.readWord();

			mp_sint32 fade = (mp_sint32)ins.FadeOut*128;
			if (fade > 65535) fade = 65535;
			instr[i].volfade = fade;
			instr[i].res = 255;
			
			ins.NNA = f.readByte();
			ins.DCT = f.readByte();

			// sanity checks
			if (ins.NNA > 3) ins.NNA = 0;
			if (ins.DCT > 3) ins.DCT = 0;
			// fill in NNA stuff
			instr[i].flags |= (((mp_uword)ins.NNA) << 4);
			instr[i].flags |= (((mp_uword)ins.DCT) << 6);
			
			ins.TrkVers = f.readWord();
			ins.NoS = f.readByte();
			
			// x?
			ins.unused2 = f.readByte();

			f.read(ins.name, 1, 26);
			replace(ins.name, 26, '\0', ' ');
			memcpy(instr[i].name, ins.name, 26);

			f.readDword();
			f.readWord();
			
			f.read(ins.snum, 1, 240);

			mp_ubyte envelope[200];
			f.read(envelope, 1, 200);
			
			mp_ubyte nodeData[25*2];
			f.read(nodeData, 1, 25*2);
			
			for (j = 0; j < 25; j++)
			{
				if (nodeData[j*2] == 0xff)
					break;
				ins.volEnv.Nodes[j*3] = nodeData[j*2+1];
				ins.volEnv.Nodes[j*3+1] = nodeData[j*2];
				ins.volEnv.Nodes[j*3+2] = 0;
			}
			ins.volEnv.Num = j;

			// convert envelope
			TEnvelope venv;
			convertITEnvelope(venv, ins.volEnv);
			
			if (!module->addVolumeEnvelope(venv)) 
			{
				return MP_OUT_OF_MEMORY;
			}
			
			instr[i].venvnum = ++header->volenvnum;
			
			for (j = 0; j < 120; j++)
			{
				instr[i].snum[j] = -1;
				instr[i].notemap[j] = 0xff;
			}
			
			mp_ubyte sampleTable[256];
			memset(sampleTable, 0, sizeof(sampleTable));
			
			for (j = 0; j < 120; j++)
			{
				if (ins.snum[j*2] != 0xff)
				{
					mp_sint32 index = (mp_sword)ins.snum[j*2+1]-1;
					instr[i].snum[j] = index;
					
					instr[i].snum[j] = index;
					instr[i].notemap[j] = ins.snum[j*2];

					//if ((j - 12) >= 0)
					//	instr[i].snum[j-12] = index;

					//if ((j - 12) >= 0 && (ins.snum[j*2] >= 12))					
					//	instr[i].notemap[j-12] = ins.snum[j*2] - 12;

					if (index >= 0 && index < 256)
						sampleTable[index] = 1;
				}
			}
			
			mp_sint32 smpNum = 0;
			for (j = 0; j < 256; j++)
				if (sampleTable[j]) smpNum++;
			
			instr[i].samp = smpNum;
		}
		
		//printf("%i\n", instr[i].flags);
		
	}

	if (header->smpnum > MP_MAXSAMPLES)
		header->smpnum = MP_MAXSAMPLES;
	
	if (header->smpnum > 256)
		header->smpnum = 256;
	
	// read samples
	for (i = 0; i < header->smpnum; i++)
	{
		f.seekWithBaseOffset(smpOffs[i]);
		
		ITSample itSmp;

		f.read(itSmp.sig, 1, 4);

		// should match "IMPS"
		if (LittleEndian::GET_DWORD(itSmp.sig) != 0x53504D49)
			return MP_LOADER_FAILED;

		f.read(itSmp.dosName, 1, 12);

		// 0x00?
		itSmp.unused1 = f.readByte();

		itSmp.GvL = f.readByte();
		itSmp.Flg = f.readByte();
		itSmp.Vol = f.readByte();

		f.read(itSmp.name, 1, 26);
		replace(itSmp.name, 26, '\0', ' ');
		memcpy(smp[i].name, itSmp.name, 26);

		// sample mode, don't use instruments
		if (!(flags & 4))
		{
			instr[i].samp = 1;			
			for (j = 0; j < 120; j++)
				instr[i].snum[j] = i;

			memcpy(instr[i].name, itSmp.name, 26);
		}

		itSmp.Cvt = f.readByte();
		itSmp.DfP = f.readByte();

		itSmp.Length = f.readDword();
		itSmp.LoopBeg = f.readDword();
		itSmp.LoopEnd = f.readDword();
		itSmp.C5Speed = f.readDword();
		itSmp.SusLBeg = f.readDword();
		itSmp.SusLEnd = f.readDword();

		itSmp.SmpPoint = f.readDword();

		itSmp.ViS = f.readByte();
		itSmp.ViD = f.readByte();
		itSmp.ViR = f.readByte();
		itSmp.ViT = f.readByte();	

		switch (itSmp.ViT)
		{
			case 0: // = Sine wave
				smp[i].vibtype = 0;
				break;
			case 1: // = Ramp down
				smp[i].vibtype = 2;
				break;
			case 2: // = Square wave
				smp[i].vibtype = 1;
				break;
			case 3: // = Random (speed is irrelevant)
			default:
				smp[i].vibtype = 0;
				break;
		}
		smp[i].vibsweep = itSmp.ViR;
		smp[i].vibdepth = itSmp.ViD<<1;
		smp[i].vibrate = itSmp.ViS;

		smp[i].vol = XModule::vol64to255(itSmp.Vol);
		smp[i].pan = (itSmp.DfP & 0x80) ? 0x80 : XModule::vol127to255(itSmp.DfP & 0x7f);
		smp[i].flags = (itSmp.DfP & 0x80) ? 3 : 1;
		
		// res field of sample becomes global sample volume
		// and use IT auto vibrato instead of XM one
		smp[i].flags |= (8+16);
		smp[i].res = XModule::vol64to255(itSmp.GvL);

		smp[i].samplen = itSmp.Length;
		smp[i].loopstart = itSmp.LoopBeg;
		mp_sint32 looplen = ((mp_sint32)itSmp.LoopEnd - (mp_sint32)smp[i].loopstart);
		if (looplen < 0) 
			looplen = 0;
		smp[i].looplen = looplen;
	
		smp[i].type |= (itSmp.Flg & 2) ? 16 : 0;
		smp[i].type |= (itSmp.Flg & 80) == 80 ? 2 : ((itSmp.Flg & 80) == 16 ? 1 : 0);
		
		XModule::convertc4spd(itSmp.C5Speed, &smp[i].finetune, &smp[i].relnote);

		f.seekWithBaseOffset(itSmp.SmpPoint);

		if ((itSmp.Flg & 1))
		{
			if (!(smp[i].type&16)) {

				smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen);

				if (smp[i].sample == NULL)
				{
					return MP_OUT_OF_MEMORY;
				}

				if (itSmp.Flg & 8)
				{
					if (!module->loadSample(f, smp[i].sample, smp[i].samplen, smp[i].samplen, (itSmp.Cvt & 4) ? XModule::ST_PACKING_IT215 : XModule::ST_PACKING_IT))
					{
						return MP_OUT_OF_MEMORY;
					}
				}
				else if (!module->loadSample(f,smp[i].sample,smp[i].samplen,smp[i].samplen, (itSmp.Cvt & 1) ? XModule::ST_DEFAULT : XModule::ST_UNSIGNED))
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

				if (itSmp.Flg & 8)
				{
					if (!module->loadSample(f, smp[i].sample, smp[i].samplen, smp[i].samplen, (itSmp.Cvt & 4) ? (XModule::ST_PACKING_IT215 | XModule::ST_16BIT) : (XModule::ST_PACKING_IT | XModule::ST_16BIT)))
					{
						return MP_OUT_OF_MEMORY;
					}
				}
				else if (!module->loadSample(f,smp[i].sample,smp[i].samplen<<1,smp[i].samplen, XModule::ST_16BIT | ((itSmp.Cvt & 1) ? XModule::ST_DEFAULT : XModule::ST_UNSIGNED)))
				{
					return MP_OUT_OF_MEMORY;
				}					
			}
		}
	}

	if (!(flags & 4))
		header->insnum = header->smpnum;
	/*else
	{
		for (i = 0; i < header->insnum; i++)
		{
			if (instr[i].samp)
			{
				for (j = 0; j < 120; j++)
				{
					if (instr[i].snum[j] >= 0)
					{
						smp[instr[i].snum[j]].venvnum = i+1;
						smp[instr[i].snum[j]].penvnum = i+1;
					}
				}
			}
		}
	}*/

	// read patterns => find number of used channels
	for (i = 0; i < header->patnum; i++)
	{
		if (patOffs[i])
		{
			
			f.seekWithBaseOffset(patOffs[i]);
			
			mp_sint32 length = f.readWord();
			mp_sint32 rows = f.readWord();
			
			f.readDword();
			
			mp_ubyte* buffer = new mp_ubyte[length];
			
			f.read(buffer, 1, length);
			
			j = 0;
			mp_ubyte maskVariable = 0;
			mp_ubyte previousMaskVariable[256];
			
			memset(previousMaskVariable, 0, sizeof(previousMaskVariable));
			
			while (j < length)
			{
				mp_ubyte channelVariable = buffer[j++];
				
				if (channelVariable)
				{
					mp_ubyte channel = (channelVariable-1) & 63;
					
					if (channel+1 >= header->channum)
						header->channum = channel+1;
					
					if (channelVariable & 128)
						maskVariable = previousMaskVariable[channel] = buffer[j++];
					else
						maskVariable = previousMaskVariable[channel];
					
					if (maskVariable & 1)
						j++;
					
					if (maskVariable & 2)
						j++;
					
					if (maskVariable & 4)
						j++;
					
					if (maskVariable & 8)
						j+=2;
				}
			}
			
			delete[] buffer;
		}
	}
	
	// read patterns
	for (i = 0; i < header->patnum; i++)
	{
		/*if (i == 6)
		{
			int k = 0;
			k++;
			k--;
		}*/
	
		if (patOffs[i])
		{
			f.seekWithBaseOffset(patOffs[i]);

			mp_sint32 length = f.readWord();

			phead[i].rows = f.readWord();
			
			f.readDword();
			
			phead[i].effnum = 2;
			phead[i].channum = (mp_ubyte)header->channum;
			
			phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*6];
			
			// out of memory?
			if (phead[i].patternData == NULL)
				return MP_OUT_OF_MEMORY;
			
			memset(phead[i].patternData, 0, phead[i].rows*header->channum*6);

			mp_ubyte* buffer = new mp_ubyte[length];

			f.read(buffer, 1, length);

			j = 0;
			mp_ubyte maskVariable = 0;
			mp_ubyte previousMaskVariable[256];
			mp_ubyte previousNote[256];
			mp_ubyte previousInstrument[256];
			mp_ubyte previousVolume[256];
			mp_ubyte previousCommand[256];
			mp_ubyte previousOperand[256];

			memset(previousMaskVariable, 0, sizeof(previousMaskVariable));
			memset(previousNote, 0, sizeof(previousNote));
			memset(previousInstrument, 0, sizeof(previousInstrument));
			memset(previousVolume, 0, sizeof(previousVolume));
			memset(previousCommand, 0, sizeof(previousCommand));
			memset(previousOperand, 0, sizeof(previousOperand));
			
			mp_sint32 row = 0;

			while (j < length && row < phead[i].rows)
			{
				mp_ubyte channelVariable = buffer[j++];

				if (channelVariable && row < phead[i].rows)
				{
					mp_sint32 channel = (channelVariable-1) & 63;

					ASSERT(channel < phead[i].channum);
					ASSERT(row < phead[i].rows);

					mp_ubyte* slot = phead[i].patternData + (row*header->channum*6) + channel*6;				

					if (channel+1 >= header->channum)
						header->channum = channel+1;

					if (channelVariable & 128)
						maskVariable = previousMaskVariable[channel] = buffer[j++];
					else
						maskVariable = previousMaskVariable[channel];
				
					mp_sint32 note = -1, instrument = -1, volume = -1, command = -1, operand = -1;

					if (maskVariable & 1)
						note = previousNote[channel] = buffer[j++];

					if (maskVariable & 2)
						instrument = previousInstrument[channel] = buffer[j++];
					
					if (maskVariable & 4)
						volume = previousVolume[channel] = buffer[j++];
					
					if (maskVariable & 8)
					{
						command = previousCommand[channel] = buffer[j++];
						operand = previousOperand[channel] = buffer[j++];
					}

					if (maskVariable & 16)
						note = previousNote[channel];

					if (maskVariable & 32)
						instrument = previousInstrument[channel];
					
					if (maskVariable & 64)
						volume = previousVolume[channel];
					
					if (maskVariable & 128)
					{
						command = previousCommand[channel];
						operand = previousOperand[channel];
					}

					if (note >= 0)
					{
						if (note == 255)
							slot[0] = XModule::NOTE_OFF; // key off
						else if (note == 254)
							slot[0] = 122; // note cut
						else if (note >= 12*10)
							slot[0] = XModule::NOTE_FADE; // fade out
						else if (note < 12*10)
						{
							note++;
							//note -= 11;
							if (note >= 1)
								slot[0] = note;
						}
					}

					if (instrument >= 1)
						slot[1] = instrument;

					if (volume >= 0)
					{
						// convert volume
						if (volume <= 64)
						{
							slot[2] = 0x0C;
							slot[3] = XModule::vol64to255(volume);
						}
						// convert Fine volume up
						else if (volume >= 65 && volume <= 74)
						{
							mp_ubyte op = (volume - 65);
							slot[2] = 0x49;
							slot[3] = op ? ((op << 4) | 0xF) : 0;
						}
						// convert Fine volume down
						else if (volume >= 75 && volume <= 84)
						{
							mp_ubyte op = (volume - 75);
							slot[2] = 0x49;
							slot[3] = op ? (op | 0xF0) : 0;
						}
						// convert volume up
						else if (volume >= 85 && volume <= 94)
						{
							mp_ubyte op = (volume - 85);
							slot[2] = 0x49;
							slot[3] = op ? (op << 4) : 0;
						}
						// convert volume down
						else if (volume >= 95 && volume <= 104)
						{
							mp_ubyte op = (volume - 95);
							slot[2] = 0x49;
							slot[3] = op ? op : 0;
						}
						// convert pitch slide down
						else if (volume >= 105 && volume <= 114)
						{
							slot[2] = 0x48;
							slot[3] = ((volume - 105) << 4);						
						}
						// convert pitch slide up
						else if (volume >= 115 && volume <= 124)
						{
							slot[2] = 0x47;
							slot[3] = (volume - 115);						
						}
						// convert panning
						else if (volume >= 128 && volume <= 192)
						{
							slot[2] = 0x08;
							slot[3] = XModule::vol64to255(volume-128);
						}
						// portamento to note
						else if (volume >= 193 && volume <= 202)
						{
							static const mp_ubyte portaTab[] = {1, 4, 8, 16, 32, 64, 96, 128, 255};

							slot[2] = 0x03;
							slot[3] = portaTab[volume-193];
						}
						// vibrato
						else if (volume >= 203 && volume <= 212)
						{
							slot[2] = 0x04;
							slot[3] = volume - 203;
						}
					}
					
					if (command >= 1)
					{
						mp_ubyte op = operand, nEff = 0, nOp = 0, eff;

						switch (command)
						{
							// Axx     Set speed
							case 0x01:
								nEff = 0x1C;
								nOp = op;
								break;
							
							// Bxx     Jump to Order
							case 0x02:
								nEff = 0x0B;
								nOp = op;
								break;
							
							// Cxx     Break to Row
							case 0x03:
								nEff = 0x0D;
								nOp = op;
								break;
							
							// Dxx     Volume slide down
							case 0x04:
								nEff = 0x49;
								nOp = op;
								break;
							
							// Exx     porta down
							case 0x05:
								nEff = 0x48;
								nOp = op;
								break;

							// Fxx     porta up
							case 0x06:
								nEff = 0x47;
								nOp = op;
								break;

							// Gxx     tone porta
							case 0x07:
								nEff = 0x03;
								nOp = op;
								break;
							
							// Hxy     Vibrato
							case 0x08:
								nEff = 0x04;
								nOp = op;
								break;

							// Ixy     Tremor, ontime x, offtime y
							case 0x09:
								nEff = 0x1D;
								nOp = op;
								break;

							// Jxy     arpeggio
							case 0x0A:
								nEff = 0x20;
								nOp = op;
								break;

							// Kxy     vibrato & volslide
							case 0x0B:
								nEff = 0x06;
								nOp = op;
								break;

							// Lxx     tone porta & volslide
							case 0x0C:
								nEff = 0x05;
								nOp = op;
								break;

							// Mxx     set channel volume
							case 0x0D:
								if (nOp <= 64)
								{
									nEff = 0x50;
									nOp = XModule::vol64to255(op);
								}
								break;

							// Nxx     channel volslide 
							case 0x0E:
								nEff = 0x5A;
								nOp = op;								
								break;
								
							// Oxx     set sample offset
							case 0x0F:
								nEff = 0x09;
								nOp = op;
								break;

							// Pxx     panning slide (TO-DO)
							case 0x10:
								nEff = 0x5B;
								nOp = op;
								break;

							// Qxx     retrig
							case 0x11:
								nEff = 0x1B;
								nOp = op;
								break;

							// Rxy     tremolo
							case 0x12:
								nEff = 0x07;
								nOp = op;
								break;

							// various subeffects
							case 0x13:
								eff = op>>4;
								op&=0xf;
								switch (eff)
								{
									case 0x0:
										break;
									// past note actions/envelope trigger control etc.
									case 0x7:
										nEff = 0x1E;
										nOp = 0x70 + op;
										break;
									// set panning
									case 0x8:
										nEff = 0x08;
										nOp = XModule::pan15to255(op);
										break;
									// set high sample offset
									case 0xA:
										nEff = 0x1E;
										nOp = 0xF0 + op;
										break;
									// pattern loop
									case 0xB:
										nEff = 0x36;
										nOp = op;
										break;
									// note cut
									case 0xC:
										nEff = 0x3C;
										nOp = op;
										break;
									// note delay
									case 0xD:
										nEff = 0x3D;
										nOp = op;
										break;
									// pattern delay
									case 0xE:
										nEff = 0x3E;
										nOp = op;
										break;
#ifdef VERBOSE
									default:
										printf("Unsupported effect: S%x Op:%x\n", eff, op);
#endif
								}
								break;

							// Txx     set tempo
							case 0x14:
								nEff = 0x16;
								nOp = op;
								break;

							// Uxx     fine vibrato
							case 0x15:
								nEff = 0x4A;
								nOp = op;
								break;

							// Vxx     set global volume
							case 0x16:
								if (op>128) op = 128;
								nEff = 0x10;
								nOp = XModule::vol128to255(op);
								break;

							// Wxx	   global volume slide (stupid IT/MPT)
							case 0x17:
								nEff = 0x59;
								nOp = op;
								break;
								
							// Xxx	   set panning (0->FF)
							case 0x18:
								nEff = 0x08;
								nOp = op;
								break;

							// Yxx	   panbrello
							case 0x19:
								nEff = 0x5C;
								nOp = op;
								break;
#ifdef VERBOSE								
							default:
								printf("Unsupported effect: %x\n", command);
#endif

						}
					
						slot[4] = nEff;
						slot[5] = nOp;
					
					}
				}
				else 
					row++;
			}

			delete[] buffer;
		}
		else
		{
			phead[i].rows = 64;
			phead[i].effnum = 2;
			phead[i].channum = (mp_ubyte)header->channum;
			
			phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*6];
			
			// out of memory?
			if (phead[i].patternData == NULL)
				return MP_OUT_OF_MEMORY;
			
			memset(phead[i].patternData, 0, phead[i].rows*header->channum*6);
		}
	}

	if (messageOffset && messageLength)
	{
		f.seekWithBaseOffset(messageOffset);
		module->allocateSongMessage(messageLength+1);
		
		if (module->message == NULL)
			return MP_OUT_OF_MEMORY;
		
		// read song message
		f.read(module->message, 1, messageLength);			
	}
	
	strcpy(header->tracker,"Impulse Tracker");
	
	//module->setDefaultPanning();
	for (i = 0; i < header->channum; i++)
		header->pan[i] = (chnPan[i] <= 64) ? XModule::vol64to255(chnPan[i]) : 0x80;
	
	module->postProcessSamples();
	
	return MP_OK;	
}
