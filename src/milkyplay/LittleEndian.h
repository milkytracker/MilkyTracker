/*
 *  milkyplay/LittleEndian.h
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
 *  LittleEndian.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on Tue Oct 19 2004.
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
