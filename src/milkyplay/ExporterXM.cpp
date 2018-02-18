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
 *  ExporterXM.cpp
 *  MilkyPlay XM writer (trying hard to export something useful)
 *
 *  --------------------------------
 *			Version History:
 *  --------------------------------
 *  26/03/06: PLM Far position jump fix & effects are relocated to other channels if possible
 *  01/01/06: Happy new year ;) MilkyTracker XMs are now even smaller than FT2 XMs
 *  02/12/05: Fixed problems with last operand restoration for S3M and MDL commands. VERY SLOW(!!) but working
 *  06/02/05: Added simulation of multitracker behaviour with FT2 features
 *  05/02/05: Added simulation of protracker behaviour with FT2 features
 *  07/12/04: Writes all other instrument related stuff too (envelopes, auto-vibrato etc.)
 *  06/12/04: Added sample relative note remapper when note range exceeds regular XM note range (RIGHT.PTM)
 *  05/12/04: First acceptable results with one-effect modules (.MOD / .MTM)
 *  04/12/04: Started work
 *
 */
#include "MilkyPlay.h"

#ifdef VERBOSE
	#include "stdio.h"
#endif

struct TWorkBuffers
{
	mp_sint32 noteRangeRemapper[256];
	mp_ubyte lastArpeggio[256];
	mp_ubyte lastVolSlide[256];
	mp_ubyte lastGVolSlide[256];
	mp_ubyte lastPorta[256];
	mp_ubyte lastTempoSlide[256];
	mp_sint32 globalVolume;
	mp_sint32 bpm, baseBpm, speed;
	
	mp_ubyte lastIns[256];
	mp_ubyte lastNote[256];

	void clearBuffers()
	{
		memset(lastArpeggio, 0, sizeof(lastArpeggio));
		memset(lastVolSlide, 0, sizeof(lastVolSlide));
		memset(lastGVolSlide, 0, sizeof(lastGVolSlide));
		memset(lastPorta, 0, sizeof(lastPorta));
		memset(lastTempoSlide, 0, sizeof(lastTempoSlide));
		memset(lastIns, 0, sizeof(lastIns));
		memset(lastNote, 0, sizeof(lastNote));
		globalVolume = 64;
		baseBpm = 125;
		bpm = 125;
		speed = 6;
	}

	TWorkBuffers()
	{
		clearBuffers();
	
		memset(noteRangeRemapper, 0, sizeof(noteRangeRemapper));
	}
};

