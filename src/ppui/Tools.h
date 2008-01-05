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
