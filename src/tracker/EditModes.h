/*
 *  tracker/EditModes.h
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
 *  EditModes.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Thu May 19 2005.
 *
 */

#ifndef __EDITMODES_H__
#define __EDITMODES_H__

enum EditModes
{
	EditModeMilkyTracker,
	EditModeFastTracker
};

enum ScrollModes
{
	ScrollModeToEnd,
	ScrollModeToCenter,
	ScrollModeStayInCenter
};

enum ViewModes 
{
	ViewPattern,
	ViewSteps
};

#endif
