/*
 *  tracker/TrackerSettingsDatabase.cpp
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
 *  TrackerSettingsDatabase.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Tue Mar 15 2005.
 *
 */

#include "BasicTypes.h"
#include "TrackerSettingsDatabase.h"
#include "XMFile.h"

TrackerSettingsDatabase::TrackerSettingsDatabase(const TrackerSettingsDatabase& source) :
	dictionary(source.dictionary),
	maxKeys(source.maxKeys)
{
}

TrackerSettingsDatabase& TrackerSettingsDatabase::operator=(const TrackerSettingsDatabase& source)
{
	if (this != &source)
	{
		dictionary = source.dictionary;		
		maxKeys = source.maxKeys;
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
		if (maxKeys >= 0 && dictionary.size() > maxKeys)
			return false;
	
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

		if (maxKeys >= 0 && size > maxKeys)
			return false;
	
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

