/*
 *  milkyplay/LittleEndian.cpp
 *
 *  Copyright 2008 Peter Barth
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
