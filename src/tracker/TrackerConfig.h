/*
 *  tracker/TrackerConfig.h
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

#ifndef TRACKERCONFIG__H
#define TRACKERCONFIG__H

#include "BasicTypes.h"

#define NUMEFFECTMACROS 20

class TrackerConfig
{
public:
	static const PPString stringButtonPlus;
	static const PPString stringButtonMinus;
	static const PPString stringButtonUp;
	static const PPString stringButtonDown;
	static const PPString stringButtonExtended;
	static const PPString stringButtonCollapsed;
	
	enum 
	{
		MAXCHANNELS = 256,
		MAXNOTES = 120, // 10 octaves, even though FT2 only uses 8
	};

	static const PPPoint trackerExitBounds;

	static PPColor colorThemeMain;
	static PPColor colorRecordModeButtonText;
	
	// Pattern colors
	static PPColor colorPatternEditorBackground;
	static PPColor colorPatternEditorCursor;
	static PPColor colorPatternEditorCursorLine;
	static PPColor colorPatternEditorCursorLineHighLight;
	static PPColor colorPatternEditorSelection;
	
	static PPColor colorPatternEditorNote;
	static PPColor colorPatternEditorInstrument;
	static PPColor colorPatternEditorVolume;
	static PPColor colorPatternEditorEffect;
	static PPColor colorPatternEditorOperand;
	
	static PPColor colorHighLight_1;
	static PPColor colorHighLight_2;
	static PPColor colorScopes;
	static PPColor colorScopesRecordIndicator;
	static PPColor colorPeakClipIndicator;

	static PPColor colorRowHighLight_1;
	static PPColor colorRowHighLight_2;

	static PPColor colorSampleEditorWaveform;

	static pp_int32 numTabs;
	
	static pp_int32 numPlayerChannels;
	static pp_int32 numVirtualChannels;
	static pp_int32 totalPlayerChannels;
	static const pp_int32 maximumPlayerChannels;

	static bool useVirtualChannels;
	
	static const pp_int32 numPredefinedEnvelopes;
	static const pp_int32 numPredefinedColorPalettes;
	
	static const PPString defaultPredefinedVolumeEnvelope;
	static const PPString defaultPredefinedPanningEnvelope;
	static const PPString defaultProTrackerPanning;
	
	static const PPString defaultColorPalette;	
	static const char* predefinedColorPalettes[];
	
	static const PPSystemString untitledSong;
	
	static const pp_int32 numMixFrequencies;
	static const pp_int32 mixFrequencies[];

	static const pp_uint32 version;
};

#endif
