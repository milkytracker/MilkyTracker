/*
 *  tracker/PatternEditorClipBoard.cpp
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
 *  PatternEditorClipBoard.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Fri Mar 11 2005.
 *
 */

#include "PatternEditor.h"
#include "XModule.h"

PatternEditor::ClipBoard::ClipBoard() :
	buffer(0)
{
	selectionStart.channel = -1;
	selectionStart.row = -1;
	selectionEnd.channel = -1;
	selectionEnd.row = -1;
}

PatternEditor::ClipBoard::~ClipBoard()
{
	delete buffer;
}

void PatternEditor::ClipBoard::makeCopy(TXMPattern& pattern, const PatternEditorTools::Position& ss, const PatternEditorTools::Position& se, bool clear/*= false*/)
{
	if (!PatternEditorTools::hasValidSelection(&pattern, ss, se))
		return;

	if (!PatternEditorTools::normalizeSelection(&pattern, ss, se, 
												selectionStart.channel, selectionStart.row, selectionStart.inner,
												selectionEnd.channel, selectionEnd.row, selectionEnd.inner))
		return;

	// only entire instrument column is allowed
	if (selectionStart.inner >= 1 && selectionStart.inner<=2)
		selectionStart.inner = 1;
	if (selectionEnd.inner >= 1 && selectionEnd.inner<=2)
		selectionEnd.inner = 2;
	// only entire volume column can be selected
	if (selectionStart.inner >= 3 && selectionStart.inner<=4)
		selectionStart.inner = 3;
	if (selectionEnd.inner >= 3 && selectionEnd.inner<=4)
		selectionEnd.inner = 4;
	
	selectionWidth = selectionEnd.channel - selectionStart.channel + 1;
	selectionHeight = selectionEnd.row - selectionStart.row + 1;

	mp_sint32 slotSize = pattern.effnum * 2 + 2;
	
	ASSERT(slotSize == 6);
	
	mp_sint32 rowSizeDst = slotSize*selectionWidth;
	mp_sint32 rowSizeSrc = slotSize*pattern.channum;
	mp_sint32 bufferSize = selectionHeight * rowSizeDst;

	if (buffer)
		delete[] buffer;

	buffer = new mp_ubyte[bufferSize];

	if (buffer == NULL)
		return;

	memset(buffer, 0, bufferSize);

	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
		
			mp_ubyte* src = pattern.patternData + (selectionStart.row+i)*rowSizeSrc+(selectionStart.channel+j)*slotSize;
			mp_ubyte* dst = buffer + i*rowSizeDst+j*slotSize;
		
			if (selectionWidth == 1)
			{
				PatternEditorTools::slotCopy(dst, src, selectionStart.inner, selectionEnd.inner);
				if (clear)
					PatternEditorTools::slotClear(src, selectionStart.inner, selectionEnd.inner);
			}
			else if (j == 0)
			{
				PatternEditorTools::slotCopy(dst, src, selectionStart.inner, 7);
				if (clear)
					PatternEditorTools::slotClear(src, selectionStart.inner, 7);
			}
			else if (j+selectionStart.channel == selectionEnd.channel)
			{
				PatternEditorTools::slotCopy(dst, src, 0, selectionEnd.inner);
				if (clear)
					PatternEditorTools::slotClear(src, 0, selectionEnd.inner);
			}
			else
			{
				//memcpy(dst, src, slotSize);
				PatternEditorTools::slotCopy(dst, src, 0, 7);
				if (clear)
					PatternEditorTools::slotClear(src, 0, 7);
			}
		}

}

void PatternEditor::ClipBoard::paste(TXMPattern& pattern, pp_int32 sc, pp_int32 sr, bool transparent/* = false*/)
{
	if (pattern.patternData == NULL)
		return;

	if (buffer == NULL)
		return;

	mp_sint32 slotSize = pattern.effnum * 2 + 2;
	mp_sint32 rowSizeSrc = slotSize*selectionWidth;
	mp_sint32 rowSizeDst = slotSize*pattern.channum;

	for (pp_int32 i = 0; i < selectionHeight; i++)
		for (pp_int32 j = 0; j < selectionWidth; j++)
		{
		
			if (i + sr < pattern.rows && i + sr >= 0 &&
				j + sc < pattern.channum && j + sc >= 0)
			{
				mp_ubyte* dst = pattern.patternData + (sr+i)*rowSizeDst+(sc+j)*slotSize;
				mp_ubyte* src = buffer + i*rowSizeSrc+j*slotSize;
				
				if (selectionWidth == 1)
				{
					if (transparent)
						PatternEditorTools::slotTransparentCopy(dst, src, selectionStart.inner, selectionEnd.inner);
					else
						PatternEditorTools::slotCopy(dst, src, selectionStart.inner, selectionEnd.inner);
				}
				else if (j == 0)
				{
					if (transparent)
						PatternEditorTools::slotTransparentCopy(dst, src, selectionStart.inner, 7);
					else
						PatternEditorTools::slotCopy(dst, src, selectionStart.inner, 7);
				}
				else if (j+selectionStart.channel == selectionEnd.channel)
				{
					if (transparent)
						PatternEditorTools::slotTransparentCopy(dst, src, 0, selectionEnd.inner);
					else
						PatternEditorTools::slotCopy(dst, src, 0, selectionEnd.inner);
				}
				else
				{
					if (transparent)
						PatternEditorTools::slotTransparentCopy(dst, src, 0, 7);
					else
						PatternEditorTools::slotCopy(dst, src, 0, 7);
				}
			}
		}
}
