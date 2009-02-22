/*
 *  tracker/wince/ButtonMapper.h
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

#ifndef BUTTONMAPPER__H
#define BUTTONMAPPER__H

struct TButtonMapping
{
	WORD keyModifiers;
	WORD virtualKeyCode;
};

enum EOrientation
{
	eOrientation90CW,
	eOrientation90CCW,
	eOrientationNormal	
};

extern TButtonMapping	mappings[];
extern EOrientation		orientation;
extern pp_int32			allowVirtualKeys;
extern pp_int32			hideTaskBar;
extern pp_int32			doublePixels;
extern pp_int32			dontTurnOffDevice;

void InitButtonRemapper();

#endif
