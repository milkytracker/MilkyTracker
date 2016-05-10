/*
 *  tracker/PatternEditorTools.cpp
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
 *  PatternEditorTools.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.11.07.
 *
 */

#include "PatternEditorTools.h"
#include "XModule.h"
#include "PatternTools.h"

PatternEditorTools::PatternEditorTools(TXMPattern* pattern) :
	pattern(pattern)
{
}

PatternEditorTools::Position PatternEditorTools::getMarkStart()
{
	Position pos;
	pos.channel = pos.row = pos.inner = 0;
	return pos;
}

PatternEditorTools::Position PatternEditorTools::getMarkEnd()
{
	Position pos;
	pos.channel = pos.row = pos.inner = 0;

	if (pattern->patternData != NULL)
	{
		pos.channel = pattern->channum - 1;
		pos.inner = 7;
		pos.row = pattern->rows - 1;
	}

	return pos;
}

bool PatternEditorTools::normalizeSelection(const TXMPattern* pattern, 
											const PatternEditorTools::Position& ss, 
											const PatternEditorTools::Position& se,
											pp_int32& selectionStartChannel, 
											pp_int32& selectionStartRow, 
											pp_int32& selectionStartInner,
											pp_int32& selectionEndChannel, 
											pp_int32& selectionEndRow, 
											pp_int32& selectionEndInner)
{
	pp_int32 ssc = ss.channel, ssr = ss.row, ssi = ss.inner;
	pp_int32 sec = se.channel, ser = se.row, sei = se.inner;

	if (ssc < 0 && ssr < 0 && sec < 0 && ser < 0)
		return false;

	// sanity checks
	if (ssc < 0) ssc = 0;
	if (ssc >= pattern->channum) ssc = pattern->channum-1;
	if (sec < 0) sec = 0;
	if (sec >= pattern->channum) sec = pattern->channum-1;

	if (ssr < 0) ssr = 0;
	if (ssr >= pattern->rows) ssr = pattern->rows-1;
	if (ser < 0) ser = 0;
	if (ser >= pattern->rows) ser = pattern->rows-1;

	if (ssi < 0) ssi = 0;
	if (ssi > 7) ssi = 7;
	if (sei < 0) sei = 0;
	if (sei > 7) sei = 7;

	// correct orientation
	selectionStartChannel = 0;
	selectionStartRow = 0;
	selectionStartInner = 0;
	selectionEndChannel = 0;
	selectionEndRow = 0;
	selectionEndInner = 0;

	if (ssc > sec)
	{
		selectionStartChannel = sec;
		selectionEndChannel = ssc;
		selectionStartInner = sei;
		selectionEndInner = ssi;
	}
	else
	{
		selectionStartChannel = ssc;
		selectionEndChannel = sec;
		selectionStartInner = ssi;
		selectionEndInner = sei;
	}

	if (ssr > ser)
	{
		selectionStartRow = ser;
		selectionEndRow = ssr;
	}
	else
	{
		selectionStartRow = ssr;
		selectionEndRow = ser;
	}
	
	if (selectionStartChannel == selectionEndChannel && 
		selectionEndInner < selectionStartInner)
	{
		mp_sint32 h = selectionEndInner;
		selectionEndInner = selectionStartInner;
		selectionStartInner = h;
	}	
	
	return true;
}
							

void PatternEditorTools::clearSelection(const PatternEditorTools::Position& ss, const PatternEditorTools::Position& se)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se, 
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return;
		
	// only entire instrument column is allowed
	if (selectionStartInner >= 1 && selectionStartInner<=2)
		selectionStartInner = 1;
	if (selectionEndInner >= 1 && selectionEndInner<=2)
		selectionEndInner = 2;
	// only entire volume column can be selected
	if (selectionStartInner >= 3 && selectionStartInner<=4)
		selectionStartInner = 3;
	if (selectionEndInner >= 3 && selectionEndInner<=4)
		selectionEndInner = 4;
		
	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;
		
			//memset(src, 0, slotSize);
			if (selectionWidth == 1)
			{
				slotClear(src, selectionStartInner, selectionEndInner);
			}
			else if (j == selectionStartChannel)
			{
				slotClear(src, selectionStartInner, 7);
			}
			else if (j == selectionEndChannel)
			{
				slotClear(src, 0, selectionEndInner);
			}
			else
			{
				//memcpy(dst, src, slotSize);
				slotClear(src, 0, 7);
			}
		}	
}

bool PatternEditorTools::expandPattern()
{
	if (pattern == NULL) 
		return false;
		
	if (pattern->patternData == NULL)
		return false;

	if (pattern->rows * 2 > 256)
		return false;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	// allocate twice the space of the current pattern
	mp_sint32 patternSize = slotSize * pattern->channum * pattern->rows * 2;

	mp_ubyte* newPatternData = new mp_ubyte[patternSize];
	
	memset(newPatternData, 0, patternSize);

	for (mp_sint32 i = 0; i < pattern->rows; i++)
	{
		mp_sint32 srcOffset = i * slotSize * pattern->channum;
		mp_sint32 dstOffset = i * 2 * slotSize * pattern->channum;
		for (mp_sint32 j = 0; j < slotSize * pattern->channum; j++)
			newPatternData[dstOffset++] = pattern->patternData[srcOffset++];
	}

	delete[] pattern->patternData;

	pattern->patternData = newPatternData;

	pattern->rows <<= 1;

	return true;
}

