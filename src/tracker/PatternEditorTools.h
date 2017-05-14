/*
 *  tracker/PatternEditorTools.h
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
 *  PatternEditorTools.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.11.07.
 *
 */
 
#ifndef __PATTERNEDITORTOOLS_H__
#define __PATTERNEDITORTOOLS_H__

#include "BasicTypes.h"
#include "MilkyPlayTypes.h"

struct TXMPattern;
class XModule;

class PatternEditorTools
{
public:
	struct Position
	{
		pp_int32 channel, row, inner;
		
		bool isValid() const { return (channel >= 0 && row >= 0 && inner >= 0); }
		
		bool operator==(const Position& other) 
		{ 
			return (channel == other.channel &&
					row == other.row &&
					inner == other.inner);
		}
		
		bool operator!=(const Position& other) 
		{ 
			return (channel != other.channel ||
					 row != other.row ||
					 inner != other.inner);
		}
	};
	
	struct OperandOptimizeParameters
	{
		bool command_1xx;
		bool command_2xx;
		bool command_3xx;
		bool command_4xx;
		bool command_56Axx;
		bool command_7xx;
		bool command_E1x;
		bool command_E2x;
		bool command_EAx;
		bool command_EBx;
		bool command_Hxx;
		bool command_Pxx;
		bool command_Rxx;
		bool command_X1x;
		bool command_X2x;

		OperandOptimizeParameters() :
			command_1xx(false),
			command_2xx(false),
			command_3xx(false),
			command_4xx(false),
			command_56Axx(false),
			command_7xx(false),
			command_E1x(false),
			command_E2x(false),
			command_EAx(false),
			command_EBx(false),
			command_Hxx(false),
			command_Pxx(false),
			command_Rxx(false),
			command_X1x(false),
			command_X2x(false)
		{
		}
	};

	struct RelocateParameters
	{
		bool command_3xx;
		bool command_4xx;
		bool command_8xx;
		bool command_Cxx;
		bool command_Axx;
		bool command_EAx;
		bool command_EBx;
		bool command_Pxx;

		RelocateParameters() :
			command_3xx(false),
			command_4xx(false),
			command_8xx(false),
			command_Cxx(false),
			command_Axx(false),
			command_EAx(false),
			command_EBx(false),
			command_Pxx(false)
		{
		}
	};

	struct TransposeParameters
	{
		enum
		{
			DefaultNoteRange = 96
		};
	
		pp_int32 maxNoteRange;
	
		pp_int32 insRangeStart;
		pp_int32 insRangeEnd;

		pp_int32 noteRangeStart;
		pp_int32 noteRangeEnd;

		pp_int32 amount;
		
		TransposeParameters() :
			maxNoteRange(DefaultNoteRange)
		{
		}
	};

private:
	TXMPattern* pattern;

	Position getMarkStart();
	Position getMarkEnd();
	
public:
	PatternEditorTools(TXMPattern* pattern = NULL);

	void attachPattern(TXMPattern* pattern) { this->pattern = pattern; }

	void clearSelection(const Position& ss, const Position& se);
	bool expandPattern();
	bool shrinkPattern();

	pp_int32 insIncSelection(const Position& ss, const Position& se);
	pp_int32 insDecSelection(const Position& ss, const Position& se);
	pp_int32 insIncTrack(pp_int32 track);
	pp_int32 insDecTrack(pp_int32 track);
	pp_int32 insRemapSelection(const Position& ss, const Position& se, pp_int32 oldIns, pp_int32 newIns);
	pp_int32 insRemap(pp_int32 oldIns, pp_int32 newIns);
	pp_int32 insRemapTrack(pp_int32 track, pp_int32 oldIns, pp_int32 newIns);

	pp_int32 noteTransposeSelection(const Position& ss, const Position& se, const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate);
	pp_int32 noteTranspose(const TransposeParameters& transposeParameters, bool evaluate = false);
	pp_int32 noteTransposeTrack(pp_int32 track, const TransposeParameters& transposeParameters, bool evaluate = false);

