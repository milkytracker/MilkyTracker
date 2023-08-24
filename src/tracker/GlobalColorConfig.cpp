/*
 *  tracker/GlobalColorConfig.cpp
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
 *  GlobalColorConfig.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 21.11.05.
 *
 */

#include "GlobalColorConfig.h"
#include "TrackerConfig.h"
#include "PPUIConfig.h"

const PPColor& GlobalColorConfig::getColor(GlobalColors whichColor) const
{
	switch (whichColor)
	{
		// Pattern color identfiers
		case ColorPatternNote:
			return TrackerConfig::colorPatternEditorNote;
		case ColorPatternInstrument:
			return TrackerConfig::colorPatternEditorInstrument;
		case ColorPatternVolume:
			return TrackerConfig::colorPatternEditorVolume;
		case ColorPatternEffect:
			return TrackerConfig::colorPatternEditorEffect;
		case ColorPatternOperand:
			return TrackerConfig::colorPatternEditorOperand;
		case ColorCursor:
			return TrackerConfig::colorPatternEditorCursor;
		// Cursor line
		case ColorCursorLine:
			return TrackerConfig::colorPatternEditorCursorLine;
		// Cursor line in record mode
		case ColorCursorLineHighlighted:
			return TrackerConfig::colorPatternEditorCursorLineHighLight;
		
		// Theme color (= desktop in FT2)
		case ColorTheme:
			return TrackerConfig::colorThemeMain;
		// Main text (= white?)
		case ColorForegroundText:
			return PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText);
		// PPButton colors
		case ColorButtons:
			return PPUIConfig::getInstance()->getColor(PPUIConfig::ColorDefaultButton);
		// PPButton foreground text
		case ColorButtonText:
			return PPUIConfig::getInstance()->getColor(PPUIConfig::ColorDefaultButtonText);
		// Record mode button foreground text
		case ColorRecordModeButtonText:
			return TrackerConfig::colorRecordModeButtonText;
		
		// Various
		case ColorSelection:
			return PPUIConfig::getInstance()->getColor(PPUIConfig::ColorSelection);

		case ColorListBoxBackground:
			return PPUIConfig::getInstance()->getColor(PPUIConfig::ColorListBoxBackground);
		case ColorScrollBarBackground:
			return PPUIConfig::getInstance()->getColor(PPUIConfig::ColorScrollBarBackground);
			
		case ColorPatternSelection:
			return TrackerConfig::colorPatternEditorSelection;			
		case ColorTextHighlited:
			return TrackerConfig::colorHighLight_1;
		case ColorTextHighlitedSecond:
			return TrackerConfig::colorHighLight_2;

		case ColorScopes:
			return TrackerConfig::colorScopes;
		case ColorScopesRecordIndicator:
			return TrackerConfig::colorScopesRecordIndicator;
		case ColorPeakClipIndicator:
			return TrackerConfig::colorPeakClipIndicator;

		case ColorRowHighlitedFirst:
			return TrackerConfig::colorRowHighLight_1;
		case ColorRowHighlitedSecond:
			return TrackerConfig::colorRowHighLight_2;

		case ColorSampleEditorWaveform:
			return TrackerConfig::colorSampleEditorWaveform;

		default:
			return dummy;
	}
}

