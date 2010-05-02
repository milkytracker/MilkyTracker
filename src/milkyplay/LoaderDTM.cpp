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
 *  LoaderDTM.cpp
 *  MilkyPlay Module Loader: Digitrekker 3.0 and Digital Tracker
 *
 *  Created by Peter Barth on 15.01.06.
 *
 */

#include "Loaders.h"

const char* LoaderDTM_2::identifyModule(const mp_ubyte* buffer)
{
	// check for .DTM module
	if (memcmp(buffer,"D.T.",4)) 
		return NULL;
		
	bool hasPATT = false, hasINST = false, hasSEQ = false;
	
	mp_sint32 i = 8 + BigEndian::GET_DWORD(buffer+4);
	while (i < 2040 && !(hasPATT && hasINST && hasSEQ))
	{
		mp_ubyte ID[4], lenBuf[4];
		
		memcpy(ID, buffer + i, 4);
		i+=4;
		memcpy(lenBuf, buffer + i, 4);
		i+=4;

		mp_uint32 chunkLen = BigEndian::GET_DWORD(lenBuf);
			
		switch (BigEndian::GET_DWORD(ID))
		{
			case 0x532E512E :
				hasSEQ = true;
				break;

			case 0x50415454:
				hasPATT = true;
				break;

			case 0x494E5354:
				hasINST = true;
				break;
		}
		
		i+=chunkLen;
	}
	
	if (hasPATT && hasINST && hasSEQ)
		return "DTM_2";

	return NULL;
}