bool PatternEditorTools::shrinkPattern()
{
	if (pattern == NULL) 
		return false;
		
	if (pattern->patternData == NULL)
		return false;

	if (pattern->rows < 2)
		return false;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	// allocate half of the space of the current pattern
	mp_sint32 patternSize = slotSize * pattern->channum * (pattern->rows >> 1);

	mp_ubyte* newPatternData = new mp_ubyte[patternSize];
	
	memset(newPatternData, 0, patternSize);

	for (mp_sint32 i = 0; i < pattern->rows >> 1; i++)
	{
		mp_sint32 srcOffset = i * 2 * slotSize * pattern->channum;
		mp_sint32 dstOffset = i * slotSize * pattern->channum;
		for (mp_sint32 j = 0; j < slotSize * pattern->channum; j++)
			newPatternData[dstOffset++] = pattern->patternData[srcOffset++];
	}

	delete[] pattern->patternData;

	pattern->patternData = newPatternData;

	pattern->rows >>= 1;

	return true;
}

pp_int32 PatternEditorTools::insIncSelection(const Position& ss, const Position& se)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return 0;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se,
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return 0;

	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 resCnt = 0;

	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;

			if (src[1] && src[1] < 0xFF)
			{
				src[1]++;
				resCnt++;
			}
		}

	return resCnt;
}

pp_int32 PatternEditorTools::insDecSelection(const Position& ss, const Position& se)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return 0;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se,
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return 0;

	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 resCnt = 0;

	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;

			if (src[1] > 1)
			{
				src[1]--;
				resCnt++;
			}
		}

	return resCnt;
}

pp_int32 PatternEditorTools::insIncTrack(pp_int32 track)
{
	Position ss, se = getMarkEnd();
	ss.channel = track;
	ss.inner = 0;
	ss.row = 0;

	se.channel = track;
	se.inner = 7;

	return insIncSelection(ss, se);
}

pp_int32 PatternEditorTools::insDecTrack(pp_int32 track)
{
	Position ss, se = getMarkEnd();
	ss.channel = track;
	ss.inner = 0;
	ss.row = 0;

	se.channel = track;
	se.inner = 7;

	return insDecSelection(ss, se);
}

pp_int32 PatternEditorTools::insRemapSelection(const Position& ss, const Position& se, pp_int32 oldIns, pp_int32 newIns)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return 0;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se, 
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return 0;
		
	// only entire instrument column is allowed
	if (selectionStartInner >= 1 && selectionStartInner<=2)
		selectionStartInner = 1;
	if (selectionEndInner >= 1 && selectionEndInner<=2)
		selectionEndInner = 2;
	// only entire volume column can be selected
	if (selectionStartInner >= 3 && selectionStartInner<=4)
		selectionStartInner = 3;
	if (selectionEndInner >= 3 && selectionEndInner<=4)
		selectionEndInner = 4;
		
	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 resCnt = 0;
	
	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;
		
			//memset(src, 0, slotSize);
			if (selectionWidth == 1)
			{
				if (selectionStartInner <= 1 && selectionEndInner >= 2)
					if (src[1] == oldIns)
					{
						src[1] = (mp_ubyte)newIns;
						resCnt++;
					}
			}
			else if (j == selectionStartChannel)
			{
				if (selectionStartInner <= 1)
					if (src[1] == oldIns)
					{
						src[1] = (mp_ubyte)newIns;
						resCnt++;
					}
			}
			else if (j == selectionEndChannel)
			{
				if (selectionEndInner >= 2)
					if (src[1] == oldIns)
					{
						src[1] = (mp_ubyte)newIns;
						resCnt++;
					}
			}
			else
			{
				if (src[1] == oldIns)
				{
					src[1] = (mp_ubyte)newIns;
					resCnt++;
				}
			}
		}

	return resCnt;
}

pp_int32 PatternEditorTools::insRemap(pp_int32 oldIns, pp_int32 newIns)
{
	Position ss = getMarkStart();
	Position se = getMarkEnd();

	return insRemapSelection(ss, se, oldIns, newIns);
}

pp_int32 PatternEditorTools::insRemapTrack(pp_int32 track, pp_int32 oldIns, pp_int32 newIns)
{
	Position ss, se = getMarkEnd();
	ss.channel = track;
	ss.inner = 0;
	ss.row = 0;

	se.channel = track;
	se.inner = 7;

	return insRemapSelection(ss, se, oldIns, newIns);
}