static void convertEffect(mp_ubyte effIn, mp_ubyte opIn, mp_ubyte& effOut, mp_ubyte& opOut, mp_sint32 curChan, TWorkBuffers& workBuffers, bool convertITTempoSlides)
{
	mp_ubyte* lastArpeggio = workBuffers.lastArpeggio;
	mp_ubyte* lastVolSlide = workBuffers.lastVolSlide;
	mp_ubyte* lastGVolSlide = workBuffers.lastGVolSlide;
	mp_ubyte* lastPorta = workBuffers.lastPorta;
	mp_ubyte* lastTempoSlide = workBuffers.lastTempoSlide;

	effOut = opOut = 0;

	// Protracker commands
	if (effIn > 0 && effIn <= 0x11)
	{
		effOut = effIn;
		opOut = opIn;
	
		if (effIn == 0x0C || effIn == 0x10)
			opOut = (mp_ubyte)(((mp_sint32)opOut*64)/255);
			
		if (effIn == 0x10)
			workBuffers.globalVolume = (mp_ubyte)(((mp_sint32)opOut*64)/255);
		
		// Cope with set BPM in case we're having a DBM "set real BPM command"
		if (effIn == 0x0f && opIn >= 32)
		{
			// real BPM is not the default value => recalculate BPM
			if (workBuffers.baseBpm != 125)
			{
				// First store current BPM
				workBuffers.bpm = opIn;
				// Now calculate new BPM (see DBM player source)
				mp_sint32 realCiaTempo = (workBuffers.bpm * (workBuffers.baseBpm << 8) / 125) >> 8;
				// clip if necessary
				if (realCiaTempo > 255)
					realCiaTempo = 255;
				if (realCiaTempo < 32)
					realCiaTempo = 32;
				// This is our new set BPM
				opOut = realCiaTempo;
			}
		}
		
	}
	// set envelope position
	else if (effIn == 0x14 || effIn == 0x51)
	{
		effOut = 0x14;
		opOut = opIn;
	}
	// set envelope position
	else if (effIn == 0x15)
	{
		effOut = effIn;
		opOut = opIn;
	}
	// set tempo
	else if (effIn == 0x16)
	{
		if (opIn >= 32)
		{
			// First store current BPM
			workBuffers.bpm = opIn;
			// Cope with set BPM in case we're having a DBM "set real BPM command"
			if (workBuffers.baseBpm != 125)
			{
				// Now calculate new BPM (see DBM player source)
				mp_sint32 realCiaTempo = (workBuffers.bpm * (workBuffers.baseBpm << 8) / 125) >> 8;
				// clip
				if (realCiaTempo > 255)
					realCiaTempo = 255;
				if (realCiaTempo < 32)
					realCiaTempo = 32;
				// new effect 0xF with new BPM
				effOut = 0x0F;
				opOut = realCiaTempo;
			}
			else
			{
				if (opIn < 32) opIn = 32;
				effOut = 0x0F;
				opOut = opIn;
			}
		}
		else if (convertITTempoSlides)
		{
			if (opIn) lastTempoSlide[curChan] = opIn;
			
			if (lastTempoSlide[curChan]) 
			{
				mp_ubyte y = lastTempoSlide[curChan]>>4;
				mp_ubyte x = lastTempoSlide[curChan]&0xf;
				
				// tempo slide up
				if (y) 
				{					
					effOut = 0x0F;
					workBuffers.bpm+=x*(workBuffers.speed-1);
					if (workBuffers.bpm > 255)
						workBuffers.bpm = 255;					
					opOut = workBuffers.bpm;
				}
				// tempo slide down
				else
				{
					effOut = 0xF;
					workBuffers.bpm-=x*(workBuffers.speed-1);
					if (workBuffers.bpm < 32)
						workBuffers.bpm = 32;					
					opOut = workBuffers.bpm;
				}
			}		
		}
	}
	// Panning slide / Multi retrig / tremor
	else if (effIn == 0x19 || effIn == 0x1B || effIn == 0x1D)
	{
		effOut = effIn;
		opOut = opIn;
	}
	// set speed
	else if (effIn == 0x1C)
	{
		if (opIn)
		{
			if (opIn > 31) opIn = 31;
			effOut = 0x0F;
			opOut = opIn;
		}
	}
	// MDL set sample offset
	else if (effIn == 0x1F)
	{
		effOut = 0x09;
		opOut = opIn;
	}	
	// PLM position jump
	else if (effIn == 0x2B)
	{
		effOut = 0x0B;
		opOut = opIn;
	}	
	// Protracker subcommands (most likely)
	else if (effIn >= 0x30 && effIn <= 0x3F)
	{
		effOut = 0x0E;
		opOut = ((effIn-0x30)<<4)+(opIn&0xF);
	}
	// arpeggio
	else if (effIn == 0x20)
	{
		if (!opIn) opIn = lastArpeggio[curChan];
	
		if (opIn)
		{
			effOut = 0;
			opOut = opIn;
			lastArpeggio[curChan] = opIn;
		}
			
	}
	// extra fine porta commands
	else if (effIn == 0x41)
	{
		effOut = 0x21;
		opOut = 0x10 + (opIn <= 0xf ? opIn&0xF : 0xF);
	}
	else if (effIn == 0x42)
	{
		effOut = 0x21;
		opOut = 0x20 + (opIn <= 0xf ? opIn&0xF : 0xF);
	}
	// MDL porta up
	else if (effIn == 0x43)
	{
		if (opIn) lastPorta[curChan] = opIn;
		 
		if (lastPorta[curChan] >= 0xE0) {
			mp_ubyte y = lastPorta[curChan]>>4;
			mp_ubyte x = lastPorta[curChan]&0xf;
			switch (y) {
				case 0xF:
					effOut = 0xE;
					opOut = 0x10 + x;
					break;
				case 0xE: 
					effOut = 0x21;
					opOut = 0x10 + (x>>1);
					break;
			}
		}
		else if (lastPorta[curChan])
		{
			effOut = 0x1;
			opOut = lastPorta[curChan];
		}
	}
	// MDL porta down
	else if (effIn == 0x44)
	{
		if (opIn) lastPorta[curChan] = opIn;
		 
		if (lastPorta[curChan] >= 0xE0) {
			mp_ubyte y = lastPorta[curChan] >> 4;
			mp_ubyte x = lastPorta[curChan] & 0xf;
			switch (y) {
				case 0xF:
					effOut = 0xE;
					opOut = 0x20 + x;
					break;
				case 0xE: 
					effOut = 0x21;
					opOut = 0x20 + (x>>1);
					break;
			}
		}
		else if (lastPorta[curChan])
		{
			effOut = 0x2;
			opOut = lastPorta[curChan];
		}
	}
	// MDL volslide up
	else if (effIn == 0x45)
	{
		if (opIn) lastVolSlide[curChan] = opIn;
		 
		if (lastVolSlide[curChan] >= 0xE0) {
			mp_ubyte y = lastVolSlide[curChan]>>4;
			mp_ubyte x = lastVolSlide[curChan]&0xf;
			switch (y) {
				case 0xF:
					effOut = 0xE;
					opOut = 0xA0 + x;
					break;
				case 0xE: 
					effOut = 0xE;
					opOut = 0xA0 + (x>>2);
					break;
			}
		}
		else if (lastVolSlide[curChan])
		{
			effOut = 0xA;
			opOut = (lastVolSlide[curChan]>>2) <= 0xF ? (lastVolSlide[curChan]>>2)<<4 : 0xF0;
		}
	}
	// MDL volslide down
	else if (effIn == 0x46)
	{
		if (opIn) lastVolSlide[curChan] = opIn;
		 
		if (lastVolSlide[curChan] >= 0xE0) {
			mp_ubyte y = lastVolSlide[curChan] >> 4;
			mp_ubyte x = lastVolSlide[curChan] & 0xf;
			switch (y) {
				case 0xF:
					effOut = 0xE;
					opOut = 0xB0 + x;
					break;
				case 0xE: 
					effOut = 0xE;
					opOut = 0xB0 + (x>>2);
					break;
			}
		}
		else if (lastVolSlide[curChan])
		{
			effOut = 0xA;
			opOut = (lastVolSlide[curChan]>>2) <= 0xF ? (lastVolSlide[curChan]>>2) : 0xF;
		}
	}
	// S3M porta up
	else if (effIn == 0x47) 
	{
		if (opIn) lastPorta[curChan] = opIn;
		if (lastPorta[curChan] >= 0xE0) {
			mp_ubyte y = lastPorta[curChan] >> 4;
			mp_ubyte x = lastPorta[curChan] & 0xf;
			switch (y) {
				case 0xF:
					effOut = 0xE;
					opOut = 0x10 + x;
					break;
				case 0xE: 
					effOut = 0x21;
					opOut = 0x10 + x;
					break;
			}
		}
		else if (lastPorta[curChan])
		{
			effOut = 0x1;
			opOut = lastPorta[curChan];
		}
	}
	// S3M porta down
	else if (effIn == 0x48) 
	{
		if (opIn) lastPorta[curChan] = opIn;
		if (lastPorta[curChan] >= 0xE0) {
			mp_ubyte y = lastPorta[curChan] >> 4;
			mp_ubyte x = lastPorta[curChan] & 0xf;
			switch (y) {
				case 0xF:
					effOut = 0xE;
					opOut = 0x20 + x;
					break;
				case 0xE: 
					effOut = 0x21;
					opOut = 0x20 + x;
					break;
			}
		}
		else if (lastPorta[curChan])
		{
			effOut = 0x2;
			opOut = lastPorta[curChan];
		}
	}
	// S3M volslide
	else if (effIn == 0x49) 
	{
		if (opIn) lastVolSlide[curChan] = opIn;
		
		if (lastVolSlide[curChan]) 
		{
			mp_ubyte y = lastVolSlide[curChan]>>4;
			mp_ubyte x = lastVolSlide[curChan]&0xf;
			
			if (!(x == 0xF && y)&&!(y == 0xF && x)) 
			{
				if (x && y) x = 0;
				
				if (y) 
				{					
					effOut = 0xA;
					opOut = y<<4;
				}
				else if (x) 
				{
					effOut = 0xA;
					opOut = x;
				}
			
			}
			else
			{
				if (!(x==0x0F && !y) && !(y==0x0F && !x)) 
				{
					if (x==0x0F)
					{
						effOut = 0xE;
						opOut = 0xA0+y;
					}
					else if (y==0x0F)
					{
						effOut = 0xE;
						opOut = 0xB0+x;
					}
				}
			}
		}
	}
	// PSM fine volslide up
	else if (effIn == 0x4B)
	{
		effOut = 0x0E;
		opOut = opIn>>2;
		if (opOut>0xF) opOut=0x0F;
		opOut+=0xA0;
	}
	// PSM fine volslide down
	else if (effIn == 0x4C)
	{
		effOut = 0x0E;
		opOut = opIn>>2;
		if (opOut>0xF) opOut=0x0F;
		opOut+=0xB0;
	}
	// PSM porta up
	else if (effIn == 0x4D)
	{
		effOut = 0x01;
		opOut = opIn>>2;
	}
	// PSM porta down
	else if (effIn == 0x4E)
	{
		effOut = 0x02;
		opOut = opIn>>2;
	}
	// "S3M" global volslide, this is an IT/MODPLUG feature
	else if (effIn == 0x59) 
	{
		if (opIn) lastGVolSlide[curChan] = opIn;
		
		if (lastGVolSlide[curChan]) 
		{
			mp_ubyte y = lastGVolSlide[curChan]>>4;
			mp_ubyte x = lastGVolSlide[curChan]&0xf;
			
			if (!(x == 0xF && y)&&!(y == 0xF && x)) 
			{
				if (x && y) x = 0;
				
				if (y) 
				{					
					effOut = 0x11;
					opOut = y<<4;
					workBuffers.globalVolume+=y*(workBuffers.speed-1);
					if (workBuffers.globalVolume > 64)
						workBuffers.globalVolume = 64;					
				}
				else if (x) 
				{
					effOut = 0x11;
					opOut = x;
					workBuffers.globalVolume-=x*(workBuffers.speed-1);
					if (workBuffers.globalVolume < 0)
						workBuffers.globalVolume = 0;					
				}
			
			}
			else
			{
				// SUCKS
				if (!(x==0x0F && !y) && !(y==0x0F && !x)) 
				{
					if (x==0x0F)
					{
						workBuffers.globalVolume += y;
						if (workBuffers.globalVolume > 64)
							workBuffers.globalVolume = 64;
						effOut = 0x10;
						opOut = (mp_ubyte)workBuffers.globalVolume;
					}
					else if (y==0x0F)
					{
						workBuffers.globalVolume -= x;
						if (workBuffers.globalVolume < 0)
							workBuffers.globalVolume = 0;
						effOut = 0x10;
						opOut = (mp_ubyte)workBuffers.globalVolume;
					}
				}
			}
		}
	}
	// set digibooster real BPM
	else if (effIn == 0x52)
	{	
		// store new BPM base
		workBuffers.baseBpm = opIn;
		// if it's not the default value recalculate BPM
		if (workBuffers.baseBpm != 125)
		{
			// see digibooster pro player source
			mp_sint32 realCiaTempo = (workBuffers.bpm * (workBuffers.baseBpm << 8) / 125) >> 8;
			// clip if necessary
			if (realCiaTempo > 255)
				realCiaTempo = 255;
			if (realCiaTempo < 32)
				realCiaTempo = 32;
			// write new command: 0xF with new operand
			effOut = 0x0F;
			opOut = realCiaTempo;
		}
	}
	// FAR/669 (effects are really different, conversion doesn't make much sense)
	else if (effIn == 0x70)
	{
		switch (opIn>>4)
		{
			case 0x0f:
				effOut = 0xf;
				opOut = opIn&0xf;
				break;
		}
	}
	else if (effIn)
	{
#ifdef VERBOSE
		printf("Missing effect %i, %i\n", effIn, opIn);
#endif
	}
}

// convert FT2 compatible effect to volumn column effect if possible
static mp_ubyte convertToVolume(mp_ubyte eff, mp_ubyte op)
{
	
	mp_ubyte vol = 0;

	/*if (eff && eff != 0x0C)
	{
		printf("%i, %i\n", eff, op);
	}*/

	// set volume
	if (eff == 0x0C)
	{
		vol = 0x10 + op;
	}
	// volslide
	else if (eff == 0x0A)
	{
		
		// use last operand?
		if (!op)
		{
			vol = 0x60;
		}
		// volslide down
		else if (op & 0xF)
		{
			vol = 0x60 + (op&0xF);
		}
		// volslide up
		else if (op >> 4)
		{
			vol = 0x70 + (op>>4);
		}
		
	}
	// extra fine volslide up
	else if (eff == 0xE && ((op>>4)==0xA))
	{
		vol = 0x90 + (op & 0xF);
	}
	// extra fine volslide down
	else if (eff == 0xE && ((op>>4)==0xB))
	{
		vol = 0x80 + (op & 0xF);
	}
	// extra vibrato
	else if (eff == 0x4)
	{
		if ((op>>4) && !(op&0xF))
		{
			vol = 0xA0 + (op>>4);
		}
		else if (!(op>>4)/* && (op&0xF)*/)
		{
			vol = 0xB0 + op;
		}
	}
	// set panning
	else if (eff == 0x8)
	{
		vol = 0xC0 + (op>>4);
	}
	// panning slide
	else if (eff == 0x19)
	{
		// use last operand?
		if (!op)
		{
			vol = 0xD0;
		}
		// panning slide left
		else if (op & 0xF)
		{
			vol = 0xD0 + (op&0xF);
		}
		// panning slide right
		else if (op >> 4)
		{
			vol = 0xE0 + (op>>4);
		}
	}
	// porta to note
	else if (eff == 0x03)
	{
		vol = 0xF0 + (op>>4);
	}
	
	return vol;
}

