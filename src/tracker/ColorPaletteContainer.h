/*
 *  ColorPaletteContainer.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 21.11.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
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
