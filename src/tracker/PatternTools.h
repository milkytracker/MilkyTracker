/*
 *  tracker/PatternTools.h
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

#ifndef PATTERNTOOLS__H
#define PATTERNTOOLS__H

#include "BasicTypes.h"

#define TONOTE(octave, note) \
((((pp_uint8)(octave)*12 + (pp_uint8)(note)) + 1) < 97 ? (((pp_uint8)(octave)*12 + (pp_uint8)(note)) + 1) : -1)

class PatternTools
{
private:
	pp_int32 offset;
	pp_int32 currentEffectIndex;
	struct TXMPattern* pattern;
	pp_int32 lockOffset;

public:
	PatternTools() :
		offset(0),
		currentEffectIndex(0),
		pattern(NULL)
	{
	}

	PatternTools(const PatternTools& src)
	{
		offset = src.offset;
		currentEffectIndex = src.currentEffectIndex;
		pattern = src.pattern;
	}

	void setPosition(TXMPattern* pattern, pp_uint32 channel, pp_uint32 row);
	pp_int32 getNote();
	void setNote(pp_uint32);
	pp_int32 getInstrument();
	void setInstrument(pp_uint32);

	void getFirstEffect(pp_int32& effect, pp_int32& operand);
	void getNextEffect(pp_int32& effect, pp_int32& operand);
	void setFirstEffect(pp_int32 effect, pp_int32 operand);
	void setNextEffect(pp_int32 effect, pp_int32 operand);

	void getEffect(pp_int32 currentEffectIndex, pp_int32& effect, pp_int32& operand);
	void setEffect(pp_int32 currentEffectIndex, pp_int32 effect, pp_int32 operand);

	static void convertEffectsToFT2(pp_int32& eff, pp_int32& op);
	static void convertEffectsFromFT2(pp_int32& eff, pp_int32& op);
	static pp_int32 getVolumeFromEffect(pp_int32 effect, pp_int32 operand);

	static void convertVolumeToEffect(pp_int32 vol, pp_int32& eff, pp_int32& op);

	static void getNoteName(char* name, pp_uint32 note, bool terminate = true);
	static void getVolumeName(char* name, pp_uint32 volume);
	static void getEffectName(char* name, pp_uint32 effect);

	static pp_uint32 getHexNumDigits(pp_uint32 value);
	static void convertToHex(char* name, pp_uint32 value, pp_uint32 numDigits);
	static pp_uint32 getDecNumDigits(pp_uint32 value);
	static void convertToDec(char* name, pp_uint32 value, pp_uint32 numDigits);
	
	static pp_uint8 getNoteOffNote();	
	static pp_uint32 normalizeVol(pp_uint32 volume);
};

#endif
