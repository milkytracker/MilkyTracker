#include "LittleEndian.h"

//////////////////////////////////////////////////////////////////////////
// system independent loading of little endian numbers					//
//////////////////////////////////////////////////////////////////////////
mp_uword LittleEndian::GET_WORD(const void* ptr)
{
	const mp_ubyte* buffer = (const mp_ubyte*)ptr;
	return ((mp_uword)buffer[0]+((mp_uword)buffer[1]<<8));
}

mp_uint32 LittleEndian::GET_DWORD(const void* ptr)
{
	const mp_ubyte* buffer = (const mp_ubyte*)ptr;
	return ((mp_uint32)buffer[0]+((mp_uint32)buffer[1]<<8)+((mp_uint32)buffer[2]<<16)+((mp_uint32)buffer[3]<<24));
}

//////////////////////////////////////////////////////////////////////////
// system independent loading of big endian numbers						//
//////////////////////////////////////////////////////////////////////////
mp_uword BigEndian::GET_WORD(const void* ptr)
{
	const mp_ubyte* buffer = (const mp_ubyte*)ptr;
	return ((mp_uword)buffer[1]+((mp_uword)buffer[0]<<8));
}

mp_uint32 BigEndian::GET_DWORD(const void* ptr)
{
	const mp_ubyte* buffer = (const mp_ubyte*)ptr;
	return ((mp_uint32)buffer[3]+((mp_uint32)buffer[2]<<8)+((mp_uint32)buffer[1]<<16)+((mp_uint32)buffer[0]<<24));
}
