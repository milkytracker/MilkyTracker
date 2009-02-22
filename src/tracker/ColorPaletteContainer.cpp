/*
 *  tracker/ColorPaletteContainer.cpp
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
 *  ColorPaletteContainer.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 21.11.05.
 *
 */

#include "ColorPaletteContainer.h"
#include "PatternTools.h"

ColorPaletteContainer::ColorPaletteContainer(pp_int32 num) :
	palettes(NULL)
{
	palettes = new TColorPalette[num];
	
	memset(palettes, 0, num*sizeof(TColorPalette));
	
	numPalettes = num;
}

ColorPaletteContainer::~ColorPaletteContainer()
{
	delete[] palettes;
}

void ColorPaletteContainer::store(pp_int32 index, const TColorPalette& palette)
{
	if (index < 0 || index > numPalettes - 1)
		return;
		
	palettes[index] = palette;
}

const TColorPalette* ColorPaletteContainer::restore(pp_int32 index)
{
	if (index < 0 || index > numPalettes - 1)
		return NULL;

	return palettes + index;
}

PPString ColorPaletteContainer::encodePalette(const TColorPalette& palette)
{
	char buffer[10];
	
	// Convert number of colors
	PatternTools::convertToHex(buffer, palette.numColors, 2);
	
	PPString str = buffer;
	
	for (pp_int32 i = 0; i < palette.numColors; i++)
	{
		// R
		PatternTools::convertToHex(buffer, palette.colors[i].r, 2);
		str.append(buffer);

		// G
		PatternTools::convertToHex(buffer, palette.colors[i].g, 2);
		str.append(buffer);

		// B
		PatternTools::convertToHex(buffer, palette.colors[i].b, 2);
		str.append(buffer);		
	}
	
	return str;
}

static pp_uint8 getNibble(const char* str)
{
	if (*str >= '0' && *str <= '9')
		return (*str - '0');
	if (*str >= 'A' && *str <= 'F')
		return (*str - 'A' + 10);
	if (*str >= 'a' && *str <= 'f')
		return (*str - 'a' + 10);
		
	return 0;
}

static pp_uint8 getByte(const char* str)
{
	return (getNibble(str)<<4) + getNibble(str+1);
}

TColorPalette ColorPaletteContainer::decodePalette(const PPString& str)
{
	TColorPalette palette;
	
	const char* ptr = str;
	
	palette.numColors = getByte(ptr);
	ptr+=2;
	
	for (pp_int32 i = 0; i < palette.numColors; i++)
	{
		palette.colors[i].r = getByte(ptr);
		ptr+=2;
		palette.colors[i].g = getByte(ptr);
		ptr+=2;
		palette.colors[i].b = getByte(ptr);
		ptr+=2;
	}
	
	return palette;
}