const char* GlobalColorConfig::getColorReadableDescription(GlobalColors whichColor) const
{
	switch (whichColor)
	{
		// Pattern color identfiers
		case ColorPatternNote:
			return "Pattern:Note";
		case ColorPatternInstrument:
			return "Pattern:Instrument";
		case ColorPatternVolume:
			return "Pattern:Volume";
		case ColorPatternEffect:
			return "Pattern:Effect";
		case ColorPatternOperand:
			return "Pattern:Operand";
		// Cursor line
		case ColorCursor:
			return "Pattern cursor";
		// Cursor line
		case ColorCursorLine:
			return "Cursor line";
		// Cursor line in record mode
		case ColorCursorLineHighlighted:
			return "Cursor line (FT2 rec.)";
		
		// Theme color (= desktop in FT2)
		case ColorTheme:
			return "Desktop";
		// Main text (= white?)
		case ColorForegroundText:
			return "Foreground text";
		// PPButton colors
		case ColorButtons:
			return "Buttons";
		// PPButton foreground text
		case ColorButtonText:
			return "Button text";
		// Record mode button foreground text
		case ColorRecordModeButtonText:
			return "Record mode button text";
		
		// Various
		case ColorSelection:
			return "Selections";
		case ColorListBoxBackground:
			return "Listbox background";
		case ColorScrollBarBackground:
			return "Scrollbar background";
		case ColorPatternSelection:
			return "Pattern block";
		case ColorTextHighlited:
			return "Highlighted text";
		case ColorTextHighlitedSecond:
			return "Pattern:2nd highlight text";
		case ColorScopes:
			return "Scopes";
		case ColorScopesRecordIndicator:
			return "Scopes record indicator";
		case ColorPeakClipIndicator:
			return "Peak clip indicator";

		case ColorRowHighlitedFirst:
			return "Pattern:1st row bg";
		case ColorRowHighlitedSecond:
			return "Pattern:2nd row bg";

		case ColorSampleEditorWaveform:
			return "Sample editor waveform";

		default:
			return NULL;
	}
}

void GlobalColorConfig::setColor(GlobalColors whichColor, const PPColor& color)
{
	switch (whichColor)
	{
		// Pattern color identfiers
		case ColorPatternNote:
			TrackerConfig::colorPatternEditorNote = color;
			break;
		case ColorPatternInstrument:
			TrackerConfig::colorPatternEditorInstrument = color;
			break;
		case ColorPatternVolume:
			TrackerConfig::colorPatternEditorVolume = color;
			break;
		case ColorPatternEffect:
			TrackerConfig::colorPatternEditorEffect = color;
			break;
		case ColorPatternOperand:
			TrackerConfig::colorPatternEditorOperand = color;
			break;
		// Cursor line
		case ColorCursor:
			TrackerConfig::colorPatternEditorCursor = color;
			break;
		// Cursor line
		case ColorCursorLine:
			TrackerConfig::colorPatternEditorCursorLine = color;
			break;
		// Cursor line in record mode
		case ColorCursorLineHighlighted:
			TrackerConfig::colorPatternEditorCursorLineHighLight = color;
			break;
		
		// Theme color (= desktop in FT2)
		case ColorTheme:
			TrackerConfig::colorThemeMain = color;
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorMessageBoxContainer, color);
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorMenuBackground, color);
			break;
		// Main text (= white?)
		case ColorForegroundText:
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorStaticText, color);
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorMenuTextDark, color);
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorMenuTextBright, color);
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorSelection, color);
			break;
		// PPButton colors
		case ColorButtons:
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorDefaultButton, color);
			break;
		// PPButton foreground text
		case ColorButtonText:
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorDefaultButtonText, color);
			break;
		// Record mode button foreground text
		case ColorRecordModeButtonText:
			TrackerConfig::colorRecordModeButtonText = color;
			break;
		
		// Various
		case ColorSelection:
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorSelection, color);
			break;

		case ColorListBoxBackground:
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorListBoxBackground, color);
			break;
		case ColorScrollBarBackground:
			PPUIConfig::getInstance()->setColor(PPUIConfig::ColorScrollBarBackground, color);
			break;
			
		case ColorPatternSelection:
			TrackerConfig::colorPatternEditorSelection = color;
			break;

		case ColorTextHighlited:
			TrackerConfig::colorHighLight_1 = color;
			break;
			
		case ColorTextHighlitedSecond:
			TrackerConfig::colorHighLight_2 = color;
			break;

		case ColorScopes:
			TrackerConfig::colorScopes = color;
			break;

		case ColorScopesRecordIndicator:
			TrackerConfig::colorScopesRecordIndicator = color;
			break;

		case ColorPeakClipIndicator:
			TrackerConfig::colorPeakClipIndicator = color;
			break;

		case ColorRowHighlitedFirst:
			TrackerConfig::colorRowHighLight_1 = color;
			break;

		case ColorRowHighlitedSecond:
			TrackerConfig::colorRowHighLight_2 = color;
			break;

		case ColorSampleEditorWaveform:
			TrackerConfig::colorSampleEditorWaveform = color;
			break;
            
        case ColorLast:
            // Not handled
            break;
	}
}

