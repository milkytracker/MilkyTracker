/*
 *  tracker/ColorPaletteContainer.h
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
 *  ColorPaletteContainer.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 21.11.05.
 *
 */

#ifndef COLORPALETTECONTAINER__H
#define COLORPALETTECONTAINER__H

#include "BasicTypes.h"

// Verx simple color palette, limited colors
struct TColorPalette
{
	// Should be more than enough
	pp_uint8 numColors;
	PPColor colors[256];
};

class ColorPaletteContainer 
{
private:
	TColorPalette* palettes;
	pp_int32 numPalettes;

public:
	ColorPaletteContainer(pp_int32 num);
	~ColorPaletteContainer();
	
	void store(pp_int32 index, const TColorPalette& palette);
	const TColorPalette* restore(pp_int32 index);

	static PPString encodePalette(const TColorPalette& palette);
	static TColorPalette decodePalette(const PPString& str);
};

#endif
