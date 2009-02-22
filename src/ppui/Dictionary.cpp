/*
 *  ppui/Dictionary.cpp
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
 *  PPDictionary.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Mon Mar 14 2005.
 *
 */

#include "Dictionary.h"
#include "SimpleVector.h"

PPDictionary::PPDictionary() :
	enumerating(false),
	enumerationIndex(0)
{
	keys = new PPSimpleVector<PPDictionaryKey>;
}

PPDictionary::~PPDictionary()
{
	delete keys;
}

// copy c'tor
PPDictionary::PPDictionary(const PPDictionary& source)
{
	keys = source.keys->clone();
}

PPDictionary& PPDictionary::operator=(const PPDictionary& source)
{
	// no self-assignment
	if (this != &source)
	{
		delete keys;	
		
		keys = source.keys->clone();
	}

	return *this;
}


PPDictionaryKey* PPDictionary::getKeyToModify(const PPString& key) const
{
	for (pp_int32 i = 0;  i < keys->size(); i++)
	{
		PPDictionaryKey* theKey = keys->get(i);
		if (theKey->getKey().compareTo(key) == 0)
			return theKey;
	}
	return NULL;
}

void PPDictionary::store(const PPString& key, const PPString& value)
{
	PPDictionaryKey* theKey = getKeyToModify(key);
	
	if (theKey)
		theKey->store(value);
	else
		keys->add(new PPDictionaryKey(key, value));
}

void PPDictionary::store(const PPString& key, const pp_uint32 value)
{
	PPDictionaryKey* theKey = getKeyToModify(key);
	
	if (theKey)
		theKey->store(value);
	else
		keys->add(new PPDictionaryKey(key, value));
}

PPDictionaryKey* PPDictionary::restore(const PPString& key)
{
	return getKeyToModify(key);
}

pp_int32 PPDictionary::size() const
{
	return keys->size();
}

PPDictionaryKey* PPDictionary::getFirstKey()
{
	if (keys->size() == 0)
		return NULL;
		
	enumerationIndex = 0;
	enumerating = true;
	PPDictionaryKey* theKey = keys->get(enumerationIndex);
	enumerationIndex++;
	return theKey;
}

PPDictionaryKey* PPDictionary::getNextKey()
{
	if (!enumerating)
		return NULL;

	PPDictionaryKey* theKey = keys->get(enumerationIndex);
	enumerationIndex++;

	if (enumerationIndex == keys->size())
	{
		enumerationIndex = 0;
		enumerating = false;
	}
	
	return theKey;
}

void PPDictionary::stopEnumeration()
{
	enumerating = false;
	enumerationIndex = 0;
}

PPString PPDictionary::serializeToString()
{
	PPString result;
	
	for (pp_int32 i = 0; i < keys->size(); i++)
	{
		PPDictionaryKey* theKey = keys->get(i);
	
		if (i)
			result.append(";");
			
		result.append(theKey->getKey());
		result.append("=");
		result.append(theKey->getStringValue());
	}
	
	return result;
}
	
PPDictionary* PPDictionary::createFromString(const PPString& string)
{
	PPDictionary* dictionary = new PPDictionary();

	const char* str = string;

	pp_uint32 index = 0;
	
	bool invalid = false;
	
	pp_uint32 startIndex = 0;
	while (index < string.length())
	{
		while (str[index] && str[index] != '=')
			index++;
		
		PPString key;
		if (str[index] == '=')
		{
			key = string.subString(startIndex, index);
			startIndex = ++index;
		}
		else
		{
			invalid = true;
			break; 
		}
		
		while (str[index] && str[index] != ';')
			index++;
		
		PPString value;
		if (index > startIndex)
		{
			value = string.subString(startIndex, index);
			startIndex = ++index;
		}
		
		dictionary->store(key, value);
	}
	
	if (invalid || (string.length() == 0))
	{
		delete dictionary;
		dictionary = NULL;
	}
	
	return dictionary;
}

pp_uint32 PPDictionary::convertFloatToIntNonLossy(float value)
{
	pp_uint32 result;
	
	// not compatible
	if (sizeof(float) != sizeof(result))
		return 0;
	
	memcpy(&result, &value, sizeof(result));
	
	return result;
}

float PPDictionary::convertIntToFloatNonLossy(pp_uint32 value)
{
	float result;
	
	// not compatible
	if (sizeof(float) != sizeof(result))
		return 0;
	
	memcpy(&result, &value, sizeof(result));
	
	return result;
}
