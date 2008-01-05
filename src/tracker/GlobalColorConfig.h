/*
 *  GlobalColorConfig.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 21.11.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef GLOBALCOLORCONFIG__H
#define GLOBALCOLORCONFIG__H

#include "BasicTypes.h"

class GlobalColorConfig
{
private:
	PPColor dummy;
	
public:
	enum GlobalColors
	{
		// Pattern color identfiers
		ColorPatternNote = 0,
		ColorPatternInstrument,
		ColorPatternVolume,
		ColorPatternEffect,
		ColorPatternOperand,
		// Pattern cursor
		ColorCursor,
		// Cursor line
		ColorCursorLine,
		// Cursor line in record mode
		ColorCursorLineHighlighted,
		
		// Theme color (= desktop in FT2)
		ColorTheme,
		// Main text (= white?)
		ColorForegroundText,
		// PPButton colors
		ColorButtons,
		// PPButton foreground text
		ColorButtonText,
		
		// Various
		ColorSelection,
		ColorListBoxBackground,
		ColorPatternSelection,
		ColorTextHighlited, // usually yellow
		ColorScopes, // usually white
		ColorTextHighlitedSecond, // usually darker yellow
		ColorRowHighlitedFirst, // First color to highlight the row background
		ColorRowHighlitedSecond, // Second color to highlight the row background

		// Unused, endmarker
		ColorLast = ColorScopes + 24 // remember to subtract one from the last value every time you add another color to the list
	};

private:
	static GlobalColorConfig* instance;
	
public:
	static GlobalColorConfig* getInstance();
	
	const PPColor& getColor(GlobalColors whichColor);
	const char* getColorReadableDescription(GlobalColors whichColor);

	void setColor(GlobalColors whichColor, const PPColor& color);
};

#endif