pp_int32 PatternEditorTools::noteTransposeSelection(const Position& ss, const Position& se, const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return 0;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se, 
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return 0;
		
	// only entire instrument column is allowed
	if (selectionStartInner >= 1 && selectionStartInner<=2)
		selectionStartInner = 1;
	if (selectionEndInner >= 1 && selectionEndInner<=2)
		selectionEndInner = 2;
	// only entire volume column can be selected
	if (selectionStartInner >= 3 && selectionStartInner<=4)
		selectionStartInner = 3;
	if (selectionEndInner >= 3 && selectionEndInner<=4)
		selectionEndInner = 4;
		
	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 resCnt = 0;
	pp_int32 fuckupCnt = 0;
	
	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;
		
			bool transpose = false;
		
			//memset(src, 0, slotSize);
			if (selectionWidth == 1)
			{
				if (selectionStartInner == 0 && selectionEndInner >= 0)
					transpose = true;
			}
			else if (j == selectionStartChannel)
			{
				if (selectionStartInner >= 0)
					transpose = true;
			}
			else if (j == selectionEndChannel)
			{
				if (selectionEndInner >= 0)
					transpose = true;
			}
			else
				transpose = true;

			if (transpose)
			{
				bool withinInsRange = false;
				bool withinNoteRange = false;
				
				if (src[0] && src[0] < 97)
				{
					if (src[1] >= transposeParameters.insRangeStart &&
						src[1] <= transposeParameters.insRangeEnd)
						withinInsRange = true;
					
					if (src[0] >= transposeParameters.noteRangeStart &&
						src[0] <= transposeParameters.noteRangeEnd)
						withinNoteRange = true;
				}
				
				if (withinNoteRange && withinInsRange)
				{
					pp_int32 note = src[0];
					note += transposeParameters.amount;
					if (!evaluate && note >= 1 && note <= transposeParameters.maxNoteRange)
					{
						src[0] = (mp_ubyte)note;
						resCnt++;
					}
					else if (evaluate && (note > transposeParameters.maxNoteRange || note < 1))
						fuckupCnt++;
					else if (!evaluate && (note > transposeParameters.maxNoteRange || note < 1))
					{
						src[0] = 0;
						fuckupCnt++;
					}
						
				}
			}
			
		}

	if (!evaluate)
		return resCnt;	
	else
		return fuckupCnt;
}

pp_int32 PatternEditorTools::noteTranspose(const TransposeParameters& transposeParameters, bool evaluate/* = false*/)
{
	Position ss = getMarkStart();
	Position se = getMarkEnd();

	return noteTransposeSelection(ss, se, transposeParameters, evaluate);
}

pp_int32 PatternEditorTools::noteTransposeTrack(pp_int32 track, const TransposeParameters& transposeParameters, bool evaluate/* = false*/)
{
	Position ss, se = getMarkEnd();
	ss.channel = track;
	ss.inner = 0;
	ss.row = 0;

	se.channel = track;
	se.inner = 7;

	return noteTransposeSelection(ss, se, transposeParameters, evaluate);
}

pp_int32 PatternEditorTools::interpolateValuesInSelection(const PatternEditorTools::Position& ss, const PatternEditorTools::Position& se)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return 0;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se, 
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return 0;

	// only entire instrument column is allowed
	if (selectionStartInner >= 1 && selectionStartInner<=2)
		selectionStartInner = 1;
	if (selectionEndInner >= 1 && selectionEndInner<=2)
		selectionEndInner = 2;
	// only entire volume column can be selected
	if (selectionStartInner >= 3 && selectionStartInner<=4)
		selectionStartInner = 3;
	if (selectionEndInner >= 3 && selectionEndInner<=4)
		selectionEndInner = 4;
	// only entire effect operand column
	if (selectionStartInner >= 6 && selectionStartInner<=7)
		selectionStartInner = 6;
	if (selectionEndInner >= 7 && selectionEndInner<=7)
		selectionEndInner = 7;
		
	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 numValues;
	pp_int32 vTab[256];
	pp_int32 vPosTab[256];
	
	ASSERT(pattern->rows <= 256);

	pp_int32 stats = 0;

	for (pp_int32 j = 0; j < selectionWidth; j++)
		for (pp_int32 k = 0; k < 8; k++)
		{			
			if (selectionWidth == 1)
			{
				if (selectionStartInner > k || selectionEndInner < k)
					continue;
			}
			if (j == selectionStartChannel)
			{
				if (selectionStartInner > k)
					continue;
			}
			if (j == selectionEndChannel)
			{
				if (selectionEndInner < k)
					continue;
			}

			if (k == 2 || k == 4 || k == 6)
				continue;
		
			numValues = 0;
			for (pp_int32 i = 0; i < selectionHeight; i++)
			{
				mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;
			
				switch (k)
				{
					case 0:
						if (*src && *src < 97)
						{
							vTab[numValues] = *src;
							vPosTab[numValues] = i;
							numValues++;
						}
						break;

					case 1:
						if (*(src+1))
						{
							vTab[numValues] = *(src+1);
							vPosTab[numValues] = i;
							numValues++;
						}
						break;

					case 3:
					{
						pp_int32 eff, op;
						
						eff = *(src + 2);
						op = *(src + 3);
						
						PatternTools::convertEffectsToFT2(eff, op);
						pp_int32 vol = PatternTools::getVolumeFromEffect(eff, op);

						if (vol)
						{
							vTab[numValues] = vol;
							vPosTab[numValues] = i;
							numValues++;
						}
						break;
					}
					
					case 7:
					{
						pp_int32 eff, op = 0xF;
						
						eff = *(src + 4);
						
						PatternTools::convertEffectsToFT2(eff, op);

						if (op)
						{
							vTab[numValues] = eff;
							vPosTab[numValues] = i;
							numValues++;
						}
						break;
					}

					// Entire effect
					case 5:
					{
						pp_int32 eff, op;
						
						eff = *(src + 4);
						
						// Make sure it's a valid effect
						if (!eff)
							op = 0;
						
						op = *(src + 5);
					
						if (eff || op)
						{
							vTab[numValues] = op;
							vPosTab[numValues] = i;
							numValues++;
						}
					}

				}
			
			}
			
			// we'll need at least 2 values to interpolate in between
			if (numValues >= 2)
			{
				for (pp_int32 n = 0; n < numValues-1; n++)
				{
					
					pp_int32 startv = vTab[n] << 16;
					pp_int32 adder = ((vTab[n+1] - vTab[n]) << 16) / (vPosTab[n+1] - vPosTab[n]);
					
					for (pp_int32 i = vPosTab[n]; i < vPosTab[n+1]; i++)
					{
						mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;
						
						pp_int32 v = startv>>16;
						
						switch (k)
						{
							case 0:
								if (!*src) 
									*src = (mp_ubyte)v;
								break;

							case 1:
								if (!*(src+1)) 
									*(src+1) = (mp_ubyte)v;
								break;
								
							case 3:
							{
								pp_int32 eff, op;
								
								PatternTools::convertVolumeToEffect(v, eff, op);
								
								if (eff)
								{
									*(src + 2) = (mp_ubyte)eff;
									*(src + 3) = (mp_ubyte)op;
								}
								
								break;
							}

							case 7:
							{
								pp_int32 eff = v, op = 0xF;
								
								PatternTools::convertEffectsFromFT2(eff, op);
								if (op)
								{
									*(src + 4) = (mp_ubyte)eff;
								}
								
								break;
							}

							case 5:
							{
								*(src + 5) = (mp_ubyte)v;
								
								break;
							}
						}
						
						startv+=adder;
						stats++;
						
					}
					
				}
			}
			
		}

	return stats;
}

