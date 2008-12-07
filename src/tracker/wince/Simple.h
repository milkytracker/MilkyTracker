// Simple vector container and string wrapper classes
#ifndef __SIMPLE_H__
#define __SIMPLE_H__

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////////////////////
// very simple vector class
/////////////////////////////////////////////////////////////////////////////////////////////
template<class Type>
class SimpleVector
{
private:
	Type** values;
	int numValuesAllocated;
	int numValues;
	bool destroy;

	// don't assign this class please
	SimpleVector& operator=(const SimpleVector& src)
	{
		return *this;
	}

public:
	SimpleVector(int initialSize = 0, bool destroy = true)
	{
		this->destroy = destroy;

		if (initialSize == 0)
			initialSize = 16;

		numValuesAllocated = initialSize;

		if (initialSize)
			values = new Type*[initialSize];
		else
			values = 0;

		numValues = 0;
	}

	~SimpleVector()
	{
	
		if (values)
		{
			if (destroy)
				for (int i = 0; i < numValues; i++)
					delete values[i];

			delete[] values;
		}

	}

	SimpleVector* const clone()
	{
		SimpleVector* clonedVector = new SimpleVector(numValuesAllocated, true);
		
		for (int i = 0; i < numValues; i++)
		{
			clonedVector->values[i] = new Type(*values[i]);
		}
		
		clonedVector->numValues = numValues;
		return clonedVector;
	}

	void clear()
	{
		if (values)
		{
			if (destroy)
				for (int i = 0; i < numValues; i++)
					delete values[i];

			numValues = 0;
		}
	}

	bool remove(int index)
	{
		if (!numValues)
			return false;
			
		if (index < 0 || index >= numValues)
			return false;
			
		if (destroy)
			delete values[index];

		for (int i = index; i < numValues-1; i++)
			values[i] = values[i+1];
			
		numValues--;
		return true;
	}

	void add(Type* value)
	{
	
		if (numValues >= numValuesAllocated)
		{
			
			numValuesAllocated += 16;

			Type** values = new Type*[numValuesAllocated];

			for (int i = 0; i < numValues; i++)
			{
				values[i] = this->values[i];
			}

			delete[] this->values;
		
			this->values = values;
		}
	
		values[numValues++] = value;
	
	}

	Type* get(int index) const
	{
		if (index < numValues)
		{
			return values[index];
		}
		else
			return 0;
	}

	int size() const { return numValues; }

	bool isEmpty() const { return numValues == 0; }

};

/////////////////////////////////////////////////////////////////////////////////////////////
// quick and dirty string wrapper class
// lacks serious overflow checks, so use with care only
/////////////////////////////////////////////////////////////////////////////////////////////
class SimpleString
{
private:
	TCHAR* strBuffer;
	unsigned int allocatedSize;

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
		cCharacters =  (ULONG)strlen(pszA)+1;

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

	void reAlloc(unsigned int newSize)
	{
		if (newSize <= allocatedSize)
			return;

		TCHAR* newStrBuffer = new TCHAR[newSize];
		memcpy(newStrBuffer, strBuffer, sizeof(TCHAR)*allocatedSize);

		delete[] strBuffer;
		strBuffer = newStrBuffer;
		allocatedSize = newSize;
	}


public:
	// Empty string
	SimpleString()
	{
		strBuffer = new TCHAR[1];
		*strBuffer = 0;
		
		allocatedSize = 1;
	}

	// String from single character
	SimpleString(TCHAR c)
	{
		strBuffer = new TCHAR[2];
		*strBuffer = c;
		*(strBuffer+1) = 0;
		
		allocatedSize = 2;
	}

	SimpleString(const TCHAR* str)
	{
		strBuffer = new TCHAR[_tcslen(str) + 1];
		_tcscpy(strBuffer, str);
		
		allocatedSize = (unsigned int)_tcslen(str) + 1;
	}

	SimpleString(const char* str)
	{
		LPWSTR tempStr = NULL;
		if (AnsiToUnicode(str, &tempStr) == NOERROR)
		{
			strBuffer = new TCHAR[_tcslen(tempStr) + 1];
			allocatedSize = (unsigned int)_tcslen(tempStr) + 1;
			_tcscpy(strBuffer, tempStr);
			free(tempStr);
		}
		else
		{
			strBuffer = new TCHAR[1];
			*strBuffer = 0;

			allocatedSize = 1;
		}
	}

	SimpleString(const TCHAR* str, unsigned int length)
	{
		strBuffer = new TCHAR[length + 1];
		memcpy(strBuffer, str, length*sizeof(TCHAR));
		strBuffer[length] = 0;

		allocatedSize = length + 1;
	}

