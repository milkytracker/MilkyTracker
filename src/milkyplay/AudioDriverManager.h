/*
 *  milkyplay/AudioDriverManager.h
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
 *  AudioDriverManager.h
 *
 *  This class serves as provider and manager of different kind of
 *	AudioDriver instances. Different kinds of AudioDrivers can be
 *	requested on the same system. 
 *
 *
 */
#ifndef __AUDIODRIVERMANAGER_H__
#define __AUDIODRIVERMANAGER_H__

#include "MilkyPlayTypes.h"

class AudioDriverInterface;

class AudioDriverManager
{
private:
	AudioDriverInterface**	driverList;
	mp_sint32				numDrivers;
	mp_sint32				defaultDriverIndex;
	mutable mp_sint32		enumerationIndex;
	
public:
	AudioDriverManager();
	~AudioDriverManager();

	/**
	 * Get the preferred audio driver for this system
	 * NULL is returned if either none is available or an alloc error occurred
	 * @return			audio driver instance
	 */
	AudioDriverInterface* getPreferredAudioDriver();
	AudioDriverInterface* getAudioDriverByName(const char* name);

	const char* getFirstDriverName() const;
	const char* getNextDriverName() const;
	
	mp_sint32 getPreferredAudioDriverSampleRate() const;
	mp_sint32 getPreferredAudioDriverBufferSize() const;
};

#endif
