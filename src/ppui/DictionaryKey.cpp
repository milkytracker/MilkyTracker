/*
 *  ppui/DictionaryKey.cpp
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
 *  PPDictionaryKey.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Mon Mar 14 2005.
 *
 */

#include "DictionaryKey.h"
#include <stdlib.h>

PPDictionaryKey::PPDictionaryKey(const PPString& newKey, const PPString& newValue) :
	key(newKey),
	value(newValue)
{
}

PPDictionaryKey::PPDictionaryKey(const PPString& newKey, const pp_uint32 value) :
	key(newKey)
{
	store(value);
}

PPDictionaryKey::PPDictionaryKey(const PPDictionaryKey& source)
{
	key = source.key;
	value = source.value;
}

void PPDictionaryKey::store(const PPString& newValue)
{
	value = newValue;
}
	
void PPDictionaryKey::store(const pp_uint32 value)
{
	char buffer[100];
	
	sprintf(buffer,"%i",value);
	
	this->value = buffer;
}

pp_uint32 PPDictionaryKey::getIntValue() const
{
	pp_uint32 v; 
	sscanf(value, "%i", &v);
	
	return v;
}