#define ZERO_EFFECT(EFFECT) \
{ \
	if (optimizeParameters.EFFECT && \
		lastOperands[j].EFFECT == op && \
		lastOperands[j].EFFECT) \
	{ \
		src[5] = 0; resCnt++; \
	} \
	else if (optimizeParameters.EFFECT && \
			 lastOperands[j].EFFECT != op && \
			 op) \
		lastOperands[j].EFFECT = op; \
}

struct LastOperands
{
	mp_ubyte command_1xx;
	mp_ubyte command_2xx;
	mp_ubyte command_3xx;
	mp_ubyte command_4xx;
	mp_ubyte command_56Axx;
	mp_ubyte command_7xx;
	mp_ubyte command_E1x;
	mp_ubyte command_E2x;
	mp_ubyte command_EAx;
	mp_ubyte command_EBx;
	mp_ubyte command_Hxx;
	mp_ubyte command_Pxx;
	mp_ubyte command_Rxx;
	mp_ubyte command_X1x;
	mp_ubyte command_X2x;
};

pp_int32 PatternEditorTools::zeroOperandsSelection(const Position& ss, const Position& se, const OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return 0;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se, 
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return 0;
		
	// only entire operand column + effect is allowed
	if (selectionStartInner >= 5 && selectionStartInner<=7)
		selectionStartInner = 5;
	if (selectionEndInner >= 5 && selectionEndInner<=7)
		selectionEndInner = 7;
		
	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 resCnt = 0;

	LastOperands* lastOperands = new LastOperands[selectionWidth];
	
	memset(lastOperands, 0, sizeof(LastOperands) * selectionWidth);

	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;
		
			const mp_ubyte eff = src[4];
			const mp_ubyte op = src[5];
			
			bool doit = false;

			if (selectionWidth == 1)
			{
				if (selectionStartInner <= 5 && selectionEndInner >= 7)
				{
					doit = true;
				}
			}
			else if (j == selectionStartChannel)
			{
				if (selectionStartInner <= 5)
				{
					doit = true;
				}
			}
			else if (j == selectionEndChannel)
			{
				if (selectionEndInner >= 7)
				{
					doit = true;
				}
			}
			else
			{
				doit = true;
			}

			if (doit)
			{
				switch (eff)
				{
					// portamento up
					case 0x01:
						ZERO_EFFECT(command_1xx);
						break;
					// portamento down
					case 0x02:
						ZERO_EFFECT(command_2xx);
						break;
					// portamento to note
					case 0x03:
						ZERO_EFFECT(command_3xx);
						break;
					// vibrato
					case 0x04:
						ZERO_EFFECT(command_4xx);
						break;

					// portamento+volslide
					case 0x05:
					// vibrato+volslide
					case 0x06:
					// volslide
					case 0x0A:
						ZERO_EFFECT(command_56Axx);
						break;

					// tremolo
					case 0x07:
						ZERO_EFFECT(command_7xx);
						break;

					// fine porta up
					case 0x31:
						ZERO_EFFECT(command_E1x);
						break;
					// fine porta down
					case 0x32:
						ZERO_EFFECT(command_E2x);
						break;
					// fine volslide up
					case 0x3A:
						ZERO_EFFECT(command_EAx);
						break;
					// fine volslide down
					case 0x3B:
						ZERO_EFFECT(command_EBx);
						break;

					// global volume slide (Hxx)
					case 0x11:
						ZERO_EFFECT(command_Hxx);
						break;
					// panning slide (Pxx)
					case 0x19:
						ZERO_EFFECT(command_Pxx);
						break;
					// retrig (Rxx)
					case 0x1B:
						ZERO_EFFECT(command_Rxx);
						break;

					// x-fine porta up
					case 0x41:
						ZERO_EFFECT(command_X1x);
						break;
					// x-fine porta down
					case 0x42: 
						ZERO_EFFECT(command_X2x);
						break;
				}
			}

		}

	delete[] lastOperands;

	return resCnt;
}

