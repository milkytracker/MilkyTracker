/*
 *  TrackerSettingsDatabase.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Tue Mar 15 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#include "BasicTypes.h"
#include "TrackerSettingsDatabase.h"
#include "XMFile.h"

TrackerSettingsDatabase::TrackerSettingsDatabase(const TrackerSettingsDatabase& source)
{
	dictionary = source.dictionary;
}

TrackerSettingsDatabase& TrackerSettingsDatabase::operator=(const TrackerSettingsDatabase& source)
{
	if (this != &source)
	{
		dictionary = source.dictionary;		
	}
	
	return *this;
}

void TrackerSettingsDatabase::store(const PPString& key, const PPString& value)
{
	dictionary.store(key, value);
}

void TrackerSettingsDatabase::store(const PPString& key, const pp_uint32 value)
{
	dictionary.store(key, value);
}

bool TrackerSettingsDatabase::hasKey(const PPString& key)
{
	return dictionary.restore(key) != NULL;
}

PPDictionaryKey* TrackerSettingsDatabase::restore(const PPString& key)
{
	return dictionary.restore(key);
}

bool TrackerSettingsDatabase::serialize(XMFile& f)
{
	if (f.isOpenForWriting())
	{
		f.writeDword(dictionary.size());

		const PPDictionaryKey* theKey = dictionary.getFirstKey();
		
		while (theKey)
		{
			f.writeDword(theKey->getKey().length());
			f.write(theKey->getKey(), 1, theKey->getKey().length());

			f.writeDword(theKey->getStringValue().length());
			f.write(theKey->getStringValue(), 1, theKey->getStringValue().length());

			theKey = dictionary.getNextKey();
		}
		
	}
	else
	{
		pp_int32 size = f.readDword();
	
		for (pp_int32 i = 0; i < size; i++)
		{
			pp_int32 len = f.readDword();
			char* keyBuffer = new char[len+1];
			memset(keyBuffer, 0, len+1);
			f.read(keyBuffer, 1, len);

			len = f.readDword();
			char* valueBuffer = new char[len+1];
			memset(valueBuffer, 0, len+1);
			f.read(valueBuffer, 1, len);
			
			store(keyBuffer, valueBuffer);
			
			delete[] valueBuffer;
			delete[] keyBuffer;
		}
	}
	
	return true;
}
	
void TrackerSettingsDatabase::dump()
{
	const PPDictionaryKey* theKey = dictionary.getFirstKey();
	
	while (theKey)
	{
		theKey = dictionary.getNextKey();
	}
}

