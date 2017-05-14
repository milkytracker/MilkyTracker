/*
 *  tracker/GlobalColorConfig.h
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
 *  GlobalColorConfig.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 21.11.05.
 *
 */

#ifndef GLOBALCOLORCONFIG__H
#define GLOBALCOLORCONFIG__H

#include "BasicTypes.h"
#include "Singleton.h"

class GlobalColorConfig : public PPSingleton<GlobalColorConfig>
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

		ColorScrollBarBackground,
		ColorRecordModeButtonText, // usually red
		ColorScopesRecordIndicator, // usually red
		ColorPeakClipIndicator, // usually red

		ColorSampleEditorWaveform,

		// Unused, endmarker
		ColorLast = 40
	};

public:
	const PPColor& getColor(GlobalColors whichColor) const;
	const char* getColorReadableDescription(GlobalColors whichColor) const;

	void setColor(GlobalColors whichColor, const PPColor& color);
	
	friend class PPSingleton<GlobalColorConfig>;
};

#endif