pp_int32 PatternEditorTools::zeroOperands(const OperandOptimizeParameters& optimizeParameters, bool evaluate/* = false*/)
{
	Position ss = getMarkStart();
	Position se = getMarkEnd();

	return zeroOperandsSelection(ss, se, optimizeParameters, evaluate);
}

pp_int32 PatternEditorTools::zeroOperandsTrack(pp_int32 track, const OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	Position ss, se = getMarkEnd();
	ss.channel = track;
	ss.inner = 0;
	ss.row = 0;

	se.channel = track;
	se.inner = 7;

	return zeroOperandsSelection(ss, se, optimizeParameters, evaluate);
}

#define FILL_EFFECT(EFFECT) \
{ \
	if (optimizeParameters.EFFECT && \
		!op && \
		lastOperands[j].EFFECT) \
	{ \
		src[5] = lastOperands[j].EFFECT; resCnt++; \
	} \
	else if (optimizeParameters.EFFECT && op) \
		lastOperands[j].EFFECT = op; \
}

pp_int32 PatternEditorTools::fillOperandsSelection(const Position& ss, const Position& se, const OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return 0;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se, 
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return 0;

	// only entire operand column + effect is allowed
	if (selectionStartInner >= 5 && selectionStartInner<=7)
		selectionStartInner = 5;
	if (selectionEndInner >= 5 && selectionEndInner<=7)
		selectionEndInner = 7;
		
	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 resCnt = 0;

	LastOperands* lastOperands = new LastOperands[selectionWidth];
	
	memset(lastOperands, 0, sizeof(LastOperands) * selectionWidth);

	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;
		
			const mp_ubyte eff = src[4];
			const mp_ubyte op = src[5];
			
			bool doit = false;

			if (selectionWidth == 1)
			{
				if (selectionStartInner <= 5 && selectionEndInner >= 7)
				{
					doit = true;
				}
			}
			else if (j == selectionStartChannel)
			{
				if (selectionStartInner <= 5)
				{
					doit = true;
				}
			}
			else if (j == selectionEndChannel)
			{
				if (selectionEndInner >= 7)
				{
					doit = true;
				}
			}
			else
			{
				doit = true;
			}

			if (doit)
			{
				switch (eff)
				{
					// portamento up
					case 0x01:
						FILL_EFFECT(command_1xx);
						break;
					// portamento down
					case 0x02:
						FILL_EFFECT(command_2xx);
						break;
					// portamento to note
					case 0x03:
						FILL_EFFECT(command_3xx);
						break;
					// vibrato
					case 0x04:
						FILL_EFFECT(command_4xx);
						break;

					// portamento+volslide
					case 0x05:
					// vibrato+volslide
					case 0x06:
					// volslide
					case 0x0A:
						FILL_EFFECT(command_56Axx);
						break;

					// tremolo
					case 0x07:
						FILL_EFFECT(command_7xx);
						break;

					// fine porta up
					case 0x31:
						FILL_EFFECT(command_E1x);
						break;
					// fine porta down
					case 0x32:
						FILL_EFFECT(command_E2x);
						break;
					// fine volslide up
					case 0x3A:
						FILL_EFFECT(command_EAx);
						break;
					// fine volslide down
					case 0x3B:
						FILL_EFFECT(command_EBx);
						break;

					// global volume slide (Hxx)
					case 0x11:
						FILL_EFFECT(command_Hxx);
						break;
					// panning slide (Pxx)
					case 0x19:
						FILL_EFFECT(command_Pxx);
						break;
					// retrig (Rxx)
					case 0x1B:
						FILL_EFFECT(command_Rxx);
						break;

					// x-fine porta up
					case 0x41:
						FILL_EFFECT(command_X1x);
						break;
					// x-fine porta down
					case 0x42: 
						FILL_EFFECT(command_X2x);
						break;
				}
			}

		}

	delete[] lastOperands;

	return resCnt;
}

pp_int32 PatternEditorTools::fillOperands(const OperandOptimizeParameters& optimizeParameters, bool evaluate/* = false*/)
{
	Position ss = getMarkStart();
	Position se = getMarkEnd();

	return fillOperandsSelection(ss, se, optimizeParameters, evaluate);
}

pp_int32 PatternEditorTools::fillOperandsTrack(pp_int32 track, const OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	Position ss, se = getMarkEnd();
	ss.channel = track;
	ss.inner = 0;
	ss.row = 0;

	se.channel = track;
	se.inner = 7;

	return fillOperandsSelection(ss, se, optimizeParameters, evaluate);
}