// Convert a bunch of effects (srcSlot with numEffects)
// into volume, eff and op (XM operands)
// curChan is the current channel
// workBuffers holds the last effect operands while processing
// effectBuffer holds a bunch (numEffectsInBuffer) of remaining 
// effects from the  last columns which might be allowed to go 
// into another channel
// swapBuffer must be at least of effectBuffer size
static void convertEffects(mp_ubyte* srcSlot, 
						   mp_sint32 numEffects, 
						   mp_ubyte& volume, 
						   mp_ubyte& eff, 
						   mp_ubyte& op, 
						   mp_sint32 curChan, 
						   TWorkBuffers& workBuffers, 
						   mp_ubyte* effectBuffer, 
						   mp_sint32& numEffectsInBuffer,
						   mp_ubyte* swapBuffer,
						   bool convertITTempoSlides)
{
	mp_sint32 i;

	// If there is only one effect this goes into the effect column
	if (numEffects == 1)
	{
		volume = 0;
		
		convertEffect(srcSlot[2], srcSlot[3], eff, op, curChan, workBuffers, convertITTempoSlides);
	}
	else if (numEffects >= 2)
	{
		mp_sint32 oldNum = numEffects;
		numEffects+=numEffectsInBuffer;
		mp_ubyte* effects = swapBuffer;
	
		// Convert effects to be FT2 compatible
		// Result will be written in effects
		for (i = 0; i < oldNum; i++)
		{
			effects[i*2] = effects[i*2+1] = 0;
			convertEffect(srcSlot[2+i*2], srcSlot[2+i*2+1], effects[i*2], effects[i*2+1], curChan, workBuffers, convertITTempoSlides);
		}
		// Append replacable effects which are left from other columns
		for (i = 0; i < numEffectsInBuffer; i++)
		{
			mp_uint32 j = oldNum + i;
			effects[j*2] = effectBuffer[i*2];
			effects[j*2+1] = effectBuffer[i*2+1];
		}
		
		// Now "effects" contains all effects+operands
		// We try to find a home for them
		for (i = 0; i < numEffects; i++)
		{
			// could be MDL portamento + volslide
			if (numEffects >= 3 && 
				effects[i*2] == 0x3 &&
				effects[i*2+1] == 0)
			{
				for (mp_sint32 j = 0; j < numEffects; j++)
				{
					if (effects[j*2] == 0xA &&
						effects[j*2+1] != 0)
					{
						effects[i*2] = 0x5;
						effects[i*2+1] = effects[j*2+1];
						
						// clear out
						effects[j*2] = 0;
						effects[j*2+1] = 0;
					}
				}
			}
			// could be MDL vibrato + volslide
			if (numEffects >= 3 &&
				effects[i*2] == 0x4 &&
				effects[i*2+1] == 0)
			{
				for (mp_sint32 j = 0; j < numEffects; j++)
				{
					if (effects[j*2] == 0xA &&
						effects[j*2+1] != 0)
					{
						effects[i*2] = 0x6;
						effects[i*2+1] = effects[j*2+1];
						
						// clear out
						effects[j*2] = 0;
						effects[j*2+1] = 0;
					}
				}
			}
		}
	
		volume = 0;
		eff = 0;
		op = 0;
		
		for (i = 0; i < numEffects; i++)
		{
			// Effect nr. 1 => try to stuff into volume column first
			if (i == 0)
			{
				// If this is a portamento to note command and it's 
				// volume column compatible we'll place it in the
				// volume column
				bool notePortaNotVolumeCompatible = ((effects[i*2] == 0x03) && (effects[i*2+1]&0xF));
				
				if (convertToVolume(effects[i*2], effects[i*2+1]) && !volume && !notePortaNotVolumeCompatible)
				{
					volume = convertToVolume(effects[i*2], effects[i*2+1]);
					// clear out
					effects[i*2] = effects[i*2+1] = 0;
				}
				// didn't work
				else if ((effects[i*2] || effects[i*2+1]) && (!eff && !op))
				{
					eff = effects[i*2];
					op = effects[i*2+1];
					// clear out
					effects[i*2] = effects[i*2+1] = 0;
				}
			}
			// for the rest of the effects, try to find 
			// free space, take effect column first, volume column secondly
			else
			{
				if ((effects[i*2] || effects[i*2+1]) && (!eff && !op))
				{
					eff = effects[i*2];
					op = effects[i*2+1];
					// clear out
					effects[i*2] = effects[i*2+1] = 0;
				}
				else if (convertToVolume(effects[i*2], effects[i*2+1]) && !volume)
				{
					volume = convertToVolume(effects[i*2], effects[i*2+1]);
					// clear out
					effects[i*2] = effects[i*2+1] = 0;
				}
			}
			
		}
		
		mp_uint32 numOutEffs = 0;
		// Scan what's left and sort out effects which are not allowed to go into another channel
		for (i = 0; i < numEffects; i++)
		{
			switch (effects[i*2])
			{
				case 0x0B:
				case 0x0D:
				
				case 0x0E:
					switch (effects[i*2+1] >> 4)
					{
						case 0xE:
							goto takeEffect;
					}
					break;
				
				case 0x0F:
takeEffect:
					effectBuffer[numOutEffs*2] = effects[i*2];
					effectBuffer[numOutEffs*2+1] = effects[i*2+1];
					numOutEffs++;
					break;
			}
		}
		
		numEffectsInBuffer = numOutEffs;
		
		//if (numOutEffs)
		//	printf("%i\n", numOutEffs);
		
	}
		
}