	// copy c'tor
	SimpleString(const SimpleString& str)
	{
		strBuffer = new TCHAR[str.allocatedSize];
		memcpy(strBuffer, str.strBuffer, str.allocatedSize*sizeof(TCHAR));
		allocatedSize = str.allocatedSize;
	}

	operator const TCHAR*() const
	{
		return strBuffer;
	}

	TCHAR& operator[] (unsigned int i) { if (i >= length()) return strBuffer[length()]; return strBuffer[i]; }

	const TCHAR* getStrBuffer() const
	{
		return strBuffer;
	}

	// assignment operator
	SimpleString& operator=(const SimpleString& str)
	{
		if (this != &str)
		{
			delete[] strBuffer;			
			strBuffer = new TCHAR[str.allocatedSize];
			memcpy(strBuffer, str.strBuffer, str.allocatedSize*sizeof(TCHAR));
			allocatedSize = str.allocatedSize;
		}
	
		return *this;
	}

	SimpleString& operator=(const char* str)
	{
		delete[] strBuffer;			
		allocatedSize = (unsigned int)strlen(str)+1;
		strBuffer = new TCHAR[allocatedSize];
		
		for (unsigned int i = 0; i < allocatedSize; i++)
			strBuffer[i] = str[i];
		
		return *this;
	}

	// comparison is necessary too
	bool operator==(const SimpleString& str) const
	{
		return _tcscmp(strBuffer, str.strBuffer) == 0;
	}

	bool operator!=(const SimpleString& str) const
	{
		return _tcscmp(strBuffer, str.strBuffer) != 0;
	}

	int compareTo(const SimpleString& str) const
	{
		return _tcscmp(strBuffer, str.strBuffer);
	}

	void toUpper()
	{
		for (unsigned int i = 0; i < length(); i++)
			if (strBuffer[i] >= 'a' && 
				strBuffer[i] <= 'z')
				strBuffer[i] -= 'a'-'A';
	}

	~SimpleString()
	{
		delete[] strBuffer;
	}

	unsigned int length() const
	{
		return (unsigned int)_tcslen(strBuffer);
	}

	void insertAt(unsigned int i, const SimpleString& s)
	{

		// doesn't work
		if (i > length())
			return;

		allocatedSize = length() + s.length() + 1;

		TCHAR* newStr = new TCHAR[allocatedSize];
		
		memcpy(newStr, strBuffer, i*sizeof(TCHAR));
		memcpy(newStr + i, s.strBuffer, s.length()*sizeof(TCHAR));
		memcpy(newStr + i + s.length(), strBuffer + i, (length() - i)*sizeof(TCHAR));
		newStr[length() + s.length()] = 0;

		delete[] strBuffer;

		strBuffer = newStr;
	}

	void append(const SimpleString& s)
	{
		insertAt(length(), s);
	}

	void deleteAt(unsigned int i, unsigned int numChars)
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

		TCHAR* newStr = new TCHAR[length() - numChars + 1];
		
		allocatedSize = length() - numChars + 1;

		memcpy(newStr, strBuffer, i*sizeof(TCHAR));
		memcpy(newStr + i, strBuffer + i + numChars, (length() - i - numChars)*sizeof(TCHAR));
		newStr[length() - numChars] = 0;

		delete[] strBuffer;

		strBuffer = newStr;
	
	
	}

	void replace(const SimpleString& str)
	{
		delete[] strBuffer;
		strBuffer = new TCHAR[str.allocatedSize];
		memcpy(strBuffer, str.strBuffer, str.allocatedSize*sizeof(TCHAR));
		allocatedSize = str.allocatedSize;
	}
	
	SimpleString stripPath() const
	{
		TCHAR* ptr = strBuffer+_tcslen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '\\')
			ptr--;
			
		if (ptr != strBuffer)
			ptr++;
			
		SimpleString str = ptr;
		return str;
	}

	SimpleString stripExtension() const
	{
		TCHAR* ptr = strBuffer+_tcslen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '.' && *ptr != '\\')
			ptr--;

		if (*ptr == '\\')
			return strBuffer;
			
		if (ptr != strBuffer)
		{	
			SimpleString str;
		
			delete[] str.strBuffer;
			str.allocatedSize = (unsigned int)((ptr-strBuffer)+1);
			str.strBuffer = new TCHAR[str.allocatedSize];
			memcpy(str.strBuffer, strBuffer, (ptr-strBuffer)*sizeof(TCHAR));
			str.strBuffer[(ptr-strBuffer)] = '\0';
		
			return str;
		}
		else
		{
			return ptr;
		}
	}

	SimpleString getExtension() const
	{
		TCHAR* ptr = strBuffer+_tcslen(strBuffer);
		
		while (ptr > strBuffer && *ptr != '.' && *ptr != '/')
			ptr--;
		
		if (*ptr != '.')
			return _T("");
			
		return ptr;
	}
};


#endif