	pp_int32 interpolateValuesInSelection(const PatternEditorTools::Position& ss, const PatternEditorTools::Position& se);

	pp_int32 zeroOperandsSelection(const Position& ss, const Position& se, const OperandOptimizeParameters& optimizeParameters, bool evaluate);
	pp_int32 zeroOperands(const OperandOptimizeParameters& optimizeParameters, bool evaluate = false);
	pp_int32 zeroOperandsTrack(pp_int32 track, const OperandOptimizeParameters& optimizeParameters, bool evaluate);

	pp_int32 fillOperandsSelection(const Position& ss, const Position& se, const OperandOptimizeParameters& optimizeParameters, bool evaluate);
	pp_int32 fillOperands(const OperandOptimizeParameters& optimizeParameters, bool evaluate = false);
	pp_int32 fillOperandsTrack(pp_int32 track, const OperandOptimizeParameters& optimizeParameters, bool evaluate);

	pp_int32 relocateCommandsSelection(const Position& ss, const Position& se, const RelocateParameters& relocateParameters, bool evaluate);
	pp_int32 relocateCommands(const RelocateParameters& relocateParameters, bool evaluate = false);
	pp_int32 relocateCommandsTrack(pp_int32 track, const RelocateParameters& relocateParameters, bool evaluate);
	
	pp_int32 scaleVolume(const Position& startSelection, const Position& endSelection, float startScale, float endScale, 
						 pp_uint32 numVisibleChannels, XModule* module);

	pp_int32 splitTrack(pp_int32 useChannels, bool insertNoteOff, pp_int32 chn, pp_int32 fromRow, pp_int32 toRow, pp_uint32 numVisibleChannels);
	
	pp_int32 swapChannels(pp_int32 dstChannel, pp_int32 srcChannel);
	
	void normalize();
	
	static void slotCopy(mp_ubyte* dst, mp_ubyte* src, pp_int32 from, pp_int32 to);
	static void slotTransparentCopy(mp_ubyte* dst, mp_ubyte* src, pp_int32 from, pp_int32 to);
	static void slotClear(mp_ubyte* dst, pp_int32 from, pp_int32 to);

	static bool hasValidSelection(const TXMPattern* pattern, const Position& ss, const Position& se, pp_int32 numVisibleChannels = -1);

	static bool normalizeSelection(const TXMPattern* pattern, 
								   const PatternEditorTools::Position& ss, 
								   const PatternEditorTools::Position& se,
								   pp_int32& selectionStartChannel, 
								   pp_int32& selectionStartRow, 
								   pp_int32& selectionStartInner,
								   pp_int32& selectionEndChannel, 
								   pp_int32& selectionEndRow, 
								   pp_int32& selectionEndInner);
	
	static void flattenSelection(Position& selectionStart, Position& selectionEnd)
	{
		Position selStart = selectionStart, selEnd = selectionEnd;
	
		if (selectionStart.channel > selectionEnd.channel)
		{
			selStart.channel = selectionEnd.channel;
			selEnd.channel = selectionStart.channel;
			selStart.inner = selectionEnd.inner;
			selEnd.inner = selectionStart.inner;
		}
		else
		{
			selStart.channel = selectionStart.channel;
			selEnd.channel = selectionEnd.channel;
			selStart.inner= selectionStart.inner;
			selEnd.inner= selectionEnd.inner;
		}
		
		if (selStart.channel == selEnd.channel && selEnd.inner < selStart.inner)
		{
			mp_sint32 h = selEnd.inner;
			selEnd.inner = selStart.inner;
			selStart.inner = h;
		}
		
		if (selectionStart.row > selectionEnd.row)
		{
			selStart.row = selectionEnd.row;
			selEnd.row = selectionStart.row;
		}
		else
		{
			selStart.row = selectionStart.row;
			selEnd.row = selectionEnd.row;
		}
		
		selectionStart = selStart;
		selectionEnd = selEnd;
	}
};

#endif