static void fillWorkBuffers(const XModule* module, mp_uint32 orderListIndex, TWorkBuffers& workBuffers)
{
	workBuffers.clearBuffers();

	mp_ubyte* lastArpeggio = workBuffers.lastArpeggio;
	mp_ubyte* lastVolSlide = workBuffers.lastVolSlide;
	mp_ubyte* lastGVolSlide = workBuffers.lastGVolSlide;
	mp_ubyte* lastPorta = workBuffers.lastPorta;
	mp_ubyte* lastNote = workBuffers.lastNote;
	mp_ubyte* lastIns = workBuffers.lastIns;
	mp_ubyte* lastTempoSlide = workBuffers.lastTempoSlide;

	for (mp_uint32 orderIndex = 0; orderIndex < orderListIndex; orderIndex++)
	{
		mp_uint32 patIndex = module->header.ord[orderIndex];
		
		TXMPattern* pattern = &module->phead[patIndex];
		
		mp_sint32 slotSize = pattern->effnum*2+2;
		mp_sint32 channum = pattern->channum, effnum = pattern->effnum;

		mp_ubyte* srcSlot = pattern->patternData;

		for (mp_sint32 rows = 0; rows < pattern->rows; rows++)
		{
			for (mp_sint32 c = 0;  c < channum; c++)
			{
				if (srcSlot[0])
					lastNote[c] = srcSlot[0];
				
				if (srcSlot[1])
					lastIns[c] = srcSlot[1];

				// ----------- store last operands for S3M/MDL/DBM effects ---------------
				const mp_ubyte* effSlot = srcSlot+2;
				for (mp_sint32 effCnt = 0; effCnt < effnum; effCnt++)
				{
					mp_ubyte effIn = *effSlot;
					mp_ubyte opIn = *(effSlot+1);
					
					effSlot+=2;
	
					if (effIn < 0x0f || effIn > 0x5a)
						continue;
					
					switch (effIn)
					{
						case 0x0f:
							if (opIn >= 32)
								workBuffers.bpm = opIn;
							break;
						case 0x10:
							workBuffers.globalVolume = (mp_ubyte)(((mp_sint32)opIn*64)/255);
							break;
						case 0x16:
							if (opIn >= 32)
							{
								workBuffers.bpm = opIn;
							}
							else
							{
								if (opIn) lastTempoSlide[c] = opIn;
								
								if (lastTempoSlide[c]) 
								{
									mp_ubyte y = lastTempoSlide[c]>>4;
									mp_ubyte x = lastTempoSlide[c]&0xf;
									
									// tempo slide up
									if (y) 
									{					
										workBuffers.bpm+=x*(workBuffers.speed-1);
										if (workBuffers.bpm > 255)
											workBuffers.bpm = 255;					
									}
									// tempo slide down
									else
									{
										workBuffers.bpm-=x*(workBuffers.speed-1);
										if (workBuffers.bpm < 32)
											workBuffers.bpm = 32;					
									}									
								}								
							}
							break;
						case 0x1C:
							if (opIn)
								workBuffers.speed = opIn;
							break;
						case 0x20:
							if (!opIn) break;
							lastArpeggio[c] = opIn;
							break;
						case 0x43:
							if (!opIn) break;
							lastPorta[c] = opIn;
							break;
						case 0x44:
							if (!opIn) break;
							lastPorta[c] = opIn;
							break;
						case 0x45:
							if (!opIn) break;
							lastVolSlide[c] = opIn;
							break;
						case 0x46:
							if (!opIn) break;
							lastVolSlide[c] = opIn;
							break;
						case 0x47:
							if (!opIn) break;
							lastPorta[c] = opIn;
							break;
						case 0x48:
							if (!opIn) break;
							lastPorta[c] = opIn;
							break;
						case 0x49:
							if (!opIn) break;
							lastVolSlide[c] = opIn;
							break;
						case 0x52:
							workBuffers.baseBpm = opIn;
							break;
							
						case 0x59:
						{
							if (opIn)
								lastGVolSlide[c] = opIn;
							
							if (lastGVolSlide[c]) 
							{
								mp_ubyte y = lastGVolSlide[c]>>4;
								mp_ubyte x = lastGVolSlide[c]&0xf;
															
								if (!(x == 0xF && y)&&!(y == 0xF && x)) 
								{
									if (x && y) x = 0;
									
									if (y) 
									{					
										workBuffers.globalVolume+=y*(workBuffers.speed-1);
										if (workBuffers.globalVolume > 64)
											workBuffers.globalVolume = 64;					
									}
									else if (x) 
									{
										workBuffers.globalVolume-=x*(workBuffers.speed-1);
										if (workBuffers.globalVolume < 0)
											workBuffers.globalVolume = 0;					
									}
									
								}
								else
								{
									// SUCKS
									if (!(x==0x0F && !y) && !(y==0x0F && !x)) 
									{
										if (x==0x0F)
										{
											workBuffers.globalVolume += y;
											if (workBuffers.globalVolume > 64)
												workBuffers.globalVolume = 64;
										}
										else if (y==0x0F)
										{
											workBuffers.globalVolume -= x;
											if (workBuffers.globalVolume < 0)
												workBuffers.globalVolume = 0;
										}
									}
								}																
							}							
							break;
						}
					}
				}
				
				srcSlot+=slotSize;
				
			}
		}
	}
}

static mp_sint32 convertPattern(const XModule* module, const TXMPattern* srcPattern, mp_ubyte* dstPattern, mp_sint32 numChannels, TWorkBuffers& workBuffers, bool verbose)
{

	bool convertITTempoSlides = module ? (module->getType() == XModule::ModuleType_IT) : false;
	bool newInsPTFlag = module ? ((module->header.flags & XModule::MODULE_PTNEWINSTRUMENT) != 0) : false;
	bool newInsST3Flag = module ? ((module->header.flags & XModule::MODULE_ST3NEWINSTRUMENT) != 0) : false;

	if (module)
	{
		mp_sint32 patNum = -1;
		for (mp_uint32 i = 0; i < module->header.patnum; i++)
		{
			if (srcPattern == &module->phead[i])
			{
				patNum = i;
				break;
			}
		}
		
		if (patNum != -1)
		{
			mp_sint32 orderListPos = -1;
			for (mp_uint32 i = 0; i < module->header.ordnum; i++)
			{
				if (module->header.ord[i] == patNum)
				{
					orderListPos = i;
					break;
				}
			}
			
			if (orderListPos != -1)
			{
				fillWorkBuffers(module, orderListPos, workBuffers);
			}
		}
	}

	mp_ubyte* lastNote = workBuffers.lastNote;
	mp_ubyte* lastIns = workBuffers.lastIns;

#ifdef MILKYTRACKER
	newInsPTFlag = false;
#endif

	mp_ubyte* effectBuffer = new mp_ubyte[srcPattern->rows * numChannels * srcPattern->effnum];
	mp_ubyte* swapBuffer = new mp_ubyte[srcPattern->rows * numChannels * srcPattern->effnum];
	mp_sint32 numEffectsInBuffer = 0;

	for (mp_sint32 rows = 0; rows < srcPattern->rows; rows++)
	{
		bool correctPLMFarJump				= false;
		mp_uint32 correctPLMFarJumpChannel	= 0;
		mp_sint32 PLMFarJumpPos				= 0;
		mp_sint32 PLMFarJumpRow				= 0;
	
		mp_sint32 c;
	
		for (c = 0;  c < srcPattern->channum; c++)
		{
			if (c < numChannels)
			{				
				mp_ubyte* dstSlot = dstPattern+(rows*(numChannels*5) + (c*5));
				mp_ubyte* srcSlot = srcPattern->patternData+(rows*(srcPattern->channum*(srcPattern->effnum*2+2)) + c*(srcPattern->effnum*2+2));
				
				if (srcSlot[0])
					lastNote[c] = srcSlot[0];
				
				if (srcSlot[1])
				{
					lastIns[c] = srcSlot[1];
					//if (srcSlot[0])
					//	lastIns2[c] = srcSlot[1];
				}
				
				mp_sint32 srcNote = (mp_sint32)srcSlot[0];

				if (lastIns[c] && srcNote > 0 && srcNote <= XModule::NOTE_LAST) 
				{
					srcNote+=workBuffers.noteRangeRemapper[lastIns[c]-1];
					if (srcNote > XModule::NOTE_LAST || srcNote < 0)
						srcNote = 0;
				}
				if (srcNote == XModule::NOTE_OFF || srcNote == XModule::NOTE_CUT) srcNote = 97;
				else if (srcNote > 96) srcNote = 0;
				
				dstSlot[0] = srcNote;
				
				// instrument
				dstSlot[1] = srcSlot[1];
				
				convertEffects(srcSlot, 
							   srcPattern->effnum, 
							   dstSlot[2], 
							   dstSlot[3], 
							   dstSlot[4], 
							   c, 
							   workBuffers, 
							   effectBuffer, 
							   numEffectsInBuffer, 
							   swapBuffer, 
							   convertITTempoSlides);
				
				// try to find workaround for PLM far-jump
				if (module && module->getType() == XModule::ModuleType_PLM && dstSlot[3] == 0x0B)
				{
					mp_ubyte* eff = srcSlot+2;
					for (mp_sint32 i = 0; i < srcPattern->effnum; i++)
					{
						if (eff[i*2] == 0x2B)
						{
							PLMFarJumpPos = eff[i*2+1];
							PLMFarJumpRow = eff[((i+1)%srcPattern->effnum)*2+1];
							correctPLMFarJump = true;
							correctPLMFarJumpChannel = c;
							break;
						}
					}
				}
				
				// * some nasty protracker style fixes
				// * trying to emulate protracker 3.15 behaviour with FT2 methods
				mp_sint32 i = srcSlot[1];
				if (module && i && newInsPTFlag)
				{
					if (!dstSlot[0])
					{
						if (module->instr[i-1].samp == 0 ||
							module->instr[i-1].snum[0] == -1)
						{
							dstSlot[0] = 97;
						}
						else
						{
							mp_sint32 s = module->instr[i-1].snum[0];
							if (s != -1 && !dstSlot[2])
								dstSlot[2] = (mp_ubyte)(((mp_sint32)module->smp[s].vol*64)/255)+0x10;
						}
					}
					else
					{
						if (module->instr[i-1].samp == 0 ||
							module->instr[i-1].snum[dstSlot[0]] == -1)
						{
							dstSlot[0] = 97;
						}
					}
					
				}
				else if (module && i && newInsST3Flag)
				{
					if (!dstSlot[0])
					{
						if (!(module->instr[i-1].samp == 0 ||
							  module->instr[i-1].snum[0] == -1))
						{
							mp_sint32 s = module->instr[i-1].snum[0];
							if (s != -1 && !dstSlot[2])
								dstSlot[2] = (mp_ubyte)(((mp_sint32)module->smp[s].vol*64)/255)+0x10;
						}
						else
						{
							dstSlot[0] = 0;
						}
					}
				}
				
				// * trying to emulate MTM behaviour
				// * Sample offset command triggers last note again
				if (module && module->getType() == XModule::ModuleType_MTM && !dstSlot[0] && dstSlot[3] == 0x9)
					dstSlot[0] = lastNote[c];
				
			}
			
			if (correctPLMFarJump)
			{
				for (c = 0;  c < srcPattern->channum; c++)
				{
					mp_ubyte* dstSlot = dstPattern+(rows*(numChannels*5) + (c*5));
					
					if (dstSlot[3] == 0)
					{
						if (c > (signed)correctPLMFarJumpChannel)
						{
							dstSlot[3] = 0x0D;
							dstSlot[4] = (PLMFarJumpRow/10)*16+(PLMFarJumpRow%10);
						}
						else
						{
							mp_ubyte* srcSlot = dstPattern+(rows*(numChannels*5) + (correctPLMFarJumpChannel*5));
							
							dstSlot[3] = srcSlot[3];
							dstSlot[4] = srcSlot[4];
							
							srcSlot[3] = 0x0D;
							srcSlot[4] = (PLMFarJumpRow/10)*16+(PLMFarJumpRow%10);
						}
						break;
					}
				}
			}
			
		}
		
	}
	
	delete[] swapBuffer;
	delete[] effectBuffer;

	return MP_OK;
}

