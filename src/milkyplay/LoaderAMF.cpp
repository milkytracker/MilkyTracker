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
 *  LoaderAMF.cpp
 *  MilkyPlay Module Loaders: Asylum Music Format 1.0 and DSMI AMF
 *
 *  Thanks to Grom PE for the additional DMF loader which is a DSMI AMF variant
 *  used in the Tronic, game by Webfoot Technologies inc.
 *
 *  Warning: This is an one-by-one conversion of an assembler version ;)
 *
 */

#include "Loaders.h"

const char* LoaderAMF_1::identifyModule(const mp_ubyte* buffer)
{
	// check for .AMF module
	if (!memcmp(buffer,"ASYLUM Music Format",19)) 
	{
		return "AMF_1";
	}

	return NULL;
}

mp_sint32 LoaderAMF_1::load(XMFileBase& f, XModule* module)
{
	mp_ubyte buffer[2048];
	
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	
	
	// we're already out of memory here
	if (!phead || !instr || !smp)
		return -7;
	
	f.read(buffer,1,294);
	
	memcpy(header->sig,buffer,6);
	
	header->ordnum = buffer[36]; // song length
	header->insnum = buffer[34]; // number of instruments
	header->patnum = buffer[35]; // number of patterns
	header->speed = buffer[33];	// default tickspeed
	header->tempo = buffer[32];	// default tempo
	header->mainvol = 255;
	header->channum = 8;
	
	header->flags = XModule::MODULE_PTNEWINSTRUMENT;

	memcpy(header->ord,buffer+38,256); // order list
	
	mp_sint32 i, s = 0;
	for (i = 0; i < header->insnum; i++) 
	{
		f.read(smp[s].name,1,22);
		memcpy(instr[i].name,smp[s].name,22);
		
		mp_ubyte finetune = f.readByte();
		mp_ubyte volume = module->vol64to255(f.readByte());
		
		f.readByte(); // skip something (maybe volume is a 16bit word?)
		
		mp_uint32 size = f.readDword();
		
		mp_uint32 loopstart = f.readDword();
		
		mp_uint32 loopend = f.readDword();
		
		module->convertc4spd(module->sfinetunes[finetune],&smp[s].finetune,&smp[s].relnote);
		
		smp[s].flags = 1;
		smp[s].samplen = size;
		smp[s].loopstart = loopstart;
		smp[s].looplen = loopend;
		smp[s].vol = volume;
		
		if (smp[s].samplen <= 2) 
		{
			instr[i].samp = 0;
		}
		else
		{
			instr[i].samp = 1;
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
	
	f.read(buffer,1,2663-1442);	
	
	for (i = 0; i < header->patnum;i++) 
	{
		f.read(buffer,1,2048);
		
		phead[i].rows=64;
		phead[i].effnum=1;
		phead[i].channum = (mp_ubyte)header->channum;
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum*4];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			return -7;
		}
		
		memset(phead[i].patternData,0,phead[i].rows*header->channum*4);
		
		mp_sint32 r,c,cnt=0;
		for (r=0;r<64;r++) {
			for (c=0;c<header->channum;c++) {
				mp_ubyte note = buffer[cnt];
				mp_ubyte ins = buffer[cnt+1];
				mp_ubyte eff = buffer[cnt+2];
				mp_ubyte op = buffer[cnt+3];
				
				if (note) 
					note++;
				
				if (eff==0xE) 
				{
					eff=(op>>4)+0x30;
					op&=0xf;
					
					if (eff == 0x38)
					{
						eff = 0x08;
						op<<=4;
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
				
				if (eff==0xC) {
					op = XModule::vol64to255(op);
				}
				
				phead[i].patternData[cnt]=note;
				phead[i].patternData[cnt+1]=ins;
				phead[i].patternData[cnt+2]=eff;
				phead[i].patternData[cnt+3]=op;
				
				cnt+=4;
			}
		}
		
	}
	
	mp_sint32 result = module->loadModuleSamples(f);
	if (result != MP_OK)
		return result;
	
	strcpy(header->tracker,"..converted..");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();
	
	return MP_OK;
}

#if 0

const char* LoaderAMF_2::identifyModule(const mp_ubyte* buffer)
{
	// check for .AMF module
	if (!memcmp(buffer,"AMF", 3)) 
	{
		// check for version
		if (buffer[3] < 0xA ||
			buffer[3] > 0xE)
			return NULL;
	
		return "AMF_2";
	}

	return NULL;
}

// shamelessly copied from MikMod
struct AMFNoteSlot
{
	mp_ubyte note,instr,fxcnt;
	mp_ubyte effect[3];
	mp_sbyte parameter[3];
};

#define CLEAN_UP \
	for (j = 0; j < maxTracks; j++) \
		delete[] tracks[j]; \
	delete[] tracks; \
	delete[] trackTable; \
	delete[] rows; \
	delete[] amfOrders;

mp_sint32 LoaderAMF_2::load(XMFileBase& f, XModule* module)
{
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	
	
	// we're already out of memory here
	if (!phead || !instr || !smp)
		return -7;
	
	// ---- read header info ----
	
	f.read(header->sig, 1, 3);
	
	mp_ubyte ver = f.readByte();
	
	f.read(header->name, 1, 32);
	
	header->insnum = f.readByte();
	header->ordnum = header->patnum = f.readByte();
	
	mp_uword numTracks = f.readWord();
	
	header->channum = f.readByte();
	
	mp_sint32 channelRemap[16];
	
	if (ver >= 11)
	{
		mp_ubyte panpos[32];
		f.read(panpos, 1, (ver >= 13) ? 32 : 16);
	}
	else
	{
		for (mp_sint32 i = 0; i < 16; i++)
			channelRemap[i] = f.readByte();
	}
	
	if (ver >= 13)
	{
		// read BPM
		header->speed = f.readByte();
		// read speed
		header->tempo = f.readByte();	
	}
	else
	{
		header->speed = 125;
		header->tempo = 6;	
	}
	
	header->mainvol = 255;
	
	mp_sint32 i,j,k;
	
	for (i = 0; i < header->ordnum; i++)
		header->ord[i] = i;

	mp_uword* amfOrders = new mp_uword[header->channum*header->ordnum];
	mp_uword* rows = new mp_uword[header->ordnum];

	for (i = 0; i < header->ordnum; i++) {
		rows[i] = 64;
		if (ver >= 14)
			rows[i] = f.readWord();
		if (ver > 10)
			f.readWords(amfOrders+(i*header->channum), header->channum);
		else
		{
			for(j = 0; j < header->channum; j++)
				amfOrders[i*header->channum+channelRemap[j]]=f.readWord();
		}
	}
	
	// ---- read sample info ----
	
	j = 0;
	for (i = 0; i < header->insnum; i++)
	{
		mp_ubyte type = f.readByte();
		f.read(instr[i].name, 1, 32);
		mp_ubyte dosname[13];
		f.read(dosname, 1, 13);
		mp_sint32 offset = f.readDword();
		mp_sint32 length = f.readDword();
		mp_dword c2spd = f.readWord();
		mp_ubyte vol = f.readByte();
		
		mp_sint32 loopstart;
		mp_sint32 loopend;
		if (ver >= 10)
		{
			loopstart = f.readDword();
			loopend = f.readDword();
		}
		else
		{
			loopstart = f.readWord();
			loopend = length;
		}
		
		if (type)
		{
			memcpy(smp[j].name, dosname, 13);
		
			module->convertc4spd(c2spd, &smp[j].finetune, &smp[j].relnote);
		
			if (loopend - loopstart > 2)
				smp[j].type = 1;
					
			smp[j].flags = 1;
			smp[j].samplen = length;
			smp[j].loopstart = loopstart;
			smp[j].looplen = loopend;
			smp[j].vol = XModule::vol64to255(vol);
			
			instr[i].samp = 1;
			for (mp_sint32 k = 0; k < 120; k++) 
				instr[i].snum[k] = j;
				
			j++;
		}
	}
	
	header->smpnum = j;
	
	mp_uword* trackTable = new mp_uword[numTracks];
	
	mp_uword maxTracks = 0;
	
	f.readWords(trackTable, numTracks);

	for (i = 0; i < numTracks; i++)
		if (trackTable[i] > maxTracks)
			maxTracks = trackTable[i];
	
	// ---- read tracks ---- 
	AMFNoteSlot** tracks = new AMFNoteSlot*[maxTracks];
	memset(tracks, 0, sizeof(AMFNoteSlot*)*maxTracks);
	
	for (i = 0; i < maxTracks; i++)
	{
		tracks[i] = new AMFNoteSlot[64];
		memset(tracks[i], 0, sizeof(AMFNoteSlot)*64);
		AMFNoteSlot* track = tracks[i];
		
		// shamelessly copied from MikMod
		mp_sint32 tracksize;
		mp_ubyte row,cmd;
		mp_sbyte arg;
		
		tracksize = f.readWord();
		tracksize+=((mp_sint32)f.readByte()) << 16;

		if (tracksize)
			while(tracksize--) 
			{
				row = f.readByte();
				cmd = f.readByte();
				arg = f.readByte();
				// unexpected end of track 
				if (!tracksize) 
				{
					if((row==0xff) && (cmd==0xff) && (arg==-1))
						break;
					/* the last triplet should be FF FF FF, but this is not
					   always the case... maybe a bug in m2amf ? 
					else
						return 0;
					*/
				}
				// invalid row (probably unexpected end of row) 
				if (row>=64)
				{
					CLEAN_UP
					return -8;
				}
				if (cmd<0x7f) 
				{
					// note, vol 
					track[row].note=cmd+1;
					
					if (track[row].fxcnt<3) 
					{
						track[row].effect[track[row].fxcnt] = 0x03;
						track[row].parameter[track[row].fxcnt] = (mp_ubyte)arg;
						track[row].fxcnt++;
					}
				} 
				else if (cmd==0x7f) 
				{
					// duplicate row
					if ((arg<0)&&(row+arg>=0)) {
						memcpy(track+row,track+(row+arg),sizeof(AMFNoteSlot));
					}
				} 
				else if (cmd==0x80) 
				{
					// instr 
					track[row].instr=arg+1;
				} 
				else if (cmd==0xff) 
				{
					// apparently, some M2AMF version fail to estimate the
					// size of the compressed patterns correctly, and end
					// up with blanks, i.e. dead triplets. Those are marked
					// with cmd == 0xff. Let's ignore them. 
				} 
				else if (track[row].fxcnt<3) 
				{
					// effect, param 
					if (cmd > 0x97)
					{
						CLEAN_UP
						return -8;
					}
					track[row].effect[track[row].fxcnt]=cmd&0x7f;
					track[row].parameter[track[row].fxcnt]=arg;
					track[row].fxcnt++;
				} 
				else
				{
					CLEAN_UP
					return -8;
				}
			}
	}
	
	// ---- convert tracks to patterns ---- 
	for (i = 0; i < header->ordnum; i++)
	{
		phead[i].rows = rows[i];
		phead[i].channum = (mp_ubyte)header->channum;
		phead[i].effnum = 3;
		
		mp_sint32 slotSize = (mp_sint32)(2+phead[i].effnum*2);
		mp_sint32 patSize = (mp_sint32)phead[i].rows*(mp_sint32)header->channum*slotSize;
		phead[i].patternData = new mp_ubyte[patSize];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			CLEAN_UP
			return -7;
		}
		
		memset(phead[i].patternData, 0, patSize);
		
		for (mp_sint32 row = 0; row < phead[i].rows; row++)
			for (mp_sint32 chn = 0; chn < phead[i].channum; chn++)
			{
				j = amfOrders[i*phead[i].channum+chn];
				if (j && j <= numTracks)
				{
					j = trackTable[j-1];
					if (j && j <= maxTracks)
					{
						AMFNoteSlot* track = tracks[j-1];
					
						mp_ubyte* dstSlot = phead[i].patternData+row*phead[i].channum*slotSize+chn*slotSize;
						
						// convert note slot
						if (track[row].note)
							dstSlot[0] = track[row].note - 12;
						dstSlot[1] = track[row].instr;
						
						mp_sint32 l = 0;
						for (k = 0; k < track[row].fxcnt; k++)
						{
							mp_ubyte nEff = 0;
							mp_ubyte nOp = 0;
							mp_sbyte op = track[row].parameter[k];

							switch (track[row].effect[k])
							{
								// Set speed (ticks)
								case 0x01:
									nEff = 0x1C;
									nOp = op;
									break;
					
								// Volume slide 
								case 2: 									
									nEff = 0x0A;
									if (op) 
									{
										if (op>=0)
											nOp = ((op&0xf)<<4);
										else
											nOp = (-op)&0xf;
									}
									break;
								
								// set volume
								case 0x03:
									nEff = 0x0C;
									nOp = XModule::vol64to255(op);
									break;

								// portamento up/down
								case 0x04:
									if (op >= 0)
									{
										nEff = 0x02;
										nOp = op & 0xf;
									}
									else
									{
										nEff = 0x01;
										nOp = (-op) & 0xf;
									}
									break;
								
								// Porta to note 
								case 0x06: 
									nEff = 0x03;
									nOp = op;
									break;
									
								// tremor
								case 0x07: 
									nEff = 0x1D;
									nOp = op;
									break;

								// arpeggio
								case 0x08: 
									nEff = 0x20;
									nOp = op;
									break;
									
								// vibrato
								case 0x09: 
									nEff = 0x04;
									nOp = op;
									break;

								// Porta + Volume slide 
								case 0xA: 									
									nEff = 0x05;
									if (op) 
									{
										if (op>=0)
											nOp = ((op&0xf)<<4);
										else
											nOp = (-op) & 0xf;
									}
									break;

								// Vibrato + Volume slide 
								case 0xB: 									
									nEff = 0x06;
									if (op) 
									{
										if (op>=0)
											nOp = ((op&0xf)<<4);
										else
											nOp = (-op) & 0xf;
									}
									break;

								// Pattern break (in hex) 
								case 0xC: 
									nEff = 0x0D;
									nOp = (op/10)*16 + (op%10);
									break;

								// Pos. jump
								case 0xD: 
									nEff = 0x0B;
									nOp = op;
									break;
									
								// Retrig
								case 0xF: 
									nEff = 0x1B;
									nOp = op;
									break;
											
								// Sample offset 
								case 0x10: 
									nEff = 0x09;
									nOp = op;
									break;
									
								// Fine Volume slide 
								case 0x11: 									
									if (op) 
									{
										if (op>=0)
										{
											nEff = 0x3A;
											nOp = op & 0x0f;
										}
										else
										{
											nEff = 0x3B;
											nOp = (-op) & 0x0f;
										}
									}
									break;
									
								// Fine Porta
								case 0x12: 									
									if (op) 
									{
										if (op>=0)
										{
											nEff = 0x32;
											nOp = op & 0x0f;
										}
										else
										{
											nEff = 0x31;
											nOp = (-op) & 0x0f;
										}
									}
									break;

								// Note delay
								case 0x13:
									nEff = 0x3D;
									nOp = op;
									break;

								// Note cut
								case 0x14:
									nEff = 0x3C;
									nOp = op;
									break;

								// Set tempo (bpm)
								case 0x15:
									nEff = 0x16;
									nOp = op;
									break;

								// Set panning
								case 0x17:
									if (op > 64)
									{
										nEff = 0x08;
										nOp = XModule::vol64to255(op);
									}
									break;
							}

							// put volume in first command
							// otherwise in second
							if (l == 0 && nEff != 0x0C)
							{
								l = 1;
							}
							// if not first effect search for empty effect slot	
							else
							{
								for (mp_sint32 m = 0; m < phead[i].effnum; m++)
									if (!dstSlot[2+m*2])
									{
										l = m;
										break;
									}
							}
							
							if (nEff)
							{
								dstSlot[2+l*2] = nEff;
								dstSlot[2+l*2+1] = nOp;
							}
						}
					}
				}
			}
	}
	
	CLEAN_UP

	mp_sint32 result = module->loadModuleSamples(f, XModule::ST_UNSIGNED);
	if (result != MP_OK)
		return result;

	// --- kick out duplicate patterns ---
	mp_ubyte patReloc[255];
	memset(patReloc, 0xFF, sizeof(patReloc));
	
	for (i = 0; i < header->patnum; i++)
	{
		if (patReloc[i] == 0xFF)
		{
			for (j = 0; j < header->patnum; j++)
			{
				if ((j != i && patReloc[j] == 0xFF) &&
					phead[i].rows == phead[j].rows && 
					phead[i].channum == phead[j].channum && 
					phead[i].effnum == phead[j].effnum)
				{
					mp_sint32 slotSize = (mp_sint32)(2+phead[i].effnum*2);
					mp_sint32 patSize = (mp_sint32)phead[i].rows*(mp_sint32)header->channum*slotSize;
					
					if (memcmp(phead[i].patternData, phead[j].patternData, patSize) == 0)
					{
						patReloc[j] = i;
					}
				}
			}
		}
	}
	
	for (i = 0; i < header->ordnum; i++)
	{
		if (patReloc[header->ord[i]] != 255)
			header->ord[i] = patReloc[header->ord[i]];
	}
	
	module->removeUnusedPatterns(false);
	
	strcpy(header->tracker,"..converted..");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();

	return MP_OK;
}

// ---------------

#else

const char* LoaderAMF_2::identifyModule(const mp_ubyte* buffer)
{
	// check for .AMF module
	if (!memcmp(buffer,"DMF", 3)) 
	{
		// check for version
		if (buffer[3] < 0xA ||
			buffer[3] > 0xE)
			return NULL;
 
		return "AMF_2";
	}
 
	return NULL;
}
 
// shamelessly copied from MikMod
struct AMFNoteSlot
{
	mp_ubyte note,instr,fxcnt;
	mp_ubyte effect[3];
	mp_sbyte parameter[3];
};
 
#define CLEAN_UP \
	for (j = 0; j < maxTracks; j++) \
		delete[] tracks[j]; \
	delete[] tracks; \
	delete[] trackTable; \
	delete[] rows; \
	delete[] amfOrders;
 
mp_sint32 LoaderAMF_2::load(XMFileBase& f, XModule* module)
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
 
	// ---- read header info ----
 
	f.read(header->sig, 1, 3);
 
	mp_ubyte ver = f.readByte();
 
//	f.read(header->name, 1, 32);
 
	header->insnum = f.readByte();
	header->ordnum = header->patnum = f.readByte();
 
	mp_uword numTracks = f.readWord();
 
	header->channum = f.readByte();
 
	mp_sint32 channelRemap[16];
 
	if (ver >= 11)
	{
		mp_ubyte panpos[32];
		f.read(panpos, 1, (ver >= 13) ? 32 : 16);
	}
	else
	{
		for (mp_sint32 i = 0; i < 16; i++)
			channelRemap[i] = f.readByte();
	}
 
	if (ver >= 13)
	{
		// read BPM
		header->speed = f.readByte();
		// read speed
		header->tempo = f.readByte();	
	}
	else
	{
		header->speed = 125;
		header->tempo = 6;	
	}
 
	header->mainvol = 255;
 
	mp_sint32 i,j,k;
 
	for (i = 0; i < header->ordnum; i++)
		header->ord[i] = i;
 
	mp_uword* amfOrders = new mp_uword[header->channum*header->ordnum];
	mp_uword* rows = new mp_uword[header->ordnum];
 
	for (i = 0; i < header->ordnum; i++) {
		rows[i] = 64;
		if (ver >= 14)
			rows[i] = f.readWord();
		if (ver > 10)
			f.readWords(amfOrders+(i*header->channum), header->channum);
		else
		{
			for(j = 0; j < header->channum; j++)
				amfOrders[i*header->channum+channelRemap[j]]=f.readWord();
		}
	}
 
	// ---- read sample info ----
 
	j = 0;
 
	for (i = 0; i < header->insnum; i++)
	{
		mp_ubyte type = f.readByte();
//		f.read(instr[i].name, 1, 32);
//		mp_ubyte dosname[13];
//		f.read(dosname, 1, 13);
//		f.readByte(); // FIXME: where's missing byte?

		mp_sint32 offset = f.readDword() >> 8;
		mp_sint32 length = f.readDword() >> 8;
		mp_ubyte vol = f.readByte();
		mp_dword c2spd = f.readWord();

//		f.seek(-1, f.SeekOffsetTypeCurrent); // Restore missing byte
 
		mp_sint32 loopstart;
		mp_sint32 loopend;
		if (ver >= 10)
		{
			loopstart = f.readDword() >> 8;
			loopend = f.readDword() >> 8;
		}
		else
		{
			loopstart = f.readWord();
			loopend = length;
		}
 
		if (type)
		{
//			memcpy(smp[j].name, dosname, 13);
 
			module->convertc4spd(c2spd, &smp[j].finetune, &smp[j].relnote);
 
			if (loopend - loopstart > 2)
				smp[j].type = 1;
 
			smp[j].flags = 1;
			smp[j].samplen = length;
			smp[j].loopstart = loopstart;
			smp[j].looplen = loopend;
			smp[j].vol = XModule::vol64to255(vol);
 
			instr[i].samp = 1;
			for (mp_sint32 k = 0; k < 120; k++) 
				instr[i].snum[k] = j;
 
			j++;
		}
	}
 
	header->smpnum = j;
 
	mp_uword* trackTable = new mp_uword[numTracks];
 
	mp_uword maxTracks = 0;
 
	f.readWords(trackTable, numTracks);
 
	for (i = 0; i < numTracks; i++)
		if (trackTable[i] > maxTracks)
			maxTracks = trackTable[i];
 
	// ---- read tracks ---- 
	AMFNoteSlot** tracks = new AMFNoteSlot*[maxTracks];
	memset(tracks, 0, sizeof(AMFNoteSlot*)*maxTracks);
 
	for (i = 0; i < maxTracks; i++)
	{
		tracks[i] = new AMFNoteSlot[64];
		memset(tracks[i], 0, sizeof(AMFNoteSlot)*64);
		AMFNoteSlot* track = tracks[i];
 
		// shamelessly copied from MikMod
		mp_sint32 tracksize;
		mp_ubyte row,cmd;
		mp_sbyte arg;
 
		tracksize = f.readWord();
		tracksize+=((mp_sint32)f.readByte()) << 16;
 
		if (tracksize)
			while(tracksize--) 
			{
				row = f.readByte();
				cmd = f.readByte();
				arg = f.readByte();
				// unexpected end of track 
				if (!tracksize) 
				{
					if((row==0xff) && (cmd==0xff) && (arg==-1))
						break;
					/* the last triplet should be FF FF FF, but this is not
					   always the case... maybe a bug in m2amf ? 
					else
						return 0;
					*/
				}
				// invalid row (probably unexpected end of row) 
				if (row>=64)
				{
					CLEAN_UP
					return MP_LOADER_FAILED;
				}
				if (cmd<0x7f) 
				{
					// note, vol 
					track[row].note=cmd+1;
 
					if (track[row].fxcnt<3) 
					{
						track[row].effect[track[row].fxcnt] = 0x03;
						track[row].parameter[track[row].fxcnt] = (mp_ubyte)arg;
						track[row].fxcnt++;
					}
				} 
				else if (cmd==0x7f) 
				{
					// duplicate row
					if ((arg<0)&&(row+arg>=0)) {
						memcpy(track+row,track+(row+arg),sizeof(AMFNoteSlot));
					}
				} 
				else if (cmd==0x80) 
				{
					// instr 
					track[row].instr=arg+1;
				} 
				else if (cmd==0xff) 
				{
					// apparently, some M2AMF version fail to estimate the
					// size of the compressed patterns correctly, and end
					// up with blanks, i.e. dead triplets. Those are marked
					// with cmd == 0xff. Let's ignore them. 
				} 
				else if (track[row].fxcnt<3) 
				{
					// effect, param 
					if (cmd > 0x97)
					{
						CLEAN_UP
						return MP_LOADER_FAILED;
					}
					track[row].effect[track[row].fxcnt]=cmd&0x7f;
					track[row].parameter[track[row].fxcnt]=arg;
					track[row].fxcnt++;
				} 
				else
				{
					CLEAN_UP
					return MP_LOADER_FAILED;
				}
			}
	}
 
	// ---- convert tracks to patterns ---- 
	for (i = 0; i < header->ordnum; i++)
	{
		phead[i].rows = rows[i];
		phead[i].channum = (mp_ubyte)header->channum;
		phead[i].effnum = 3;
 
		mp_sint32 slotSize = (mp_sint32)(2+phead[i].effnum*2);
		mp_sint32 patSize = (mp_sint32)phead[i].rows*(mp_sint32)header->channum*slotSize;
		phead[i].patternData = new mp_ubyte[patSize];
 
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			CLEAN_UP
			return MP_OUT_OF_MEMORY;
		}
 
		memset(phead[i].patternData, 0, patSize);
 
		for (mp_sint32 row = 0; row < phead[i].rows; row++)
			for (mp_sint32 chn = 0; chn < phead[i].channum; chn++)
			{
				j = amfOrders[i*phead[i].channum+chn];
				if (j && j <= numTracks)
				{
					j = trackTable[j-1];
					if (j && j <= maxTracks)
					{
						AMFNoteSlot* track = tracks[j-1];
 
						mp_ubyte* dstSlot = phead[i].patternData+row*phead[i].channum*slotSize+chn*slotSize;
 
						// convert note slot
						if (track[row].note)
							dstSlot[0] = track[row].note - 12;
						dstSlot[1] = track[row].instr;
 
						mp_sint32 l = 0;
						for (k = 0; k < track[row].fxcnt; k++)
						{
							mp_ubyte nEff = 0;
							mp_ubyte nOp = 0;
							mp_sbyte op = track[row].parameter[k];
 
							switch (track[row].effect[k])
							{
								// Set speed (ticks)
								case 0x01:
									nEff = 0x1C;
									nOp = op;
									break;
 
								// Volume slide 
								case 2: 									
									nEff = 0x0A;
									if (op) 
									{
										if (op>=0)
											nOp = ((op&0xf)<<4);
										else
											nOp = (-op)&0xf;
									}
									break;
 
								// set volume
								case 0x03:
									nEff = 0x0C;
									nOp = XModule::vol64to255(op);
									break;
 
								// portamento up/down
								case 0x04:
									if (op >= 0)
									{
										nEff = 0x02;
										nOp = op & 0xf;
									}
									else
									{
										nEff = 0x01;
										nOp = (-op) & 0xf;
									}
									break;
 
								// Porta to note 
								case 0x06: 
									nEff = 0x03;
									nOp = op;
									break;
 
								// tremor
								case 0x07: 
									nEff = 0x1D;
									nOp = op;
									break;
 
								// arpeggio
								case 0x08: 
									nEff = 0x20;
									nOp = op;
									break;
 
								// vibrato
								case 0x09: 
									nEff = 0x04;
									nOp = op;
									break;
 
								// Porta + Volume slide 
								case 0xA: 									
									nEff = 0x05;
									if (op) 
									{
										if (op>=0)
											nOp = ((op&0xf)<<4);
										else
											nOp = (-op) & 0xf;
									}
									break;
 
								// Vibrato + Volume slide 
								case 0xB: 									
									nEff = 0x06;
									if (op) 
									{
										if (op>=0)
											nOp = ((op&0xf)<<4);
										else
											nOp = (-op) & 0xf;
									}
									break;
 
								// Pattern break (in hex) 
								case 0xC: 
									nEff = 0x0D;
									nOp = (op/10)*16 + (op%10);
									break;
 
								// Pos. jump
								case 0xD: 
									nEff = 0x0B;
									nOp = op;
									break;
 
								// Retrig
								case 0xF: 
									nEff = 0x1B;
									nOp = op;
									break;
 
								// Sample offset 
								case 0x10: 
									nEff = 0x09;
									nOp = op;
									break;
 
								// Fine Volume slide 
								case 0x11: 									
									if (op) 
									{
										if (op>=0)
										{
											nEff = 0x3A;
											nOp = op & 0x0f;
										}
										else
										{
											nEff = 0x3B;
											nOp = (-op) & 0x0f;
										}
									}
									break;
 
								// Fine Porta
								case 0x12: 									
									if (op) 
									{
										if (op>=0)
										{
											nEff = 0x32;
											nOp = op & 0x0f;
										}
										else
										{
											nEff = 0x31;
											nOp = (-op) & 0x0f;
										}
									}
									break;
 
								// Note delay
								case 0x13:
									nEff = 0x3D;
									nOp = op;
									break;
 
								// Note cut
								case 0x14:
									nEff = 0x3C;
									nOp = op;
									break;
 
								// Set tempo (bpm)
								case 0x15:
									nEff = 0x16;
									nOp = op;
									break;
 
								// Set panning
								case 0x17:
									if (op > 64)
									{
										nEff = 0x08;
										nOp = XModule::vol64to255(op);
									}
									break;
							}
 
							// put volume in first command
							// otherwise in second
							if (l == 0 && nEff != 0x0C)
							{
								l = 1;
							}
							// if not first effect search for empty effect slot	
							else
							{
								for (mp_sint32 m = 0; m < phead[i].effnum; m++)
									if (!dstSlot[2+m*2])
									{
										l = m;
										break;
									}
							}
 
							if (nEff)
							{
								dstSlot[2+l*2] = nEff;
								dstSlot[2+l*2+1] = nOp;
							}
						}
					}
				}
			}
	}
 
	CLEAN_UP
 
	mp_sint32 result = module->loadModuleSamples(f, XModule::ST_DEFAULT);
	if (result != MP_OK)
		return result;
 
	// --- kick out duplicate patterns ---
	mp_ubyte patReloc[255];
	memset(patReloc, 0xFF, sizeof(patReloc));
 
	for (i = 0; i < header->patnum; i++)
	{
		if (patReloc[i] == 0xFF)
		{
			for (j = 0; j < header->patnum; j++)
			{
				if ((j != i && patReloc[j] == 0xFF) &&
					phead[i].rows == phead[j].rows && 
					phead[i].channum == phead[j].channum && 
					phead[i].effnum == phead[j].effnum)
				{
					mp_sint32 slotSize = (mp_sint32)(2+phead[i].effnum*2);
					mp_sint32 patSize = (mp_sint32)phead[i].rows*(mp_sint32)header->channum*slotSize;
 
					if (memcmp(phead[i].patternData, phead[j].patternData, patSize) == 0)
					{
						patReloc[j] = i;
					}
				}
			}
		}
	}
 
	for (i = 0; i < header->ordnum; i++)
	{
		if (patReloc[header->ord[i]] != 255)
			header->ord[i] = patReloc[header->ord[i]];
	}
 
	module->removeUnusedPatterns(false);
 
	strcpy(header->tracker,"..converted..");
 
	module->setDefaultPanning();
 
	module->postProcessSamples();
 
	return MP_OK;
}

#endif