pp_int32 PatternEditorTools::relocateCommandsSelection(const Position& ss, const Position& se, const RelocateParameters& relocateParameters, bool evaluate)
{
	if (!PatternEditorTools::hasValidSelection(pattern, ss, se))
		return 0;

	pp_int32 selectionStartChannel;
	pp_int32 selectionStartRow;
	pp_int32 selectionStartInner;
	pp_int32 selectionEndChannel;
	pp_int32 selectionEndRow;
	pp_int32 selectionEndInner;

	if (!normalizeSelection(pattern, ss, se, 
							selectionStartChannel, selectionStartRow, selectionStartInner,
							selectionEndChannel, selectionEndRow,selectionEndInner))
		return 0;

	// only entire operand column + effect is allowed
	if (selectionStartInner >= 5 && selectionStartInner<=7)
		selectionStartInner = 5;
	if (selectionEndInner >= 5 && selectionEndInner<=7)
		selectionEndInner = 7;
		
	pp_int32 selectionWidth = selectionEndChannel - selectionStartChannel + 1;
	pp_int32 selectionHeight = selectionEndRow - selectionStartRow + 1;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 result = 0;

	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* src = pattern->patternData + (selectionStartRow+i)*rowSizeSrc+(selectionStartChannel+j)*slotSize;		
			mp_ubyte* eff = src+2;
			
			bool doit = false;

			if (selectionWidth == 1)
			{
				if (selectionStartInner <= 5 && selectionEndInner >= 7)
				{
					doit = true;
				}
			}
			else if (j == selectionStartChannel)
			{
				if (selectionStartInner <= 5)
				{
					doit = true;
				}
			}
			else if (j == selectionEndChannel)
			{
				if (selectionEndInner >= 7)
				{
					doit = true;
				}
			}
			else
			{
				doit = true;
			}

			if (doit)
			{
				if (!eff[0])
				{
					// check for effects which can be converted into volume column
					switch (eff[2])
					{
						// convert portamento to note
					case 0x03:
						if (!relocateParameters.command_3xx)
							break;
						// if the lower nibble is used, this can't be
						// properly converted
						if (eff[3] & 0x0F)
							break;

						if (!evaluate)
						{
							eff[0] = eff[2];
							eff[1] = eff[3];
							eff[2] = eff[3] = 0;
						}
						result++;
						break;

						// convert vibrato
					case 0x04:
						if (!relocateParameters.command_4xx)
							break;
						// both nibbles are used, this can't be
						// properly converted
						if ((eff[3] & 0x0F) && (eff[3] & 0xF0))
							break;

						if (!evaluate)
						{
							eff[0] = eff[2];
							eff[1] = eff[3];
							eff[2] = eff[3] = 0;
						}

						result++;
						break;

						// convert set panning
					case 0x08:
						if (!relocateParameters.command_8xx)
							break;
						// if the lower nibble is used, this can't be
						// properly converted
						if ((eff[3] & 0x0F) && eff[3] != 0xFF)
							break;

						if (!evaluate)
						{
							eff[0] = eff[2];
							eff[1] = eff[3];
							eff[2] = eff[3] = 0;
						}

						result++;
						break;

						// convert set volume
					case 0x0C:
						if (!relocateParameters.command_Cxx)
							break;

						if (!evaluate)
						{
							eff[0] = eff[2];
							eff[1] = eff[3];
							eff[2] = eff[3] = 0;
						}

						result++;
						break;

						// convert volume slide
					case 0x0A:
						if (!relocateParameters.command_Axx)
							break;
						// both nibbles are used or operand is zero, 
						// this can't be properly converted
						if (((eff[3] & 0x0F) && (eff[3] & 0xF0)) || !eff[3])
							break;

						if (!evaluate)
						{
							eff[0] = eff[2];
							eff[1] = eff[3];
							eff[2] = eff[3] = 0;
						}

						result++;
						break;

						// convert panning slide
					case 0x19:
						if (!relocateParameters.command_Pxx)
							break;
						// both nibbles are used or operand is zero, 
						// this can't be properly converted
						if (((eff[3] & 0x0F) && (eff[3] & 0xF0)) || !eff[3])
							break;

						if (!evaluate)
						{
							eff[0] = eff[2];
							eff[1] = eff[3];
							eff[2] = eff[3] = 0;
						}

						result++;
						break;

						// convert fine volslide
					case 0x3A:
						if (!relocateParameters.command_EAx)
							break;
						// both nibbles are used or operand is zero, 
						// this can't be properly converted
						if (!eff[3])
							break;

						if (!evaluate)
						{
							eff[0] = eff[2];
							eff[1] = eff[3];
							eff[2] = eff[3] = 0;
						}

						result++;
						break;

						// convert fine volslide
					case 0x3B:
						if (!relocateParameters.command_EBx)
							break;
						// both nibbles are used or operand is zero, 
						// this can't be properly converted
						if (!eff[3])
							break;

						if (!evaluate)
						{
							eff[0] = eff[2];
							eff[1] = eff[3];
							eff[2] = eff[3] = 0;
						}

						result++;
						break;
					}
				}
			}

		}

	return result;
}

pp_int32 PatternEditorTools::relocateCommands(const RelocateParameters& relocateParameters, bool evaluate/* = false*/)
{
	Position ss = getMarkStart();
	Position se = getMarkEnd();

	return relocateCommandsSelection(ss, se, relocateParameters, evaluate);
}

pp_int32 PatternEditorTools::relocateCommandsTrack(pp_int32 track, const RelocateParameters& relocateParameters, bool evaluate)
{
	Position ss, se = getMarkEnd();
	ss.channel = track;
	ss.inner = 0;
	ss.row = 0;

	se.channel = track;
	se.inner = 7;

	return relocateCommandsSelection(ss, se, relocateParameters, evaluate);
}

static inline pp_uint8 scaleByte(pp_uint8 val, float scale)
{
	//pp_int32 v = (pp_int32)((float)val*scale);
	
	pp_int32 v = XModule::vol64to255((mp_sint32)((float)(((mp_sint32)val*64)/255)*scale));
	
	return (pp_uint8)(v > 255 ? 255 : v);
}