mp_sint32 packPattern(const mp_ubyte* pattern, mp_ubyte* outputPattern, mp_sint32 numRows, mp_sint32 numChannels)
{
	mp_sint32 i,j,z,b1,x,y;
	mp_ubyte pack[6];
	
	/*i = numRows*numChannels*5 - 1;
	while (i > 0 && !pattern[i])
		i--;
	mp_sint32 max = i;

	printf("%i, %i\n", numRows*numChannels*5, max);	*/
	
	// -------------------------
	// pack pattern (xm packing)
	// -------------------------
	j = z = b1 = 0;
	for (x=0; x < numRows; x++)
		for (y=0; y < numChannels; y++) 
		{
			//if (z > max)
			//	goto finishedPacking;
		
            memset(&pack,0,6);
            i=0;
            if (pattern[z]) 
			{
				b1=1;
				pack[0]|=1;
				pack[i+1]=pattern[z];
				i++;
            }
            if (pattern[z+1]) 
			{
				b1=1;
				pack[0]|=2;
				pack[i+1]=pattern[z+1];
				i++;
            }
            if (pattern[z+2]) 
			{
				b1=1;
				pack[0]|=4;
				pack[i+1]=pattern[z+2];
				i++;
            }
            if (pattern[z+3]) 
			{
				b1=1;
				pack[0]|=8;
				pack[i+1]=pattern[z+3];
				i++;
            }
            if (pattern[z+4]) 
			{
				b1=1;
				pack[0]|=16;
				pack[i+1]=pattern[z+4];
				i++;
            }
            if (i<5) 
			{
				pack[0]|=128;
				memcpy(outputPattern+j,&pack,i+1);
				j+=i+1;
            }
            else 
			{
				memcpy(outputPattern+j,pattern+z,5);
				j+=5;
            }

            z+=5;

		}

//finishedPacking:	
	return j;
}

static void sort(mp_sword* array,mp_sint32 l, mp_sint32 r)
{
	mp_sint32 i,j;
	mp_sword x,y;
	i=l; j=r; x=array[(l+r)/2];
	do 
	{
		while (array[i]<x) i++;
		while (x<array[j]) j--;
		if (i<=j) 
		{
			y=array[i]; array[i]=array[j]; array[j]=y;
			i++; j--;
		}
	} while (i<=j);
	if (l<j) sort(array,l,j);
	if (i<r) sort(array,i,r);
}

