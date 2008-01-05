/*
 *  TrackerSettingsDatabase.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Tue Mar 15 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
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
	
public:
	TrackerSettingsDatabase() {}
	
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

