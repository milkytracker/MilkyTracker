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
 *  LoaderMXM.cpp
 *  MilkyPlay Module Loader: Cubic Tiny XM (.MXM)
 *
 *  --------------------------------
 *			Version History:
 *  --------------------------------
 *  07/30/05: Added the worst hack ever to load original orbitalism.mxm by netpoet
 *  11/23/04: Corrected bugs in 16 bit support, added new MXM format (Elitegroup modifications)
 *  11/17/04: First work
 */
#include "Loaders.h"

//#define FR_HACK

//#define VERBOSE

/////////////////////////////////////////////////////
// MXM structures
/////////////////////////////////////////////////////
struct TMXMHeader 
{
	mp_uint32 sig;
	mp_uint32 ordnum;
	mp_uint32 restart;
	mp_uint32 channum;
	mp_uint32 patnum;
	mp_uint32 insnum;
	mp_ubyte tempo;
	mp_ubyte speed;
	mp_uword opt;
	mp_uint32 sampstart;
	mp_uint32 samples8;
	mp_uint32 samples16;
	mp_sint32 lowpitch;
	mp_sint32 highpitch;
	mp_ubyte panpos[32];
};

struct TMXMInstrument 
{
	mp_uint32 sampnum;
	mp_ubyte snum[96];
	mp_uword volfade;
	mp_ubyte vibtype, vibsweep, vibdepth, vibrate;
	mp_ubyte vnum, vsustain, vloops, vloope;
	mp_uword venv[12][2];
	mp_ubyte pnum, psustain, ploops, ploope;
	mp_uword penv[12][2];
	mp_ubyte res[46];
};

struct TMXMSample 
{
	mp_uword gusstartl;
	mp_ubyte gusstarth;
	mp_uword gusloopstl;
	mp_ubyte gusloopsth;
	mp_uword gusloopendl;
	mp_ubyte gusloopendh;
	mp_ubyte gusmode;
	mp_ubyte vol;
	mp_ubyte pan;
	mp_uword relpitch;
	mp_ubyte res[2];
};

/*struct TMXMSampleNew {
	unsigned long  loopstart;
	unsigned long  end;
	mp_ubyte  gusmode;
	mp_ubyte  vol;
	mp_ubyte  pan;
	mp_uword relpitch;
	mp_uword offsindex;
	mp_ubyte  res;
};*/

struct TNoteSlot 
{
	mp_ubyte Note,Ins,Vol,Eff,Op;
};

const char* LoaderMXM::identifyModule(const mp_ubyte* buffer)
{
	// check for .MXM module first
	if (!memcmp(buffer,"MXM",3))
	{
		return "MXM";
	}

	// this is not an .MXM
	return NULL;
}