mp_sint32 XModule::saveExtendedModule(const SYSCHAR* fileName)
{
	mp_sint32 i,j,k,l;
	
	TWorkBuffers workBuffers;
	workBuffers.bpm = header.speed;
	workBuffers.speed = header.tempo;
	
	// ------ prerequisites ---------------------------------
	
	// step one, find last used pattern
	mp_sint32 patNum = getNumUsedPatterns();
	if (!patNum)
		patNum++;

	// step two, find last used instrument
	mp_sint32 insNum = getNumUsedInstruments();
	if (!insNum)
		insNum++;
	
	// ------ start ---------------------------------
	XMFile f(fileName, true);
	
	if (!f.isOpenForWriting())
		return MP_DEVICE_ERROR;

	f.write("Extended Module: ",1,17);
	
	char titleBuffer[MP_MAXTEXT+1], titleBufferTemp[MP_MAXTEXT+1];
	memset(titleBuffer, 0, sizeof(titleBuffer));
	memset(titleBufferTemp, 0, sizeof(titleBufferTemp));
	convertStr(reinterpret_cast<char*>(titleBuffer), reinterpret_cast<char*>(header.name), MP_MAXTEXT);
	if (strlen(titleBuffer) > 20)
	{
		mp_sint32 i = 0;
		mp_sint32 len = (mp_sint32)strlen(titleBuffer);
		while (titleBuffer[i] <= ' ' && i < len)
			i++;
			
		memcpy(titleBufferTemp, titleBuffer+i, strlen(titleBuffer) - i);
		f.write(titleBufferTemp, 1, 20);
	}
	else
		f.write(header.name, 1, 20);
	header.whythis1a = 0x1a;
	f.writeByte(header.whythis1a);
#ifdef MILKYTRACKER
#include "../tracker/version.h"
	f.write(MILKYTRACKER_VERSION_STRING, 1, 20);
#else
	f.write(header.tracker, 1, 20);
#endif
	header.ver = 0x104;
	f.writeWord(header.ver);
	header.hdrsize = 276;
	f.writeDword(header.hdrsize);
	f.writeWord(header.ordnum);
	f.writeWord(header.restart);
	
	mp_uword numChannels = header.channum&1 ? header.channum+1 : header.channum;
	
	f.writeWord(numChannels);
	f.writeWord(patNum);
	f.writeWord(insNum);
	f.writeWord(header.freqtab);
	f.writeWord(header.tempo);
	f.writeWord(header.speed);
	f.write(header.ord,1,256);

	mp_ubyte lowerNoteBound[256];
	mp_ubyte upperNoteBound[256];

	for (i = 0; i < 256; i++)
	{
		lowerNoteBound[i] = XModule::NOTE_LAST;
		upperNoteBound[i] = 0;
	}

	mp_ubyte* lastIns = new mp_ubyte[header.channum];
	memset(lastIns, 0, header.channum);

	for (i = 0; i < patNum; i++)
	{
		TXMPattern* srcPattern = &phead[i];
		
		mp_sint32 channum = srcPattern->channum >= header.channum ? header.channum : srcPattern->channum;
		
		for (mp_sint32 rows = 0; rows < srcPattern->rows; rows++)
			for (mp_sint32 c = 0;  c < channum; c++)
			{
				
				mp_ubyte* srcSlot = srcPattern->patternData+(rows*(srcPattern->channum*(srcPattern->effnum*2+2)) + c*(srcPattern->effnum*2+2));
			
				if (srcSlot[1])
					lastIns[c] = srcSlot[1];
			
				if (lastIns[c] && srcSlot[0] && srcSlot[0] < XModule::NOTE_OFF)
				{
					if (srcSlot[0] > upperNoteBound[lastIns[c]-1]) upperNoteBound[lastIns[c]-1] = srcSlot[0];
					if (srcSlot[0] < lowerNoteBound[lastIns[c]-1]) lowerNoteBound[lastIns[c]-1] = srcSlot[0];						
				}
			}
				
	}
	
	delete[] lastIns;
	
	for (i = 0; i < insNum; i++)
	{
		mp_sint32 remapper = 0;
		if (upperNoteBound[i] > 96)
			remapper = upperNoteBound[i] - 96;
		
		if (remapper < lowerNoteBound[i])
			workBuffers.noteRangeRemapper[i] = -remapper;

#ifdef VERBOSE
		printf("%i - %i (%i)\n", lowerNoteBound[i], upperNoteBound[i], workBuffers.noteRangeRemapper[i]);
#endif
	}

	for (i = 0; i < patNum; i++)
	{
		mp_sint32 numRows = phead[i].rows;
		
		if (numRows == 0)
			numRows = 1;
	
		mp_sint32 patChNum = (phead[i].channum+(phead[i].channum&1));
		
		if (patChNum < numChannels)
			patChNum = numChannels;

		mp_sint32 len = numRows * patChNum * 5;

		mp_ubyte* srcPattern = new mp_ubyte[len];
		mp_ubyte* dstPattern = new mp_ubyte[len];

		memset(srcPattern, 0, len);
		memset(dstPattern, 0, len);

		convertPattern(this, &phead[i], srcPattern, numChannels, workBuffers, false);		
	
		len = packPattern(srcPattern, dstPattern, numRows, numChannels);
	
#ifdef VERBOSE
		printf("Uncompressed pattern size: %i, compressed: %i\n", numRows * numChannels * 5, len);
#endif
	
		f.writeDword(9);
		f.writeByte(0);
		f.writeWord(numRows);
		f.writeWord(len);
		
		f.write(dstPattern, 1, len);
		
		//printf("Packed Size: %i\n", len);
		
		delete[] srcPattern;
		delete[] dstPattern;
	}
	
	for (i = 0; i < insNum; i++)
	{
			//mp_sint32 maxSample = instr[i].samp - 1;
			if (instr[i].samp > 0)
			{	
				mp_sword usedSamples[256];
				memset(usedSamples, 0, sizeof(usedSamples));
				mp_sint32 numUsedSamples = 0;

#ifdef MILKYTRACKER	
				if (type == ModuleType_XM && header.smpnum == header.insnum*16)
				{
					// save all samples within an instrument rather than the
					// used ones (referenced by note mapping)
					for (j = 0; j < 16; j++)
						usedSamples[j] = i*16+j;
					
					numUsedSamples = 0;
					
					// find last used sample in instrument
					for (j = 15; j >= 0; j--)
					{
						mp_sint32 index = usedSamples[j];
						char buffer[MP_MAXTEXT+1];
						convertStr(buffer, reinterpret_cast<char*>(smp[index].name), MP_MAXTEXT);
						if (strlen(buffer) || smp[index].samplen)
						{
							numUsedSamples = j+1;
							break;
						}
					}
				}
				else
#endif
				{
					// find referenced samples in instrument and save those
					for (j = 0; j < 96; j++)
					{
						mp_sword s = instr[i].snum[j];
						
						if (s == -1)
							continue;
						
						bool used = false;
						for (k = 0; k < numUsedSamples; k++)
						{
							if (usedSamples[k] == s)
							{
								used = true;
								break;
							}
						}
						if (!used && smp[s].sample && smp[s].samplen)
							usedSamples[numUsedSamples++] = s;
					}
					
					sort(usedSamples, 0, numUsedSamples-1);
				}
				
				f.writeDword(numUsedSamples > 0 ? 263 : 29);
				f.write(&instr[i].name,1,22);		
				f.writeByte(0);
				f.writeWord(numUsedSamples);
				
				if (!numUsedSamples)
					continue;
				
				f.writeDword(40);

				mp_ubyte nbu[96];

				for (j = 0; j < 96; j++)
				{
					mp_sword s = instr[i].snum[j];
					
					for (k = 0; k < numUsedSamples; k++)
						if (usedSamples[k] == s)
						{
							nbu[j] = k;
							break;
						}
				}
				
				f.write(nbu, 1, 96);
				
				mp_sint32 venvIndex = -1;
				mp_sint32 penvIndex = -1;

				for (j = 0; j < numUsedSamples; j++)
				{
					if (smp[usedSamples[j]].venvnum && venvIndex == -1)
						venvIndex = smp[usedSamples[j]].venvnum - 1;

					if (smp[usedSamples[j]].penvnum && penvIndex == -1)
						penvIndex = smp[usedSamples[j]].penvnum - 1;

				}
				
#ifdef VERBOSE
				printf("%i, %i\n", venvIndex, penvIndex);
#endif
				mp_sint32 step = 0;

				if (venvIndex >= 0 && venvIndex < header.volenvnum)
				{
					step = venvs[venvIndex].num > 12 ? venvs[venvIndex].num*256 / 12 : 256;
					l = 0;
					for (k = 0; k < 12; k++)
					{
						f.writeWord(venvs[venvIndex].env[l>>8][0]);
						f.writeWord(venvs[venvIndex].env[l>>8][1]>>2);
						l+=step;
					}
				}
				else
				{
					// emptyness
					for (k = 0; k < 12; k++)
					{
						f.writeWord(0);
						f.writeWord(0);
					}
				}
				if (penvIndex >= 0 && penvIndex < header.panenvnum)
				{
					step = penvs[penvIndex].num > 12 ? penvs[penvIndex].num*256 / 12 : 256;
					l = 0;
					for (k = 0; k < 12; k++)
					{
						f.writeWord(penvs[penvIndex].env[l>>8][0]);
						f.writeWord(penvs[penvIndex].env[l>>8][1]>>2);
						l+=step;
					}
				}
				else
				{
					// emptyness
					for (k = 0; k < 12; k++)
					{
						f.writeWord(0);
						f.writeWord(0);
					}
				}
				
				if (venvIndex >= 0 && venvIndex < header.volenvnum)
					f.writeByte(venvs[venvIndex].num > 12 ? 12 : venvs[venvIndex].num);	// number of volume points
				else
					f.writeByte(0);	// number of volume points

				if (penvIndex >= 0 && penvIndex < header.panenvnum)
					f.writeByte(penvs[penvIndex].num > 12 ? 12 : penvs[penvIndex].num);	// number of panning points
				else
					f.writeByte(0);	// number of panning points

				if (venvIndex >= 0 && venvIndex < header.volenvnum)
				{
					f.writeByte(venvs[venvIndex].sustain); // volume sustain point
					f.writeByte(venvs[venvIndex].loops); // volume start point
					f.writeByte(venvs[venvIndex].loope); // volume end point
				}
				else
				{
					f.writeByte(0);
					f.writeByte(0);
					f.writeByte(0);
				}
				
				if (penvIndex >= 0 && penvIndex < header.panenvnum)
				{
					f.writeByte(penvs[penvIndex].sustain); // panning sustain point
					f.writeByte(penvs[penvIndex].loops); // panning start point
					f.writeByte(penvs[penvIndex].loope); // panning end point
				}
				else
				{
					f.writeByte(0);
					f.writeByte(0);
					f.writeByte(0);
				}
				
				if (venvIndex >= 0 && venvIndex < header.volenvnum)
					f.writeByte(venvs[venvIndex].type); // volume type
				else
					f.writeByte(0);
					
				if (penvIndex >= 0 && penvIndex < header.panenvnum)
					f.writeByte(penvs[penvIndex].type); // panning type
				else
					f.writeByte(0);

				// take rest of the instrument info from first sample in the instrument
				// will probably not work for exported .MDL songs.
				// solution might be to create one instrument for each
				// sample and remap instrument field in the patterns
				l = usedSamples[0];

				f.writeByte(smp[l].vibtype); // vibrato type
				f.writeByte(smp[l].vibsweep); // vibrato sweep
				f.writeByte(smp[l].vibdepth>>1); // vibrato depth
				f.writeByte(smp[l].vibrate); // vibrato rate
				
				if (instr[i].flags & TXMInstrument::IF_ITFADEOUT)
					f.writeWord(instr[i].volfade>>1); // volume fadeout
				else
					f.writeWord(smp[l].volfade>>1); // volume fadeout

				f.writeWord(0); // reserved
				
				mp_ubyte extra[20];
				memset(extra, 0, sizeof(extra));
				f.write(extra, 1, 20);
				
				for (j = 0; j < numUsedSamples; j++)
				{
					k = usedSamples[j];
				
					f.writeDword((smp[k].type&16) ? smp[k].samplen <<1 : smp[k].samplen);
					f.writeDword((smp[k].type&16) ? smp[k].loopstart << 1 : smp[k].loopstart);
					f.writeDword((smp[k].type&16) ? smp[k].looplen << 1 : smp[k].looplen);

					mp_sint32 relnote = smp[k].relnote - workBuffers.noteRangeRemapper[i] + header.relnote;
					if (relnote<-96) relnote = -96;
					if (relnote>95) relnote = 95;

					mp_sint32 finetune = smp[k].finetune;
					
					// make some ULT linear finetune to finetune & relative note num approximation
					if (smp[k].freqadjust != 0)
					{
						mp_sint32 c4spd = getc4spd(relnote,finetune);
						c4spd+=((c4spd*smp[k].freqadjust)/32768);
						
						mp_sbyte rn,ft;
						convertc4spd(c4spd, &ft, &rn);
						finetune = ft;
						relnote = rn;
					}
					
					f.writeByte(smp[k].vol*64/255);  
					f.writeByte((mp_sbyte)finetune);
					
					mp_ubyte type = smp[k].type;
					// Only alowed bits 0,1 and 3
					type &= 3+16;
					// Bits 0 and 1 are not allowed to be set at the same time
					if ((type & 3) == 3) type &= ~1;
					
					f.writeByte(type);
					f.writeByte((smp[k].flags & 2) ? smp[k].pan : 0x80);
										
					f.writeByte((mp_sbyte)relnote);
					f.writeByte(0);
					
					f.write(smp[k].name, 1, 22);
				}
				
				for (j = 0; j < numUsedSamples; j++)
				{
					k = usedSamples[j];
					
					if (!smp[k].sample)
						continue;
					
					if (smp[k].type & 16)
					{
						mp_sword* packedSampleData = new mp_sword[smp[k].samplen];
						
						mp_sword b1,b2,b3;
						
						b1=0;
						for (mp_uint32 j = 0; j < smp[k].samplen; j++) 
						{
							b3 = smp[k].getSampleValue(j);
							b2 = b3-b1;
							packedSampleData[j] = b2;
							b1 = b3;
						}
						
						f.writeWords((mp_uword*)packedSampleData, smp[k].samplen);
						
						delete[] packedSampleData;
						
					}
					else
					{
						mp_sbyte* packedSampleData = new mp_sbyte[smp[k].samplen];
						
						mp_sbyte b1,b2,b3;
						
						b1=0;
						for (mp_uint32 j = 0; j < smp[k].samplen; j++) 
						{
							b3 = smp[k].getSampleValue(j);
							b2 = b3-b1;
							packedSampleData[j] = b2;
							b1 = b3;
						}
						
						f.write(packedSampleData, 1, smp[k].samplen);
						
						delete[] packedSampleData;
					}
				}
			}
			else
			{
				f.writeDword(instr[i].samp > 0 ? 263 : 29);
				f.write(&instr[i].name,1,22);		
				f.writeByte(0);
				f.writeWord(instr[i].samp);
			}
	
	}
	

	return MP_OK;
}

