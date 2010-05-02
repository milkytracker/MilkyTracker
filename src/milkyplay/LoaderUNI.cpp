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
 *  LoaderUNI.cpp
 *  MilkyPlay Module Loader: MikMod UNIversal Module
 *
 *  Warning: This is an one-by-one conversion of my pascal uni2xm converter ;)
 *
 */
#include "Loaders.h"

//#define VERBOSE

struct Uniheader
{
	mp_ubyte FileID[4];
	mp_ubyte NumChannels;
	mp_uword SongLength;
	mp_uword RepeatPos;
	mp_uword NumPatterns;
	mp_uword NumTracks;
	mp_uword NumIns;
	mp_ubyte InitSpeed;
	mp_ubyte InitTempo;
	mp_ubyte OrderList[256];
	mp_ubyte DefPanning[32];
	mp_ubyte Flags;
};

struct Uniinstrument
{
	   mp_ubyte NumSamps;
	   mp_ubyte SampleNums[96];
	   mp_ubyte volflg;
	   mp_ubyte volpts;
	   mp_ubyte volsus;
	   mp_ubyte volbeg;
	   mp_ubyte volend;
	   mp_sword VolPoints[12*2];
	   mp_ubyte panflg;
	   mp_ubyte panpts;
	   mp_ubyte pansus;
	   mp_ubyte panbeg;
	   mp_ubyte panend;
	   mp_sword PanPoints[12*2];
	   mp_ubyte vibtype;
	   mp_ubyte vibsweep;
	   mp_ubyte vibdepth;
	   mp_ubyte vibrate;
	   mp_uword volfade;
};

struct Unisample
{
	   mp_uword C2spd;
	   mp_sbyte relnotenum;
	   mp_ubyte volume;
	   mp_ubyte panning;
	   mp_uint32 smplength;
	   mp_uint32 smploopstart;
	   mp_uint32 smploopend;
	   mp_uword flags;
 };
 
 struct MytrackSlot
 {
	mp_ubyte note, instrument;
	mp_ubyte effects[7];
	mp_ubyte operands[7];
 };
 
const char* LoaderUNI::identifyModule(const mp_ubyte* buffer)
{
	// check for .UNI module
	if (!memcmp(buffer,"UN0",3)) 
	{
		if (buffer[3] != '4' &&
			buffer[3] != '5')
			return NULL;
		
		return "UNI";
	}

	return NULL;
}

static mp_sint32 getMaxEffectsFromTrack(mp_ubyte* track)
{
	mp_sint32 y;
	mp_sint32 i = 0;
	mp_sint32 rowcnt = 0;

	mp_sint32 effectCnt = 1;
	mp_sint32 maxEffectsPerRow = 0;

	do
	{

		mp_ubyte x = track[i];
		mp_ubyte rep = x >> 5;
		mp_ubyte len = x & 31;

		for (mp_ubyte repcnt = 0; repcnt <= rep; repcnt++)
		{
			y = 1;
			if (len > 1) 
			{
				for (mp_ubyte c = 1; c <= len-1; c++) 
				{
					if (track[y+i])
					{
						switch (track[y+i])
						{
							case 1: 
								//DeTrack[rowcnt].Note:=Track[y+i+1]+1;
								y+=2;
								c++;
								break;
							case 2:
								//DeTrack[rowcnt].Instrument:=Track[y+i+1]+1;
								y+=2;
								c++;
								break;
							case 13:
								//if (track[y+i+1]) {
									//DeTrack[rowcnt].Effect:=$A;
									//DeTrack[rowcnt].Operand:=Track[y+i+1];
								//}
								//else {
									//DeTrack[rowcnt].Effect:=0;
									//DeTrack[rowcnt].Operand:=0;
								//}
								y+=2;
								c++;
								effectCnt++;
								break;
							case 15:
								//DeTrack[rowcnt].Volume:=Track[y+i+1]+$10
								y+=2;
								c++;
								break;
							case 19:
								//DeTrack[rowcnt].Effect:=$F;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 20:
								//DeTrack[rowcnt].Effect:=250;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 21:
								//DeTrack[rowcnt].Effect:=251;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 22:
								//DeTrack[rowcnt].Effect:=252;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 23:
								//DeTrack[rowcnt].Effect:=29;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 24:
								//DeTrack[rowcnt].Effect:=27;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 25:
								//DeTrack[rowcnt].Effect:=$F;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 26:
								//DeTrack[rowcnt].Effect:=$A;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 27:
								//DeTrack[rowcnt].Effect:=$14;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							default:
								//DeTrack[rowcnt].Effect:=Track[y+i]-3;
								//DeTrack[rowcnt].Operand:=Track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
						}
					}
					
				}
				
			}

			/*if Detrack[rowcnt].Effect = $E then
				if Detrack[rowcnt].Operand shr 4 = 8 then begin
				Detrack[rowcnt].Effect:=8;
			Detrack[rowcnt].Operand:=(Detrack[rowcnt].Operand and $F) shl 4;
			end;
			
			if modeffmode then begin
				if (Detrack[rowcnt].Effect=5) and (Detrack[rowcnt].Operand=0) then
				Detrack[rowcnt].Effect:=3;
			if (Detrack[rowcnt].Effect=6) and (Detrack[rowcnt].Operand=0) then
				Detrack[rowcnt].Effect:=4;
			if (Detrack[rowcnt].Effect=$A) and (Detrack[rowcnt].Operand=0) then begin
				Detrack[rowcnt].Effect:=0;
			Detrack[rowcnt].Operand:=0;
			end;
			end;*/
			
			rowcnt++;
			
			if (effectCnt > maxEffectsPerRow)
				maxEffectsPerRow = effectCnt;
			
			effectCnt = 1;
			
		}
					
		i+=len;

	} while (rowcnt<256);

	return maxEffectsPerRow;

}

