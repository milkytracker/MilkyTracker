/*
 *  ppui/Singleton.h
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
 *  Singleton.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 10.04.08.
 *
 */

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template<class type>
class PPSingleton
{
private:
	static type* instance;
	
public:
	static type* getInstance()
	{
		if (instance == NULL)
			instance = new type();
		
		return instance;
	}
};

template<class type>
type* PPSingleton<type>::instance = NULL;

#endif
