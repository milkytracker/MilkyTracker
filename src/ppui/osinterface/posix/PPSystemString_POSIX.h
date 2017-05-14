/*
 *  ppui/osinterface/posix/PPSystemString_POSIX.h
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

/*
 *  PPSystemString.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Apr 03 2005.
 *
 */

#ifndef PPSYSTEMSTRING__H
#define PPSYSTEMSTRING__H

#include "BasicTypes.h"

class PPSystemString
{
private:
	char* strBuffer;
	pp_uint32 allocatedSize;

	void reAlloc(pp_uint32 newSize)
	{
		if (newSize <= allocatedSize)
			return;

		char* newStrBuffer = new char[newSize];
		memcpy(newStrBuffer, strBuffer, allocatedSize);

		delete[] strBuffer;
		strBuffer = newStrBuffer;
		allocatedSize = newSize;
	}


public:
	// Empty string
	PPSystemString() :
		strBuffer(new char[1]),
		allocatedSize(1)
	{
		*strBuffer = 0;		
	}

	// String from single character
	PPSystemString(char c) :
		strBuffer(new char[2]),
		allocatedSize(2)
	{
		*strBuffer = c;
		*(strBuffer+1) = 0;
	}

	PPSystemString(const char* str) :
		strBuffer(new char[strlen(str) + 1]),
		allocatedSize(static_cast<pp_uint32> (strlen(str) + 1))
	{
		strcpy(strBuffer, str);		
	}

	PPSystemString(const char* str, pp_uint32 length) :
		strBuffer(new char[length + 1]),
		allocatedSize(length + 1)
	{
		memcpy(strBuffer, str, length);
		strBuffer[length] = 0;
	}

	// copy c'tor
	PPSystemString(const PPSystemString& str) :
		strBuffer(new char[str.allocatedSize]),
		allocatedSize(str.allocatedSize)
	{
		memcpy(strBuffer, str.strBuffer, str.allocatedSize);
	}

	operator const char*() const
	{
		return strBuffer;
	}

	const char* getStrBuffer() const
	{
		return strBuffer;
	}

	// Delete this pointer after usage
	char* toASCIIZ() const
	{
		char* newStr = new char[length() + 1];
		strcpy(newStr, strBuffer);
		return newStr;
	}

	// assignment operator
	PPSystemString& operator=(const PPSystemString& str)
	{
		if (this != &str)
		{
			delete[] strBuffer;			
			strBuffer = new char[str.allocatedSize];
			memcpy(strBuffer, str.strBuffer, str.allocatedSize);
			allocatedSize = str.allocatedSize;
		}
	
		return *this;
	}

	// comparison is necessary too
	bool operator==(const PPSystemString& str) const
	{
		return strcmp(strBuffer, str.strBuffer) == 0;
	}

	bool operator!=(const PPSystemString& str) const
	{
		return strcmp(strBuffer, str.strBuffer) != 0;
	}

	pp_int32 compareTo(const PPSystemString& str) const
	{
		return strcmp(strBuffer, str.strBuffer);
	}

	pp_int32 compareToNoCase(const PPSystemString& str) const
	{
		return strcasecmp(strBuffer, str.strBuffer);
	}
	
	void toUpper()
	{
		for (pp_uint32 i = 0; i < length(); i++)
			if (strBuffer[i] >= 'a' && 
				strBuffer[i] <= 'z')
				strBuffer[i] -= 'a'-'A';
	}

	~PPSystemString()
	{
		delete[] strBuffer;
	}

	pp_uint32 length() const
	{
		return static_cast<pp_uint32> (strlen(strBuffer));
	}

	void insertAt(pp_uint32 i, const PPSystemString& s)
	{

		// doesn't work
		if (i > length())
			return;

		char* newStr = new char[length() + s.length() + 1];
		
		allocatedSize = length() + s.length() + 1;

		memcpy(newStr, strBuffer, i);
		memcpy(newStr + i, s.strBuffer, s.length());
		memcpy(newStr + i + s.length(), strBuffer + i, length() - i);
		newStr[length() + s.length()] = 0;

		delete[] strBuffer;

		strBuffer = newStr;
	}

	void append(const PPSystemString& s)
	{
		insertAt(length(), s);
	}

