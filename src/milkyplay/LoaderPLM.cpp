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
 *  LoaderPLM.cpp
 *  MilkyPlay Module Loader: DisorderTracker PLM Loader
 *
 *  --------------------------------
 *			Version History:
 *  --------------------------------
 *  02/24/05: Added a flag in the sample field (bit 2 = 4) to take the sample 
 *            volume as channel mastervolume (see PlayerSTD.h)
 *  11/22/04: Position jump (tested) & pattern break (untested) support
 *  11/21/04: Overlapping patterns
 *  11/19/04: First work
 */ 
#include "Loaders.h"

#define PATTERNSIZE 64

struct TOrdHdr
{
	mp_uword startPos;
	mp_ubyte startChannel;
	mp_ubyte patternIndex;
};

const char* LoaderPLM::identifyModule(const mp_ubyte* buffer)
{
	// check for .PLM module first
	if (!memcmp(buffer,"PLM\x1A",4))
	{
		return "PLM";
	}

	// this is not an .PLM
	return NULL;
}

//////////////////////////////////////////////////////
// Load DisorderTracker II module
//////////////////////////////////////////////////////
mp_sint32 LoaderPLM::load(XMFileBase& f, XModule* module)
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

	///////////////////////////////////////////////////
	// read header
	///////////////////////////////////////////////////
	
	f.read(header->sig,1,3);
	f.readByte();

	mp_uint32 hdrSize = f.readByte();
	mp_ubyte ver = f.readByte();

	if (ver != 0x10)
		return MP_LOADER_FAILED;

	f.read(header->name, 1, 32);

	// skip remaining bytes from the song name
	f.readDword(); // 36
	f.readDword(); // 40
	f.readDword(); // 44
	f.readDword(); // 48 => skipped rest of song name :)
	
	header->channum = f.readByte();

	f.readByte();	// doc says ignore flags byte

	mp_ubyte maxVol = f.readByte();

	f.readByte();	// skip soundblaster amplify

	header->speed = f.readByte();

	header->tempo = f.readByte();
	
	// panning positions
	for (i = 0; i < 32; i++)
		header->pan[i] = (mp_ubyte)XModule::pan15to255(f.readByte());

	header->smpnum = header->insnum = f.readByte();

	mp_sint32 numPatterns = f.readByte();

	mp_sint32 numOrders = f.readWord();

	header->mainvol = 255;

	///////////////////////////////////////////////////
	// read orderlist, special with PLM
	///////////////////////////////////////////////////

	f.seekWithBaseOffset(hdrSize);

	TOrdHdr* ordHeaders = new TOrdHdr[numOrders];

	for (i = 0; i < numOrders; i++)
	{
		ordHeaders[i].startPos = f.readWord();
		ordHeaders[i].startChannel = f.readByte();
		ordHeaders[i].patternIndex = f.readByte();

		/*printf("%x, %i, %i, %i\n",f.posWithBaseOffset(), ordHeaders[i].startPos, ordHeaders[i].startChannel, ordHeaders[i].patternIndex);
		getch();*/

	}

	mp_uint32* patOffsets = new mp_uint32[numPatterns];

	mp_uint32* smpOffsets = new mp_uint32[header->insnum];

	f.readDwords(patOffsets, numPatterns);
	f.readDwords(smpOffsets, header->insnum);

	///////////////////////////////////////////////////
	// read patterns
	///////////////////////////////////////////////////
	mp_ubyte* patterns[256];

	for (i = 0; i < numPatterns; i++)
	{
		
		if (patOffsets[i] != 0)
		{
			f.seekWithBaseOffset(patOffsets[i]);
			
			mp_uint32 size = f.readDword();
			
			patterns[i] = new mp_ubyte[size];
			
			if (patterns[i] == NULL)
			{
				
				for (j = 0; j < i; j++)
					delete[] patterns[j];
				
				delete[] smpOffsets;
				delete[] patOffsets;
				delete[] ordHeaders;
				return MP_OUT_OF_MEMORY;
			}
			
			f.read(patterns[i], 1, size);
		}
		else
		{
			patterns[i] = NULL;
		}

	}

	///////////////////////////////////////////////////
	// read instruments (+samples)
	///////////////////////////////////////////////////
	/* Sample layout:
	id              4 bytes  0			; ID (PLS+28)
	headersize      1 byte   4			; size of header in bytes, including ID etc
	version         1 byte   5
	fullname        32 byte  6          ; NOT asciiz
	filename        12 byte  38			; ditto
	pan             byte     50         ; default pan, 0..f, >f=none
	vol             byte     51			; default vol 0..40h
	flags           byte     52			; 1 = 16 bit , 0=8 bit
	c4spd           word     53			; c4spd (as for S3M)
	gusloc          dword    55			; posn in gusram (not used in file)
	loopst          dword    59			; loopstart
	loopen          dword    63			; loopend
	len             dword    67			; data size IN BYTES
	data			lots of bytes		; unsigned data*/

	for (i = 0; i < header->insnum; i++)
	{

		if (!smpOffsets[i])
			continue;

		f.seekWithBaseOffset(smpOffsets[i]);

		mp_uint32 id = f.readDword();

		if (id != 0x1a534c50)
		{
			for (j = 0; j < numPatterns; j++)
				delete[] patterns[j];
			
			delete[] smpOffsets;
			delete[] patOffsets;
			delete[] ordHeaders;
			
			return MP_LOADER_FAILED;
		}

		mp_uint32 sHdrSize = f.readByte();
		mp_ubyte ver = f.readByte();
		
		f.read(instr[i].name, 1, 32);

		f.read(smp[i].name, 1, 12);	

		mp_ubyte pan = f.readByte();

		smp[i].flags = 4;

		if (pan <= 0xf)
		{
			smp[i].pan = (mp_ubyte)XModule::pan15to255(pan);
			smp[i].flags|=2;
		}

		mp_ubyte vol = f.readByte(); 
		
		smp[i].vol = vol <= 64 ? XModule::vol64to255(vol) : 255/*0xff*/;

		mp_ubyte flags = f.readByte();

		smp[i].type = (flags&1)?16:0;

		XModule::convertc4spd(f.readWord(), &smp[i].finetune, &smp[i].relnote);

		f.readDword();	// skip guspos

		smp[i].loopstart = f.readDword();
		//smp[i].looplen = f.readDword();

		mp_sint32 looplen = ((mp_sint32)f.readDword() - (mp_sint32)smp[i].loopstart) - ((flags&1)?2:1);
		if (looplen < 0) 
			looplen = 0;
		smp[i].looplen = looplen;

		if (smp[i].looplen)
		{
			smp[i].type = (flags & 2) ? 2 : 1;
		}
	
		smp[i].samplen = f.readDword();

#ifdef VERBOSE
		printf("%i: %i, %i, %x\n",i+1,vol,flags,smp[i].samplen);		
#endif

		if (smp[i].samplen)
		{

			instr[i].samp = 1;

			for (j = 0; j < 120; j++)
			{
				instr[i].snum[j] = i;
			}

			smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen);
			
			if (smp[i].sample == NULL)
			{
				for (j = 0; j < numPatterns; j++)
					delete[] patterns[j];
				
				delete[] smpOffsets;
				delete[] patOffsets;
				delete[] ordHeaders;
				return MP_OUT_OF_MEMORY;
			}
			
			if (flags&1)
			{		
				module->loadSample(f, smp[i].sample, smp[i].samplen, smp[i].samplen>>1, XModule::ST_UNSIGNED);
				smp[i].samplen>>=1;
			}
			else 
			{
				module->loadSample(f, smp[i].sample, smp[i].samplen, smp[i].samplen, XModule::ST_UNSIGNED);				
				// due to some bug in DT2 it seems all samples are starting with
				// signed byte -47
				// we're trying to apply some correction to that
				if (smp[i].samplen)
					smp[i].sample[0] = smp[i].sample[1];
			}

		}

	}

	delete[] smpOffsets;

	delete[] patOffsets;

	///////////////////////////////////////////////////
	// convert song
	///////////////////////////////////////////////////
	mp_sint32 maxLen = 0;

	for (i = 0; i < numOrders; i++)
	{
		j = ordHeaders[i].patternIndex;

		if (patterns[j] != NULL)
		{
			
			mp_sint32 rows = *patterns[j];

			if ((ordHeaders[i].startPos + rows) > maxLen)
				maxLen = ordHeaders[i].startPos + rows;
		}
	}

	mp_sint32 numConvertedPatterns = maxLen / PATTERNSIZE;
	mp_sint32 lastPatternLength = maxLen & (PATTERNSIZE-1);
	if (lastPatternLength)
		numConvertedPatterns++;

	if (numConvertedPatterns > 255)
		numConvertedPatterns = 255;

