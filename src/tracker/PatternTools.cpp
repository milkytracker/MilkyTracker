/*
 *  tracker/PatternTools.cpp
 *
 *  Copyright 2009 Peter Barth
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

///////////////////////////////////////////////////////////////
// Handle with care
///////////////////////////////////////////////////////////////
#include "PatternTools.h"
#include "MilkyPlay.h"

static const char* noteNames[12] = {"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};
static const char hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

/*TXMPattern* PatternTools::pattern = NULL;
pp_int32 PatternTools::currentEffectIndex = 0;
pp_int32 PatternTools::offset = 0;

pp_int32 PatternTools::lockOffset = 0;*/

void PatternTools::setPosition(TXMPattern* pattern, pp_uint32 channel, pp_uint32 row)
{
	if (channel >= pattern->channum ||
		row >= pattern->rows)
		offset = -1;
	else
	{
		pp_uint32 slotSize = pattern->effnum*2 + 2;
		
		offset = (slotSize*pattern->channum)*row + (channel*slotSize);
		
		PatternTools::pattern = pattern;
	}
}

pp_int32 PatternTools::getNote()
{
	if (offset < 0)
		return 0;
		
	return *(pattern->patternData + offset);
}

void PatternTools::setNote(pp_uint32 note)
{
	if (offset < 0)
		return;

	*(pattern->patternData + offset) = (pp_uint8)note;
}

pp_int32 PatternTools::getInstrument()
{
	if (offset < 0)
		return 0;

	return *(pattern->patternData + offset + 1);
}

void PatternTools::setInstrument(pp_uint32 instrument)
{
	if (offset < 0)
		return;

	*(pattern->patternData + offset + 1) = (pp_uint8)instrument;
}

void PatternTools::getFirstEffect(pp_int32& effect, pp_int32& operand)
{
	if (offset < 0)
	{
		effect = operand = 0;
		return;
	}

	currentEffectIndex = 0;
	
	effect = *(pattern->patternData + offset + 2 + currentEffectIndex*2);
	
	operand = *(pattern->patternData + offset + 2 + currentEffectIndex*2 + 1);
}

void PatternTools::getNextEffect(pp_int32& effect, pp_int32& operand)
{
	if (offset < 0)
	{
		effect = operand = 0;
		return;
	}

	currentEffectIndex++;
	
	effect = *(pattern->patternData + offset + 2 + currentEffectIndex*2);
	
	operand = *(pattern->patternData + offset + 2 + currentEffectIndex*2 + 1);
}

void PatternTools::setFirstEffect(pp_int32 effect, pp_int32 operand)
{
	if (offset < 0)
		return;

	currentEffectIndex = 0;
	
	*(pattern->patternData + offset + 2 + currentEffectIndex*2) = (pp_uint8)effect;
	
	*(pattern->patternData + offset + 2 + currentEffectIndex*2 + 1) = (pp_uint8)operand;
}

void PatternTools::setNextEffect(pp_int32 effect, pp_int32 operand)
{
	if (offset < 0)
		return;

	currentEffectIndex++;
	
	*(pattern->patternData + offset + 2 + currentEffectIndex*2) = (pp_uint8)effect;
	
	*(pattern->patternData + offset + 2 + currentEffectIndex*2 + 1) = (pp_uint8)operand;
}

void PatternTools::setEffect(pp_int32 currentEffectIndex, pp_int32 effect, pp_int32 operand)
{
	if (offset < 0)
		return;

	*(pattern->patternData + offset + 2 + currentEffectIndex*2) = (pp_uint8)effect;
	
	*(pattern->patternData + offset + 2 + currentEffectIndex*2 + 1) = (pp_uint8)operand;
}

void PatternTools::getEffect(pp_int32 currentEffectIndex, pp_int32& effect, pp_int32& operand)
{
	if (offset < 0)
	{	
		effect = operand = 0;
		return;
	}

	effect = *(pattern->patternData + offset + 2 + currentEffectIndex*2);
	
	operand = *(pattern->patternData + offset + 2 + currentEffectIndex*2 + 1);
}