static void demuxTrack(mp_ubyte* track, MytrackSlot* deTrack)
{
	memset(deTrack, 0, sizeof(MytrackSlot)*256);

	mp_sint32 y;
	mp_sint32 i = 0;
	mp_sint32 rowcnt = 0;

	mp_sint32 effectCnt = 1;
	mp_sint32 maxEffectsPerRow = 0;

	do
	{

		mp_ubyte x = track[i];
		mp_ubyte rep = x >> 5;
		mp_ubyte len = x & 31;

		for (mp_ubyte repcnt = 0; repcnt <= rep; repcnt++)
		{
			y = 1;
			if (len > 1) 
			{
				for (mp_ubyte c = 1; c <= len-1; c++) 
				{
					if (track[y+i])
					{
						switch (track[y+i])
						{
							case 1: 
								deTrack[rowcnt].note = track[y+i+1]+1;
								if (deTrack[rowcnt].note == 97)
									deTrack[rowcnt].note = XModule::NOTE_OFF;
								y+=2;
								c++;
								break;
							case 2:
								deTrack[rowcnt].instrument = track[y+i+1]+1;
								y+=2;
								c++;
								break;
							case 13:
								if (track[y+i+1]) {
									deTrack[rowcnt].effects[effectCnt] = 0xA;
									deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								}
								else {
									deTrack[rowcnt].effects[effectCnt] = 0;
									deTrack[rowcnt].operands[effectCnt] = 0;
								}
								y+=2;
								c++;
								effectCnt++;
								break;
							case 15:
								deTrack[rowcnt].effects[0] = 0x0C;
								deTrack[rowcnt].operands[0] = XModule::vol64to255(track[y+i+1]);
								y+=2;
								c++;
								break;
							case 19:
								deTrack[rowcnt].effects[effectCnt] = 0xF;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 20:
								deTrack[rowcnt].effects[effectCnt] = 0x49;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 21:
								deTrack[rowcnt].effects[effectCnt] = 0x48;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 22:
								deTrack[rowcnt].effects[effectCnt] = 0x47;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 23:
								// key off?
								deTrack[rowcnt].effects[effectCnt] = 51;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 24:
								deTrack[rowcnt].effects[effectCnt] = 27;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 25:
								deTrack[rowcnt].effects[effectCnt] = 0x0F;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 26:
								deTrack[rowcnt].effects[effectCnt] = 0x0A;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							case 27:
								deTrack[rowcnt].effects[effectCnt] = 0x14;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								y+=2;
								c++;
								effectCnt++;
								break;
							default:
								deTrack[rowcnt].effects[effectCnt] = track[y+i]-3;
								deTrack[rowcnt].operands[effectCnt] = track[y+i+1];
								
								if (deTrack[rowcnt].effects[effectCnt] == 0xC)
								{
									deTrack[rowcnt].operands[effectCnt] = XModule::vol64to255(deTrack[rowcnt].operands[effectCnt]);
								}
								else if (deTrack[rowcnt].effects[effectCnt] == 0xE)
								{
									deTrack[rowcnt].effects[effectCnt] = (deTrack[rowcnt].operands[effectCnt]>>4)+0x30;
									deTrack[rowcnt].operands[effectCnt]&=0x0F;
								}
								else if (deTrack[rowcnt].effects[effectCnt] == 0xD)
								{
									deTrack[rowcnt].operands[effectCnt] = ((deTrack[rowcnt].operands[effectCnt]/10)<<4) + (deTrack[rowcnt].operands[effectCnt]%10);
								}
								else if (deTrack[rowcnt].effects[effectCnt] == 0)
								{
									deTrack[rowcnt].effects[effectCnt] = 0x20;
								}
								
								y+=2;
								c++;
								effectCnt++;
						}
					}
					
				}
				
			}

			/*if (detrack[rowcnt].Effect = $E then
				if Detrack[rowcnt].Operand shr 4 = 8 then begin
				Detrack[rowcnt].Effect:=8;
			Detrack[rowcnt].Operand:=(Detrack[rowcnt].Operand and $F) shl 4;
			end;
			
			if modeffmode then begin
				if (Detrack[rowcnt].Effect=5) and (Detrack[rowcnt].Operand=0) then
				Detrack[rowcnt].Effect:=3;
			if (Detrack[rowcnt].Effect=6) and (Detrack[rowcnt].Operand=0) then
				Detrack[rowcnt].Effect:=4;
			if (Detrack[rowcnt].Effect=$A) and (Detrack[rowcnt].Operand=0) then begin
				Detrack[rowcnt].Effect:=0;
			Detrack[rowcnt].Operand:=0;
			end;
			end;*/
			
			rowcnt++;
			
			if (effectCnt > maxEffectsPerRow)
				maxEffectsPerRow = effectCnt;
			
			effectCnt = 1;
			
		}
					
		i+=len;

	} while (rowcnt<256);

}

