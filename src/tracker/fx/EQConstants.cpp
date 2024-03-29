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
#include "math.h"

#define p(e) ldexp(66904.0f, e-4*4)

const float EQConstants::EQ3bands[3] = 
{
	p(5),  // 32     Hz
	p(9),  // 522    Hz
	p(12), // 4181.5 Hz
};

const float EQConstants::EQ3bandwidths[3] = 
{
	170.0f,
	600.0f,
	1000.0f
};

const float EQConstants::EQ10bands[10] = 
{
	p(3),  // 8      Hz
	p(4),  // 16     Hz
	p(5),  // 32     Hz
	p(6),  // 65     Hz
	p(7),  // 130    Hz
	p(8),  // 261    Hz
	p(9),  // 522    Hz
	p(10), // 1     kHz
	p(11), // 2     kHz
	p(12), // 4181.5 Hz
};

const float EQConstants::EQ10bandwidths[10] = 
{
	16,
	p(3)*1.5f,
	p(4)*1.5f,
	p(5)*1.5f,
	p(6)*1.5f,
	p(7)*1.5f,
	p(8)*1.0f,
	p(9)*1.0f,
	600.0f,
	800.0f
};
