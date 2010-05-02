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
 *  LoaderULT.cpp
 *  MilkyPlay Module Loader: Ultratracker
 */
#include "Loaders.h"

const char* LoaderULT::identifyModule(const mp_ubyte* buffer)
{
	if (memcmp(buffer, "MAS_UTrack_V00", 14) == 0)
	{
		if (buffer[14] < '1' || buffer[14] > '4')
			return NULL;
			
		return "ULT";
	}

	// check for .ULT
	return NULL;
}

static void convertULTEffects(mp_ubyte& effect, mp_ubyte& operand)
{
	switch (effect)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x07:
		case 0x0A:
		case 0x0C:
		case 0x0D:
			break;

		case 0x05:
			effect = 0x23;
			break;
			
		case 0x09:
			effect = 0x21;
			break;

		case 0x0B:
			effect = 0x08;
			operand=(mp_ubyte)XModule::pan15to255(operand);
			break;

		case 0x0E:
			effect = 0x30 + (operand >> 4);
			operand &= 0xF;
			break;

		case 0x0F:
			if (operand >= 0x30)
				effect = 0x16;
			else
				effect = 0x1C;
			break;
			
		default:
			effect = operand = 0;
	}
	
}

mp_sint32 LoaderULT::load(XMFileBase& f, XModule* module)
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

	mp_sint32 i,j,k;
	
	// read header
	f.read(header->sig, 1, 15);
	
	mp_ubyte ver = header->sig[14];
	
	f.read(header->name, 1, 32);
	
	i = f.readByte();
	
	// read song message?
	if (ver >= '2')
	{
		char line[33], dummy[40];
		
		for (j = 0; j < i; j++)
		{
			memset(line, 0, sizeof(line));
			memset(dummy, 0, sizeof(dummy));

			f.read(dummy, 1, 32);
			XModule::convertStr(line, dummy, 33, false);					
			if (*line)
				module->addSongMessageLine(line);
		}
	}
	
	// read instruments	
	header->insnum = f.readByte();
	
	mp_sint32 s = 0;
	for (i = 0; i < header->insnum; i++)
	{
		mp_ubyte sampleName[32];
		mp_ubyte dosName[12];
		mp_sint32 loopstart;
		mp_sint32 loopend;
		mp_sint32 sizestart;
		mp_sint32 sizeend;
		mp_ubyte vol;
		mp_ubyte looptype;
		mp_sword finetune;
		mp_sint32 c2spd = 8363;
		
		f.read(sampleName, 1, 32);
		f.read(dosName, 1, 12);
		loopstart = f.readDword();
		loopend = f.readDword();
		sizestart = f.readDword();
		sizeend = f.readDword();
		vol = f.readByte();
		looptype = f.readByte();
		
		if (ver >= '4')
			c2spd = f.readWord();
		
		finetune = f.readWord();
		
		mp_sint32 smplen = sizeend - sizestart;
		
		memcpy(instr[i].name, sampleName, 32);
		
		if (smplen)
		{
			memcpy(smp[s].name, dosName, 12);
			
			instr[i].samp = 1;

			for (j=0;j<120;j++) 
				instr[i].snum[j] = s;
			
			mp_sint32 finalc4spd = c2spd/*+((c2spd*finetune)/32768)*/;
						
			XModule::convertc4spd(finalc4spd, &smp[s].finetune, &smp[s].relnote);
			
			smp[s].freqadjust = finetune;
			
			smp[s].flags = 1;
			smp[s].vol = vol;
			smp[s].pan = 0x80;
			
			smp[s].samplen = smplen;
			
			mp_sint32 looplen = loopend - loopstart;
			if (looplen < 0) 
				looplen = 0;
			
			smp[s].loopstart = loopstart;
			smp[s].looplen = looplen;
			
			if (looptype & 4)
			{
				smp[s].loopstart >>= 1;
				smp[s].looplen >>= 1;
				
				smp[s].type |= 16;
			}
				
			if ((looptype & (8+16)) == 24)
				smp[s].type |= 2;
			else if (looptype & 8)
				smp[s].type |= 1;
			
			s++;			
		}
	}

	header->smpnum = s;

	f.read(header->ord, 1, 256);
	
	bool slenFound = false;
	for (i = 0; i < 256; i++)
	{
		if (header->ord[i] != 255 && !slenFound)
			header->ordnum = i+1;
		else
		{
			header->ord[i] = 0;
			slenFound = true;
		}
	}
	
	header->channum = (mp_sword)f.readByte()+1;
	header->patnum = (mp_sword)f.readByte()+1;
	
	// panning positions
	module->setDefaultPanning();

	if (ver >= '3')
	{
		for (i = 0; i < header->channum; i++)
			header->pan[i] = (mp_ubyte)XModule::pan15to255(f.readByte());
	}

	mp_sint32 numTracks = header->channum*header->patnum;
	
	mp_uword* trackSeq = new mp_uword[numTracks];
	
	if (trackSeq == NULL)
	{
		return MP_OUT_OF_MEMORY;
	}

	k = 0;
	for (i = 0; i < header->channum; i++)
		for (j = 0; j < header->patnum; j++)
			trackSeq[j*header->channum+i] = k++;

	mp_sint32 trackSize = 64*5;

	mp_ubyte* tracks = new mp_ubyte[trackSize*numTracks];

	if (tracks == NULL)
	{
		delete[] trackSeq;
		return MP_OUT_OF_MEMORY;
	}

	// decode tracks
	mp_ubyte* track = tracks;
	for (i = 0; i < numTracks; i++)
	{
	
		mp_sint32 row = 0;
		while (row < 64)
		{
		
			mp_sint32 rep = 1;
			mp_ubyte note, ins, eff, op1, op2;
		
			note = f.readByte();
			if (note == 0xFC)
			{
				rep = f.readByte();
				note = f.readByte();
			}

			ins = f.readByte();
			eff = f.readByte();
			op1 = f.readByte();
			op2 = f.readByte();
			
			for (j = 0; j < rep; j++)
			{
				*track++ = note;
				*track++ = ins;
				*track++ = eff;
				*track++ = op1;
				*track++ = op2;
				row++;
			}
		
		}
	
	}
	
	// build patterns
	mp_uword* pTrackSeq = trackSeq;

	for (i = 0; i < header->patnum;i++) 
	{
		phead[i].rows = 64;
		phead[i].effnum = 2;
		phead[i].channum = (mp_ubyte)header->channum;
		
		mp_sint32 slotSize = 2+(2*phead[i].effnum);
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*slotSize];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			delete[] tracks;
			delete[] trackSeq;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum*slotSize);
		
		mp_sint32 c;
		for (c=0;c<header->channum;c++) 
		{
			if (*pTrackSeq < numTracks &&
			    c < header->channum)
			{
				
				mp_ubyte *track = *pTrackSeq*trackSize + tracks;
				
				bool portaNote1 = false;
				bool portaNote2 = false;

				for (mp_sint32 row = 0; row < 64; row++)
				{
					mp_ubyte* dstSlot = phead[i].patternData+row*phead[i].channum*slotSize+c*slotSize;

					mp_ubyte note = 0, ins = 0, eff1 = 0, op1 = 0, eff2 = 0, op2 = 0;
					
					if (*track)
						note = *track + 24;
						
					ins = *(track+1);

					eff1 = *(track+2) >> 4;
					eff2 = *(track+2) & 0xF;

					op2 = *(track+3);
					op1 = *(track+4);

					if (*(track+2) != 0x99)
					{
						convertULTEffects(eff1, op1);
						convertULTEffects(eff2, op2);
					}
					else
					{
						eff1 = 0x22;
						eff2 = 0x0;
					}
					
					// I need to fill in some portamento to note commands on the first effect
					if (portaNote1 && !note/* && !(eff1 == 0x03 && op1)*/)
					{
						eff1 = 0x03;
						op1 = 0;
					}
					
					// I need to fill in some portamento to note commands on the second effect
					if (portaNote2 && !note/* && !(eff2 == 0x03 && op2)*/)
					{
						eff2 = 0x03;
						op2 = 0;
					}
					
					dstSlot[0] = note;
					dstSlot[1] = ins;
					dstSlot[2] = eff1;
					dstSlot[3] = op1;
					dstSlot[4] = eff2;
					dstSlot[5] = op2;

					// we're having a portamento to note, set flag to continue on following
					// rows if no other note intersects
					if (eff1 == 0x03 && op1 && note)
						portaNote1 = true;
					// note intersects, stop portamento to note
					else if (note)
						portaNote1 = false;

					// we're having a portamento to note, set flag to continue on following
					// rows if no other note intersects
					if (eff2 == 0x03 && op2 && note)
						portaNote2 = true;
					// note intersects, stop portamento to note
					else if (note)
						portaNote2 = false;

				
					track+=5;
				}
			}
			
			pTrackSeq++;
		
		}
		
	}
	
	delete[] trackSeq;
	delete[] tracks;
	
	mp_sint32 result = module->loadModuleSamples(f);
	if (result != MP_OK)
		return result;
	
	header->speed=125;
	header->tempo=6;
	header->mainvol=255;

	strcpy(header->tracker,"Ultratracker");

	module->postProcessSamples();

	return MP_OK;
}
