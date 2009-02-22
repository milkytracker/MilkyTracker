/*
 *  tracker/wince/VirtualKeyToScanCodeTable.cpp
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

#include "VirtualKeyToScanCodeTable.h"

const pp_int16 vkeyToScancode[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, 14, 15, -1, -1, 
	-1, 28, -1, -1, 54, 29, -1, -1, 58, -1, -1, -1, 
	-1, -1, -1, 1, -1, -1, -1, -1, 57, 73, 81, 79, 
	71, 75, 72, 77, 80, -1, -1, -1, -1, 82, 83, -1, 
	11, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1, -1, 
	-1, -1, -1, -1, -1, 30, 48, 46, 32, 18, 33, 34, 
	35, 23, 36, 37, 38, 50, 49, 24, 25, 16, 19, 31, 
	20, 22, 47, 17, 45, 21, 44, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, 59, 60, 61, 62, 63, 64, 65, 66, 
	67, 68, 87, 88, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, 29, -1, 56, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, 39, 13, 51, 12, 52, 53, 
	41, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, 26, 43, 27, 40, -1, -1, -1, 86, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, 56
};