mp_sint32 LoaderDTM_2::load(XMFileBase& f, XModule* module)
{
	enum PatternFormatTypes
	{
		PTStyle,
		DTStyle
	};
	
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	

	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;
	
	header->mainvol = 255;
	header->tempo = 6;
	header->speed = 125;

	header->flags = XModule::MODULE_OLDPTINSTRUMENTCHANGE | XModule::MODULE_PTNEWINSTRUMENT;

	mp_sint32 i = 0, s = 0;

	PatternFormatTypes patternFormat = DTStyle;

	while (true)
	{		
		mp_ubyte ID[4], buffer[4];
		
		mp_uint32 bytesRead = f.read(ID, 4, 1);

		if (bytesRead != 4)
			break;	

		bytesRead = f.read(buffer, 4, 1);

		if (bytesRead != 4)
			break;	

		mp_uint32 chunkLen = BigEndian::GET_DWORD(buffer);
			
		switch (BigEndian::GET_DWORD(ID))
		{
			case 0x442E542E :	// 'D.T.'
			{
				mp_sint32 pos = f.posWithBaseOffset();
				f.read(buffer, 1, 2);
				mp_uword type = BigEndian::GET_WORD(buffer);
				if (type)
				{
					return MP_LOADER_FAILED;
				}
				f.read(buffer, 1, 4);
				f.read(buffer, 1, 2);
				if (BigEndian::GET_WORD(buffer))
					header->tempo = BigEndian::GET_WORD(buffer);
				f.read(buffer, 1, 2);				
				if (BigEndian::GET_WORD(buffer))
					header->speed = BigEndian::GET_WORD(buffer);
				
				mp_uint32 len = chunkLen - 10;
				
				mp_ubyte* name = new mp_ubyte[len];
				if (name == NULL)
					return MP_OUT_OF_MEMORY;
				
				f.read(name, 1, len);
				
				mp_uint32 i = 0;
				mp_uint32 fLen = len;
				while (i < len && (!name[i] || name[i] == 32)) 
				{
					i++;
					fLen--;
				}

				memcpy(header->name, name+i, fLen < sizeof(header->name) ? fLen : sizeof(header->name));
				
				for (i = 0; i < sizeof(header->name); i++)
					if (!header->name[i])
						header->name[i] = 32;
													
				delete[] name;
				
				f.seekWithBaseOffset(pos + chunkLen);
				break;
			}		
			
			case 0x532E512E:	// 'S.Q.'
			{
				mp_uint32 pos = f.posWithBaseOffset();
				
				f.read(buffer, 1, 2);				
				header->ordnum = BigEndian::GET_WORD(buffer);				
				f.read(buffer, 1, 2);				
				header->restart = BigEndian::GET_WORD(buffer);				
				f.read(buffer, 1, 4);				

				f.read(header->ord, 1, header->ordnum);
				
				f.seekWithBaseOffset(pos + chunkLen);
				break;
			}

			case 0x50415454:	// 'PATT'
			{
				mp_uint32 pos = f.posWithBaseOffset();
				f.read(buffer, 1, 2);				
				header->channum = BigEndian::GET_WORD(buffer);				
				f.read(buffer, 1, 2);				
				header->patnum = BigEndian::GET_WORD(buffer);				
				f.read(buffer, 1, 4);	
				
				if (!BigEndian::GET_DWORD(buffer))
					patternFormat = PTStyle;
				else if (memcmp(buffer,"2.04",4) == 0)
					patternFormat = DTStyle;
				else
					return MP_LOADER_FAILED;
													
				f.seekWithBaseOffset(pos + chunkLen);
				break;
			}
			
			case 0x494E5354:	// 'INST'
			{
				mp_uint32 pos = f.posWithBaseOffset();
				f.read(buffer, 1, 2);				
				header->insnum = BigEndian::GET_WORD(buffer);				
				
				s = 0;
				for (i = 0; i < header->insnum; i++)
				{
					f.readDword(); // reserved

					f.read(buffer, 1, 4);	
					smp[s].samplen = BigEndian::GET_DWORD(buffer);
					mp_ubyte fine = f.readByte();
					
					smp[s].vol = module->vol64to255(f.readByte());				
					f.read(buffer, 1, 4);	
					smp[s].loopstart = BigEndian::GET_DWORD(buffer);					
					f.read(buffer, 1, 4);	
					smp[s].looplen = BigEndian::GET_DWORD(buffer);
					f.read(instr[i].name, 1, 22); // instrument name				

					f.read(buffer, 1, 2);	
					mp_uword type = BigEndian::GET_WORD(buffer);

					mp_ubyte bits = (mp_ubyte)type;		

					f.read(buffer, 1, 4);	// MIDI
					f.read(buffer, 1, 4);	
					mp_uint32 c4spd = BigEndian::GET_DWORD(buffer);

					mp_sint32 newC4spd = XModule::sfinetunes[fine & 0xF];
				
					newC4spd = (c4spd*newC4spd) / 8363;
				
					XModule::convertc4spd(newC4spd, &smp[s].finetune, &smp[s].relnote);

					//XModule::convertc4spd(c4spd,&smp[s].finetune,&smp[s].relnote);
					
					smp[s].type = (bits == 16 ? 16 : 0);
					smp[s].flags = 1;
					smp[s].pan = 0x80;
					
					// 16 bit sample
					if (smp[s].type & 16)
					{
						smp[s].type |= 16;
						smp[s].samplen >>= 1;
						smp[s].loopstart >>= 1;
						smp[s].looplen >>= 1;
					}

					// looping
					if (smp[s].looplen > 2)
						smp[s].type |= 1;						
					
					if (smp[s].samplen)
					{
						instr[i].samp = 1;
						for (mp_uint32 j = 0; j < 120; j++) 
							instr[i].snum[j] = s;
						s++;
					}
				}

				header->smpnum = s;
				
				f.seekWithBaseOffset(pos + chunkLen);
				break;
			}

			case 0x44415054:
			{
				mp_uint32 pos = f.posWithBaseOffset();
				f.readDword(); // reserved
				
				f.read(buffer, 1, 2);					
				i = BigEndian::GET_WORD(buffer);
				f.read(buffer, 1, 2);					
				mp_uint32 numRows = BigEndian::GET_WORD(buffer);
				
				if (i >= 0 && i < header->patnum)
				{
					mp_ubyte* patData = new mp_ubyte[chunkLen - 8];
					
					phead[i].rows = numRows;
					phead[i].effnum = 2;
					phead[i].channum = (mp_ubyte)header->channum;
		
					phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*6];
					memset(phead[i].patternData,0,phead[i].rows*header->channum*6);
					
					f.read(patData, 1, chunkLen - 8);
					
					mp_ubyte* pData = patData;
					mp_ubyte* dstSlot = phead[i].patternData;
					for (mp_uint32 r = 0; r < numRows; r++)
					{
						for (mp_sint32 c = 0; c < phead[i].channum; c++)
						{
							mp_ubyte ins = 0,eff = 0,notenum = 0, op = 0, vol = 0;

							if (patternFormat == PTStyle)
							{
								mp_sint32 note = 0;
								mp_ubyte b1 = pData[0];
								mp_ubyte b2 = pData[1];
								mp_ubyte b3 = pData[2];
								mp_ubyte b4 = pData[3];
								
								note = ((b1&0xf)<<8)+b2;
								ins = (b1&0xf0)+(b3>>4);
								eff = b3&0xf;
								
								if (note) 
									notenum = XModule::amigaPeriodToNote(note);
								
								dstSlot[0] = notenum;
								op = b4;
							}
							else if (patternFormat == DTStyle)
							{
								mp_ubyte octave = pData[0] >> 4;
								notenum = pData[0]&0xF;
								
								vol = pData[1]>>2;
								ins = ((pData[1]&3)<<4) | (pData[2]>>4);
								
								eff = pData[2]&0xF;
								op = pData[3];
								
								if (octave && notenum)
								{
									dstSlot[0] = octave*12 + notenum;
								}
							}
							else ASSERT(false);
							
							dstSlot[1] = ins;
							
							if (vol)
							{
								dstSlot[2] = 0xC;
								mp_sint32 finalVol = ((mp_sint32)(vol-1)*0x41CE8)>>16;
								if (finalVol > 255) finalVol = 255;
								dstSlot[3] = finalVol;
							}
							
							
							if (eff==0xE) 
							{
								eff=(op>>4)+0x30;
								op&=0xf;
							}
							
							if ((!eff)&&op) 
								eff=0x20;
							
							// old style modules don't support last effect for:
							// - portamento up/down
							// - volume slide
							if (eff==0x1&&(!op)) eff = 0;
							if (eff==0x2&&(!op)) eff = 0;
							if (eff==0xA&&(!op)) eff = 0;
							
							if (eff==0x5&&(!op)) eff = 0x3;
							if (eff==0x6&&(!op)) eff = 0x4;
							
							if (eff==0xC) {
								op = XModule::vol64to255(op);
							}

							dstSlot[4] = eff;
							dstSlot[5] = op;
							
							pData+=4;
							dstSlot+=6;
						}
					}
					
					delete[] patData;
				}
				
				f.seekWithBaseOffset(pos + chunkLen);
				break;
			}

			case 0x44414954:
			{
				mp_uint32 pos = f.posWithBaseOffset();

				f.read(buffer, 1, 2);					
				i = BigEndian::GET_WORD(buffer);
				
				if (instr[i].samp)
				{
					s =	instr[i].snum[0];			

					if (module->loadModuleSample(f, s, XModule::ST_DEFAULT, XModule::ST_16BIT | XModule::ST_BIGENDIAN) != 0)
						return MP_OUT_OF_MEMORY;
				}
		
				f.seekWithBaseOffset(pos + chunkLen);
				break;
			}

			default:
			{
				mp_uint32 pos = f.posWithBaseOffset();
				f.seekWithBaseOffset(pos + chunkLen);
				break;
			}
		}
			
	}

	strcpy(header->tracker,"Digital Tracker");

	module->setDefaultPanning();
	
	module->postProcessSamples();	

	return MP_OK;
}