void PatternTools::convertEffectsToFT2(pp_int32& eff, pp_int32& op)
{
	if (eff == 0 && op == 0)
		return;
	
	pp_uint32 effIn = eff;
	pp_uint32 opIn = op;	
	
	pp_uint32 effOut = 0;
	pp_uint32 opOut = 0;

	// Protracker commands
	if (effIn > 0 && effIn <= 0x11)
	{
		effOut = effIn;
		opOut = opIn;
	
		if (effIn == 0x0C || effIn == 0x10)
			opOut = (mp_ubyte)(((mp_sint32)opOut*64)/255);
	}
	// panning slide
	else if (effIn == 0x14 || effIn == 0x51)
	{
		effOut = 0x14;
		opOut = opIn;
	}
	// panning slide
	else if (effIn == 0x15)
	{
		effOut = effIn;
		opOut = opIn;
	}
	// set tempo
	else if (effIn == 0x16)
	{
		if (opIn)
		{
			if (opIn < 32) opIn = 32;
			effOut = 0x0F;
			opOut = opIn;
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
		if (opIn)
		{
			effOut = 0;
			opOut = opIn;
		}
			
	}
	// extra fine porta commands
	else if (effIn == 0x41)
	{
		effOut = 0x21;
		opOut = 0x10 + (opIn&0xF);
	}
	else if (effIn == 0x42)
	{
		effOut = 0x21;
		opOut = 0x20 + (opIn&0xF);
	}
	// MDL porta up
	else if (effIn == 0x43)
	{
		if (opIn >= 0xE0) {
			mp_ubyte y = (mp_ubyte)(opIn>>4);
			mp_ubyte x = (mp_ubyte)(opIn&0xf);
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
		else if (opIn)
		{
			effOut = 0x1;
			opOut = opIn;
		}
	}
	// MDL porta down
	else if (effIn == 0x44)
	{
		if (opIn >= 0xE0) {
			mp_ubyte y = (mp_ubyte)(opIn >> 4);
			mp_ubyte x = (mp_ubyte)(opIn & 0xf);
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
		else if (opIn)
		{
			effOut = 0x2;
			opOut = opIn;
		}
	}
	// MDL volslide up
	else if (effIn == 0x45)
	{
		if (opIn >= 0xE0) {
			mp_ubyte y = (mp_ubyte)(opIn>>4);
			mp_ubyte x = (mp_ubyte)(opIn&0xf);
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
		else if (opIn)
		{
			effOut = 0xA;
			opOut = (opIn>>2)<<4;
		}
	}
	// MDL volslide down
	else if (effIn == 0x46)
	{
		if (opIn >= 0xE0) {
			mp_ubyte y = (mp_ubyte)(opIn >> 4);
			mp_ubyte x = (mp_ubyte)(opIn & 0xf);
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
		else if (opIn)
		{
			effOut = 0xA;
			opOut = (opIn>>2)&0xF;
		}
	}
	// S3M porta up
	else if (effIn == 0x47) 
	{
		if (opIn >= 0xE0) {
			mp_ubyte y = (mp_ubyte)(opIn >> 4);
			mp_ubyte x = (mp_ubyte)(opIn & 0xf);
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
		else if (opIn)
		{
			effOut = 0x1;
			opOut = opIn;
		}
	}
	// S3M porta down
	else if (effIn == 0x48) 
	{
		if (opIn >= 0xE0) {
			mp_ubyte y = (mp_ubyte)(opIn >> 4);
			mp_ubyte x = (mp_ubyte)(opIn & 0xf);
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
		else if (opIn)
		{
			effOut = 0x2;
			opOut = opIn;
		}
	}
	// S3M volslide
	else if (effIn == 0x49) 
	{
		if (opIn) 
		{
			mp_ubyte y = (mp_ubyte)(opIn>>4);
			mp_ubyte x = (mp_ubyte)(opIn&0xf);
			
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

	eff = effOut;
	op = opOut;

}

void PatternTools::convertEffectsFromFT2(pp_int32& eff, pp_int32& op)
{
	bool f = false;
	for (pp_uint32 i = 0; i < XModule::numValidXMEffects; i++)
		if (eff == XModule::validXMEffects[i]) 
			f = true;

	if (!f)
	{
		eff = op = 0;
		return;
	}

	if ((eff==0xC)||(eff==0x10)) 
	{
		op = XModule::vol64to255(op);
	}
	
	else if ((!eff)&&(op)) 
		eff=0x20;
	
	else if (eff==0xE) 
	{
		eff=(op>>4)+0x30;
		op=op&0xf;
	}
	
	else if (eff==0x21) 
	{
		eff=(op>>4)+0x40;
		op=op&0xf;
	}
	
}
	
pp_int32 PatternTools::getVolumeFromEffect(pp_int32 eff, pp_int32 op)
{
	
	mp_ubyte vol = 0;

	// set volume
	if (eff == 0x0C)
	{
		vol = (mp_ubyte)(0x10 + op);
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
			vol = 0x60 + (mp_ubyte)(op&0xF);
		}
		// volslide up
		else if (op >> 4)
		{
			vol = 0x70 + (mp_ubyte)(op>>4);
		}
		
	}
	// extra fine volslide up
	else if (eff == 0xE && ((op>>4) == 0xA))
	{
		vol = 0x90 + (mp_ubyte)(op & 0xF);
	}
	// extra fine volslide down
	else if (eff == 0xE && ((op>>4) == 0xB))
	{
		vol = 0x80 + (mp_ubyte)(op & 0xF);
	}
	// extra vibrato
	else if (eff == 0x4)
	{
		if ((op>>4) && !(op&0xF))
		{
			vol = 0xA0 + (mp_ubyte)(op>>4);
		}
		else if (!(op>>4)/* && (op&0xF)*/)
		{
			vol = 0xB0 + (mp_ubyte)op;
		}
	}
	// set panning
	else if (eff == 0x8)
	{
		vol = 0xC0 + (mp_ubyte)(op>>4);
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
			vol = 0xD0 + (mp_ubyte)(op&0xF);
		}
		// panning slide right
		else if (op >> 4)
		{
			vol = 0xE0 + (mp_ubyte)(op>>4);
		}
	}
	// porta to note
	else if (eff == 0x03)
	{
		vol = 0xF0 + (mp_ubyte)(op>>4);
	}
	
	return vol;
}

void PatternTools::getNoteName(char* name, pp_uint32 note, bool terminate /* = true*/)
{
	
	if (note == 0)
	{
		strcpy(name, "\xf4\xf4\xf4");
		return;
	}
	else if ( note >= getNoteOffNote())
	{
		strcpy(name, "\xf1\xf2\xf3");
		return;
	}

	memcpy(name, noteNames[(note-1)%12], 2);
	name[2] = '0' + (mp_ubyte)((note-1)/12);
	if (terminate)
		name[3] = 0;
}

void PatternTools::getVolumeName(char* name, pp_uint32 volume)
{
	if (volume<0x10)
	{
		name[0] = name[1] = '\xf4';
		name[2] = 0; 
	}
	else if (volume>=0x10 && volume<=0x50)
	{
		PatternTools::convertToHex(name, volume-0x10, 2);
	}
	else 
	{
		name[2] = 0; 
		name[1] = hex[volume&0xF];
		switch (volume>>4)
		{
			case 0x6:
				name[0] = '-';
				break;
			case 0x7:
				name[0] = '+';
				break;
			case 0x8:
				name[0] = (char)253;
				break;
			case 0x9:
				name[0] = (char)254;
				break;
			case 0xA:
				name[0] = 'S';
				break;
			case 0xB:
				name[0] = 'V';
				break;
			case 0xC:
				name[0] = 'P';
				break;
			case 0xD:
				name[0] = '<';
				break;
			case 0xE:
				name[0] = '>';
				break;
			case 0xF:
				name[0] = 'M';
				break;
		}
	}
}

void PatternTools::getEffectName(char* name, pp_uint32 effect)
{
	name[1] = 0;
	if (effect > sizeof(hex))
	{
		name[0] = (char)0xfa;
		return;
	}
	name[0] = hex[effect];	
}

pp_uint32 PatternTools::getHexNumDigits(pp_uint32 value)
{
	if (value == 0)
		return 1;

	pp_uint32 i = 0;
	while (value >> i*4)
		i++;

	return i;
}

void PatternTools::convertToHex(char* name, pp_uint32 value, pp_uint32 numDigits)
{
	pp_uint32 i = 0;
	for (i = 0; i < numDigits; i++)
		name[i] = hex[((value>>(numDigits-1-i)*4)&0xF)];
	
	name[i] = 0;
}

pp_uint32 PatternTools::getDecNumDigits(pp_uint32 value)
{
	if (value == 0)
		return 1;

	pp_uint32 i = 0;
	pp_uint32 dec = 1;
	while (value / dec)
	{
		i++;
		dec*=10;
	}

	return i;
}

void PatternTools::convertToDec(char* name, pp_uint32 value, pp_uint32 numDigits)
{
	pp_int32 i = 0;
	pp_int32 dec = 1;
	for (i = 0; i < (signed)numDigits-1; i++)
		dec*=10;

	for (i = 0; i < (signed)numDigits; i++)
	{
		name[i] = hex[(value/dec)%10];
		dec/=10;
	}
	
	name[i] = 0;
}

void PatternTools::convertVolumeToEffect(pp_int32 vol, pp_int32& effect, pp_int32& operand)
{
	effect = operand = 0;

	if (vol>=0x10&&vol<=0x50) 
	{
		effect = 0x0C;
		operand = XModule::vol64to255(vol-0x10);
		return;
	}
	
	if (vol>=0x60) 
	{
		mp_ubyte eff = (mp_ubyte)(vol>>4);
		mp_ubyte op  = (mp_ubyte)(vol&0xf);
		
		switch (eff) 
		{
		case 0x6 : 
			eff=0x0A;
			op=op;
			break;
			
		case 0x7 : 
			eff=0x0A;
			op=op<<4;
			break;
			
		case 0x8 : 
			eff=0x3B;
			op=op;
			break;
			
		case 0x9 : 
			eff=0x3A;
			op=op;
			break;
			
		case 0xA : 
			eff=0x4;
			op=op<<4;
			break;
			
		case 0xB : 
			eff=0x4;
			op=op;
			break;
			
		case 0xC : 
			eff=0x8;
			op=op*0x11;
			break;
			
		case 0xD : 
			eff=0x19;
			op=op;
			break;
			
		case 0xE : 
			eff=0x19;
			op=op<<4;
			break;
			
		case 0xF : 
			eff=0x3;
			op=op<<4;
			break;
			
		}

		effect = eff;
		operand = op;

	}
	
}

pp_uint8 PatternTools::getNoteOffNote() { return XModule::NOTE_OFF; }
	
pp_uint32 PatternTools::normalizeVol(pp_uint32 volume) { return XModule::vol64to255((volume*64)/255); }
