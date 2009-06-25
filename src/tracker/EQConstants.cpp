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

/*  
 * 10 BandsEQ fix by Kmuland
 * ------------------------------------
 *
 * 12000.0f*0.25f means that the band frequency of 16000.0f have a 
 * gaussian bell of 12000hz centered at 16000hz of frequency. 
 * The 16000hz band affect to freqs between 10000hz and 22000hz.
 *
 * The *0.25f is the Q of each band. A  *0.25f creates a very thin bell arround the
 * center frequency. Higher values like 0.5f create a wider gaussian bell.
 *
 * Note about 31.25hz band: subfreqs. You need subwoofer equipment to 
 * notice changes. This band at the 19.43hz on left tail of bell is not audible. 
 * In the right tail of the bell, the 42,97hz is boosted lightly. If you need extra
 * boost of sub freqs 30-40hz increase the *0.25f value.
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
	23.44f*0.25f,
	46.87f*0.30f,
	93.75f*0.35f,
	187.5f*0.40f,
	375.0f*0.45f,
	750.0f*0.50f,
	1500.0f*0.55f,
	3000.0f*0.50f,
	6000.0f*0.30f,
	12000.0f*0.25f
};
