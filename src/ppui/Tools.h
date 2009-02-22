/*
 *  ppui/Tools.h
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

#ifndef PPUI_TOOLS__H
#define PPUI_TOOLS__H

#include "BasicTypes.h"
#include "SimpleVector.h"

class PPTools
{
private:
	static const char hex[];

public:
	static pp_uint32 getHexNumDigits(pp_uint32 value);
	static void convertToHex(char* name, pp_uint32 value, pp_uint32 numDigits);
	static pp_uint32 getDecNumDigits(pp_uint32 value);
	static void convertToDec(char* name, pp_uint32 value, pp_uint32 numDigits);

	// you're responsible for deleting this vector
	static PPSimpleVector<PPString>* extractStringList(const PPString& str); 

	static pp_uint8 getNibble(const char* str);	
	static pp_uint8 getByte(const char* str);
	static pp_uint16 getWord(const char* str);
	static pp_uint32 getDWord(const char* str);	
	
	static PPString encodeByteArray(const pp_uint8* array, pp_uint32 size);	
	static bool decodeByteArray(pp_uint8* array, pp_uint32 size, const PPString& str);
};

#endif
