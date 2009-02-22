/*
 *  ppui/DictionaryKey.h
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
 *  DictionaryKey.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Mon Mar 14 2005.
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