static mp_uword swap(mp_uword x)
{
	return (x>>8)+((x&255)<<8);
}

static mp_uword prep(mp_sint32 v)
{
	const int MAXSIZE = 0x1ffff;
	if (v&1) v++;
	if (v > MAXSIZE)
		v = MAXSIZE;

	return (mp_uword)(v >> 1);
}

mp_sint32 XModule::saveProtrackerModule(const SYSCHAR* fileName)
{
	static const mp_sint32 periods[12] = {1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907};

	static const mp_sint32 originalPeriods[] = {1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,906,
												856,808,762,720,678,640,604,570,538,508,480,453,
											    428,404,381,360,339,320,302,285,269,254,240,226,
												214,202,190,180,170,160,151,143,135,127,120,113,
												107,101,95,90,85,80,75,71,67,63,60,56};

	TWorkBuffers workBuffers;

	XMFile f(fileName, true);

	if (!f.isOpenForWriting())
		return MP_DEVICE_ERROR;

	f.write(header.name,1,20);
	
	mp_sint32 i,j,k;
	
// - instruments -------------------------------------------
	for (i = 0; i < 31; i++) 
	{
		f.write(instr[i].name, 1, 22);

		// sample seems to be used
		if (instr[i].samp)
		{
			mp_sint32 s = -1;
			for (j = 0; j < 120; j++)
				if (instr[i].snum[j] >= 0)
				{
					s = instr[i].snum[j];
					break;
				}
				
			if (s == -1)
				goto unused;
				
			mp_sint32 fti = (mp_sint32)smp[s].finetune + 128;
			if (!(fti & 0xF) && !smp[s].relnote)
			{
				k = (((mp_uint32)(fti-128)) >> 4) & 0xF;
			}
			else
			{
				mp_sint32 c4spd = getc4spd(smp[s].relnote, smp[s].finetune);
				
				mp_sint32 dc4 = abs(sfinetunes[0]-c4spd);
				
				k = 0;
				for (j = 1; j < 16; j++)
					if (abs(sfinetunes[j]-c4spd) < dc4)
					{
						dc4 = abs(sfinetunes[j]-c4spd);
						k = j;
					}
			}
				
			f.writeWord(swap(prep(smp[s].samplen)));

			f.writeByte(k);
			f.writeByte((mp_ubyte)(((mp_sint32)smp[s].vol*64)/255));
							
			if (smp[s].type & 3)
			{
				mp_sint32 loopend = /*smp[s].loopstart + */smp[s].looplen;
				
				if (smp[s].type & 32)
				{
					f.writeWord(0);
				}
				else
				{
					if (!smp[s].loopstart && smp[s].looplen != smp[s].samplen)
					{
						f.writeWord(swap(1));
					}
					else if (!smp[s].loopstart && smp[s].looplen == smp[s].samplen)
					{
						f.writeWord(0);
					}
					else
						f.writeWord(swap(prep(smp[s].loopstart)));
				}
				
				f.writeWord(swap(prep(loopend)));
			}
			else
			{
				f.writeWord(swap(0));
				f.writeWord(swap(1));
			}
			
		}
		else
		{
unused:
			f.writeWord(swap(0));
			f.writeByte(0);
			f.writeByte(0);
			f.writeWord(swap(0));
			f.writeWord(swap(1));
		}
				
	}
	
// - orderlist -------------------------------------------
	f.writeByte((mp_ubyte)header.ordnum);
	
	f.writeByte(127);
	
	mp_ubyte ord[128];
	
	memset(ord, 0, sizeof(ord));
	
	j = 0;
	for (i = 0; i < 128; i++)
	{
		if (header.ord[i] < 254)
			ord[j++] = header.ord[i];
		else if (header.ord[i] == 255)
			break;
	}
			
	f.write(ord, 1, 128);
	
	mp_uword numChannels = header.channum&1 ? header.channum+1 : header.channum;	

	if (numChannels < 1 || numChannels > 99)
		return MP_UNSUPPORTED;
	
// - patterns -------------------------------------------
	mp_sint32 numPatterns = 0;
	for (i = 0; i < 128; i++)
	{
		if (ord[i] > numPatterns)
			numPatterns = ord[i];
	}

	char modMagic[4];
	if(numChannels == 4)
	{
		// ProTracker may not load files with more than 64 patterns correctly if we do not specify the M!K! magic.
		if(numPatterns <= 63)
			memcpy(modMagic, "M.K.", 4);
		else
			memcpy(modMagic, "M!K!", 4);
	} else if(numChannels < 10)
	{
		memcpy(modMagic, "0CHN", 4);
		modMagic[0] += static_cast<char>(numChannels);
	} else
	{
		memcpy(modMagic, "00CH", 4);
		modMagic[0] += static_cast<char>(numChannels / 10u);
		modMagic[1] += static_cast<char>(numChannels % 10u);
	}
	f.write(modMagic, 1, 4);
	
	for (i = 0; i < numPatterns+1; i++)
	{
		mp_sint32 numRows = phead[i].rows;
		
		if (numRows == 0)
			numRows = 1;

		mp_sint32 patChNum = (phead[i].channum+(phead[i].channum&1));
		
		if (patChNum < numChannels)
			patChNum = numChannels;

		mp_sint32 len = numRows * patChNum * 5;
		mp_sint32 lenDst = (numRows < 64 ? 64 : numRows) * patChNum * 4;

		mp_ubyte* srcPattern = new mp_ubyte[len];
		mp_ubyte* dstPattern = new mp_ubyte[lenDst];
	
		memset(srcPattern, 0, len);
		memset(dstPattern, 0, lenDst);

		convertPattern(this, &phead[i], srcPattern, numChannels, workBuffers, false);		
	
		for (mp_sint32 r = 0; r < 64; r++)
			for (mp_sint32 c = 0; c < numChannels; c++)
			{
			
				if (r < numRows)
				{
				
					mp_sint32 srcIndex = (r*numChannels*5)+(c*5);
					mp_sint32 dstIndex = (r*numChannels*4)+(c*4);
					
					mp_sint32 period = 0;
					
					mp_ubyte note = srcPattern[srcIndex];
					
					//note = r+24+1;
					
					if (note)
					{
						note--;
						if (note >= 24 && note < 24+12*5)
							period = originalPeriods[note-24];
						else
							period = (periods[note%12]*16>>((note/12)))>>2;
					}
				
					mp_ubyte ins = srcPattern[srcIndex+1];
				
					if (ins > 31)
						ins = 0;
				
					mp_ubyte eff = 0;
					mp_ubyte op = 0;

					// First convert volume command to PT compatible effect again :)
					XModule::convertXMVolumeEffects(srcPattern[srcIndex+2], eff, op);
					mp_ubyte tmpEff = eff, tmpOp = op;
					convertEffect(tmpEff, tmpOp, eff, op, c, workBuffers, getType() == XModule::ModuleType_IT);

					// Having an effect? Overwrite what we already have...
					if (srcPattern[srcIndex+3] || srcPattern[srcIndex+4])
					{
						eff = srcPattern[srcIndex+3];
						op = srcPattern[srcIndex+4];
					}

					if (eff > 0x0f)
					{
						eff = op = 0;
					}
				
					/*if (srcPattern[srcIndex+2] >= 0x10 && srcPattern[srcIndex+2] <= 0x50 &&
						eff == 0 && op == 0)
					{
						eff = 0x0C;
						op = srcPattern[srcIndex+2] - 0x10;
					}*/
				
					dstPattern[dstIndex] = (ins & 0xF0) + ((period>>8)&0x0F);
					dstPattern[dstIndex+1] = (mp_ubyte)(period&0xFF);
					dstPattern[dstIndex+2] = ((ins & 0x0F) << 4) + (eff);
					dstPattern[dstIndex+3] = op;
					
				}
			
			}
			
		f.write(dstPattern, 4, numChannels*64);

		delete[] srcPattern;
		delete[] dstPattern;	
	}

	for (i = 0; i < header.smpnum; i++) 
	{
		mp_uint32 smplen = prep(smp[i].samplen) << 1;
		mp_uint32 j = 0;

		// Ensure first 2 bytes are zero in non-looping
		// samples (for Protracker/Amiga compatibility)
		if(!(smp[i].type & 0xef) && smplen >= 2)
		{
			f.writeWord(0);
			j = 2;;
		}

		if (smp[i].type & 16)
		{
			for (; j < smplen; j++)
				f.writeByte(smp[i].getSampleValue(j) >> 8);
		}
		else
		{
			for (; j < smplen; j++)
				f.writeByte(smp[i].getSampleValue(j));
		}
	}
	
	return MP_OK;
}

