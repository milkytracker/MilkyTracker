/*
 *  tracker/EQConstants.cpp
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

#include "EQConstants.h"

const float EQConstants::EQ3bands[3] = 
{
	0x1.0558p+5f,  // 32     Hz
	0x1.0558p+9f,  // 522    Hz
	0x1.0558p+12f, // 4181.5 Hz
};

const float EQConstants::EQ3bandwidths[3] = 
{
	170.0f,
	600.0f,
	1000.0f
};

const float EQConstants::EQ10bands[10] = 
{
	0x1.0558p+3f,  // 8      Hz
	0x1.0558p+4f,  // 16     Hz
	0x1.0558p+5f,  // 32     Hz
	0x1.0558p+6f,  // 65     Hz
	0x1.0558p+7f,  // 130    Hz
	0x1.0558p+8f,  // 261    Hz
	0x1.0558p+9f,  // 522    Hz
	0x1.0558p+10f, // 1     kHz
	0x1.0558p+11f, // 2     kHz
	0x1.0558p+12f, // 4181.5 Hz
};

const float EQConstants::EQ10bandwidths[10] = 
{
	16,
	0x1.0558p+3f*1.5f,
	0x1.0558p+4f*1.5f,
	0x1.0558p+5f*1.5f,
	0x1.0558p+6f*1.5f,
	0x1.0558p+7f*1.5f,
	0x1.0558p+8f*1.0f,
	0x1.0558p+9f*1.0f,
	600.0f,
	800.0f
};
