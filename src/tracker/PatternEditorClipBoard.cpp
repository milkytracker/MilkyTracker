/*
 *  PatternEditorClipBoard.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Fri Mar 11 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
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

	pp_int32 ssc = ss.channel;
	pp_int32 ssr = ss.row;
	pp_int32 ssi = ss.inner;

	pp_int32 sec = se.channel;
	pp_int32 ser = se.row;
	pp_int32 sei = se.inner;

	if (ssc < 0 && ssr < 0 && sec < 0 && ser < 0)
		return;

	if (ssc < 0) ssc = 0;
	if (ssc >= pattern.channum) ssc = pattern.channum-1;
	if (sec < 0) sec = 0;
	if (sec >= pattern.channum) sec = pattern.channum-1;

	if (ssr < 0) ssr = 0;
	if (ssr >= pattern.rows) ssr = pattern.rows-1;
	if (ser < 0) ser = 0;
	if (ser >= pattern.rows) ser = pattern.rows-1;

	if (ssi < 0) ssi = 0;
	if (ssi > 7) ssi = 7;
	if (sei < 0) sei = 0;
	if (sei > 7) sei = 7;

	selectionStart.channel = 0;
	selectionStart.row = 0;
	selectionEnd.channel = 0;
	selectionEnd.row = 0;

	if (ssc > sec)
	{
		selectionStart.channel = sec;
		selectionEnd.channel = ssc;
		selectionStart.inner = sei;
		selectionEnd.inner = ssi;
	}
	else
	{
		selectionStart.channel = ssc;
		selectionEnd.channel = sec;
		selectionStart.inner = ssi;
		selectionEnd.inner = sei;
	}

	if (ssr > ser)
	{
		selectionStart.row = ser;
		selectionEnd.row = ssr;
	}
	else
	{
		selectionStart.row = ssr;
		selectionEnd.row = ser;
	}
	
	if (selectionStart.channel == selectionEnd.channel && selectionEnd.inner < selectionStart.inner)
	{
		mp_sint32 h = selectionEnd.inner;
		selectionEnd.inner = selectionStart.inner;
		selectionStart.inner = h;
	}	

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