#ifdef VERBOSE
	printf("Number of rows in song %i => %i patterns\n", maxLen, numConvertedPatterns);	
#endif

	header->patnum = numConvertedPatterns;
	header->ordnum = numConvertedPatterns;
	
	mp_uint32 rowCnt = 0;
	
	mp_uword* ordTable = new mp_uword[65536];
	
	for (i = 0; i < numConvertedPatterns; i++)
	{
		
		header->ord[i] = i;
		
		phead[i].rows = PATTERNSIZE;
		phead[i].effnum = 3;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*8];

		memset(phead[i].patternData, 0, phead[i].rows*header->channum*8);

		// find possible hits within this pattern
		mp_sint32 numResults = 0;
		for (j = 0; j < numOrders; j++)
		{
			
			// valid pattern?
			if (patterns[ordHeaders[j].patternIndex] != NULL)
			{
				// pattern intersection scenario
				
				if ((mp_uint32)ordHeaders[j].startPos+(mp_uint32)*patterns[ordHeaders[j].patternIndex] < rowCnt ||
					ordHeaders[j].startPos > (rowCnt+PATTERNSIZE))
				{
					continue;
				}
				
				ordTable[numResults++] = j;
			}
			
			
		}
		
		for (mp_sint32 rows = 0; rows < PATTERNSIZE; rows++)
		{


			for (mp_sint32 c = 0; c < header->channum; c++)
			{
				

				mp_sint32 lastStartPos = -1;
				mp_sint32 lastIndex = -1;	
				mp_sint32 theIndex = -1;

				for (j = 0; j < numResults/*numOrders*/; j++)
				{	

					mp_sint32 index = ordTable[j];
					//mp_sint32 index = j;
					if (ordHeaders[index].startPos <= rowCnt &&
						((mp_uint32)ordHeaders[index].startPos + (mp_uint32)*patterns[ordHeaders[index].patternIndex]) > rowCnt &&
						ordHeaders[index].startChannel <= c &&
						ordHeaders[index].startChannel + *(patterns[ordHeaders[index].patternIndex]+1) > c /*&&
						ordHeaders[index].startPos >= lastStartPos*/)
					{

						if (ordHeaders[index].startPos == lastStartPos)
						{
							if (ordHeaders[index].patternIndex > lastIndex)
							{
								theIndex = index;
								lastIndex = ordHeaders[index].patternIndex;
								lastStartPos = ordHeaders[index].startPos;
							}
						}
						else
						{
							theIndex = index;
							lastIndex = ordHeaders[index].patternIndex;
							lastStartPos = ordHeaders[index].startPos;
						}

					}				

				}

				if (theIndex != -1)
				{
					// position within pattern
					mp_sint32 baseRow = rowCnt - ordHeaders[theIndex].startPos;
					mp_sint32 numChannels = *(patterns[ordHeaders[theIndex].patternIndex]+1);
					mp_ubyte* pattern = patterns[ordHeaders[theIndex].patternIndex]+28;
					mp_ubyte* srcSlot = pattern + ((c-ordHeaders[theIndex].startChannel)*5 + baseRow*numChannels*5);
					mp_ubyte* dstSlot = phead[i].patternData + (c*8 + rows*header->channum*8);

					mp_ubyte note = srcSlot[0];
					mp_ubyte ins = srcSlot[1];
					mp_ubyte vol = srcSlot[2];
					mp_ubyte eff = srcSlot[3];
					mp_ubyte op = srcSlot[4];

					
					dstSlot[0] = note ? (((note>>4)*12+(note&0xf))+1) : 0;
					dstSlot[1] = ins;

					if (vol!=255)
					{
						dstSlot[2] = 0xC;
						dstSlot[3] = vol<=64 ? XModule::vol64to255(vol) : 255;
					}

					mp_ubyte dstEff = 0;
					mp_ubyte dstOp = 0;


					switch (eff)
					{

						case 0x00:
							break;

						// s3m porta up
						case 0x01:
							dstEff = 0x47;
							dstOp = op;
							break;
						
						// s3m porta down
						case 0x02:
							dstEff = 0x48;
							dstOp = op;
							break;
						
						// porta to note
						case 0x03:
							dstEff = 0x03;
							dstOp = op;
							break;

						// s3m volslide
						case 0x04:
							dstEff = 0x49;
							dstOp = op;
							break;

						case 0x05:
							dstEff = 0x07;
							dstOp = op;
							break;

						case 0x06:
							dstEff = 0x04;
							dstOp = op;
							break;
							
						case 0x07:
							dstEff = 0x37;
							dstOp = op;
							break;
	
						case 0x08:
							dstEff = 0x34;
							dstOp = op;
							break;
							
						case 0x09:
							dstEff = 0x16;
							dstOp = op;
							break;
	
						case 0x0A:
							dstEff = 0x1C;
							dstOp = op;
							break;
							
						// position jump
						case 0x0B:
							dstEff = 0x2B;
							dstOp = (ordHeaders[op].startPos / PATTERNSIZE);
							dstSlot[7] = ordHeaders[op].startPos & (PATTERNSIZE-1);
							break;
						
						// pattern break
						case 0x0C:
							dstEff = 0x2B;
							dstOp = ((ordHeaders[theIndex+1].startPos+op) / PATTERNSIZE);
							dstSlot[7] = (ordHeaders[theIndex+1].startPos+op) & (PATTERNSIZE-1);
							break;
							
						case 0x0D:
							dstEff = 0x09;
							dstOp = op;
							break;
							
						case 0x0E:
							dstEff = 0x08;
							dstOp = (mp_ubyte)XModule::pan15to255(op);
							break;
	
						case 0x0F:
							dstEff =  0x1B;
							dstOp = op;
							break;
							
						case 0x10:
							dstEff =  0x3D;
							dstOp = op;
							break;
	
						case 0x11:
							dstEff =  0x3C;
							dstOp = op;
							break;
	
						case 0x12:
							dstEff =  0x3E;
							dstOp = op;
							break;
							
						case 0x13:
							dstEff =  0x4A;
							dstOp = op;
							break;
	
						case 0x14:
							dstEff =  0x6;
							dstOp = op;
							break;	
	
						case 0x15:
							dstEff =  0x5;
							dstOp = op;
							break;
	
						case 0x16:
							dstEff = 0x8;
							dstOp = op;
							break;

#ifdef VERBOSE
						default:
							printf("%x:%x\n",eff,op);
#endif

					}

					dstSlot[4] = dstEff;
					dstSlot[5] = dstOp;

#ifdef VERBOSE
					if (c >= 0x02 && c < 0x3 && rowCnt >= 0xA0 && rowCnt <= 0xB0)
					{
						printf("row %i: %i, %i, %i, %x, %x (source: %i, %i, %i)\n",rowCnt,srcSlot[0],srcSlot[1],srcSlot[2],srcSlot[3],srcSlot[4],ordHeaders[theIndex].patternIndex,ordHeaders[theIndex].startChannel,numChannels);
					}						
#endif

				}						

						
			}
								
			rowCnt++;
		}
		
	}

	delete[] ordTable;
	
	for (j = 0; j < numPatterns; j++)
		delete[] patterns[j];	
	
	delete[] ordHeaders;

	strcpy(header->tracker,"DisorderTracker 2");

	// take panning positions from start
	//module->setDefaultPanning();	

	module->postProcessSamples(true);

	return MP_OK;
}
