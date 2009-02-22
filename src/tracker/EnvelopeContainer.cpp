/*
 *  tracker/EnvelopeContainer.cpp
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
 *  EnvelopeContainer.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 20.06.05.
 *
 */

#include "EnvelopeContainer.h"
#include "XModule.h"
#include "PatternTools.h"

EnvelopeContainer::EnvelopeContainer(pp_int32 num) :
	envelopes(NULL)
{
	envelopes = new TEnvelope[num];
	
	memset(envelopes, 0, num*sizeof(TEnvelope));
	
	numEnvelopes = num;
}

EnvelopeContainer::~EnvelopeContainer()
{
	delete[] envelopes;
}

void EnvelopeContainer::store(pp_int32 index, const TEnvelope& env)
{
	if (index < 0 || index > numEnvelopes - 1)
		return;
		
	envelopes[index] = env;
}

const TEnvelope* EnvelopeContainer::restore(pp_int32 index)
{
	if (index < 0 || index > numEnvelopes - 1)
		return NULL;

	return envelopes + index;
}

PPString EnvelopeContainer::encodeEnvelope(const TEnvelope& env)
{
	char buffer[10];
	
	// Convert number of points
	PatternTools::convertToHex(buffer, env.num, 2);
	
	PPString str = buffer;
	
	// Sustain point
	PatternTools::convertToHex(buffer, env.sustain, 2);	
	str.append(buffer);

	// Loop start
	PatternTools::convertToHex(buffer, env.loops, 2);
	str.append(buffer);

	// Loop end
	PatternTools::convertToHex(buffer, env.loope, 2);
	str.append(buffer);

	// Loop type
	PatternTools::convertToHex(buffer, env.type, 2);
	str.append(buffer);

	// Loop speed
	PatternTools::convertToHex(buffer, env.speed, 2);
	str.append(buffer);

	for (pp_int32 i = 0; i < env.num; i++)
	{
		// X-coordinate
		PatternTools::convertToHex(buffer, (env.env[i][0]>>8)&0xFF, 2);
		str.append(buffer);
		PatternTools::convertToHex(buffer, env.env[i][0]&0xFF, 2);
		str.append(buffer);

		// Y-coordinate
		PatternTools::convertToHex(buffer, (env.env[i][1]>>8)&0xFF, 2);
		str.append(buffer);
		PatternTools::convertToHex(buffer, env.env[i][1]&0xFF, 2);
		str.append(buffer);
		
	}
	
	return str;
}

static pp_uint8 getNibble(const char* str)
{
	if (*str >= '0' && *str <= '9')
		return (*str - '0');
	if (*str >= 'A' && *str <= 'F')
		return (*str - 'A' + 10);
	if (*str >= 'a' && *str <= 'f')
		return (*str - 'a' + 10);
		
	return 0;
}

static pp_uint8 getByte(const char* str)
{
	return (getNibble(str)<<4) + getNibble(str+1);
}

TEnvelope EnvelopeContainer::decodeEnvelope(const PPString& str)
{
	TEnvelope env;
	
	const char* ptr = str;
	
	env.num = getByte(ptr);
	ptr+=2;
	env.sustain = getByte(ptr);
	ptr+=2;
	env.loops = getByte(ptr);
	ptr+=2;
	env.loope = getByte(ptr);
	ptr+=2;
	env.type = getByte(ptr);
	ptr+=2;
	env.speed = getByte(ptr);
	ptr+=2;
	
	for (pp_int32 i = 0; i < env.num; i++)
	{
		pp_uint16 b1 = getByte(ptr);
		pp_uint16 b2 = getByte(ptr+2);
		env.env[i][0] = (b1 << 8) + b2;

		b1 = getByte(ptr+4);
		b2 = getByte(ptr+6);
		env.env[i][1] = (b1 << 8) + b2;
		ptr+=8;
	}
	
	return env;
}

