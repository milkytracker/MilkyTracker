/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
