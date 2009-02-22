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
	80.0f,
	2500.0f,
	12000.0f
};

const float EQConstants::EQ3bandwidths[3] = 
{
	2500.0f*0.25f,
	2500.0f*0.5f,
	2500.0f
};

const float EQConstants::EQ10bands[10] = 
{
	31.25f,
	62.5f,
	125.0f,
	250.0f,
	500.0f,
	1000.0f,
	2000.0f,
	4000.0f,
	8000.0f,
	16000.0f
};

const float EQConstants::EQ10bandwidths[10] = 
{
	15.625f*0.25f,
	31.25f*0.25f,
	62.5f*0.25f,
	125.0f*0.25f,
	250.0f*0.25f,
	500.0f*0.25f,
	1000.0f*0.25f,
	2000.0f*0.25f,
	4000.0f*0.25f,
	5000.0f*0.25f
};