#ifdef MILKYTRACKER
bool TXMPattern::saveExtendedPattern(const SYSCHAR* fileName) const
{
	// Note: For FT2 compatibility, .XP files are fixed at 32 channels.
	// TODO:  Create a version 2 format for variable channel counts
	TWorkBuffers workBuffers;

	// ------ start ---------------------------------
	XMFile f(fileName, true);

	if (!f.isOpenForWriting())
		return false;

	f.writeWord(0x1);
	f.writeWord(rows);
	
	mp_sint32 len = rows * 32 * 5;
	
	mp_ubyte* srcPattern = new mp_ubyte[len];
	
	memset(srcPattern, 0, len);
	
	convertPattern(NULL, this, srcPattern, 32, workBuffers, false);
	
	f.write(srcPattern, 1, len);
	
	delete[] srcPattern;	
	
	return true;
}

bool TXMPattern::saveExtendedTrack(const SYSCHAR* fileName, mp_uint32 channel) const
{
	if (channel >= channum)
		return false;

	TWorkBuffers workBuffers;

	// ------ start ---------------------------------
	XMFile f(fileName, true);
	
	if (!f.isOpenForWriting())
		return false;
	
	f.writeWord(0x1);
	f.writeWord(rows);
	
	mp_sint32 len = rows * 5;
	
	mp_ubyte* srcPattern = new mp_ubyte[rows * channum * 5];
	mp_ubyte* dstPattern = new mp_ubyte[len];
	
	convertPattern(NULL, this, srcPattern, channum, workBuffers, false);
	
	for (mp_sint32 r = 0; r < rows; r++)
	{
		mp_sint32 srcIndex = r*this->channum*5 + channel*5;
		dstPattern[r*5] = srcPattern[srcIndex];
		dstPattern[r*5+1] = srcPattern[srcIndex+1];
		dstPattern[r*5+2] = srcPattern[srcIndex+2];
		dstPattern[r*5+3] = srcPattern[srcIndex+3];
		dstPattern[r*5+4] = srcPattern[srcIndex+4];
	}
	
	f.write(dstPattern, 1, len);
	
	delete[] srcPattern;	
	delete[] dstPattern;

	return true;
}

bool TXMPattern::loadExtendedPattern(const SYSCHAR* fileName)
{
	XMFile f(fileName);
	
	if (f.readWord() != 0x01)
		return false;
		
	mp_uword rows = f.readWord();
	
	if (rows == 0 || rows > 256)
		return false;
		
	mp_sint32 len = rows * 32 * 5;
	
	mp_ubyte* srcPattern = new mp_ubyte[len];
	
	f.read(srcPattern, 1, len);
	
	// throw away old pattern
	delete[] patternData;
	this->rows = rows;
	this->channum = 32;
	this->effnum = 2;
	
	patternData = new mp_ubyte[rows*channum*(effnum*2+2)];
	
	mp_ubyte* slot = srcPattern;
	
	mp_sint32 bc = 0;
	for (mp_sint32 r=0;r<rows;r++) {
		for (mp_sint32 c=0;c<channum;c++) {
			
			char gl=0;
			for (mp_sint32 i=0;i<XModule::numValidXMEffects;i++)
				if (slot[3]==XModule::validXMEffects[i]) gl=1;
			
			if (!gl) slot[3]=slot[4]=0;
			
			if ((slot[3]==0xC)||(slot[3]==0x10)) {
				slot[4] = XModule::vol64to255(slot[4]);
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
			
			patternData[bc]=slot[0];
			patternData[bc+1]=slot[1];
			
			XModule::convertXMVolumeEffects(slot[2], patternData[bc+2], patternData[bc+3]);
			
			patternData[bc+4]=slot[3];
			patternData[bc+5]=slot[4];
			
			bc+=6;
			slot+=5;
		} // for c
		
	} // for r
	
	
	delete[] srcPattern;	
		
	return true;
}

bool TXMPattern::loadExtendedTrack(const SYSCHAR* fileName, mp_uint32 channel)
{
	XMFile f(fileName);
	
	if (f.readWord() != 0x01)
		return false;
		
	mp_uword rows = f.readWord();
	
	if (rows == 0 || rows > 256)
		return false;

	if (rows > this->rows)
		rows = this->rows;
		
	mp_sint32 len = rows * 32 * 5;
	
	mp_ubyte* srcPattern = new mp_ubyte[len];
	
	f.read(srcPattern, 1, len);
	
	// throw away old pattern
	mp_ubyte* slot = srcPattern;
	
	mp_sint32 bc = 0, r;
	for (r=0;r<rows;r++) 
	{
		bc = r*this->channum*(this->effnum*2+2) + channel*(this->effnum*2+2);
	
		char gl=0;
		for (mp_sint32 i=0;i<XModule::numValidXMEffects;i++)
			if (slot[3]==XModule::validXMEffects[i]) gl=1;
		
		if (!gl) slot[3]=slot[4]=0;
		
		if ((slot[3]==0xC)||(slot[3]==0x10)) {
			slot[4] = XModule::vol64to255(slot[4]);
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
		
		patternData[bc]=slot[0];
		patternData[bc+1]=slot[1];
		
		XModule::convertXMVolumeEffects(slot[2], patternData[bc+2], patternData[bc+3]);
		
		patternData[bc+4]=slot[3];
		patternData[bc+5]=slot[4];
		
		slot+=5;
	} // for r
	
	for (r = rows; r < this->rows; r++)
	{
		bc = r*this->channum*(this->effnum*2+2) + channel*(this->effnum*2+2);
		memset(patternData + bc, 0, (this->effnum*2+2));
	}
	
	delete[] srcPattern;	
		
	return true;
}

#endif

mp_sint32 XModule::getNumUsedPatterns()
{
	mp_sint32 i;

	mp_sint32 patNum = header.patnum;
	for (i = header.patnum - 1; i > 0; i--)
	{
		TXMPattern* pattern = &phead[i];

		if (pattern->patternData == NULL)
			continue;
			
		mp_sint32 slotSize = pattern->effnum * 2 + 2;
		
		mp_sint32 patternSize = slotSize * pattern->channum * pattern->rows;
		
		bool empty = true;
		for (mp_sint32 j = 0; j < patternSize; j++)
			if (pattern->patternData[j])
			{
				empty = false;
				patNum = i+1;
				break;
			}
		
		if (empty)
		{
			bool found = false;
			for (mp_sint32 j = 0; j < header.ordnum; j++)
				if (header.ord[j] == i)
				{
					found = true;
					break;
				}
			
			if (found)
			{
				patNum = i+1;
				break;
			}			
		}
		else
		{
			patNum = i+1;
			break;
		}
	} 
	
	if (i == 0)
		return 0;
			
	return patNum;
}

mp_sint32 XModule::getNumUsedInstruments()
{
	mp_sint32 i;

	mp_sint32 insNum = header.insnum;
	for (i = header.insnum - 1; i > 0; i--)
	{
		mp_ubyte buffer[MP_MAXTEXT+1];
		
		convertStr(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(instr[i].name), MP_MAXTEXT, false);
		
		if (strlen((char*)buffer))
		{
			insNum = i+1;
			break;
		}
		
		if (instr[i].samp)
		{
		
			for (mp_sint32 j = 0; j < 120; j++)
			{
				mp_sint32 s = instr[i].snum[j];
				if (s >= 0)
				{
					convertStr(reinterpret_cast<char*>(buffer), reinterpret_cast<char*>(smp[s].name), MP_MAXTEXT, false);
					if (strlen((char*)buffer) || (smp[s].sample && smp[s].samplen))
					{
						insNum = i+1;
						goto insFound;
					}					
				}
			}		
		}
	}
	
insFound:
	if (i == 0)
		return 0;
		
	return insNum;
}