mp_sint32 LoaderUNI::load(XMFileBase& f, XModule* module)
{
	Uniheader uniheader;
	Uniinstrument uniinstrument;
	Unisample unisample;

	mp_uint32 i,j;
	
	module->cleanUp();

	// this will make code much easier to read
	TXMHeader*		header = &module->header;
	TXMInstrument*	instr  = module->instr;
	TXMSample*		smp	   = module->smp;
	TXMPattern*		phead  = module->phead;	
	
	// we're already out of memory here
	if (!phead || !instr || !smp)
		return MP_OUT_OF_MEMORY;
	
	f.read(uniheader.FileID, 1, 4);
	
	mp_sbyte uniVersion = -1;
	
	uniheader.NumChannels = f.readByte();
	uniheader.SongLength = f.readWord();

	if (uniheader.FileID[3] == '4')
		uniVersion = 4;
	else if (uniheader.FileID[3] == '5')
		uniVersion = 5;
		
	if (uniVersion == -1)
		return MP_LOADER_FAILED;

	if (uniVersion == 5)
		uniheader.RepeatPos = f.readWord();
	else
		uniheader.RepeatPos = 0;
		
	uniheader.NumPatterns = f.readWord();
	uniheader.NumTracks = f.readWord();
	uniheader.NumIns = f.readWord();
	uniheader.InitSpeed = f.readByte();
	uniheader.InitTempo = f.readByte();
	//f.read(uniheader.OrderList, 1, 256);
	f.read(header->ord, 1, 256);
	f.read(uniheader.DefPanning, 1, 32);
	uniheader.Flags = f.readByte();

	i = f.readWord();
	mp_ubyte* text = new mp_ubyte[i];
	if (text == NULL) 
		return MP_OUT_OF_MEMORY;
	f.read(text, 1, i);
	if (i > 32) i = 32;
	memcpy(header->name, text, i);
	delete[] text;

	i = f.readWord();
	text = new mp_ubyte[i];	
	if (text == NULL) 
		return MP_OUT_OF_MEMORY;
	f.read(text, 1, i);
	if (i > 16) i = 16;
	memcpy(header->sig, text, i);
	delete[] text;
	
	i = f.readWord();
	f.seekWithBaseOffset(f.posWithBaseOffset() + i);

	// convert header data
	header->ordnum = (mp_uword)uniheader.SongLength;
	header->restart = (mp_uword)uniheader.RepeatPos;
	header->channum = (mp_uword)uniheader.NumChannels;
	header->patnum = (mp_uword)uniheader.NumPatterns;
	header->insnum = (mp_uword)uniheader.NumIns;
	header->tempo = uniheader.InitSpeed;
	header->speed = uniheader.InitTempo;
	header->freqtab = (uniheader.Flags>>1)&1;
	header->mainvol = 255;

	mp_sint32 s = 0;
	mp_sint32 e = 0;
	for (i = 0; i < uniheader.NumIns; i++)
	{
		uniinstrument.NumSamps = f.readByte();
	    f.read(uniinstrument.SampleNums, 1, 96);
	    uniinstrument.volflg = f.readByte();
	    uniinstrument.volpts = f.readByte();
		uniinstrument.volsus = f.readByte();
	    uniinstrument.volbeg = f.readByte();
	    uniinstrument.volend = f.readByte();
	    f.readWords((mp_uword*)uniinstrument.VolPoints, 12*2);
	    uniinstrument.panflg = f.readByte();
	    uniinstrument.panpts = f.readByte();
		uniinstrument.pansus = f.readByte();
	    uniinstrument.panbeg = f.readByte();
	    uniinstrument.panend = f.readByte();
	    f.readWords((mp_uword*)uniinstrument.PanPoints, 12*2);
	    uniinstrument.vibtype = f.readByte();
	    uniinstrument.vibsweep = f.readByte();
	    uniinstrument.vibdepth = f.readByte();
	    uniinstrument.vibrate = f.readByte();
	    uniinstrument.volfade = f.readWord();
	
		mp_sint32 len = f.readWord();
		text = new mp_ubyte[len];	
		if (text == NULL) 
			return MP_OUT_OF_MEMORY;
		f.read(text, 1, len);
		if (len > 32) len = 32;
		memcpy(instr[i].name, text, len);
		delete[] text;
			
#ifdef VERBOSE
		printf("%s\n",text);
#endif

		// convert instrument data
		instr[i].samp = uniinstrument.NumSamps;
		
		for (j = 0; j < 96; j++)
			instr[i].snum[j] = uniinstrument.SampleNums[j] + s;
		
		//memcpy(instr[i].name, text, len);
		
		// envelopes
		TEnvelope venv;
		TEnvelope penv;
		memset(&venv,0,sizeof(venv));
		memset(&penv,0,sizeof(penv));
		
		mp_sint32 k;
		for (k = 0; k < 12; k++)
		{
			venv.env[k][0] = uniinstrument.VolPoints[k*2];
			venv.env[k][1] = uniinstrument.VolPoints[k*2+1];
		}
		for (k = 0; k < 12; k++)
		{
			penv.env[k][0] = uniinstrument.PanPoints[k*2];
			penv.env[k][1] = uniinstrument.PanPoints[k*2+1];
		}
		
		venv.num = uniinstrument.volpts;	
		penv.num = uniinstrument.panpts;	
		venv.sustain = uniinstrument.volsus;
		venv.loops = uniinstrument.volbeg;
		venv.loope = uniinstrument.volend;
		penv.sustain = uniinstrument.pansus;
		penv.loops = uniinstrument.panbeg;
		penv.loope = uniinstrument.panend;
		venv.type = uniinstrument.volflg;
		penv.type = uniinstrument.panflg;				
		
		if (!module->addVolumeEnvelope(venv)) 
			return MP_OUT_OF_MEMORY;
		if (!module->addPanningEnvelope(penv)) 
			return MP_OUT_OF_MEMORY;
				
		for (j = 0; j < uniinstrument.NumSamps; j++)
		{
			unisample.C2spd = f.readWord();
			unisample.relnotenum = f.readByte();
			unisample.volume = f.readByte();
			unisample.panning = f.readByte();
			unisample.smplength = f.readDword();
			unisample.smploopstart = f.readDword();
			unisample.smploopend = f.readDword();
			unisample.flags = f.readWord();
			
			mp_sint32 len = f.readWord();
			text = new mp_ubyte[len];	
			if (text == NULL) 
				return MP_OUT_OF_MEMORY;
			f.read(text, 1, len);
			if (len > 32) len = 32;
			memcpy(smp[s].name, text, len);
			delete[] text;
			
			// convert sample data
			smp[s].vibtype = uniinstrument.vibtype;
			smp[s].vibsweep = uniinstrument.vibsweep;
			smp[s].vibdepth = uniinstrument.vibdepth << 1;
			smp[s].vibrate = uniinstrument.vibrate;
			smp[s].volfade = uniinstrument.volfade << 1;

			smp[s].venvnum = e + 1;			
			smp[s].penvnum = e + 1;			
			
			if (uniVersion == 4) 
			{
				if ((unisample.flags & 32) == 0) smp[s].flags |= 128; // no delta
				if ((unisample.flags & 4) == 0) smp[s].flags |= 64; // usigned
			}
			else
			{
				if ((unisample.flags & 4) == 0) smp[s].flags |= 128; // no delta 
				if ((unisample.flags & 2) == 0) smp[s].flags |= 64; // unsigned
			}
			
			// 16 bit sample
			/*if (unisample.flags & 1)
			{
				unisample.smplength <<= 1;
				unisample.smploopstart <<= 1;
				unisample.smploopend <<= 1;
			}*/			
			
			//memcpy(smp[s].name, text, len);
			
			smp[s].vol = XModule::vol64to255(unisample.volume);
			smp[s].pan = unisample.panning;
			smp[s].samplen  = unisample.smplength;
			smp[s].loopstart = unisample.smploopstart;
			smp[s].looplen = unisample.smploopend - unisample.smploopstart;
			smp[s].flags |= 3;
			
			mp_ubyte type = 0;
			if (uniVersion == 4)
			{
				type=0;
				if ((unisample.flags & 8) == 8) type = 1;
				if ((unisample.flags & 16) == 16) type = 2;
				if ((unisample.flags & 1) == 1) type+=16;
			}
			else if (uniVersion == 5)
			{
				if (((unisample.flags>>4) & 3) == 0) type = 0;
				if (((unisample.flags>>4) & 3) == 1) type = 1;
				if (((unisample.flags>>4) & 3) == 3) type = 2;
				
				if (unisample.flags&1) type+=16;	
			}
			
			smp[s].type = type;
			
			if (!(uniheader.Flags&1))
			{
				mp_sbyte rnn = 0;
				mp_sbyte ft = 0;

				switch (unisample.C2spd)
				{
					case 8280: 
						ft=-16;
						break;
			        case 8232: 
						ft=-32;
						break;
					case 8169: 
						ft=-48;
						break;
					case 8107: 
						ft=-64;
						break;
					case 8046: 
						ft=-80;
						break;
					case 7985: 
						ft=-96;
						break;
					case 7941: 
						ft=-112;
						break;
					case 7895: 
						ft=-128;
						break;
					case 8363: 
						ft=0;
						break;
					case 8413:
						ft=16;
						break;
					case 8463: 
						ft=32;
						break;
					case 8529: 
						ft=48;
						break;
					case 8581: 
						ft=64;
						break;
					case 8651: 
						ft=96;
						break;
					case 8723: 
						ft=112;
						break;
					case 8757: 
						ft=127;
						break;
					default:
						XModule::convertc4spd(unisample.C2spd, &ft, &rnn);
						break;
				}
				
				smp[s].relnote = rnn;
				smp[s].finetune = ft;
			}
			else
			{
				smp[s].finetune = (mp_sbyte)(unisample.C2spd - 128);
				smp[s].relnote = unisample.relnotenum;
			}
			
			s++;
		}
		
		e++;
		
	}
	
	header->smpnum = s;

	mp_uword* patternRows = new mp_uword[uniheader.NumPatterns];

	mp_uword* trackSeq = new mp_uword[uniheader.NumPatterns*uniheader.NumChannels];
	
	f.readWords(patternRows, uniheader.NumPatterns);
	f.readWords(trackSeq, uniheader.NumPatterns*uniheader.NumChannels);

	mp_ubyte** tracks = new mp_ubyte*[uniheader.NumTracks];

	for (i = 0; i < uniheader.NumTracks; i++)
	{
		mp_uint32 len = f.readWord();
		tracks[i] = new mp_ubyte[len];
		f.read(tracks[i], 1, len);
	
#ifdef VERBOSE
		printf("Maximum number of effects in track %i: %i\n",i,getMaxEffectsFromTrack(tracks[i]));
#endif
	}

	MytrackSlot* deTrack = new MytrackSlot[256];
	
	mp_sint32 trackIndex = 0;
	for (i = 0; i < uniheader.NumPatterns; i++)
	{
		mp_sint32 maxEffects = 0;
		for (j = 0; j < uniheader.NumChannels; j++)
		{
#ifdef VERBOSE
			printf("%i, ",trackSeq[trackIndex]);
#endif
			if ((s=getMaxEffectsFromTrack(tracks[trackSeq[trackIndex]])) > maxEffects)
				maxEffects = s;
				
			trackIndex++;
		}
		
		trackIndex-=uniheader.NumChannels;
		
#ifdef VERBOSE
		printf("Maxeffects for pattern %i: %i\n", i, maxEffects);
#endif
			
		phead[i].rows = patternRows[i];
		phead[i].effnum = maxEffects;
		phead[i].channum = uniheader.NumChannels;
		
		mp_sint32 slotSize = (phead[i].effnum * 2 + 2);
		
		phead[i].patternData = new mp_ubyte[phead[i].rows*header->channum * slotSize];
		
		// out of memory?
		if (phead[i].patternData == NULL)
		{
			delete[] deTrack;
			
			for (i = 0; i < uniheader.NumTracks; i++)
				delete[] tracks[i];
			
			delete[] tracks;
			
			delete[] trackSeq;
			
			delete[] patternRows;
			return MP_OUT_OF_MEMORY;
		}
		
		memset(phead[i].patternData, 0, phead[i].rows*header->channum * slotSize);		
		
		mp_ubyte* pattern = phead[i].patternData;

		for (mp_sint32 c = 0; c < uniheader.NumChannels; c++)
		{
			demuxTrack(tracks[trackSeq[trackIndex]], deTrack);
		
			for (mp_sint32 rows = 0; rows < phead[i].rows; rows++)
			{
				mp_sint32 offset = (rows * slotSize*uniheader.NumChannels) + (slotSize * c);
				
				pattern[offset++] = deTrack[rows].note;
				pattern[offset++] = deTrack[rows].instrument;
				for (mp_sint32 eff = 0;  eff < phead[i].effnum; eff++)
				{
					pattern[offset++] = deTrack[rows].effects[eff];
					pattern[offset++] = deTrack[rows].operands[eff];
				
#ifdef VERBOSE
					if (c == 9 && i == 1 && eff == 1)
					{
						printf("%x, %x\n",deTrack[rows].effects[eff],deTrack[rows].operands[eff]);
					}
#endif
					
				}
				
			}
	
			trackIndex++;
		}
		
	}

	delete[] deTrack;
	
	for (i = 0; i < uniheader.NumTracks; i++)
		delete[] tracks[i];
		
	delete[] tracks;

	delete[] trackSeq;

	delete[] patternRows;

	// lad ma samples du penner
	for (i=0;i<header->smpnum;i++) {
		
#ifdef VERBOSE
		printf("%i\n",smp[i].type);
#endif

		//if (!smp[i].samplen)
		//	continue;

		if (smp[i].type & 16)
			smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen<<1);
		else
			smp[i].sample = (mp_sbyte*)module->allocSampleMem(smp[i].samplen);
		
		if (smp[i].sample == NULL)
		{
			return MP_OUT_OF_MEMORY;
		}
		
		mp_sint32 loadFlags = XModule::ST_DEFAULT;
		
		if (smp[i].type & 16)
			loadFlags |= XModule::ST_16BIT;		
		if (((smp[i].flags >> 7) & 1) == 0)
			loadFlags |= XModule::ST_DELTA;
		if (((smp[i].flags >> 6) & 1) == 1)
			loadFlags |= XModule::ST_UNSIGNED;
		
		// 16bit sample
		if (smp[i].type & 16)
		{
			if (!module->loadSample(f,smp[i].sample,smp[i].samplen<<1,smp[i].samplen,loadFlags))
			{
				return MP_OUT_OF_MEMORY;
			}
		}
		// 8bit sample
		else 
		{
			if (!module->loadSample(f,smp[i].sample,smp[i].samplen,smp[i].samplen,loadFlags))
			{
				return MP_OUT_OF_MEMORY;
			}
		}
		
	}	

	header->volenvnum = module->numVEnvs;
	header->panenvnum = module->numPEnvs;

	strcpy(header->tracker,"..converted..");
	
	module->setDefaultPanning();
	
	module->postProcessSamples();

	return MP_OK;
}