#define CLEAN_DTM_1 \
	if (trackSeq) delete[] trackSeq; \
	if (tracks) for (mp_sint32 i = 0; i < numTracks; i++) delete[] tracks[i]; delete[] tracks; 

const char* LoaderDTM_1::identifyModule(const mp_ubyte* buffer)
{
	// check for .DTM module
	if (memcmp(buffer,"SONG",4)) 
		return NULL;

	bool hasINFO = false, hasINIT = false, hasPSEQ = false;
	
	mp_sint32 i = 8;
	while (i < 2040 && !(hasINFO && hasINIT && hasPSEQ))
	{
		mp_ubyte ID[4], lenBuf[4];
		
		memcpy(ID, buffer + i, 4);
		i+=4;
		memcpy(lenBuf, buffer + i, 4);
		i+=4;

		mp_uint32 chunkLen = LittleEndian::GET_DWORD(lenBuf);
			
		switch (BigEndian::GET_DWORD(ID))
		{
			case 0x494E464F:
				hasINFO = true;
				break;

			case 0x494E4954:
				hasINIT = true;
				break;

			case 0x50534551 :
				hasPSEQ = true;
				break;
		}
		
		i+=chunkLen;
	}
	
	if (hasINFO && hasINIT && hasPSEQ)
		return "DTM_1";

	return NULL;
}

