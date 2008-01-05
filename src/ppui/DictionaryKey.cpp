/*
 *  PPDictionaryKey.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Mon Mar 14 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
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

