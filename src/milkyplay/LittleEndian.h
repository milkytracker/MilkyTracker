/*
 *  LittleEndian.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Tue Oct 19 2004.
 *  Copyright (c) 2004 milkytracker.net, All rights reserved.
 *
 */
#ifndef __LITTLEENDIAN_H__
#define __LITTLEENDIAN_H__

#include "MilkyPlayTypes.h"

class LittleEndian
{
public:
	static mp_uword			GET_WORD(const void* ptr);
	static mp_uint32		GET_DWORD(const void* ptr);
};

class BigEndian
{
public:
	static mp_uword			GET_WORD(const void* ptr);
	static mp_uint32		GET_DWORD(const void* ptr);
};

#endif
