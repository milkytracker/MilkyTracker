/*
 *  Dictionary.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Mon Mar 14 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef DICTIONARY__H
#define DICTIONARY__H

#include "BasicTypes.h"
#include "DictionaryKey.h"

template<class Type>
class PPSimpleVector;

class PPDictionary
{
private:
	PPSimpleVector<PPDictionaryKey>* keys;
	
	bool enumerating;
	pp_int32 enumerationIndex;
	
	PPDictionaryKey* getKeyToModify(const PPString& key) const;
	
public:
	PPDictionary();
	~PPDictionary();
	
	// copy c'tor
	PPDictionary(const PPDictionary& source);	

	PPDictionary& operator=(const PPDictionary& source);
	
	void store(const PPString& key, const PPString& value);

	void store(const PPString& key, const pp_uint32 value);
	
	PPDictionaryKey* restore(const PPString& key);
	
	pp_int32 size() const;

	PPDictionaryKey* getFirstKey();
	PPDictionaryKey* getNextKey();
	void stopEnumeration();
};

#endif

