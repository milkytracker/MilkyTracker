/*
 *  ppui/Dictionary.h
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
 *  Dictionary.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Mon Mar 14 2005.
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
	
	PPString serializeToString();
	
	static PPDictionary* createFromString(const PPString& string);
	
	static pp_uint32 convertFloatToIntNonLossy(float value);
	static float convertIntToFloatNonLossy(pp_uint32 value);
};

#endif

