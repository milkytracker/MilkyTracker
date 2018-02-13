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
 *  LoaderXM.cpp
 *  MilkyPlay Module Loader: Fasttracker 2
 *
 *  --------------------------------
 *			Version History:
 *  --------------------------------
 *  02/27/05: Added support for MODPLUG song message
 *  10/19/04: Fixed MOOH.XM loading problems. Header says there are more instruments than actually stored in the file.
 *  10/14/04: Added sample relocation technique to get rid of unused samples
 *  ??/??/98: First version of this XM loader
 */
#include "Loaders.h"

//#define VERBOSE

#define XM_ENVELOPENUMPOINTS 12

const char* LoaderXM::identifyModule(const mp_ubyte* buffer)
{
	// check for .XM module first
	if (!memcmp(buffer,"Extended Module:",16))
	{
		return "XM";
	}

	// this is not an .XM
	return NULL;
}

//////////////////////////////////////////////////////
// load fasttracker II extended module
//////////////////////////////////////////////////////
mp_sint32 LoaderXM::load(XMFileBase& f, XModule* module)
{
	mp_ubyte insData[230];		
	mp_sint32 smpReloc[96];
	mp_ubyte nbu[96];
	mp_uint32 fileSize = 0;
			
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	

	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;
	
	fileSize = f.sizeWithBaseOffset();
	
	f.read(&header->sig,1,17);
	f.read(&header->name,1,20);
	f.read(&header->whythis1a,1,1);
	header->whythis1a=0;
	f.read(&header->tracker,1,20);
	f.readWords(&header->ver,1);
	
	if (header->ver != 0x102 && 
		header->ver != 0x103 && // untested
		header->ver != 0x104)
		return MP_LOADER_FAILED;
	
	f.readDwords(&header->hdrsize,1);
	
	header->hdrsize-=4;
	
	mp_uint32 hdrSize = 0x110;
	if (header->hdrsize > hdrSize)
		hdrSize = header->hdrsize;
				
	mp_ubyte* hdrBuff = new mp_ubyte[hdrSize];
	memset(hdrBuff, 0, hdrSize);
	
	f.read(hdrBuff, 1, header->hdrsize);
	
	header->ordnum = LittleEndian::GET_WORD(hdrBuff);
	header->restart = LittleEndian::GET_WORD(hdrBuff+2);
	header->channum = LittleEndian::GET_WORD(hdrBuff+4);
	header->patnum = LittleEndian::GET_WORD(hdrBuff+6);
	header->insnum = LittleEndian::GET_WORD(hdrBuff+8);
	header->freqtab = LittleEndian::GET_WORD(hdrBuff+10);
	header->tempo = LittleEndian::GET_WORD(hdrBuff+12);
	header->speed = LittleEndian::GET_WORD(hdrBuff+14);
	memcpy(header->ord, hdrBuff+16, 256);
	if(header->ordnum > MP_MAXORDERS)
		header->ordnum = MP_MAXORDERS;

	delete[] hdrBuff;
	
	header->mainvol=255;
	header->flags = XModule::MODULE_XMNOTECLIPPING | 
		XModule::MODULE_XMARPEGGIO | 
		XModule::MODULE_XMPORTANOTEBUFFER | 
		XModule::MODULE_XMVOLCOLUMNVIBRATO;

	header->uppernotebound = 119;
	
	mp_sint32 i,y,sc;
	for (i=0;i<32;i++) header->pan[i]=0x80;
	
	// old version?
	if (header->ver == 0x102 || header->ver == 0x103)
	{
		mp_sint32 s = 0;
		mp_sint32 e = 0;
		for (y=0;y<header->insnum;y++) {
			
			f.readDwords(&instr[y].size,1);
			f.read(&instr[y].name,1,22);		
			f.read(&instr[y].type,1,1);
			mp_uword numSamples = 0;
			f.readWords(&numSamples,1);
			if(numSamples > 96)
				return MP_LOADER_FAILED;
			instr[y].samp = numSamples;

			if (instr[y].size == 29)
			{
#ifdef MILKYTRACKER
				s+=16;
#endif
				for (mp_sint32 i = 0; i < 120; i++)
					instr[y].snum[i] = -1;
				continue;
			}

			f.readDwords(&instr[y].shsize,1);

			memset(insData, 0, 230);
			
			if (instr[y].size - 33 > 230)
				return MP_OUT_OF_MEMORY;
			
			f.read(insData, 1, instr[y].size - 33);
						
			if (instr[y].samp) {
				mp_ubyte* insDataPtr = insData;
				
				memcpy(nbu, insDataPtr, 96);
				insDataPtr+=96;
				
				TEnvelope venv;
				TEnvelope penv;
				memset(&venv,0,sizeof(venv));
				memset(&penv,0,sizeof(penv));
				
				mp_sint32 k;
				for (k = 0; k < XM_ENVELOPENUMPOINTS; k++)
				{
					venv.env[k][0] = LittleEndian::GET_WORD(insDataPtr);
					venv.env[k][1] = LittleEndian::GET_WORD(insDataPtr+2);
					insDataPtr+=4;
				}
				for (k = 0; k < XM_ENVELOPENUMPOINTS; k++)
				{
					penv.env[k][0] = LittleEndian::GET_WORD(insDataPtr);
					penv.env[k][1] = LittleEndian::GET_WORD(insDataPtr+2);
					insDataPtr+=4;
				}
				
				venv.num = *insDataPtr++;	
				if (venv.num > XM_ENVELOPENUMPOINTS) venv.num = XM_ENVELOPENUMPOINTS;
				penv.num = *insDataPtr++;	
				if (penv.num > XM_ENVELOPENUMPOINTS) penv.num = XM_ENVELOPENUMPOINTS;
				venv.sustain = *insDataPtr++;
				venv.loops = *insDataPtr++;
				venv.loope = *insDataPtr++;
				penv.sustain = *insDataPtr++;
				penv.loops = *insDataPtr++;
				penv.loope = *insDataPtr++;
				venv.type = *insDataPtr++;
				penv.type = *insDataPtr++;				
				
				mp_ubyte vibtype, vibsweep, vibdepth, vibrate;
				mp_uword volfade;
				
				vibtype = *insDataPtr++;
				vibsweep = *insDataPtr++;
				vibdepth = *insDataPtr++;
				vibrate = *insDataPtr++;
				
				vibdepth<<=1;
				
				volfade = LittleEndian::GET_WORD(insDataPtr);
				insDataPtr+=2;
				volfade<<=1;
				
				//instr[y].res = LittleEndian::GET_WORD(insDataPtr);
				insDataPtr+=2;
				
				for (mp_sint32 l=0;l<XM_ENVELOPENUMPOINTS;l++) {
					venv.env[l][1]<<=2;
					penv.env[l][1]<<=2;
				}
				
				if (!module->addVolumeEnvelope(venv)) 
					return MP_OUT_OF_MEMORY;
				if (!module->addPanningEnvelope(penv)) 
					return MP_OUT_OF_MEMORY;
				
				mp_sint32 g=0, sc;
				for (sc=0;sc<instr[y].samp;sc++) {
					
					smp[g+s].flags=3;
					smp[g+s].venvnum=e+1;
					smp[g+s].penvnum=e+1;
					
					smp[g+s].vibtype=vibtype;
					smp[g+s].vibsweep=vibsweep;
					smp[g+s].vibdepth=vibdepth;
					smp[g+s].vibrate=vibrate;
					smp[g+s].volfade=volfade;
					
					// not sure why I did that, actually doesn't make sense
					//if (!(venv.type&1)) smp[g+s].volfade=0;
					
					f.readDwords(&smp[g+s].samplen,1);
					f.readDwords(&smp[g+s].loopstart,1);
					f.readDwords(&smp[g+s].looplen,1);
					smp[g+s].vol=XModule::vol64to255(f.readByte());
					//f.read(&smp[g+s].vol,1,1);
					f.read(&smp[g+s].finetune,1,1);
					f.read(&smp[g+s].type,1,1);
#ifdef VERBOSE
					printf("Before: %i, After: %i\n", smp[g+s].type, smp[g+s].type & (3+16));
#endif
					f.read(&smp[g+s].pan,1,1);
					f.read(&smp[g+s].relnote,1,1);
					f.read(&smp[g+s].res,1,1);
					f.read(&smp[g+s].name,1,22);
					
					char line[30];
					memset(line, 0, sizeof(line));
					XModule::convertStr(line, smp[g+s].name, 23, false);					
					if (line[0])
						module->addSongMessageLine(line);
					
					// ignore empty samples
#ifndef MILKYTRACKER
					// ignore empty samples when not being a tracker
					if (smp[g+s].samplen) {
						smpReloc[sc] = g;
						g++;
					}
					else
						smpReloc[sc] = -1;
#else
					smpReloc[sc] = g;
					g++;
#endif
				}

				instr[y].samp = g;

				for (sc = 0; sc < 96; sc++) {
					if (smpReloc[nbu[sc]] == -1)
						instr[y].snum[sc] = -1;
					else
						instr[y].snum[sc] = smpReloc[nbu[sc]]+s;					
				}

				e++;
				
			}
			else
			{
				for (mp_sint32 i = 0; i < 120; i++)
					instr[y].snum[i] = -1;
			}

#ifdef MILKYTRACKER
			s+=16;
#else
			s+=instr[y].samp;
#endif				
				
			
		}
		
		header->smpnum=s;
		header->volenvnum=e;
		header->panenvnum=e;
	}
	
	for (y=0;y<header->patnum;y++) {
		
		if (header->ver == 0x104 || header->ver == 0x103)
		{
			f.readDwords(&phead[y].len,1);
			f.read(&phead[y].ptype,1,1);
			f.readWords(&phead[y].rows,1);
			f.readWords(&phead[y].patdata,1);
		}
		else
		{
			f.readDwords(&phead[y].len,1);
			f.read(&phead[y].ptype,1,1);
			phead[y].rows = (mp_uword)f.readByte()+1;
			f.readWords(&phead[y].patdata,1);			
		}
		
		phead[y].effnum=2;
		phead[y].channum=(mp_ubyte)header->channum;
		
		phead[y].patternData = new mp_ubyte[phead[y].rows*header->channum*6];
		
		// out of memory?
		if (phead[y].patternData == NULL)
		{
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[y].patternData,0,phead[y].rows*header->channum*6);
		
		if (phead[y].patdata) {
			mp_ubyte *buffer = new mp_ubyte[phead[y].patdata];
			
			// out of memory?
			if (buffer == NULL)
			{
				return MP_OUT_OF_MEMORY;			
			}
			
			f.read(buffer,1,phead[y].patdata);
			
			//printf("%i\n", phead[y].patdata);
			
			mp_sint32 pc = 0, bc = 0;
			for (mp_sint32 r=0;r<phead[y].rows;r++) {
				for (mp_sint32 c=0;c<header->channum;c++) {
					
					mp_ubyte slot[5];
					memset(slot,0,5);
					
					if ((buffer[pc]&128)) {
						
						mp_ubyte pb = buffer[pc];
						pc++;
						
						if ((pb&1)) {
							//phead[y].patternData[bc]=buffer[pc];
							slot[0]=buffer[pc];
							pc++;
						}
						if ((pb&2)) {
							//phead[y].patternData[bc+1]=buffer[pc];
							slot[1]=buffer[pc];
							pc++;
						}
						if ((pb&4)) {
							//phead[y].patternData[bc+2]=buffer[pc];
							slot[2]=buffer[pc];
							pc++;
						}
						if ((pb&8)) {
							//phead[y].patternData[bc+3]=buffer[pc];
							slot[3]=buffer[pc];
							pc++;
						}
						if ((pb&16)) {
							//phead[y].patternData[bc+4]=buffer[pc];
							slot[4]=buffer[pc];
							pc++;
						}
						
					}
					else {
						//memcpy(phead[y].patternData+bc,buffer+pc,5);
						memcpy(slot,buffer+pc,5);
						pc+=5;
					}
					
					char gl=0;
					for (mp_sint32 i=0;i<XModule::numValidXMEffects;i++)
						if (slot[3]==XModule::validXMEffects[i]) gl=1;
					
					if (!gl) slot[3]=slot[4]=0;
					
					if ((slot[3]==0xC)||(slot[3]==0x10)) {
						slot[4] = XModule::vol64to255(slot[4]);
						/*mp_sint32 bl = slot[4];
						if (bl>64) bl=64;
						slot[4]=(bl*261120)>>16;*/
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
					
					phead[y].patternData[bc]=slot[0];
					phead[y].patternData[bc+1]=slot[1];
					
					XModule::convertXMVolumeEffects(slot[2], phead[y].patternData[bc+2], phead[y].patternData[bc+3]);

					phead[y].patternData[bc+4]=slot[3];
					phead[y].patternData[bc+5]=slot[4];
					
					/*if ((y==3)&&(c==2)) {
						for (mp_sint32 bl=0;bl<6;bl++) cprintf("%x ",phead[y].patternData[bc+bl]);
					cprintf("\r\n");
					getch();
					};*/
					
					/*printf("Note : %i\r\n",phead[y].patternData[bc]);
					printf("Ins  : %i\r\n",phead[y].patternData[bc+1]);
					printf("Vol  : %i\r\n",phead[y].patternData[bc+2]);
					printf("Eff  : %i\r\n",phead[y].patternData[bc+3]);
					printf("Effop: %i\r\n",phead[y].patternData[bc+4]);
					getch();*/
					
					bc+=6;
				} // for c
					
			} // for r
				
			delete[] buffer;
		}
			
	}
		
	if (header->ver == 0x104)
	{
		mp_sint32 s = 0;
		mp_sint32 e = 0;
		for (y=0;y<header->insnum;y++) {

			// fixes MOOH.XM loading problems
			// seems to store more instruments in the header than in the actual file
			if (f.posWithBaseOffset() >= fileSize)
				break;
		
			//TXMInstrument* ins = &instr[y];
		
			f.readDwords(&instr[y].size,1);
			
			if (instr[y].size < 29)
			{
				mp_ubyte buffer[29];
				memset(buffer, 0, sizeof(buffer));
				f.read(buffer, 1, instr[y].size - 4);
				memcpy(instr[y].name, buffer, 22);
				instr[y].type = buffer[22];
				instr[y].samp = LittleEndian::GET_WORD(buffer + 23);
			}
			else
			{
				f.read(&instr[y].name,1,22);		
				f.read(&instr[y].type,1,1);
				f.readWords(&instr[y].samp,1);
			}

			//printf("%i, %i\n", instr[y].size, instr[y].samp);

			if (instr[y].size <= 29)
			{
#ifdef MILKYTRACKER
				s+=16;
#endif
				for (mp_sint32 i = 0; i < 120; i++)
					instr[y].snum[i] = -1;
				continue;
			}

			f.readDwords(&instr[y].shsize,1);
#ifdef VERBOSE
			printf("%i/%i: %i, %i, %i, %s\n",y,header->insnum-1,instr[y].size,instr[y].shsize,instr[y].samp,instr[y].name);			
#endif
			memset(insData, 0, 230);
			
			if (instr[y].size - 33 > 230)
			{
				//return -7;
				break;
			}
			
			f.read(insData, 1, instr[y].size - 33);
			
			/*printf("%i\r\n",instr[y].size);
			printf("%s\r\n",instr[y].name);
			printf("%i\r\n",instr[y].type);
			printf("%i\r\n",instr[y].samp);
			printf("%i\r\n",instr[y].shsize);*/
			//getch();
					
			memset(smpReloc, 0, sizeof(smpReloc));
			
			if (instr[y].samp) {
				mp_ubyte* insDataPtr = insData;
				
				//f.read(&nbu,1,96);
				
				memcpy(nbu, insDataPtr, 96);
				insDataPtr+=96;
				
				TEnvelope venv;
				TEnvelope penv;
				memset(&venv,0,sizeof(venv));
				memset(&penv,0,sizeof(penv));
				
				mp_sint32 k;
				for (k = 0; k < XM_ENVELOPENUMPOINTS; k++)
				{
					venv.env[k][0] = LittleEndian::GET_WORD(insDataPtr);
					venv.env[k][1] = LittleEndian::GET_WORD(insDataPtr+2);
					insDataPtr+=4;
				}
				for (k = 0; k < XM_ENVELOPENUMPOINTS; k++)
				{
					penv.env[k][0] = LittleEndian::GET_WORD(insDataPtr);
					penv.env[k][1] = LittleEndian::GET_WORD(insDataPtr+2);
					insDataPtr+=4;
				}
				
				venv.num = *insDataPtr++;	
				if (venv.num > XM_ENVELOPENUMPOINTS) venv.num = XM_ENVELOPENUMPOINTS;
				penv.num = *insDataPtr++;					
				if (penv.num > XM_ENVELOPENUMPOINTS) penv.num = XM_ENVELOPENUMPOINTS;
				venv.sustain = *insDataPtr++;
				venv.loops = *insDataPtr++;
				venv.loope = *insDataPtr++;
				penv.sustain = *insDataPtr++;
				penv.loops = *insDataPtr++;
				penv.loope = *insDataPtr++;
				venv.type = *insDataPtr++;
				penv.type = *insDataPtr++;				
				
				mp_ubyte vibtype, vibsweep, vibdepth, vibrate;
				mp_uword volfade;
				
				vibtype = *insDataPtr++;
				vibsweep = *insDataPtr++;
				vibdepth = *insDataPtr++;
				vibrate = *insDataPtr++;
				
				vibdepth<<=1;
				
				//f.readWords(&volfade,1);
				volfade = LittleEndian::GET_WORD(insDataPtr);
				insDataPtr+=2;
				volfade<<=1;
				
				//instr[y].res = LittleEndian::GET_WORD(insDataPtr);
				insDataPtr+=2;
				
				for (mp_sint32 l=0;l<XM_ENVELOPENUMPOINTS;l++) {
					venv.env[l][1]<<=2;
					penv.env[l][1]<<=2;
				}
				
				if (!module->addVolumeEnvelope(venv)) 
					return MP_OUT_OF_MEMORY;
				if (!module->addPanningEnvelope(penv)) 
					return MP_OUT_OF_MEMORY;
				
				mp_sint32 g=0, sc;
				for (sc=0;sc<instr[y].samp;sc++) {
					//TXMSample* smpl = &smp[g+s];
					
					smp[g+s].flags=3;
					smp[g+s].venvnum=e+1;
					smp[g+s].penvnum=e+1;
					
					smp[g+s].vibtype=vibtype;
					smp[g+s].vibsweep=vibsweep;
					smp[g+s].vibdepth=vibdepth;
					smp[g+s].vibrate=vibrate;
					smp[g+s].volfade=volfade;
					
					// not sure why I did that, actually doesn't make sense
					//if (!(venv.type&1)) smp[g+s].volfade=0;
					
					f.readDwords(&smp[g+s].samplen,1);
					
					f.readDwords(&smp[g+s].loopstart,1);
					f.readDwords(&smp[g+s].looplen,1);
					smp[g+s].vol=XModule::vol64to255(f.readByte());
					//f.read(&smp[g+s].vol,1,1);
					f.read(&smp[g+s].finetune,1,1);
					f.read(&smp[g+s].type,1,1);
#ifdef VERBOSE
					printf("Before: %i, After: %i\n", smp[g+s].type, smp[g+s].type & (3+16));
#endif
					f.read(&smp[g+s].pan,1,1);
					f.read(&smp[g+s].relnote,1,1);
					f.read(&smp[g+s].res,1,1);
					f.read(&smp[g+s].name,1,22);

					char line[30];
					memset(line, 0, sizeof(line));
					XModule::convertStr(line, smp[g+s].name, 23, false);					
					if (line[0])
						module->addSongMessageLine(line);
					
#ifndef MILKYTRACKER
					// ignore empty samples when not being a tracker
					if (smp[g+s].samplen) {
						smpReloc[sc] = g;
						g++;
					}
					else
						smpReloc[sc] = -1;
#else
					smpReloc[sc] = g;
					g++;
#endif
				}

				instr[y].samp = g;

				for (sc = 0; sc < 96; sc++) {					
					if (smpReloc[nbu[sc]] == -1)
						instr[y].snum[sc] = -1;
					else
						instr[y].snum[sc] = smpReloc[nbu[sc]]+s;
				}
						
				for (sc=0;sc<instr[y].samp;sc++) {
				
					if (smp[s].samplen)
					{
						bool adpcm = (smp[s].res == 0xAD);
					
						mp_uint32 oldSize = smp[s].samplen;
						if (smp[s].type&16) 
						{
							smp[s].samplen>>=1;
							smp[s].loopstart>>=1;
							smp[s].looplen>>=1;
						}
						
						mp_sint32 result = module->loadModuleSample(f, s, 
													 adpcm ? XModule::ST_PACKING_ADPCM : XModule::ST_DELTA, 
													 adpcm ? (XModule::ST_PACKING_ADPCM | XModule::ST_16BIT) : (XModule::ST_DELTA | XModule::ST_16BIT), 
													 oldSize);
						if (result != MP_OK)
							return result;					
						
						if (adpcm)
							smp[s].res = 0;
					}
					
					s++;
					
					if (s>=MP_MAXSAMPLES)
						return MP_OUT_OF_MEMORY;
					
				}

				e++;
				
			}
			else
			{
				for (mp_sint32 i = 0; i < 120; i++)
					instr[y].snum[i] = -1;
			}

#ifdef MILKYTRACKER
			s+=16 - instr[y].samp;
#endif				
			
		}
		
		header->smpnum=s;
		header->volenvnum=e;
		header->panenvnum=e;		
		
	}
	else
	{
		mp_sint32 s = 0;
		for (y=0;y<header->insnum;y++) {
			for (sc=0;sc<instr[y].samp;sc++) {

				if (smp[s].samplen)
				{
					mp_uint32 oldSize = smp[s].samplen;
					if (smp[s].type&16) 
					{
						smp[s].samplen>>=1;
						smp[s].loopstart>>=1;
						smp[s].looplen>>=1;
					}
					
					mp_sint32 result = module->loadModuleSample(f, s, XModule::ST_DELTA, XModule::ST_DELTA | XModule::ST_16BIT, oldSize);
					if (result != MP_OK)
						return result;					
				}
				
				s++;
				
				if (s>=MP_MAXSAMPLES)
					return MP_OUT_OF_MEMORY;				
			}
			
#ifdef MILKYTRACKER
			s+=16 - instr[y].samp;
#endif
			
		}		
	}
	
	// convert modplug stereo samples
	for (mp_sint32 s = 0; s < header->smpnum; s++)
	{
		if (smp[s].type & 32)
		{		
			// that's what's allowed, stupid modplug tracker
			smp[s].type &= 3+16;					

			if (smp[s].sample == NULL)
				continue;
			
			if (!(smp[s].type&16)) {			
				smp[s].samplen>>=1;
				smp[s].loopstart>>=1;
				smp[s].looplen>>=1;
				
				mp_sbyte* sample = (mp_sbyte*)smp[s].sample;
				mp_sint32 samplen = smp[s].samplen;
				for (mp_sint32 i = 0; i < samplen; i++)
				{
					mp_sint32 s = ((mp_sint32)sample[i] + (mp_sint32)sample[i + samplen]) >> 1;
					if (s < -128) s = -128;
					if (s > 127) s = 127;
					sample[i] = (mp_sbyte)s;
				}
			}
			else
			{
				smp[s].samplen>>=1;
				smp[s].loopstart>>=1;
				smp[s].looplen>>=1;
				
				mp_sword* sample = (mp_sword*)smp[s].sample;
				mp_sint32 samplen = smp[s].samplen;
				for (mp_sint32 i = 0; i < samplen; i++)
				{
					mp_sint32 s = ((mp_sint32)sample[i] + (mp_sint32)sample[i + samplen]) >> 1;
					if (s < -32768) s = -32768;
					if (s > 32767) s = 32767;
					sample[i] = (mp_sword)s;
				}
			}
		}
		
		// correct loop type 0x03 (undefined)
		// will become ping pong loop
		// note that FT2 will refuse to load XM files with such a loop type
		if ((smp[s].type & 0x3) == 0x3)
			smp[s].type&=~1;		
	}

	// correct number of patterns if necessary, otherwise the post processing will remove
	// the "invalid" patterns from the order list
	bool addPatterns = false;
	for (i = 0; i < header->ordnum; i++)
		if (header->ord[i]+1 > header->patnum)
		{
			header->patnum = header->ord[i]+1;	
			addPatterns = true;
		}
	
	// if the pattern number has been adjusted, add some empty patterns
	if (addPatterns)
	{
		for (i = 0; i < header->patnum; i++)
			if (phead[i].patternData == NULL)
			{
				phead[i].rows = 64;
				phead[i].effnum = 2;
				phead[i].channum = (mp_ubyte)header->channum;

				phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*6];
			
				// out of memory?
				if (phead[i].patternData == NULL)
				{
					return MP_OUT_OF_MEMORY;
				}
		
				memset(phead[i].patternData,0,phead[i].rows*header->channum*6);
			}
	}
	
	// check for MODPLUG extensions
	if (f.posWithBaseOffset() + 8 <= fileSize)
	{
		char buffer[4];
		f.read(buffer, 1, 4);
		if (memcmp(buffer, "text", 4) == 0)
		{
			mp_uint32 len = f.readDword();
			module->allocateSongMessage(len+1);
			
			memset(module->message, 0, len+1);
			
			f.read(module->message, 1, len);
		}
	}
	
	module->postProcessSamples();
	
	return MP_OK;
}
