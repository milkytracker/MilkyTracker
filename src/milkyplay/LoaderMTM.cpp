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
 *  LoaderMTM.cpp
 *  MilkyPlay Module Loader: Multitracker
 *
 *  Warning: This is an one-by-one conversion of an assembler version ;)
 *
 */
#include "Loaders.h"

const char* LoaderMTM::identifyModule(const mp_ubyte* buffer)
{
	// check for .MTM module
	if (!memcmp(buffer,"MTM\x10",4)) 
	{
		return "MTM";
	}

	return NULL;
}

mp_sint32 LoaderMTM::load(XMFileBase& f, XModule* module)
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
	
	f.read(header->sig, 1, 3);	// read signature

	if (f.readByte() != 0x10)
		return MP_LOADER_FAILED;

	f.read(header->name, 1, 20);

	mp_uword numTracks = f.readWord();

	header->flags = XModule::MODULE_PTNEWINSTRUMENT;

	header->mainvol = 255;
	header->tempo = 6;
	header->speed = 125;
	header->patnum = f.readByte() + 1;
	header->ordnum = f.readByte() + 1;

	mp_uword locf = f.readWord();

	header->insnum = f.readByte();
	header->smpnum = header->insnum;

	f.readByte();

	mp_sint32 numRows = f.readByte();
	mp_sint32 trackSize = numRows*3;

	header->channum = f.readByte();

	mp_ubyte panPositions[32];
	f.read(panPositions, 1, 32); 

	mp_sint32 i,s = 0;
	for (i = 0; i < header->insnum; i++) 
	{		
		f.read(instr[i].name,1,22);

		mp_uint32 size = f.readDword();
		mp_uint32 loopstart = f.readDword();
		mp_uint32 loopend = f.readDword();		

		mp_sbyte finetune = f.readByte()*16;

		mp_ubyte volume = XModule::vol64to255(f.readByte());
		
		/*mp_ubyte flags = */f.readByte(); 
		
		if (size)
		{
			instr[i].samp = 1;
			
			memcpy(smp[s].name,instr[i].name,22);
		
			smp[s].flags = 1;
			smp[s].samplen = size;
			smp[s].loopstart = loopstart;
			smp[s].looplen = loopend - loopstart;
			smp[s].vol = volume;
			smp[s].finetune = finetune;
			
			for (mp_sint32 j = 0; j < 120; j++) 
				instr[i].snum[j] = s;
				
			if ((smp[s].loopstart+smp[s].looplen)>smp[s].samplen)
				smp[s].looplen-=(smp[s].loopstart+smp[s].looplen)-smp[s].samplen;
				
			if (smp[s].loopstart<=2) smp[s].loopstart=0;
			if (smp[s].looplen<=2) 
				smp[s].looplen=0;
			else smp[s].type=1;
			
			s++;
		}
			
	}
	
	header->smpnum = s;

	mp_ubyte orders[128];
	f.read(orders, 1, 128);
	
	for (i = 0; i < header->ordnum; i++)
		header->ord[i] = orders[i];	

	mp_ubyte* tracks = new mp_ubyte[trackSize*numTracks];

	if (tracks == NULL)
		return MP_OUT_OF_MEMORY;

	f.read(tracks, trackSize, numTracks);

	mp_uword* trackSeq = new mp_uword[header->patnum * 32];
	
	if (trackSeq == NULL)
	{
		delete[] tracks;
		return MP_OUT_OF_MEMORY;
	}

	f.readWords(trackSeq, header->patnum * 32);

	mp_uword* pTrackSeq = trackSeq;

	for (i = 0; i < header->patnum;i++) 
	{
		phead[i].rows = numRows;
		phead[i].effnum = 1;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*4];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			delete[] tracks;
			delete[] trackSeq;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum*4);
		
		mp_sint32 c;
		for (c=0;c<32;c++) 
		{
			if (*pTrackSeq &&
				(*pTrackSeq-1) < numTracks &&
			    c < header->channum)
			{
				
				mp_ubyte *track = (*pTrackSeq-1)*trackSize + tracks;
				
#ifdef VERBOSE
				if ((*pTrackSeq-1) >= numTracks)
					printf("piiiiep");
#endif

				for (mp_sint32 row = 0; row < numRows; row++)
				{
					mp_ubyte* dstSlot = phead[i].patternData+row*phead[i].channum*4+c*4;

					mp_ubyte note = track[0]>>2;

					if (note)
						note+=25;

					mp_ubyte ins = ((track[0]&0x03)<<4)+(track[1]>>4);
					mp_ubyte eff = track[1]&0x0f;
					mp_ubyte op = track[2];
					
					if (eff==0xE) 
					{
						eff=(op>>4)+0x30;
						op&=0xf;
					
						if (eff == 0x38)
						{
							eff = 0x08;
							op <<= 4;
						}
					
					}
					
					if ((!eff)&&op) 
						eff=0x20;

					// old style modules don't support last effect for:
					// - portamento up/down
					// - volume slide
					if (eff==0x1&&(!op)) eff = 0;
					if (eff==0x2&&(!op)) eff = 0;
					if (eff==0xA&&(!op)) eff = 0;
					
					if (eff==0x5&&(!op)) eff=0x3;
					if (eff==0x6&&(!op)) eff=0x4;
					
					if (eff==0xC) 
						op = XModule::vol64to255(op);
					
					dstSlot[0] = note;
					dstSlot[1] = ins;
					dstSlot[2] = eff;
					dstSlot[3] = op;
				
					track+=3;
				}
			}
			
			pTrackSeq++;
		
		}
		
	}
	
	delete[] trackSeq;
	
	delete[] tracks;

	/*for (i = 0; i < locf; i++)
	{
		printf("%c",f.readByte());
	}
	
	printf("\n");*/
	
	// song message
	if (locf)
	{		
		char* unpackedSongMessage = new char[locf];
		
		f.read(unpackedSongMessage, 1, locf);
		
		mp_sint32 size = locf;
		
		// song message isn't null terminated
		for (i = 0; i < size; i++)
			if (unpackedSongMessage[i] == '\0') unpackedSongMessage[i] = ' ';
		
		for (i = 0; i < size; i++)
		{
			char line[50];
			memset(line, 0, sizeof(line));
			
			if (size - i >= 39)
			{
				XModule::convertStr(line, unpackedSongMessage+i, 40, false);
				i+=39;
			}
			else
			{
				XModule::convertStr(line, unpackedSongMessage+i, size-i, false);
				i+=size-i;
			}
			module->addSongMessageLine(line);
		}				
		
		delete[] unpackedSongMessage;
		
#ifdef VERBOSE
		printf("%s\n",module->message);
#endif

	}
	
	mp_sint32 result = module->loadModuleSamples(f, XModule::ST_UNSIGNED);
	if (result != MP_OK)
		return result;

	strcpy(header->tracker,"Multitracker");

	module->setDefaultPanning();
	
	module->postProcessSamples();

	return MP_OK;
}