mp_sint32 LoaderMXM::load(XMFileBase& f, XModule* module)
{
	TMXMHeader		MXMHeader;
	TMXMInstrument	MXMIns;
	TMXMSample		MXMSmp;
	TNoteSlot		Row[32];

	mp_ubyte	packbyte,*smpbuffer;
	mp_sword	*smpbuffer16;
	mp_ubyte	note,ins,vol,eff,op,b1;
	mp_sword	b1_2;

	mp_uint32	patofs[256];
	mp_uint32	insofs[128];
	
	mp_uint32*	smpofs = new mp_uint32[2048];

	if (smpofs == NULL)
		return MP_OUT_OF_MEMORY;

	mp_uint32	inscnt,smpcnt,patcnt,rowcnt,x,y,z;
	mp_uint32	numrows,j,i;
	
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	

	// we're already out of memory here
	if (!phead || !instr || !smp)
	{
		delete[] smpofs;
		return MP_OUT_OF_MEMORY;
	}

	mp_uword* lut = new mp_uword[65536];

	if (lut == NULL)
	{
		delete[] smpofs;
		return MP_OUT_OF_MEMORY;
	}

	// calculate nifty table :)
	for (i = 0; i < 256; i++)
		for (j = 0; j < 256; j++)
			lut[i*256+j] = (mp_uword)((i-96)*256+(j-128)*2);

	//fread(&MXMHeader,1,sizeof(TMXMHeader),f);
	
	MXMHeader.sig = f.readDword();
	MXMHeader.ordnum = f.readDword();
	MXMHeader.restart = f.readDword();
	MXMHeader.channum = f.readDword();
	MXMHeader.patnum = f.readDword();
	MXMHeader.insnum = f.readDword();
	MXMHeader.tempo = f.readByte();
	MXMHeader.speed = f.readByte();
	MXMHeader.opt = f.readWord();
	MXMHeader.sampstart = f.readDword();
	MXMHeader.samples8 = f.readDword();
	MXMHeader.samples16 = f.readDword();
	MXMHeader.lowpitch = f.readDword();
	MXMHeader.highpitch = f.readDword();

	if(MXMHeader.ordnum > 256 || MXMHeader.patnum > 256 || MXMHeader.insnum > 256)
		return MP_LOADER_FAILED;

	f.read(MXMHeader.panpos,1,32);
		
	f.read(&header->ord, 1, 256);

	memcpy(&header->sig, &MXMHeader.sig, 3);

	strcpy(header->tracker,"..converted..");
	
	header->ordnum = (mp_uword)MXMHeader.ordnum;
	header->restart = (mp_uword)MXMHeader.restart;
	header->channum = (mp_uword)MXMHeader.channum;
	header->patnum = (mp_uword)MXMHeader.patnum;
	header->insnum = (mp_uword)MXMHeader.insnum;
	header->tempo = MXMHeader.tempo;
	header->speed = MXMHeader.speed;
	header->freqtab = MXMHeader.opt&1;
	header->mainvol = 255;
	header->flags = XModule::MODULE_XMNOTECLIPPING | 
		XModule::MODULE_XMARPEGGIO | 
		XModule::MODULE_XMPORTANOTEBUFFER | 
		XModule::MODULE_XMVOLCOLUMNVIBRATO;
		
	header->uppernotebound = 119;

	f.readDwords(insofs,512/4);
	f.readDwords(patofs,1024/4);

	// old version
	mp_uint32 mxmVer = 0;

	// detect MXM version
	mp_uint32 curPos = f.posWithBaseOffset();
	
	// read number of samples for first instrument
	mp_uint32 dummy = f.readDword();
	// must be smaller or equal 16
	if (dummy <= 16)
	{
		mp_ubyte nbu[96];
		// read sample layout
		f.read(nbu, 1, 96);
		for (i = 0; i < 96; i++)
			if (nbu[i] > (dummy-1))
			{
				// invalid entry => should be new version of MXM format
				mxmVer = 1;
				break;
			}
	}
	else mxmVer = 1;

	f.seekWithBaseOffset(curPos);

	// read smp offsets for MXM format rev. 2
	if (mxmVer == 1)
		f.readDwords(smpofs,2048);

#ifdef FR_HACK
	////////// blabla
	{
		fpos_t filesize;
		
		mxmVer = 1;
		static char hexTab[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
		char fileName[] = "/Volumes/192.168.1.3/deus-ex - MXM ohne Samples.rar Folder/xx.wav";
		mp_sint32 size = 0;
		for (i = 1; i <= 0x15; i++)
		{
			
			fileName[59] = hexTab[(i>>4)&0xf];
			fileName[60] = hexTab[i&0xf];
			
			FILE *f = fopen(fileName,"rb");
			fseek(f,0,SEEK_END);
			fgetpos(f,&filesize);
			fclose(f);
			
			smpofs[i-1] = size;
			
			size+=filesize - 44;
			
		}		
	}
	/////////////////
#endif

	///////////////////////////////////////////////////
	// read instruments
	///////////////////////////////////////////////////

	mp_sint32 sampleIndex = 0;
	mp_sint32 envelopeIndex = 0;
	
	mp_sint32 smpOffset16 = 0;
	bool correctSmpSize16 = false;
	mp_sint32 smpOffset8 = 0;
	bool correctSmpSize8 = false;

	for (inscnt=0;inscnt<MXMHeader.insnum;inscnt++) {
		//fread(&MXMIns,1,sizeof(MXMIns),f);
		//memset(&XMIns,0,sizeof(XMIns));
		
		mp_ubyte nbu[96];
		MXMIns.sampnum = f.readDword();
		
		f.read(nbu, 1, 96);

		MXMIns.volfade = f.readWord();
		MXMIns.vibtype = f.readByte();
		MXMIns.vibsweep = f.readByte();
		MXMIns.vibdepth = f.readByte();
		MXMIns.vibrate = f.readByte();

		MXMIns.vnum = f.readByte();
		MXMIns.vsustain = f.readByte();
		MXMIns.vloops = f.readByte();
		MXMIns.vloope = f.readByte();
		f.readWords((mp_uword*)&MXMIns.venv, 12*2);

		MXMIns.pnum = f.readByte();
		MXMIns.psustain = f.readByte();
		MXMIns.ploops = f.readByte();
		MXMIns.ploope = f.readByte();
		f.readWords((mp_uword*)&MXMIns.penv, 12*2);
		mp_ubyte res[46];
		f.read(res, 1, 46);

		instr[inscnt].samp = MXMIns.sampnum;

		for (x=0;x<96;x++)
			if (nbu[x]>(MXMIns.sampnum-1))
				instr[inscnt].snum[x] = 255;
			else
				instr[inscnt].snum[x] = nbu[x]+sampleIndex;
				
		TEnvelope venv;
		TEnvelope penv;
		memset(&venv,0,sizeof(venv));
		memset(&penv,0,sizeof(penv));

		if (MXMIns.vnum !=0 && MXMIns.vnum<24) 
		{
			venv.num = MXMIns.vnum + 1;
			venv.type|=1;
			x=0;
			for (j = 0;j <= MXMIns.vnum; j++) 
			{
				venv.env[j][0] = x;
				venv.env[j][1] = MXMIns.venv[j][1];
				x+=MXMIns.venv[j][0];
			}
		}
			
		if (MXMIns.vsustain < 24) {
			venv.type|=2;
			venv.sustain=MXMIns.vsustain;
		}
		if (MXMIns.vloope<24) {
			venv.type|=4;
			venv.loops=MXMIns.vloops;
			venv.loope=MXMIns.vloope;
		}
		
		if (MXMIns.pnum!=0 && MXMIns.pnum<24) {
			penv.num=MXMIns.pnum+1;
			penv.type|=1;
			x=0;
			for (j=0;j<=MXMIns.pnum;j++) {
				penv.env[j][0]=x;
				penv.env[j][1]=MXMIns.penv[j][1];
				x+=MXMIns.penv[j][0];
			}
		}
		if (MXMIns.psustain<24) {
			penv.type|=2;
			penv.sustain=MXMIns.psustain;
		}
		if (MXMIns.ploope<24) {
			penv.type|=4;
			penv.loops=MXMIns.ploops;
			penv.loope=MXMIns.ploope;
		}
		
		for (mp_sint32 l=0;l<24;l++) {
			venv.env[l][1]<<=2;
			penv.env[l][1]<<=2;
		}
		
		if (!module->addVolumeEnvelope(venv)) 
		{
			delete[] lut;
			delete[] smpofs;
			return MP_OUT_OF_MEMORY;
		}
		if (!module->addPanningEnvelope(penv)) 
		{
			delete[] lut;
			delete[] smpofs;
			return MP_OUT_OF_MEMORY;
		}
		
		//XMIns.volfade=MXMIns.volfade;
		
		//XMIns.vibsweep=MXMIns.vibsweep;
		//XMIns.vibdepth=MXMIns.vibdepth;
		//XMIns.vibrate=MXMIns.vibrate;
		
		//XMIns.vibtype=(MXMIns.vibtype==2)?1:(MXMIns.vibtype==3)?2:(MXMIns.vibtype==1)?3:0;
		
		//fwrite(&XMIns,1,sizeof(XMIns),f2);
		//fwrite(&XMIns.name,1,20,f2);
		
		for (smpcnt=0;smpcnt<MXMIns.sampnum;smpcnt++) {
			smp[sampleIndex].flags = 3;
			smp[sampleIndex].venvnum = envelopeIndex+1;
			smp[sampleIndex].penvnum = envelopeIndex+1;
			
			smp[sampleIndex].vibtype = (MXMIns.vibtype==2)?1:(MXMIns.vibtype==3)?2:(MXMIns.vibtype==1)?3:0;
			smp[sampleIndex].vibsweep = MXMIns.vibsweep;
			smp[sampleIndex].vibdepth = MXMIns.vibdepth<<1;
			smp[sampleIndex].vibrate = MXMIns.vibrate;
			smp[sampleIndex].volfade = MXMIns.volfade<<1;
			
			if (!(venv.type&1)) smp[sampleIndex].volfade=0;
			
			if (mxmVer == 0)
			{
				MXMSmp.gusstartl = f.readWord();
				MXMSmp.gusstarth = f.readByte();
				MXMSmp.gusloopstl = f.readWord();
				MXMSmp.gusloopsth = f.readByte();
				MXMSmp.gusloopendl = f.readWord();
				MXMSmp.gusloopendh = f.readByte();
				MXMSmp.gusmode = f.readByte();
				MXMSmp.vol = f.readByte();
				MXMSmp.pan = f.readByte();
				MXMSmp.relpitch = f.readWord();
				f.read(MXMSmp.res, 1, 2);
			}
			else if (mxmVer == 1)
			{
				unsigned long loopstart = f.readDword();
				unsigned long end = f.readDword();
				MXMSmp.gusmode = f.readByte();
				MXMSmp.vol = f.readByte();
				MXMSmp.pan = f.readByte();
				MXMSmp.relpitch = f.readWord();
				mp_uword offsindex = f.readWord();
				mp_ubyte res = f.readByte();
			
				mp_sint32 smppos;

				if ((MXMSmp.gusmode>>2)&1)
				{
					smppos = (smpofs[sampleIndex]-MXMHeader.samples8-MXMHeader.sampstart)>>1;
					if (!smpOffset16)
						smpOffset16 = smpofs[sampleIndex];
					if (smppos > (signed)MXMHeader.samples16)
						correctSmpSize16 = true;
				}
				else
				{
					smppos = (smpofs[sampleIndex]-MXMHeader.sampstart);
					if (!smpOffset8)
						smpOffset8 = smpofs[sampleIndex];
					if (smppos > (signed)MXMHeader.samples8)
						correctSmpSize8 = true;
				}

				MXMSmp.gusstartl = (mp_uword)(smppos&0xFFFF);
				MXMSmp.gusstarth = (mp_ubyte)((smppos>>16)&0xFF);

				MXMSmp.gusloopstl = (mp_uword)(loopstart+smppos)&0xFFFF;
				MXMSmp.gusloopsth = (mp_ubyte)((loopstart+smppos)>>16)&0xFF;
				MXMSmp.gusloopendl = (mp_uword)(end+smppos)&0xFFFF;
				MXMSmp.gusloopendh = (mp_ubyte)((end+smppos)>>16)&0xFF;
			}
			else
			{
				delete[] lut;
				delete[] smpofs;
				return MP_LOADER_FAILED;
			}
			
			//fread(&MXMSmp,1,sizeof(MXMSmp),f);
			
			//memset(&XMSmp[0],0,sizeof(TXMSample));
			
			smp[sampleIndex].samplen = ((mp_uint32)MXMSmp.gusstarth<<16)+(mp_uint32)MXMSmp.gusstartl;
			smp[sampleIndex].loopstart=((mp_uint32)MXMSmp.gusloopsth<<16)+(mp_uint32)MXMSmp.gusloopstl;				
			smp[sampleIndex].looplen=((mp_uint32)MXMSmp.gusloopendh<<16)+(mp_uint32)MXMSmp.gusloopendl;
			smp[sampleIndex].vol = XModule::vol64to255(MXMSmp.vol);
			smp[sampleIndex].pan = MXMSmp.pan;
			
			bool bFound = false;
			mp_sint32 relNote = -1;
			mp_sint32 fineTune = -1;
			for (mp_sint32 i = 0; i < 256; i++)
				for (mp_sint32 j = 0; j < 256; j++)
				{
					if (lut[i*256+j] == MXMSmp.relpitch)
					{
						bFound = true;
						relNote = i-96;
						fineTune = j-128;
						break;
					}
					if (bFound)
						break;
				}
				
			if (!bFound)
			{
				relNote = fineTune = 0;
			}
				
			smp[sampleIndex].relnote = relNote;
			smp[sampleIndex].finetune = fineTune;
			
			y=(MXMSmp.gusmode>>3)&1;
			if (y)
				smp[sampleIndex].type=1;
			
			y=(MXMSmp.gusmode>>4)&1;
			if (y)
				smp[sampleIndex].type=2;
			
			y=(MXMSmp.gusmode>>2)&1;
			if (y)
				smp[sampleIndex].type|=16;
				
			sampleIndex++;
		}

		envelopeIndex++;
			
	}

	if (!correctSmpSize8)
		smpOffset8 = 0;
	if (!correctSmpSize16)
		smpOffset16 = 0;

	header->smpnum = sampleIndex;
	header->volenvnum = header->panenvnum = envelopeIndex;

	delete[] lut;

	///////////////////////////////////////////////////
	// read patterns
	///////////////////////////////////////////////////
	for (patcnt=0;patcnt<header->patnum;patcnt++) {
		
		f.seekWithBaseOffset(patofs[patcnt]);
		
		numrows = f.readDword();

		//pattern = new TNoteSlot[numrows*XMHeader.channum];

		//if (pattern==NULL) {
			//cprintf("Pattern to big (%d)\n",numrows*XMHeader.channum*5);
		//	exit(1);
		//}

		phead[patcnt].rows = numrows;
		phead[patcnt].effnum = 2;
		phead[patcnt].channum= (mp_ubyte)header->channum;
		
		phead[patcnt].patternData = new mp_ubyte[phead[patcnt].rows*header->channum*6];
		
		// out of memory?
		if (phead[patcnt].patternData == NULL)
		{
			delete[] smpofs;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[patcnt].patternData,0,phead[patcnt].rows*header->channum*6);

		z=0;
		for (rowcnt=0;rowcnt<numrows;rowcnt++) {
			memset(Row,0,sizeof(Row));
			do {
				
				note=ins=vol=eff=op=0;
				
				packbyte = f.readByte();
				
				y=(packbyte>>5)&1;
				
				if (packbyte>0) {
					
					if (y) {
						note = f.readByte();
						ins = f.readByte();
					}
					y=(packbyte>>6)&1;
					if (y) {
						vol = f.readByte();
					}
					y=(packbyte>>7)&1;
					if (y) {
						eff = f.readByte();
						op = f.readByte();
						if (eff>=36) {
							op=((eff-36)<<4)|op;
							eff=0xE;
						}
					}
					
					y=packbyte&0x1F;
					Row[y].Note=note;
					Row[y].Ins=ins;
					Row[y].Vol=vol;
					Row[y].Eff=eff;
					Row[y].Op=op;
				}
				
			} while (packbyte!=0);
			
			for (x=0;x<header->channum;x++) {
				/*pattern[z].Note=Row[x].Note;
				pattern[z].Ins=Row[x].Ins;
				pattern[z].Vol=Row[x].Vol;
				pattern[z].Eff=Row[x].Eff;
				pattern[z].Op=Row[x].Op;*/

				mp_ubyte slot[5];
				slot[0] = Row[x].Note;
				slot[1] = Row[x].Ins;
				slot[2] = Row[x].Vol;
				slot[3] = Row[x].Eff;
				slot[4] = Row[x].Op;
				
				// filter invalid effects
				// they could have a different meaning in my playercode
				bool valid = false;
				for (i=0;i<XModule::numValidXMEffects;i++)
					if (slot[3]==XModule::validXMEffects[i]) 
					{
						valid = true;
						break;
					}
					
				if (!valid) slot[3]=slot[4]=0;				

				if ((slot[3]==0xC)||(slot[3]==0x10)) {
					slot[4] = XModule::vol64to255(slot[4]);					
					//mp_sint32 bl = slot[4];
					//if (bl>64) bl=64;
					//slot[4]=(bl*261120)>>16;
				}
				
				if ((!slot[3])&&(slot[4])) slot[3]=0x20;
				
				if (slot[3]==0xE) {
					slot[3]=(slot[4]>>4)+0x30;
					slot[4]=slot[4]&0xf;
				}
				
				if (slot[3]==0x21) {
					slot[3]=(slot[4]>>4)+0x40;
					slot[4]=slot[4]&0xf;
				}
				
				if (slot[0]==97) slot[0]=XModule::NOTE_OFF;
				
				phead[patcnt].patternData[z]=slot[0];
				phead[patcnt].patternData[z+1]=slot[1];
				
				XModule::convertXMVolumeEffects(Row[x].Vol, phead[patcnt].patternData[z+2], phead[patcnt].patternData[z+3]);
						
				phead[patcnt].patternData[z+4]=slot[3];
				phead[patcnt].patternData[z+5]=slot[4];

				z+=6;
			}
			
		}
		
			
	}

	///////////////////////////////////////////////////
	// read samples
	///////////////////////////////////////////////////
	mp_uint32*	gusstart = new mp_uint32[16*256];
	mp_uint32*	gusend = new mp_uint32[16*256];
	mp_uint32*	smpOffsets8 = new mp_uint32[256*16];
	mp_uint32*	smpOffsets16 = new mp_uint32[256*16];

	smpbuffer = NULL;
	smpbuffer16 = NULL;

#ifdef FR_HACK
	////////// blabla
	{
		smpbuffer16 = new mp_sword[3220000>>1];
		fpos_t filesize;
		
		mxmVer = 1;
		static char hexTab[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
		char fileName[] = "/Volumes/192.168.1.3/deus-ex - MXM ohne Samples.rar Folder/xx.wav";
		mp_sint32 size = 0;
		for (i = 1; i <= 0x15; i++)
		{
			
			fileName[59] = hexTab[(i>>4)&0xf];
			fileName[60] = hexTab[i&0xf];
			
			XMFile* f2 = new XMFile(fileName);

			filesize = f2->sizeWithBaseOffset();

			f2->seekWithBaseOffset(44);
			
			f2->readWords(smpbuffer16+size, (filesize-44)>>1);
			delete f2;

			size+=(filesize-44)>>1;
		}
		goto hack;
	}	
#endif

	f.seekWithBaseOffset(MXMHeader.sampstart);

	if (smpOffset8)
		MXMHeader.samples8 = f.sizeWithBaseOffset() - f.posWithBaseOffset();

	if (MXMHeader.samples8!=0) {
		
		smpbuffer = new mp_ubyte[MXMHeader.samples8];
		
		if (smpbuffer==NULL) {
			//cprintf("Not enough memory for 8 bit samples (%u)",MXMHeader.samples8);
			delete[] gusstart;
			delete[] gusend;
			delete[] smpOffsets8;
			delete[] smpOffsets16;
			
			delete[] smpofs;
			
			return MP_OUT_OF_MEMORY;
		}
	}
 
	if (smpOffset16)
	{
		f.seekWithBaseOffset(smpOffset16);
		MXMHeader.samples16 = (f.sizeWithBaseOffset() - f.posWithBaseOffset())>>1;
	}
 
	if (MXMHeader.samples16!=0) {
		
		smpbuffer16 = new mp_sword[MXMHeader.samples16];
		
		if (smpbuffer16==NULL) {
			//cprintf("Not enough memory for 16 bit samples (%u)",MXMHeader.samples16*2);
			delete[] smpbuffer;

			delete[] gusstart;
			delete[] gusend;
			delete[] smpOffsets8;
			delete[] smpOffsets16;
			
			delete[] smpofs;

			return MP_OUT_OF_MEMORY;
		}
	}
	
	f.seekWithBaseOffset(MXMHeader.sampstart);
	f.read(smpbuffer, 1, MXMHeader.samples8);
	if (smpOffset16)
	{
		f.seekWithBaseOffset(smpOffset16);
	}
	f.readWords((mp_uword*)smpbuffer16, MXMHeader.samples16);

	y=(MXMHeader.opt>>2)&1;
	if (y) {		
		if (MXMHeader.samples8!=0) {
			b1=0;
			for (j = 0; j<MXMHeader.samples8; j++)
				smpbuffer[j]=b1+=smpbuffer[j];
		}
		
		if (MXMHeader.samples16!=0) {
			b1_2=0;
			for (j = 0; j < MXMHeader.samples16; j++)
				smpbuffer16[j]=b1_2+=smpbuffer16[j];
		
/*#ifdef VERBOSE
			for (j = 0; j < 42; j++)
			{
				printf("%i\n",smpbuffer16[j]);
			}
#endif*/
		}
	}

#ifdef FR_HACK
hack:
#endif
	mp_uint32 nSmpCnt8 = 0;
	mp_uint32 nSmpCnt16 = 0;
	
	for (smpcnt=0;smpcnt<header->smpnum;smpcnt++) {
				
		if (!(smp[smpcnt].type & 16))
		{
			smpOffsets8[nSmpCnt8++] = smp[smpcnt].samplen;
		}
		else
		{
			smpOffsets16[nSmpCnt16++] = smp[smpcnt].samplen;
		}
	}

	mp_uint32 nNumSamples8 = nSmpCnt8;
	mp_uint32 nNumSamples16 = nSmpCnt16;
	
	mp_uint32 baseOffset = smpOffsets16[0];
	for (inscnt = 0; inscnt < nSmpCnt16; inscnt++)
		smpOffsets16[inscnt]-=baseOffset;

#ifdef VERBOSE
	for (inscnt = 0; inscnt < nSmpCnt16; inscnt++)
		printf("New sampleoffsets: %x\n",smpOffsets16[inscnt]);

	printf("8 bit (complete) samplesize: %i\n",MXMHeader.samples8);
	printf("8 bit (numsamples): %i\n",nSmpCnt8);

	printf("16 bit (complete) samplesize: %i\n",MXMHeader.samples16);
	printf("16 bit (numsamples): %i\n",nSmpCnt16);
#endif


	nSmpCnt8 = nSmpCnt16 = 0;
	
	for (smpcnt=0;smpcnt<header->smpnum;smpcnt++) {
			
		if (!(smp[smpcnt].type&16))
		{
			
			if (nSmpCnt8 == (nNumSamples8-1))
			{
				gusstart[smpcnt] = smpOffsets8[nSmpCnt8];
				gusend[smpcnt] = MXMHeader.samples8;
			}
			else
			{
				gusstart[smpcnt] = smpOffsets8[nSmpCnt8];
				gusend[smpcnt] = smpOffsets8[nSmpCnt8+1];
			}
			
			nSmpCnt8++;
			
		}
		else
		{
			
			if (nSmpCnt16 == (nNumSamples16-1))
			{
				gusstart[smpcnt] = smpOffsets16[nSmpCnt16];
				gusend[smpcnt] = MXMHeader.samples16;
			}
			else
			{
				gusstart[smpcnt] = smpOffsets16[nSmpCnt16];
				gusend[smpcnt] = smpOffsets16[nSmpCnt16+1];
			}
			
			nSmpCnt16++;
			
		}
		
		mp_sint32 loopstart = smp[smpcnt].loopstart;
		
		smp[smpcnt].loopstart = smp[smpcnt].loopstart-smp[smpcnt].samplen;
		
		smp[smpcnt].samplen = smp[smpcnt].looplen-smp[smpcnt].samplen;

#ifdef VERBOSE
		printf("Len:%i Pos:%i All:%i\n",smp[smpcnt].samplen, gusstart[smpcnt], MXMHeader.samples8);
#endif

		if (smp[smpcnt].type&3)
		{
			smp[smpcnt].looplen = smp[smpcnt].looplen-loopstart;//smp[smpcnt].loopstart;
			
			if (smp[smpcnt].looplen > smp[smpcnt].samplen ||
				smp[smpcnt].loopstart > smp[smpcnt].samplen)
			{
				if (smpbuffer)
					delete[] smpbuffer;
				if (smpbuffer16)
					delete[] smpbuffer16;

				delete[] gusstart;
				delete[] gusend;
				delete[] smpOffsets8;
				delete[] smpOffsets16;
				
				delete[] smpofs;
				
				return MP_LOADER_FAILED;
			}
			
		}
		else
			smp[smpcnt].looplen = smp[smpcnt].loopstart = 0;		
	}
	
	for (smpcnt=0;smpcnt<header->smpnum;smpcnt++) {
		
		y=(smp[smpcnt].type>>4)&1;
		if (!y) {		
			if (gusstart[smpcnt] + smp[smpcnt].samplen > MXMHeader.samples8)
				continue;
			
			smp[smpcnt].sample = (mp_sbyte*)module->allocSampleMem(smp[smpcnt].samplen);

			if (smp[smpcnt].sample == NULL || smpbuffer == NULL)
			{
				if (smpbuffer)
					delete[] smpbuffer;
				if (smpbuffer16)
					delete[] smpbuffer16;
				
				delete[] gusstart;
				delete[] gusend;
				delete[] smpOffsets8;
				delete[] smpOffsets16;
				
				delete[] smpofs;
					
				return MP_OUT_OF_MEMORY;
			}

			memcpy(smp[smpcnt].sample,smpbuffer+gusstart[smpcnt],smp[smpcnt].samplen);
			
		}
		else {
			if (gusstart[smpcnt] + smp[smpcnt].samplen > MXMHeader.samples16)
				continue;

			smp[smpcnt].sample = (mp_sbyte*)module->allocSampleMem(smp[smpcnt].samplen*2);
			
			if (smp[smpcnt].sample == NULL || smpbuffer16 == NULL)
			{
				if (smpbuffer)
					delete[] smpbuffer;
				if (smpbuffer16)
					delete[] smpbuffer16;
				
				delete[] gusstart;
				delete[] gusend;
				delete[] smpOffsets8;
				delete[] smpOffsets16;
				
				delete[] smpofs;
					
				return MP_OUT_OF_MEMORY;
			}
			
			memcpy(smp[smpcnt].sample,smpbuffer16+gusstart[smpcnt],smp[smpcnt].samplen*2);
		}
		
	}

	if (smpbuffer)
		delete[] smpbuffer;
	
	if (smpbuffer16)
		delete[] smpbuffer16;

	delete[] gusstart;
	delete[] gusend;
	delete[] smpOffsets8;
	delete[] smpOffsets16;
	
	delete[] smpofs;

	module->postProcessSamples();

	return MP_OK;
}
