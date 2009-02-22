/*
 *  tracker/TrackerSettingsDatabase.h
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
 *  TrackerSettingsDatabase.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Tue Mar 15 2005.
 *
 */

#ifndef TRACKERSETTINGSDATABASE__H
#define TRACKERSETTINGSDATABASE__H

#include "BasicTypes.h"
#include "Dictionary.h"

class PPDictionaryKey;
class XMFile;

class TrackerSettingsDatabase
{
private:
	PPDictionary dictionary;
	pp_int32 maxKeys;
	
public:
	TrackerSettingsDatabase(pp_int32 maxKeys = -1) :
		maxKeys(maxKeys)
	{
	}
	
	// copy c'tor
	TrackerSettingsDatabase(const TrackerSettingsDatabase& source);	

	TrackerSettingsDatabase& operator=(const TrackerSettingsDatabase& source);

	void store(const PPString& key, const PPString& value);

	void store(const PPString& key, const pp_uint32 value);

	bool hasKey(const PPString& key);

	PPDictionaryKey* restore(const PPString& key);
	
	bool serialize(XMFile& file);
	
	void dump();

	PPDictionaryKey* getFirstKey() { return dictionary.getFirstKey(); }
	PPDictionaryKey* getNextKey() { return dictionary.getNextKey(); }
	void stopEnumeration() { dictionary.stopEnumeration(); }
};

#endif

