/*
 *  ppui/Event.cpp
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
 *  PPEvent.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 23.05.05.
 *
 */

#include "Event.h"

pp_uint32 keyModifier = 0;
pp_uint32 forceKeyModifier = 0;

void QueryKeyModifiers();

void setKeyModifier(KeyModifiers eModifier)
{
	keyModifier |= eModifier;
}

void clearKeyModifier(KeyModifiers eModifier)
{
	keyModifier &= ~eModifier;
}

void setForceKeyModifier(KeyModifiers eModifier)
{
	forceKeyModifier |= eModifier;
}

void clearForceKeyModifier(KeyModifiers eModifier)
{
	forceKeyModifier &= ~eModifier;
}

pp_uint32 getKeyModifier()
{
	QueryKeyModifiers();
	return keyModifier | forceKeyModifier; 
}