	void deleteAt(pp_uint32 i, pp_uint32 numChars)
	{

		// not possible
		if (i > length())
			return;

		// nothing to delete
		if ((signed)length() - (signed)numChars < 0)
			return;

		// nothing to delete
		if (strBuffer[i] == 0)
			return;

		char* newStr = new char[length() - numChars + 1];
		
		allocatedSize = length() - numChars + 1;

		memcpy(newStr, strBuffer, i);
		memcpy(newStr + i, strBuffer + i + numChars, length() - i - numChars);
		newStr[length() - numChars] = 0;

		delete[] strBuffer;

		strBuffer = newStr;
	}

	void replace(const PPSystemString& str)
	{
		delete[] strBuffer;
		strBuffer = new char[str.allocatedSize];
		memcpy(strBuffer, str.strBuffer, str.allocatedSize);
		allocatedSize = str.allocatedSize;
	}
	
	PPSystemString stripPath() const
	{
		char* ptr = strBuffer+strlen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '/')
			ptr--;
			
		if (ptr != strBuffer)
			ptr++;
			
		PPSystemString str = ptr;
		return str;
	}

	PPSystemString stripExtension() const
	{
		char* ptr = strBuffer+strlen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '.' && *ptr != '/')
			ptr--;
		
		if (*ptr == '/')
			return strBuffer;
			
		if (ptr != strBuffer)
		{	
			PPSystemString str;
		
			delete[] str.strBuffer;
			str.allocatedSize = static_cast<pp_uint32> (ptr-strBuffer+1);
			str.strBuffer = new char[str.allocatedSize];
			memcpy(str.strBuffer, strBuffer, (ptr-strBuffer));
			str.strBuffer[(ptr-strBuffer)] = '\0';
		
			return str;
		}
		else
		{
			return ptr;
		}
	}

	PPSystemString getExtension() const
	{
		char* ptr = strBuffer+strlen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '.' && *ptr != '/')
			ptr--;
		
		if (*ptr != '.')
			return "";
			
		return ptr;
	}

	pp_int32 compareExtensions(const PPSystemString& str) const	
	{
		char* ptrSrc = strBuffer+strlen(strBuffer);
		
		while (ptrSrc > strBuffer && *ptrSrc != '.' && *ptrSrc != '/')
			ptrSrc--;
		
		bool noExt1 = false;
		
		if (*ptrSrc != '.')
			noExt1 = true;

		ptrSrc++;
		if (*ptrSrc == '\0')
			noExt1 = true;

		char* ptrDst = str.strBuffer+strlen(str.strBuffer);
		
		while (ptrDst > str.strBuffer && *ptrDst != '.' && *ptrDst != '/')
			ptrDst--;
		
		if (*ptrDst != '.')
			return noExt1 ? 0 : 1;

		ptrDst++;
		if (*ptrDst == '\0')
			return noExt1 ? 0 : 1;

		return strcasecmp(ptrSrc, ptrDst);
	}

	
	bool compareToExtension(const PPSystemString& extension) const	
	{
		char* ptrSrc = strBuffer+strlen(strBuffer);
		
		while (ptrSrc > strBuffer && *ptrSrc != '.' && *ptrSrc != '/')
			ptrSrc--;
		
		if (*ptrSrc != '.')
			return false;

		ptrSrc++;
		if (*ptrSrc == '\0')
			return false;

		return strcasecmp(ptrSrc, extension.strBuffer) == 0;
	}
	
	void ensureTrailingCharacter(char chr)
	{
		pp_uint32 len = length();
		if (len)
		{
			char* ptr = strBuffer+(len-1);
			if (*ptr != chr)
				append(chr);
		}
	}
	
	bool startsWith(const PPSystemString& src) const
	{
		pp_uint32 srcLen = src.length();
		pp_uint32 thisLen = length();
	
		if (srcLen > thisLen)
			return false;
			
		if (srcLen == 0)
			return true;
			
		if (srcLen == thisLen)
			return compareTo(src) == 0;
			
		for (pp_uint32 i = 0; i < srcLen; i++)
			if (src.strBuffer[i] != strBuffer[i])
				return false;
				
		return true;
	}
};

#endif