pp_int32 PatternEditorTools::scaleVolume(const Position& startSelection, const Position& endSelection, 
										 float startScale, float endScale, 
										 pp_uint32 numVisibleChannels, XModule* module)
{
	if (!hasValidSelection(pattern, startSelection, endSelection, numVisibleChannels))
		return 0;
		
	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 stats = 0;
	
	pp_int32 startSelRow = startSelection.row, endSelRow = endSelection.row;
	pp_int32 startSelChn = startSelection.channel, endSelChn = endSelection.channel;
	pp_int32 h;
	if (startSelRow > endSelRow) 
	{
		h = startSelRow; startSelRow = endSelRow; endSelRow = h;
	}	
	if (startSelChn > endSelChn) 
	{
		h = startSelChn; startSelChn = endSelChn; endSelChn = h;
	}	
	
	if (endSelChn > (signed)numVisibleChannels - 1)
		endSelChn = (signed)numVisibleChannels - 1;
	if (endSelRow > pattern->rows - 1)
		endSelRow = pattern->rows - 1;
	
	float step = (endScale - startScale) / ((endSelRow - startSelRow) != 0.0f ? 
											(float)(endSelRow - startSelRow) : 1.0f);
	
	for (pp_int32 r = startSelRow; r <= endSelRow; r++)
	{
		for (pp_int32 c = startSelChn; c <= endSelChn; c++)
		{
			mp_ubyte* src = pattern->patternData + r*rowSizeSrc+c*slotSize;
			
			mp_sint32 note = src[0];
			mp_sint32 ins = src[1];
			
			bool hasVolumeEff1 = (src[2] == 0xC);
			bool hasVolumeEff2 = (src[4] == 0xC);
			
			if (!hasVolumeEff1 && !hasVolumeEff2)
			{
				if (ins)
				{
					mp_sint32 s = 0;
					if (note)
						s = module->instr[ins-1].snum[note-1];
					else
						s = module->instr[ins-1].snum[0];
				
					if ((!src[2] && src[4]) || (!src[2] && !src[4]))
					{					
						src[2] = 0x0C;
						src[3] = scaleByte(module->smp[s].sample ? module->smp[s].vol : 255, startScale);
						stats++;
					}
					else if (src[2] && !src[4])
					{
						src[4] = 0x0C;
						src[5] = scaleByte(module->smp[s].sample ? module->smp[s].vol : 255, startScale);
						stats++;
					}
				}
			}
			
			if (hasVolumeEff1)
			{
				src[3] = scaleByte(src[3], startScale);
				stats++;
			}
			if (hasVolumeEff2)
			{
				src[5] = scaleByte(src[5], startScale);
				stats++;
			}
			
		}
		startScale+=step;
	}

	return stats;
}

pp_int32 PatternEditorTools::splitTrack(pp_int32 useChannels, bool insertNoteOff, 
										pp_int32 chn, pp_int32 fromRow, pp_int32 toRow, pp_uint32 numVisibleChannels)
{
	if (pattern == NULL || pattern->patternData == NULL) 
		return 0;

	pp_int32 stats = 0;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	pp_int32 dstChn = chn;
	pp_int32 lastChn = chn;

	pp_int32 lastNoteChannel = -1;
	
	for (pp_int32 r = fromRow; r < toRow; r++)
	{
		pp_int32 i;
	
		pp_int32 finalDstCh = dstChn % numVisibleChannels;
		
		mp_ubyte* src = pattern->patternData + r*rowSizeSrc+chn*slotSize;
		mp_ubyte* dst = pattern->patternData + r*rowSizeSrc+finalDstCh*slotSize;
	
		bool b = false;
		for (i = 1; i < slotSize; i++)
		{
			if (src[i])
			{					
				b = true;
				break;
			}
		}
		
		bool notePorta = false;
		for (i = 2; i < slotSize; i+=2)
		{
			if (src[i] == 0x3 || src[i] == 0x5)
				notePorta = true;
		}
		
		if (src[0] && src[0] <= XModule::NOTE_LAST && !notePorta)
			lastNoteChannel = dstChn;
		else if ((lastNoteChannel != -1) && 
				 ((src[0] > XModule::NOTE_LAST) || b || notePorta))
		{
			dstChn = lastNoteChannel;
			finalDstCh = dstChn % numVisibleChannels;
			dst = pattern->patternData + r*rowSizeSrc+finalDstCh*slotSize;
		}
		
		b = false;
		for (i = 0; i < slotSize; i++)
		{
			if (src[i])
			{					
				b = true;
				break;
			}
		}
		
		if (b && chn != finalDstCh)
		{
			memcpy(dst, src, slotSize);
			memset(src, 0, slotSize);
			if (insertNoteOff)
				*(pattern->patternData + r*rowSizeSrc+lastChn*slotSize) = XModule::NOTE_OFF;
			lastChn = finalDstCh;
			dstChn++;
			stats++;
		}
		else if (b && chn == finalDstCh)
		{
			if (insertNoteOff && finalDstCh != lastChn)
				*(pattern->patternData + r*rowSizeSrc+lastChn*slotSize) = XModule::NOTE_OFF;
			lastChn = dstChn;
			dstChn++;
		}
			
		if (dstChn > (chn+useChannels))
			dstChn = chn;		
	}

	return stats;
}