mp_sint32 LoaderDTM_1::load(XMFileBase& f, XModule* module)
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
	
	bool hasSong = false, hasInfo = false, hasInit = false, hasInst = false, hasTrak = false, hasSamp = false;
	bool insActive = false;

	mp_sint32 numTracks = 0;

	header->mainvol = 255;
	header->tempo = 6;
	header->speed = 125;

	mp_uword* trackSeq = NULL;
	mp_ubyte** tracks = NULL;
	mp_sint32 i = 0, s = 0, t = 0;

	while (true)
	{		
		mp_ubyte ID[4], buffer[4];
		
		mp_uint32 bytesRead = f.read(ID, 4, 1);

		if (bytesRead != 4)
			break;	

		bytesRead = f.read(buffer, 4, 1);

		if (bytesRead != 4)
			break;	

		mp_uint32 chunkLen = LittleEndian::GET_DWORD(buffer);
			
		switch (BigEndian::GET_DWORD(ID))
		{
			case 0x534F4E47:	// 'SONG'
			{
				hasSong = true;
				break;
			}			

			case 0x4E414D45:	// 'NAME'
			{
				mp_uint32 pos = f.posWithBaseOffset();
				f.read(header->name, 1, chunkLen < sizeof(header->name) ? chunkLen : sizeof(header->name));
				f.seekWithBaseOffset(pos + chunkLen);				
				break;
			}

			case 0x494E464F:	// 'INFO'
			{
				hasInfo = true;
				mp_uint32 pos = f.posWithBaseOffset();
				
				// just assume everything is correct here
				header->channum = f.readWord();
				header->ordnum = f.readWord();
				header->patnum = f.readWord();
				numTracks = f.readWord();
				header->insnum = f.readWord();

				tracks = new mp_ubyte*[numTracks];

				if (tracks == NULL)
				{
					CLEAN_DTM_1
					return MP_OUT_OF_MEMORY;
				}

				memset(tracks, 0, sizeof(mp_ubyte*)*numTracks);
				
				f.seekWithBaseOffset(pos + chunkLen);				
				break;
			}			

			case 0x494E4954:	// 'INIT'
				hasInit = true;
				break;
			
			case 0x73706564:	// 'sped'
				header->tempo = f.readWord();
				header->speed = f.readWord();
				break;
			
			case 0x50534551:	// 'PSEQ'
				f.read(header->ord, 1, chunkLen);
				break;
			
			case 0x50415454:	// 'PATT'
			{
				trackSeq = new mp_uword[header->patnum * header->channum];
	
				if (trackSeq == NULL)
				{
					CLEAN_DTM_1
					return MP_OUT_OF_MEMORY;
				}

				f.readWords(trackSeq, header->patnum * header->channum);
				break;
			}
			
			case 0x494E5354:	// 'INST'
				hasInst = true;
				insActive = true;
				s = i = 0;
				break;

			case 0x53414D50 :	// 'SAMP'
				hasSamp = true;
				insActive = false;
				s = i = 0;
				break;

			case 0x73616D70:	// 'samp'
			{
				mp_uint32 pos = f.posWithBaseOffset();

				if (insActive)
				{
					f.read(instr[i].name, 1, 32); // instrument name				
					smp[s].samplen = f.readDword();
					smp[s].loopstart = f.readDword();
					mp_sint32 looplen = ((mp_sint32)f.readDword() - (mp_sint32)smp[s].loopstart);
					if (looplen < 0) 
						looplen = 0;
					smp[s].looplen = looplen;
					mp_uint32 c4spd = f.readWord();				
					XModule::convertc4spd(c4spd,&smp[s].finetune,&smp[s].relnote);
					smp[s].vol = module->vol64to255(f.readByte());				
					mp_ubyte bits = f.readByte();		
					
					smp[s].type = (bits == 16 ? 16 : 0);
					f.read(smp[s].name,1,13);	// read dos filename
					smp[s].flags = 1;
					smp[s].pan = 0x80;
					
					// looping
					if (smp[s].looplen)
						smp[s].type |= 1;	
					
					// 16 bit sample
					/*if (flags & 4)
					{
						smp[s].type |= 16;
						smp[s].samplen >>= 1;
						smp[s].loopstart >>= 1;
						smp[s].looplen >>= 1;
					}*/
					
					if (smp[s].samplen)
					{
						instr[i].samp = 1;
						for (mp_uint32 j = 0; j < 120; j++) 
							instr[i].snum[j] = s;
						s++;
					}
					i++;
				}
				else
				{
					if (instr[i].samp)
					{
						if (module->loadModuleSample(f, s) != 0)
						{
							CLEAN_DTM_1
							return MP_OUT_OF_MEMORY;
						}
						
						s++;
						header->smpnum = s;
					}
					i++;
				}
				f.seekWithBaseOffset(pos + chunkLen);				
				
				break;
			}

			case 0x5452414B:	// 'TRAK'
			{
				hasTrak = true;
				t = 0;
				break;
			}

			case 0x7472616B:	// 'trak'
			{
				if (t >= numTracks)
				{
					CLEAN_DTM_1
					return MP_LOADER_FAILED;
				}
				tracks[t] = new mp_ubyte[chunkLen];
				
				if (tracks[t] == NULL)
				{
					CLEAN_DTM_1
					return MP_OUT_OF_MEMORY;
				}
				
				f.read(tracks[t], 1, chunkLen);
				t++;
				break;
			}
			
			default:
			{
				mp_uint32 pos = f.posWithBaseOffset();
				f.seekWithBaseOffset(pos + chunkLen);
				break;
			}
		}
			
	}

	if (!hasSong || !hasInfo || !hasInit || !hasInst || !hasTrak || !hasSamp)
	{
		CLEAN_DTM_1
		return MP_LOADER_FAILED;
	}
		
	mp_ubyte lTab[256];
	mp_ubyte rTab[256];
	for (mp_sint32 pan = 0; pan < 256; pan++)
	{
		mp_sint32 left = 255-pan;
		if (left>128) left=128;
		mp_sint32	right = pan;
		if (right>128) right=128;
		lTab[pan] = left;
		rTab[pan] = right;
	}
	
	// rebuild pattern data
	mp_uword* pTrackSeq = trackSeq;

	for (i = 0; i < header->patnum;i++) 
	{
		mp_sint32 numRows = 0;
		mp_sint32 c;
		for (c=0;c<header->channum;c++)
		{
			if (pTrackSeq[c] < numTracks)
				if (LittleEndian::GET_WORD(tracks[pTrackSeq[c]]) > numRows)
					numRows = LittleEndian::GET_WORD(tracks[*pTrackSeq]);
		}
	
		phead[i].rows = numRows;
		phead[i].effnum = 2;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*6];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			CLEAN_DTM_1
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum*6);
		
		for (c=0;c<header->channum;c++) 
		{
			if (*pTrackSeq < numTracks)
			{
				
				mp_sint32 numRows = LittleEndian::GET_WORD(tracks[*pTrackSeq]);
				mp_ubyte *track = tracks[*pTrackSeq] + 2;
				
				for (mp_sint32 row = 0; row < numRows; row++)
				{
					mp_ubyte* dstSlot = phead[i].patternData+row*phead[i].channum*6+c*6;
					
					mp_ubyte note = (track[row] == 0x80 ? (mp_ubyte)XModule::NOTE_OFF : track[row]);
					mp_ubyte ins = track[row+numRows];
					
					mp_ubyte eff1 = track[row+numRows*2] >= 1 ? 0x0C : 0;
					mp_ubyte op1 = track[row+numRows*2] >= 1 ? XModule::vol64to255(track[row+numRows*2]-1) : 0;
					
					mp_ubyte hi = track[row+numRows*4];
					mp_ubyte lo = track[row+numRows*5];
					//printf("%x, %x, %x\n", track[row+numRows*3], track[row+numRows*4], track[row+numRows*5]);
					
					mp_ubyte eff2 = 0;
					mp_ubyte op2 = 0;
					
					switch (track[row+numRows*3]-1)
					{
						case 0x07:	
							eff2 = 0x07;
							op2 = ((hi > 0x0F ? 0x0F : hi) << 4) + (lo > 0x0F ? 0x0F : lo);
							break;
						
						case 0x09:
							eff2 = 0x09;
							op2 = (((((mp_uint32)hi) << 8) + (mp_uint32)lo)<<4)>>8;
							break;
									
						case 0x0A:	
							// volume slide up
							if (hi != 0 && hi != 0xFF)
							{
								eff2 = 0x0A;
								op2 = ((hi > 0x0F) ? 0xF : hi) << 4;
							}
							else if (lo != 0 && lo != 0xFF)
							{
								eff2 = 0x0A;
								op2 = ((lo > 0x0F) ? 0xF : lo);
							}
							break;
							
						case 0x10: // set tempo
							eff2 = 0x16;
							op2 = lo;
							break;
							
						case 0x11: // set panning
						{
							if (hi > 64) hi = 64;
							if (lo > 64) lo = 64;
							mp_sint32 dist = 0x7FFFFFFF;
							mp_sint32 x = hi<<1;
							mp_sint32 y = lo<<1;
							mp_sint32 index = -1;
							for (mp_sint32 j = 0; j < 256; j++)
							{
								if (((lTab[j]-y)*(lTab[j]-y) + (rTab[j]-x)*(rTab[j]-x)) < dist)
								{
									dist = ((lTab[j]-y)*(lTab[j]-y) + (rTab[j]-x)*(rTab[j]-x));
									index = j;
								}
							}
							if (index != -1)
							{
								eff2 = 0x08;
								op2 = index;
							}
							break;
						}
							
					}
					
					dstSlot[0] = note;
					dstSlot[1] = ins;
					dstSlot[2] = eff1;
					dstSlot[3] = op1;				
					dstSlot[4] = eff2;
					dstSlot[5] = op2;				
				}
			}
			
			pTrackSeq++;
		
		}
		
	}
	
	CLEAN_DTM_1
	
	strcpy(header->tracker,"Digitrekker");

	module->setDefaultPanning();
	
	module->postProcessSamples();

	return MP_OK;
}
