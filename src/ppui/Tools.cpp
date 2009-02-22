/*
 *  ppui/Tools.cpp
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

#include "Tools.h"

const char PPTools::hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

pp_uint32 PPTools::getHexNumDigits(pp_uint32 value)
{
	if (value == 0 || (signed)value < 0)
		return 1;

	pp_uint32 i = 0;
	while (value >> i*4)
		i++;

	return i;
}

void PPTools::convertToHex(char* name, pp_uint32 value, pp_uint32 numDigits)
{
	pp_uint32 i;
	for (i = 0; i < numDigits; i++)
		name[i] = hex[((value>>(numDigits-1-i)*4)&0xF)];
	
	name[i] = 0;
}

pp_uint32 PPTools::getDecNumDigits(pp_uint32 value)
{
	if (value == 0)
		return 1;

	pp_uint32 i = 0;
	pp_uint32 start = 1;
	while (value / start)
	{
		i++;
		start*=10;
	}

	return i;
}

void PPTools::convertToDec(char* name, pp_uint32 value, pp_uint32 numDigits)
{
	pp_uint32 i;
	pp_uint32 start = 1;
	for (i = 0; i < numDigits-1; i++)
		start*=10;
		
	for (i = 0; i < numDigits; i++)
	{
		name[i] = hex[(value / start)%10];
		start/=10;
	}

	name[i] = 0;
}

// you're responsible for deleting this vector
PPSimpleVector<PPString>* PPTools::extractStringList(const PPString& str)
{
	PPSimpleVector<PPString>* stringList = new PPSimpleVector<PPString>();
	
	const char* sz = str;
	
	PPString* line = new PPString();
	
	while (*sz)
	{
		if (*sz == '\n')
		{
			stringList->add(line);
			line = new PPString();
		}
		else
		{
			line->append(*sz);
		}
		sz++;
	}
	
	if (!line->length())
		delete line;
	else
		stringList->add(line);
	
	return stringList;
}

PPString PPTools::encodeByteArray(const pp_uint8* array, pp_uint32 size)
{
	char buffer[10];
	
	// Convert number of bytes
	convertToHex(buffer, size, 8);
	
	PPString str = buffer;
	
	for (pp_uint32 i = 0; i < size; i++)
	{
		convertToHex(buffer, array[i], 2);
		str.append(buffer);
	}
	
	return str;
}

pp_uint8 PPTools::getNibble(const char* str)
{
	if (*str >= '0' && *str <= '9')
		return (*str - '0');
	if (*str >= 'A' && *str <= 'F')
		return (*str - 'A' + 10);
	if (*str >= 'a' && *str <= 'f')
		return (*str - 'a' + 10);
	
	return 0;
}

pp_uint8 PPTools::getByte(const char* str)
{
	return (getNibble(str)<<4) + getNibble(str+1);
}

pp_uint16 PPTools::getWord(const char* str)
{
	return (getByte(str)<<8) + getByte(str+2);
}

pp_uint32 PPTools::getDWord(const char* str)
{
	return (getWord(str)<<16) + getWord(str+4);
}

bool PPTools::decodeByteArray(pp_uint8* array, pp_uint32 size, const PPString& str)
{
	const char* ptr = str;
	
	pp_uint32 length = getDWord(ptr);
	
	if (length != size)
		return false;
	
	ptr+=8;
	
	for (pp_uint32 i = 0; i < length; i++)
	{
		*array++ = getByte(ptr);
		ptr+=2;
	}
	
	return true;
}

