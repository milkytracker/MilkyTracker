/*
 *  DictionaryKey.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Mon Mar 14 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef DICTIONARYKEY__H
#define DICTIONARYKEY__H

#include "BasicTypes.h"

class PPDictionaryKey
{
private:
	PPString key;
	PPString value;
	
public:
	PPDictionaryKey(const PPString& newKey, const PPString& newValue);

	PPDictionaryKey(const PPString& newKey, const pp_uint32 value);

	// copy c'tor
	PPDictionaryKey(const PPDictionaryKey& source);
	
	void store(const PPString& newValue);
	
	void store(const pp_uint32 value);
	
	const PPString& getStringValue() const { return value; }
	pp_uint32 getIntValue() const;
	bool getBoolValue() const { return getIntValue() != 0; }

	const PPString& getKey() const { return key; }

};

#endif
