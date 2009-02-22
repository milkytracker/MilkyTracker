/*
 *  tracker/Zapper.h
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
 *  Zapper.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 26.12.07.
 *
 */

#ifndef __ZAPPER_H__
#define __ZAPPER_H__

class Zapper
{
private:
	class Tracker& tracker;

public:
	Zapper(Tracker& tracker) :
		tracker(tracker)
	{
	}

	Zapper(const Zapper& src) :
		tracker(src.tracker)
	{
	}
	
	void zapAll();
	void zapSong();
	void zapPattern();
	void zapInstruments();	
};

#endif