pp_int32 PatternEditorTools::swapChannels(pp_int32 dstChannel, pp_int32 srcChannel)
{
	if (pattern == NULL || pattern->patternData == NULL) 
		return 0;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*pattern->channum;

	char* buffer = new char[slotSize];

	pp_int32 stats = 0;
	
	for (pp_int32 r = 0; r < pattern->rows; r++)
	{
		mp_ubyte* src = pattern->patternData + r*rowSizeSrc+srcChannel*slotSize;
		mp_ubyte* dst = pattern->patternData + r*rowSizeSrc+dstChannel*slotSize;
	
		memcpy(buffer, src, slotSize);
		memcpy(src, dst, slotSize);
		memcpy(dst, buffer, slotSize);
	
		stats++;
	}
	
	delete[] buffer;

	return stats;
}

void PatternEditorTools::normalize()
{
	if (pattern == NULL || pattern->patternData == NULL) 
		return;
		
	pp_int32 selectionWidth = pattern->channum;
	pp_int32 selectionHeight = pattern->rows;

	mp_sint32 slotSize = pattern->effnum * 2 + 2;

	mp_ubyte* src = pattern->patternData;		
	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
			mp_ubyte* eff = src+2;
			for (pp_int32 k = 0; k < pattern->effnum; k++)
			{
				// check for empty arpeggio
				if (eff[k*2] == 0x20 && eff[k*2+1] == 0)
					eff[k*2] = 0;
			}
			src+=slotSize;
		}
	
}

bool PatternEditorTools::hasValidSelection(const TXMPattern* pattern, const Position& ss, const Position& se, pp_int32 numVisibleChannels/* = -1*/)
{
	if (pattern == NULL || pattern->patternData == NULL)
		return false;
		
	if (numVisibleChannels == -1 || numVisibleChannels > pattern->channum)
		numVisibleChannels = pattern->channum;
		
	Position selectionStart = ss, selectionEnd = se;

	flattenSelection(selectionStart, selectionEnd);		

	return !(selectionStart.channel > numVisibleChannels-1 ||
			selectionStart.row > pattern->rows-1 ||
			selectionEnd.channel < 0 ||
			selectionEnd.row < 0);
}

void PatternEditorTools::slotCopy(mp_ubyte* dst, mp_ubyte* src, pp_int32 from, pp_int32 to)
{
	pp_int32 i = 0;
	
	if (i >= from && i <= to)
	{
		*dst = *src;
	}
	i++;
		
	if (i >= from && i <= to)
	{
		*(dst+1) &= 0x0F;
		*(dst+1) |= *(src+1)&0xF0;		
	}
	i++;
	
	if (i >= from && i <= to)
	{
		*(dst+1) &= 0xF0;
		*(dst+1) |= *(src+1)&0xF;		
	}
	i++;

	if (i >= from && i <= to)
	{
		*(dst+2) = *(src+2);
		*(dst+3) = *(src+3);
	}
	i+=2;

	if (i >= from && i <= to)
	{
		*(dst+4) = *(src+4);
	}
	i++;

	if (i >= from && i <= to)
	{
		*(dst+5) &= 0x0F;
		*(dst+5) |= *(src+5)&0xF0;		
	}
	i++;
	
	if (i >= from && i <= to)
	{
		*(dst+5) &= 0xF0;
		*(dst+5) |= *(src+5)&0xF;		
	}
	i++;
}

void PatternEditorTools::slotTransparentCopy(mp_ubyte* dst, mp_ubyte* src, pp_int32 from, pp_int32 to)
{
	pp_int32 i = 0;
	
	if (i >= from && i <= to && *src)
	{
		*dst = *src;
	}
	i++;
		
	if (i >= from && i <= to && (*(src+1)&0xF0))
	{
		*(dst+1) &= 0x0F;
		*(dst+1) |= *(src+1)&0xF0;		
	}
	i++;
	
	if (i >= from && i <= to && (*(src+1)&0xF))
	{
		*(dst+1) &= 0xF0;
		*(dst+1) |= *(src+1)&0xF;		
	}
	i++;

	if (i >= from && i <= to && (*(src+2) || *(src+3)))
	{
		*(dst+2) = *(src+2);
		*(dst+3) = *(src+3);
	}
	i+=2;

	if (i >= from && i <= to && (*(src+4)))
	{
		*(dst+4) = *(src+4);
	}
	i++;

	if (i >= from && i <= to && (*(src+5)&0xF0))
	{
		*(dst+5) &= 0x0F;
		*(dst+5) |= *(src+5)&0xF0;		
	}
	i++;
	
	if (i >= from && i <= to && (*(src+5)&0xF))
	{
		*(dst+5) &= 0xF0;
		*(dst+5) |= *(src+5)&0xF;		
	}
	i++;
}

void PatternEditorTools::slotClear(mp_ubyte* dst, pp_int32 from, pp_int32 to)
{
	pp_int32 i = 0;
	
	if (i >= from && i <= to)
	{
		*dst = 0;
	}
	i++;
		
	if (i >= from && i <= to)
	{
		*(dst+1) &= 0x0F;
	}
	i++;
	
	if (i >= from && i <= to)
	{
		*(dst+1) &= 0xF0;
	}
	i++;

	if (i >= from && i <= to)
	{
		*(dst+2) = 0;
		*(dst+3) = 0;
	}
	i+=2;

	if (i >= from && i <= to)
	{
		*(dst+4) = 0;
	}
	i++;

	if (i >= from && i <= to)
	{
		*(dst+5) &= 0x0F;
	}
	i++;
	
	if (i >= from && i <= to)
	{
		*(dst+5) &= 0xF0;
	}
	i++;
}


