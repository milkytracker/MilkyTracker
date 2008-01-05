/*
 *  PPDictionary.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Mon Mar 14 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
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


