/*
 *  ppui/osinterface/win32/PPSystemString_WIN32.h
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
#include <tchar.h>

typedef TCHAR SYSCHAR;

class PPSystemString
{
private:
	SYSCHAR* strBuffer;
	pp_uint32 allocatedSize;

	HRESULT __fastcall AnsiToUnicode(LPCSTR pszA, LPWSTR* ppszW)
	{

		ULONG cCharacters;
		DWORD dwError;

		// If input is null then just return the same.
		if (NULL == pszA)
		{
			*ppszW = NULL;
			return NOERROR;
		}

		// Determine number of wide characters to be allocated for the
		// Unicode string.
		cCharacters =  strlen(pszA)+1;

		// Use of the OLE allocator is required if the resultant Unicode
		// string will be passed to another COM component and if that
		// component will free it. Otherwise you can use your own allocator.
		*ppszW = (LPWSTR) malloc(cCharacters*2);
		if (NULL == *ppszW)
			return E_OUTOFMEMORY;

		// Covert to Unicode.
		if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters,
			*ppszW, cCharacters))
		{
			dwError = GetLastError();
			free(*ppszW);
			*ppszW = NULL;
			return HRESULT_FROM_WIN32(dwError);
		}

		return NOERROR;
	}

	void reAlloc(pp_uint32 newSize)
	{
		if (newSize <= allocatedSize)
			return;

		SYSCHAR* newStrBuffer = new SYSCHAR[newSize];
		memcpy(newStrBuffer, strBuffer, sizeof(SYSCHAR)*allocatedSize);

		delete[] strBuffer;
		strBuffer = newStrBuffer;
		allocatedSize = newSize;
	}


public:
	// Empty string
	PPSystemString()
	{
		strBuffer = new SYSCHAR[1];
		*strBuffer = 0;
		
		allocatedSize = 1;
	}

	// String from single character
	PPSystemString(SYSCHAR c)
	{
		strBuffer = new SYSCHAR[2];
		*strBuffer = c;
		*(strBuffer+1) = 0;
		
		allocatedSize = 2;
	}

	PPSystemString(const SYSCHAR* str)
	{
		strBuffer = new SYSCHAR[_tcslen(str) + 1];
		_tcscpy(strBuffer, str);
		
		allocatedSize = (pp_uint32)_tcslen(str) + 1;
	}

	PPSystemString(const char* str)
	{
		LPWSTR tempStr = NULL;
		if (AnsiToUnicode(str, &tempStr) == NOERROR)
		{
			strBuffer = new SYSCHAR[_tcslen(tempStr) + 1];
			allocatedSize = (pp_uint32)_tcslen(tempStr) + 1;
			_tcscpy(strBuffer, tempStr);
			free(tempStr);
		}
		else
		{
			strBuffer = new SYSCHAR[1];
			*strBuffer = 0;

			allocatedSize = 1;
		}
	}

	PPSystemString(const SYSCHAR* str, pp_uint32 length)
	{
		strBuffer = new SYSCHAR[length + 1];
		memcpy(strBuffer, str, length*sizeof(SYSCHAR));
		strBuffer[length] = 0;

		allocatedSize = length + 1;
	}

	// copy c'tor
	PPSystemString(const PPSystemString& str)
	{
		strBuffer = new SYSCHAR[str.allocatedSize];
		memcpy(strBuffer, str.strBuffer, str.allocatedSize*sizeof(SYSCHAR));
		allocatedSize = str.allocatedSize;
	}

	operator const SYSCHAR*() const
	{
		return strBuffer;
	}

	const SYSCHAR* getStrBuffer() const
	{
		return strBuffer;
	}

	// assignment operator
	PPSystemString& operator=(const PPSystemString& str)
	{
		if (this != &str)
		{
			delete[] strBuffer;			
			strBuffer = new SYSCHAR[str.allocatedSize];
			memcpy(strBuffer, str.strBuffer, str.allocatedSize*sizeof(SYSCHAR));
			allocatedSize = str.allocatedSize;
		}
	
		return *this;
	}

	PPSystemString& operator=(const char* str)
	{
		delete[] strBuffer;			
		allocatedSize = (pp_uint32)strlen(str)+1;
		strBuffer = new SYSCHAR[allocatedSize];
		
		for (pp_uint32 i = 0; i < allocatedSize; i++)
			strBuffer[i] = str[i];
		
		return *this;
	}

	// comparison is necessary too
	bool operator==(const PPSystemString& str) const
	{
		return _tcscmp(strBuffer, str.strBuffer) == 0;
	}

	bool operator!=(const PPSystemString& str) const
	{
		return _tcscmp(strBuffer, str.strBuffer) != 0;
	}

	pp_int32 compareTo(const PPSystemString& str) const
	{
		return _tcscmp(strBuffer, str.strBuffer);
	}

	pp_int32 compareToNoCase(const PPSystemString& str) const
	{
		return _tcsicmp(strBuffer, str.strBuffer);
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
		return (pp_uint32)_tcslen(strBuffer);
	}

	void insertAt(pp_uint32 i, const PPSystemString& s)
	{
		// doesn't work
		if (i > length())
			return;

		allocatedSize = length() + s.length() + 1;

		SYSCHAR* newStr = new SYSCHAR[allocatedSize];
		
		memcpy(newStr, strBuffer, i*sizeof(SYSCHAR));
		memcpy(newStr + i, s.strBuffer, s.length()*sizeof(SYSCHAR));
		memcpy(newStr + i + s.length(), strBuffer + i, (length() - i)*sizeof(SYSCHAR));
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

		SYSCHAR* newStr = new SYSCHAR[length() - numChars + 1];
		
		allocatedSize = length() - numChars + 1;

		memcpy(newStr, strBuffer, i*sizeof(SYSCHAR));
		memcpy(newStr + i, strBuffer + i + numChars, (length() - i - numChars)*sizeof(SYSCHAR));
		newStr[length() - numChars] = 0;

		delete[] strBuffer;

		strBuffer = newStr;
	}

	void replace(const PPSystemString& str)
	{
		delete[] strBuffer;
		strBuffer = new SYSCHAR[str.allocatedSize];
		memcpy(strBuffer, str.strBuffer, str.allocatedSize*sizeof(SYSCHAR));
		allocatedSize = str.allocatedSize;
	}
	
	PPSystemString stripPath() const
	{
		SYSCHAR* ptr = strBuffer+_tcslen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '\\')
			ptr--;
			
		if (ptr != strBuffer)
			ptr++;
			
		PPSystemString str = ptr;
		return str;
	}

	PPSystemString stripExtension() const
	{
		SYSCHAR* ptr = strBuffer+_tcslen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '.' && *ptr != '\\')
			ptr--;

		if (*ptr == '\\')
			return strBuffer;
			
		if (ptr != strBuffer)
		{	
			PPSystemString str;
		
			delete[] str.strBuffer;
			str.allocatedSize = (pp_uint32)((ptr-strBuffer)+1);
			str.strBuffer = new SYSCHAR[str.allocatedSize];
			memcpy(str.strBuffer, strBuffer, (ptr-strBuffer)*sizeof(SYSCHAR));
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
		SYSCHAR* ptr = strBuffer+_tcslen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '.' && *ptr != '/')
			ptr--;
		
		if (*ptr != '.')
			return _T("");
			
		return ptr;
	}

	pp_int32 compareExtensions(const PPSystemString& str) const	
	{
		SYSCHAR* ptrSrc = strBuffer+_tcslen(strBuffer);
		
		while (ptrSrc > strBuffer && *ptrSrc != '.' && *ptrSrc != '\\')
			ptrSrc--;
		
		bool noExt1 = false;
		
		if (*ptrSrc != '.')
			noExt1 = true;

		ptrSrc++;
		if (*ptrSrc == '\0')
			noExt1 = true;

		SYSCHAR* ptrDst = str.strBuffer+_tcslen(str.strBuffer);
		
		while (ptrDst > str.strBuffer && *ptrDst != '.' && *ptrDst != '\\')
			ptrDst--;
		
		if (*ptrDst != '.')
			return noExt1 ? 0 : 1;

		ptrDst++;
		if (*ptrDst == '\0')
			return noExt1 ? 0 : 1;

		return _tcsicmp(ptrSrc, ptrDst);
	}

	
	bool compareToExtension(const PPSystemString& extension) const	
	{
		SYSCHAR* ptrSrc = strBuffer+_tcslen(strBuffer);
		
		while (ptrSrc > strBuffer && *ptrSrc != '.' && *ptrSrc != '\\')
			ptrSrc--;
		
		if (*ptrSrc != '.')
			return false;

		ptrSrc++;
		if (*ptrSrc == '\0')
			return false;

		return _tcsicmp(ptrSrc, extension.strBuffer) == 0;
	}

	void ensureTrailingCharacter(char chr)
	{
		pp_uint32 len = length();
		if (len)
		{
			SYSCHAR* ptr = strBuffer+(len-1);
			if (*ptr != (SYSCHAR)chr)
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

	// Delete this pointer after usage
	char* toASCIIZ() const
	{
		char* newStr = new char[length() + 1];
		
		SYSCHAR* ptr = strBuffer;

		pp_int32 i = 0;
		while (*ptr)
		{
			newStr[i] = (char)(*ptr++);
			i++;
		}

		newStr[i] = '\0';

		return newStr;
	}
};

#endif
