/*
 *  tracker/PatternEditor.cpp
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
 *  PatternEditor.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.11.07.
 *
 */

#include "PatternEditor.h"
#include "XModule.h"
#include "PatternTools.h"
#include "SimpleVector.h"

void PatternEditor::Selection::backup()
{
	startCopy = start;
	endCopy = end;
	copyValid = true;
}

void PatternEditor::Selection::restore()
{
	start = startCopy;
	end = endCopy;
	copyValid = false;
}

PatternEditor::ClipBoard* PatternEditor::ClipBoard::instances[PatternEditor::ClipBoardTypeLAST] = {NULL, NULL, NULL};

PatternEditor::ClipBoard* PatternEditor::ClipBoard::getInstance(ClipBoardTypes type)
{
	if (instances[type] == NULL)
		instances[type] = new ClipBoard();
		
	return instances[type];
}

void PatternEditor::prepareUndo()
{
	PatternEditorTools patternEditorTools(pattern); 
	patternEditorTools.normalize(); 

	undoUserData.clear();
	notifyListener(NotificationFeedUndoData);

	delete before; 
	before = new PatternUndoStackEntry(*pattern, cursor.channel, cursor.row, cursor.inner, &undoUserData);
}

bool PatternEditor::finishUndo(LastChanges lastChange, bool nonRepeat/* = false*/)
{
	bool result = false;

	PatternEditorTools patternEditorTools(pattern); 
	patternEditorTools.normalize(); 

	undoUserData.clear();
	notifyListener(NotificationFeedUndoData);

	PatternUndoStackEntry after(*pattern, cursor.channel, cursor.row, cursor.inner, &undoUserData); 
	if (*before != after) 
	{ 
		PatternEditorTools::Position afterPos;
		afterPos.channel = after.getCursorPositionChannel();
		afterPos.row = after.getCursorPositionRow();
		afterPos.inner = after.getCursorPositionInner();

		PatternEditorTools::Position beforePos;		
		beforePos.channel = before->getCursorPositionChannel();
		beforePos.row = before->getCursorPositionRow();
		beforePos.inner = before->getCursorPositionInner();
	
		result = true;
		
		lastOperationDidChangeRows = after.GetPattern().rows != before->GetPattern().rows;
		lastOperationDidChangeCursor = beforePos != afterPos;
		notifyListener(NotificationChanges);
		if (undoStack) 
		{ 
			if (nonRepeat && this->lastChange != lastChange)
				undoStack->Push(*before); 	
			else if (!nonRepeat)
				undoStack->Push(*before); 				
			undoStack->Push(after); 
			undoStack->Pop(); 
		} 
	} 
	this->lastChange = lastChange; 

	return result;
}

PatternEditor::PatternEditor() :
	EditorBase(),
	pattern(NULL),
	numVisibleChannels(-1),
	autoResize(false), 
	currentInstrument(1),
	instrumentEnabled(true),
	instrumentBackTrace(false),
	currentOctave(5),
	before(NULL),
	undoStack(NULL),
	lastChange(LastChangeNone)	
{
	// Undo history
	undoHistory = new UndoHistory<TXMPattern, PatternUndoStackEntry>(UNDOHISTORYSIZE_PATTERNEDITOR);
	
	resetCursor();
	resetSelection();
	
	memset(effectMacros, 0, sizeof(effectMacros));
}

PatternEditor::~PatternEditor()
{
	delete undoHistory;
	delete undoStack;
	delete before;
}

void PatternEditor::attachPattern(TXMPattern* pattern, XModule* module) 
{
	if (this->pattern == pattern && this->module == module)
		return;

	// --------- update undo history information --------------------	
	if (undoStack)
	{	
		// if the undo stack is empty, we don't need to save current undo stack
		if (!undoStack->IsEmpty() || !undoStack->IsTop())
		{	
			undoStack = undoHistory->getUndoStack(pattern, this->pattern, undoStack);
		}
		// delete it if it's empty
		else
		{
			delete undoStack;
			undoStack = NULL;

			undoStack = undoHistory->getUndoStack(pattern, NULL, NULL);
		}
	}

	attachModule(module);	
	this->pattern = pattern; 
	
	// couldn't get any from history, create new one
	if (!undoStack)
	{
		undoStack = new PPUndoStack<PatternUndoStackEntry>(UNDODEPTH_PATTERNEDITOR);
	}

	notifyListener(NotificationReload);
}

void PatternEditor::reset()
{
	resetSelection();

	delete undoHistory;
	undoHistory = new UndoHistory<TXMPattern, PatternUndoStackEntry>(UNDOHISTORYSIZE_PATTERNEDITOR);
			
	delete undoStack;
	undoStack = new PPUndoStack<PatternUndoStackEntry>(UNDODEPTH_PATTERNEDITOR);	
}

pp_int32 PatternEditor::getNumChannels() const
{
	if (numVisibleChannels >= 0)
		return numVisibleChannels;
		
	if (pattern == NULL)
		return 0;
		
	return pattern->channum;
}

pp_int32 PatternEditor::getNumRows() const
{	
	if (pattern == NULL)
		return 0;
		
	return pattern->rows;
}

bool PatternEditor::hasValidSelection()
{
	return PatternEditorTools::hasValidSelection(pattern, selection.start, selection.end, getNumChannels());
}

bool PatternEditor::selectionContains(const PatternEditorTools::Position& pos)
{
	return PatternEditorTools::selectionContains(pattern, selection.start, selection.end, pos);
}

bool PatternEditor::canMoveSelection(pp_int32 channels, pp_int32 rows)
{
	PatternEditorTools::Position ss = selection.start;
	PatternEditorTools::Position se = selection.end;
	ss.channel += channels;
	ss.row += rows;
	se.channel += channels;
	se.row += rows;
	return PatternEditorTools::hasValidSelection(pattern, ss, se, getNumChannels());
}

void PatternEditor::selectChannel(pp_int32 channel)
{
	if (pattern == NULL)
		return;

	selection.start.channel = channel;
	selection.start.row = 0;
	selection.start.inner = 0;
	selection.end.channel = channel;
	selection.end.row = pattern->rows-1;
	selection.end.inner = 7;
}

void PatternEditor::selectAll()
{
	if (pattern == NULL)
		return;
		
	selection.start.channel = 0;
	selection.start.row = 0;
	selection.start.inner = 0;
	selection.end.channel = getNumChannels()-1;
	selection.end.row = pattern->rows-1;
	selection.end.inner = 7;	
}

pp_int32 PatternEditor::getCurrentActiveInstrument()
{
	PatternTools patternTools;

	if (pattern == NULL)
		return -1;

	if (pattern->patternData == NULL)
		return -1;
		
	if (!instrumentEnabled)
		return 0;
		
	if (instrumentBackTrace)
	{
		for (pp_int32 i = cursor.row; i >= 0; i--)
		{
			patternTools.setPosition(pattern, cursor.channel, i);
			
			pp_int32 ins = patternTools.getInstrument();
			
			if (ins != 0)
			{
				return ins;
			}
		}
		
		//return -1;
	}
	
	return currentInstrument;
}

bool PatternEditor::undo()
{
	if (undoStack->IsEmpty()) return false;
	if (undoStack)
		return revoke(undoStack->Pop());
	return false;	
}

bool PatternEditor::redo()
{
	if (undoStack->IsTop()) return false;
	if (undoStack)
		return revoke(undoStack->Advance());
	return false;
}

bool PatternEditor::revoke(const PatternUndoStackEntry* stackEntry)
{
	const TXMPattern& stackPattern = stackEntry->GetPattern();

	enterCriticalSection();

	bool res = false;

	if (stackPattern.rows != pattern->rows ||
		stackPattern.channum != pattern->channum ||
		stackPattern.effnum != pattern->effnum)
	{
		pattern->rows = stackPattern.rows;
		pattern->channum = stackPattern.channum;
		pattern->effnum = stackPattern.effnum;
	
		mp_sint32 patternSize = pattern->rows*pattern->channum*(2+pattern->effnum*2);	

		if (pattern->patternData)
		{
			delete[] pattern->patternData;
			pattern->patternData = new mp_ubyte[patternSize];
			memset(pattern->patternData, 0, patternSize);
		}
	}
	
	if (stackPattern.rows == pattern->rows &&
		stackPattern.channum == pattern->channum &&
		stackPattern.effnum == pattern->effnum)
	{
		cursor.channel = stackEntry->getCursorPositionChannel();
		cursor.row = stackEntry->getCursorPositionRow();
		cursor.inner = stackEntry->getCursorPositionInner();
		
		pattern->decompress(stackPattern.patternData, stackPattern.len);

		// keep over userdata
		undoUserData = stackEntry->getUserData();
		notifyListener(NotificationFetchUndoData);

		notifyListener(NotificationChanges);
		res = true;
	}
	
	leaveCriticalSection();
	
	return res;
}

void PatternEditor::clearRange(const PatternEditorTools::Position& rangeStart, const PatternEditorTools::Position& rangeEnd)
{
	PatternEditorTools patternEditorTools(pattern);
	patternEditorTools.clearSelection(rangeStart, rangeEnd);
}

void PatternEditor::clearSelection()
{
	if (pattern == NULL)
		return;

	if (pattern->patternData == NULL)
		return;
	
	prepareUndo();
	
	clearRange(selection.start, selection.end);
	
	finishUndo(LastChangeDeleteSelection);
}

void PatternEditor::clearPattern()
{
	if (pattern == NULL)
		return;

	if (pattern->patternData == NULL)
		return;

	pp_int32 patSize = pattern->channum * (pattern->effnum*2+2) * pattern->rows;	

	memset(pattern->patternData, 0, patSize);
}

void PatternEditor::cut(ClipBoard& clipBoard)
{
	if (pattern == NULL)
		return;

	if (pattern->patternData == NULL)
		return;
	
	prepareUndo();
	
	clipBoard.makeCopy(*pattern,
					   getSelection().start,
					   getSelection().end,
					   true);
	
	finishUndo(LastChangeCut);
}

void PatternEditor::copy(ClipBoard& clipBoard)
{
	clipBoard.makeCopy(*pattern,
					   getSelection().start,
					   getSelection().end);
}

void PatternEditor::paste(ClipBoard& clipBoard, bool transparent/* = false*/, pp_int32 fromChannel/* = -1*/)
{
	if (pattern == NULL)
		return;

	if (pattern->patternData == NULL)
		return;

	prepareUndo();

	if (autoResize && cursor.row + clipBoard.getNumRows() > pattern->rows)
	{
		pp_int32 newLen = cursor.row + clipBoard.getNumRows();
		if (newLen > 256)
			newLen = 256;
		resizePattern(newLen, false);
	}
	
	if (fromChannel == -1)
		clipBoard.paste(*pattern, cursor.channel, cursor.row, transparent);
	else
		clipBoard.paste(*pattern, fromChannel, cursor.row, transparent);
	
	finishUndo(LastChangePaste);
}

void PatternEditor::cut(ClipBoardTypes clipBoardType)
{
	cut(*ClipBoard::getInstance(clipBoardType));
}

void PatternEditor::copy(ClipBoardTypes clipBoardType)
{
	copy(*ClipBoard::getInstance(clipBoardType));
}

void PatternEditor::paste(ClipBoardTypes clipBoardType, bool transparent/* = false*/, pp_int32 fromChannel/* = -1*/)
{
	paste(*ClipBoard::getInstance(clipBoardType), transparent, fromChannel);
}

bool PatternEditor::resizePattern(pp_int32 newRowNum, bool withUndo /*= true*/)
{
	if (newRowNum < 1 || newRowNum > 256)
		return false;
	
	if (pattern == NULL) 
		return false;
	
	if (pattern->patternData == NULL)
		return false;

	if (newRowNum == pattern->rows)
		return true;

	enterCriticalSection();

	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	// allocate half of the space of the current pattern
	mp_sint32 patternSize = slotSize * pattern->channum * newRowNum;
	
	mp_ubyte* newPatternData = new mp_ubyte[patternSize];
	
	memset(newPatternData, 0, patternSize);
	
	mp_sint32 numRows = newRowNum < pattern->rows ? newRowNum : pattern->rows;
	
	for (mp_sint32 i = 0; i < numRows; i++)
	{
		mp_sint32 srcOffset = i * slotSize * pattern->channum;
		mp_sint32 dstOffset = i * slotSize * pattern->channum;
		for (mp_sint32 j = 0; j < slotSize * pattern->channum; j++)
			newPatternData[dstOffset++] = pattern->patternData[srcOffset++];
	}
	
	if (withUndo)
	{
		prepareUndo();
		
		delete[] pattern->patternData;
	
		pattern->patternData = newPatternData;
		
		pattern->rows = newRowNum;
		
		// see if something has changed, if this is the case
		// save original & changes
		// Special treatment for pattern resizing:
		// If user resizes pattern and the last stack entry has already been
		// a resize modification we don't store the current changes		
		finishUndo(LastChangeResizePattern, true);		
	}
	else
	{
		delete[] pattern->patternData;
		
		pattern->patternData = newPatternData;
		
		pattern->rows = newRowNum;
		
		lastOperationDidChangeRows = true;
		lastOperationDidChangeCursor = false;
		notifyListener(NotificationChanges);
	}
	
	leaveCriticalSection();
	
	return true;
}

bool PatternEditor::expandPattern()
{
	if (pattern == NULL) 
		return false;
		
	if (pattern->patternData == NULL)
		return false;

	enterCriticalSection();

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	bool res = patternEditorTools.expandPattern();

	// see if something has changed, if this is the case
	// save original & changes
	finishUndo(LastChangeExpandPattern);
	
	leaveCriticalSection();
	
	return res;
}

bool PatternEditor::shrinkPattern()
{
	if (pattern == NULL) 
		return false;
		
	if (pattern->patternData == NULL)
		return false;

	enterCriticalSection();

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	bool res = patternEditorTools.shrinkPattern();
	
	// see if something has changed, if this is the case
	// save original & changes
	finishUndo(LastChangeShrinkPattern);

	leaveCriticalSection();

	return res;
}

bool PatternEditor::loadExtendedPattern(const PPSystemString& fileName)
{
	if (pattern == NULL)
		return false;

	if (pattern->patternData == NULL)
		return false;
	
	enterCriticalSection();
				
	prepareUndo();
	
	bool res = pattern->loadExtendedPattern(fileName);

	// see if something has changed, if this is the case
	// save original & changes
	finishUndo(LastChangeLoadXPattern);
	
	leaveCriticalSection();
	
	return res;
}

bool PatternEditor::saveExtendedPattern(const PPSystemString& fileName)
{
	if (pattern == NULL)
		return false;

	if (pattern->patternData == NULL)
		return false;

	return pattern->saveExtendedPattern(fileName);
}

bool PatternEditor::loadExtendedTrack(const PPSystemString& fileName)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;
		
	enterCriticalSection();
		
	prepareUndo();
	
	bool res = pattern->loadExtendedTrack(fileName, cursor.channel);

	// see if something has changed, if this is the case
	// save original & changes
	finishUndo(LastChangeLoadXTrack);
	
	leaveCriticalSection();
	
	return res;
}

bool PatternEditor::saveExtendedTrack(const PPSystemString& fileName)
{
	if (pattern == NULL)
		return false;

	if (pattern->patternData == NULL)
		return false;

	return pattern->saveExtendedTrack(fileName, cursor.channel);
}

pp_int32 PatternEditor::insRemapTrack(pp_int32 oldIns, pp_int32 newIns)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);

	pp_int32 resCnt = patternEditorTools.insRemapTrack(cursor.channel, oldIns, newIns);
	
	finishUndo(LastChangeInsRemapTrack);
	
	return resCnt;
}

pp_int32 PatternEditor::insRemapPattern(pp_int32 oldIns, pp_int32 newIns)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);

	pp_int32 resCnt = patternEditorTools.insRemap(oldIns, newIns);

	finishUndo(LastChangeInsRemapPattern);
	
	return resCnt;
}

pp_int32 PatternEditor::insIncSelection()
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	pp_int32 resCnt = patternEditorTools.insIncSelection(getSelection().start, getSelection().end);

	finishUndo(LastChangeInsIncSelection);

	return resCnt;
}

pp_int32 PatternEditor::insDecSelection()
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	pp_int32 resCnt = patternEditorTools.insDecSelection(getSelection().start, getSelection().end);

	finishUndo(LastChangeInsDecSelection);

	return resCnt;
}

pp_int32 PatternEditor::insIncTrack()
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	pp_int32 resCnt = patternEditorTools.insIncTrack(getCursor().channel);

	finishUndo(LastChangeInsIncTrack);

	return resCnt;
}

pp_int32 PatternEditor::insDecTrack()
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	pp_int32 resCnt = patternEditorTools.insDecTrack(getCursor().channel);

	finishUndo(LastChangeInsDecTrack);

	return resCnt;
}

pp_int32 PatternEditor::insRemapSelection(pp_int32 oldIns, pp_int32 newIns)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	pp_int32 resCnt = patternEditorTools.insRemapSelection(getSelection().start, getSelection().end, oldIns, newIns);

	finishUndo(LastChangeInsRemapSelection);
		
	return resCnt;
}

pp_int32 PatternEditor::noteTransposeTrackCore(const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;
	pp_int32 fuckupCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	if (!evaluate)
		resCnt = patternEditorTools.noteTransposeTrack(cursor.channel, transposeParameters, evaluate);
	else
		fuckupCnt = patternEditorTools.noteTransposeTrack(cursor.channel, transposeParameters, evaluate);
	
	finishUndo(LastChangeNoteTransposeTrack);
	
	if (!evaluate)
		return resCnt;	
	else
		return fuckupCnt;
}

pp_int32 PatternEditor::noteTransposePatternCore(const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);

	pp_int32 resCnt = 0;
	pp_int32 fuckupCnt = 0;
	
	if (!evaluate)
		resCnt = patternEditorTools.noteTranspose(transposeParameters, evaluate);
	else
		fuckupCnt = patternEditorTools.noteTranspose(transposeParameters, evaluate);

	finishUndo(LastChangeNoteTransposePattern);
	
	if (!evaluate)
		return resCnt;	
	else
		return fuckupCnt;
}

pp_int32 PatternEditor::noteTransposeSelectionCore(const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	pp_int32 resCnt = 0;
	pp_int32 fuckupCnt = 0;
	
	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);

	if (!evaluate)
		resCnt = patternEditorTools.noteTransposeSelection(getSelection().start, getSelection().end, transposeParameters, evaluate);
	else
		fuckupCnt = patternEditorTools.noteTransposeSelection(getSelection().start, getSelection().end, transposeParameters, evaluate);

	finishUndo(LastChangeNoteTransposeSelection);
		
	if (!evaluate)
		return resCnt;	
	else
		return fuckupCnt;
}

pp_int32 PatternEditor::interpolateValuesInSelection()
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	pp_int32 stats = 0;
	
	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	stats = patternEditorTools.interpolateValuesInSelection(getSelection().start, getSelection().end);

	finishUndo(LastChangeNoteInterpolate);

	return stats;
}

pp_int32 PatternEditor::splitTrack(pp_int32 useChannels, bool selectionOnly, bool insertNoteOff)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	pp_int32 stats = 0;
	
	pp_int32 chn = cursor.channel;

	pp_int32 fromRow = 0;
	pp_int32 toRow = pattern->rows;
	
	if (selectionOnly && hasValidSelection())
	{
		if (getSelection().start.channel != getSelection().end.channel)
			return -1;
		
		chn = getSelection().start.channel;
		fromRow = getSelection().start.row;
		toRow = getSelection().end.row+1;
	}
	else if (selectionOnly && !hasValidSelection())
		return 0;
	
	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	
	stats = patternEditorTools.splitTrack(useChannels, insertNoteOff, chn, fromRow, toRow, getNumChannels());
	
	finishUndo(LastChangeSplitTrack);	

	return stats;
}

pp_int32 PatternEditor::swapChannels(pp_int32 dstChannel, pp_int32 srcChannel)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	if (dstChannel == srcChannel)
		return 0;

	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);
	
	pp_int32 stats = patternEditorTools.swapChannels(dstChannel, srcChannel);
	
	finishUndo(LastChangeSwapChannels);	

	return stats;
}

pp_int32 PatternEditor::scaleVolume(const PatternEditorTools::Position& startSelection, const PatternEditorTools::Position& endSelection, float startScale, float endScale)
{
	prepareUndo();

	PatternEditorTools patternEditorTools(pattern);

	pp_int32 stats = patternEditorTools.scaleVolume(startSelection, 
											   endSelection, 
											   startScale, 
											   endScale, 
											   getNumChannels(), module);
	
	finishUndo(LastChangeScaleVolume);	

	return stats;
}

pp_int32 PatternEditor::scaleVolumeTrack(float startScale, float endScale)
{
	if (pattern == NULL)
		return 0;
		
	PatternEditorTools::Position startSel, endSel;
	
	startSel.row = startSel.inner = 0;
	startSel.channel = cursor.channel;
	endSel = startSel;
	endSel.row = pattern->rows-1;

	return scaleVolume(startSel, endSel, startScale, endScale);
}

pp_int32 PatternEditor::scaleVolumePattern(float startScale, float endScale)
{
	if (pattern == NULL)
		return 0;
		
	PatternEditorTools::Position startSel, endSel;
	
	startSel.channel = startSel.row = startSel.inner = 0;
	endSel.channel = pattern->channum-1; endSel.row = pattern->rows-1; endSel.inner = 7;

	return scaleVolume(startSel, endSel, startScale, endScale);
}

pp_int32 PatternEditor::scaleVolumeSelection(float startScale, float endScale)
{
	if (pattern == NULL || !hasValidSelection())
		return 0;

	return scaleVolume(getSelection().start, getSelection().end, startScale, endScale);
}

pp_int32 PatternEditor::zeroOperandsTrack(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.zeroOperandsTrack(cursor.channel, optimizeParameters, evaluate);
	
	finishUndo(LastChangeZeroOperandsTrack);
	
	return resCnt;	
}

pp_int32 PatternEditor::zeroOperandsPattern(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.zeroOperands(optimizeParameters, evaluate);
	
	finishUndo(LastChangeZeroOperandsPattern);
	
	return resCnt;	
}

pp_int32 PatternEditor::zeroOperandsSelection(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.zeroOperandsSelection(getSelection().start, getSelection().end, optimizeParameters, evaluate);
	
	finishUndo(LastChangeZeroOperandsSelection);
	
	return resCnt;	
}

pp_int32 PatternEditor::fillOperandsTrack(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.fillOperandsTrack(cursor.channel, optimizeParameters, evaluate);
	
	finishUndo(LastChangeFillOperandsTrack);
	
	return resCnt;	
}

pp_int32 PatternEditor::fillOperandsPattern(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.fillOperands(optimizeParameters, evaluate);
	
	finishUndo(LastChangeFillOperandsPattern);
	
	return resCnt;	
}

pp_int32 PatternEditor::fillOperandsSelection(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.fillOperandsSelection(getSelection().start, getSelection().end, optimizeParameters, evaluate);
	
	finishUndo(LastChangeFillOperandsSelection);
	
	return resCnt;	
}

pp_int32 PatternEditor::relocateCommandsTrack(const PatternEditorTools::RelocateParameters& relocateParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.relocateCommandsTrack(cursor.channel, relocateParameters, evaluate);
	
	finishUndo(LastChangeRelocateCommandsTrack);
	
	return resCnt;	
}

pp_int32 PatternEditor::relocateCommandsPattern(const PatternEditorTools::RelocateParameters& relocateParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.relocateCommands(relocateParameters, evaluate);
	
	finishUndo(LastChangeRelocateCommandsPattern);
	
	return resCnt;	
}

pp_int32 PatternEditor::relocateCommandsSelection(const PatternEditorTools::RelocateParameters& relocateParameters, bool evaluate)
{
	if (pattern == NULL)
		return 0;

	if (pattern->patternData == NULL)
		return 0;

	prepareUndo();

	pp_int32 resCnt = 0;

	PatternEditorTools patternEditorTools(pattern);

	resCnt = patternEditorTools.relocateCommandsSelection(getSelection().start, getSelection().end, relocateParameters, evaluate);
	
	finishUndo(LastChangeRelocateCommandsSelection);
	
	return resCnt;	
}

bool PatternEditor::writeNote(pp_int32 note, 
							  bool withUndo/* = false*/, 
							  PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	if (withUndo)
		prepareUndo();

	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);

	// means to delete note
	if (note == 0xFF)
	{
		patternTools.setNote(0);
		
		if (advanceImpl)
			advanceImpl->advance();
		
		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;
	}

	if (note >= 1 && note <= PatternTools::getNoteOffNote())
	{
		pp_int32 currentInstrument = getCurrentActiveInstrument();
		
		patternTools.setNote(note);
		if (currentInstrument && note != PatternTools::getNoteOffNote())
			patternTools.setInstrument(currentInstrument);

		if (advanceImpl)
			advanceImpl->advance();

		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;
	}

	return false;
}

void PatternEditor::writeDirectNote(pp_int32 note,
									pp_int32 track/* = -1*/,
									pp_int32 row/* = -1*/,
									pp_int32 order/* = -1*/)
{
	TXMPattern* pattern = this->pattern;
	if (order != -1)
		pattern = &module->phead[module->header.ord[order]];

	if (track == -1)
		track = cursor.channel;
		
	if (row == -1)
		row = cursor.row;

	PatternTools patternTools;
	patternTools.setPosition(pattern, track, row);

	// means to delete note
	if (note == 0xFF)
	{
		patternTools.setNote(0);
	}

	if (note >= 1 && note <= PatternTools::getNoteOffNote())
	{
		pp_int32 currentInstrument = getCurrentActiveInstrument();
		
		patternTools.setNote(note);
		if (currentInstrument && note != PatternTools::getNoteOffNote())
			patternTools.setInstrument(currentInstrument);
	}
}
									
bool PatternEditor::writeEffect(pp_int32 effNum, pp_uint8 eff, pp_uint8 op, 
								bool withUndo/* = false*/, 
								PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	if (withUndo)
		prepareUndo();

	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	
	// only write effect, when valid effect 
	// (0 is not a valid effect in my internal format, arpeggio is mapped to 0x20)
	if (eff)
		patternTools.setEffect(effNum, eff, op);
	else
		return false;

	if (advanceImpl)
		advanceImpl->advance();

	if (withUndo)
		finishUndo(LastChangeSlotChange);
	return true;
}

void PatternEditor::writeDirectEffect(pp_int32 effNum, pp_uint8 eff, pp_uint8 op, 
									  pp_int32 track/* = -1*/,
									  pp_int32 row/* = -1*/,
									  pp_int32 order/* = -1*/)
{
	TXMPattern* pattern = this->pattern;
	if (order != -1)
		pattern = &module->phead[module->header.ord[order]];

	if (track == -1)
		track = cursor.channel;
		
	if (row == -1)
		row = cursor.row;

	PatternTools patternTools;
	patternTools.setPosition(pattern, track, row);
	
	// only write effect, when valid effect 
	// (0 is not a valid effect in my internal format, arpeggio is mapped to 0x20)
	if (eff)
		patternTools.setEffect(effNum, eff, op);
}


bool PatternEditor::writeInstrument(NibbleTypes nibleType, pp_uint8 value, bool withUndo/* = false*/, PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	if (withUndo)
		prepareUndo();

	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);

	if (nibleType == NibbleTypeBoth)
	{
		patternTools.setInstrument(value);
		
		if (advanceImpl)
			advanceImpl->advance();
		
		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;
	}

	pp_uint32 i = patternTools.getInstrument();
	
	if (nibleType == NibbleTypeHigh)
	{
		i &= 0x0F;
		i |= (pp_uint32)value << 4;
		patternTools.setInstrument(i);

		if (advanceImpl)
			advanceImpl->advance();

		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;
	}
	else if (nibleType == NibbleTypeLow)
	{
		i &= 0xF0;
		i |= (pp_uint32)value;
		patternTools.setInstrument(i);

		if (advanceImpl)
			advanceImpl->advance();

		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;
	}
	
	return false;
}

bool PatternEditor::writeFT2Volume(NibbleTypes nibleType, pp_uint8 value, bool withUndo/* = false*/, PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	if (withUndo)
		prepareUndo();

	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);

	if (value == 0xFF)
	{
		patternTools.setFirstEffect(0, 0);

		if (advanceImpl)
			advanceImpl->advance();

		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;
	}

	pp_int32 eff,op;

	patternTools.getFirstEffect(eff, op);		
	patternTools.convertEffectsToFT2(eff, op);
		
	pp_int32 volume = patternTools.getVolumeFromEffect(eff, op) - 0x10;
	
	if (volume < 0)
		volume = 0;

	if (volume >= 0xF0)
		return false;

	if (volume >= 0)
	{
		if (nibleType == NibbleTypeHigh)
		{
			volume &= 0x0F;
			volume |= (pp_int32)value << 4;

			if (volume>=0xF0)
				return false;
			else if ((volume > 0x40 && volume < 0x50))
				volume = 0x40;

			if (volume >= 0x50 && ((volume & 0xF) == 0))
			{
				switch ((volume+0x10) >> 4)
				{
					// For the following effects we don't allow zero operand
					case 0x6: // Volslide down
					case 0x7: // Volslide up
					case 0x8: // Fine volslide down
					case 0x9: // Fine volslide up
					case 0xa: // Set vibrato speed
					case 0xd: // Panslide left
					case 0xe: // Panslide right
						volume |= 1;
						break;
				}					
			}
		}
		else if (nibleType == NibbleTypeLow)
		{
			volume &= 0xF0;
			volume |= (pp_int32)value;
			
			if (volume>=0xF0)
				return false;
			else if ((volume > 0x40 && volume < 0x50))
				volume = 0x40;
			
			if (volume >= 0x50 && ((volume & 0xF) == 0))
			{
				switch ((volume+0x10) >> 4)
				{
					// For the following effects we don't allow zero operand
					case 0x6: // Volslide down
					case 0x7: // Volslide up
					case 0x8: // Fine volslide down
					case 0x9: // Fine volslide up
					case 0xa: // Set vibrato speed
					case 0xd: // Panslide left
					case 0xe: // Panslide right
						volume |= 1;
						break;
				}					
			}
		}
		
		patternTools.convertVolumeToEffect(volume + 0x10, eff, op);		
		patternTools.setFirstEffect(eff, op);							

		if (advanceImpl)
			advanceImpl->advance();

		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;		
	}

	return false;
}

bool PatternEditor::writeEffectNumber(pp_uint8 value, bool withUndo/* = false*/, PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	if (withUndo)
		prepareUndo();

	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	
	// clear out entire effect + operand
	if (value == 0xFF)
	{
		patternTools.setEffect(1, 0, 0);

		if (advanceImpl)
			advanceImpl->advance();

		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;
	}
	
	pp_int32 eff,op;
	
	// skip first effect (= volume command)
	patternTools.getFirstEffect(eff, op);
	patternTools.getNextEffect(eff, op);
	
	patternTools.convertEffectsToFT2(eff, op);
	
	pp_int32 newEff = value;
	patternTools.convertEffectsFromFT2(newEff, op);
	
	if (newEff == 0x40)
	{
		newEff = 0x41;
		op &= 0x0F;
	}
	else if (newEff >= 0x43)
	{
		newEff = 0x42;
		op &= 0x0F;
	}
	
	patternTools.setEffect(1, newEff, op);

	if (advanceImpl)
		advanceImpl->advance();

	if (withUndo)
		finishUndo(LastChangeSlotChange);
	return true;
}

bool PatternEditor::writeEffectOperand(NibbleTypes nibleType, pp_uint8 value, bool withUndo/* = false*/, PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	if (withUndo)
		prepareUndo();

	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	
	// clear out entire effect + operand
	if (value == 0xFF)
	{
		patternTools.setEffect(1, 0, 0);

		if (advanceImpl)
			advanceImpl->advance();

		if (withUndo)
			finishUndo(LastChangeSlotChange);
		return true;
	}

	pp_int32 eff,op;
	
	patternTools.getFirstEffect(eff, op);
	patternTools.getNextEffect(eff, op);
	
	patternTools.convertEffectsToFT2(eff, op);
	
	if (nibleType == NibbleTypeHigh)
	{
		op &= 0x0F;
		op |= (pp_int32)value << 4;
	}
	else if (nibleType == NibbleTypeLow)
	{
		op &= 0xF0;
		op |= (pp_int32)value;
	}
	
	patternTools.convertEffectsFromFT2(eff, op);
	
	if (eff == 0x40)
		eff = 0x41;
	if (eff >= 0x43)
		eff = 0x42;
	
	patternTools.setEffect(1, eff, op);

	if (advanceImpl)
		advanceImpl->advance();

	if (withUndo)
		finishUndo(LastChangeSlotChange);
	return true;
}

void PatternEditor::storeMacroFromCursor(pp_int32 slot)
{
	if (pattern == NULL)
		return;

	if (slot > 10 || slot < 0)
		return;

	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	
	pp_int32 eff, op;
	
	if (cursor.inner >= 3 && cursor.inner <= 4)
	{
		patternTools.getEffect(0, eff, op);
		// feature shall also work with empty effects
		//if (!eff && !op)
		//	return;
		effectMacros[slot].effect = (pp_uint8)eff;
		effectMacros[slot].operand = (pp_uint8)op;
	}
	else if (cursor.inner > 4)
	{
		patternTools.getEffect(1, eff, op);
		// feature shall also work with empty effects
		//if (!eff && !op)
		//	return;
		effectMacros[slot+10].effect = (pp_uint8)eff;
		effectMacros[slot+10].operand = (pp_uint8)op;
	}
}

void PatternEditor::writeMacroToCursor(pp_int32 slot, PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	if (pattern == NULL)
		return;

	if (slot > 10 || slot < 0)
		return;

	prepareUndo();	

	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	
	if (cursor.inner >= 3 && cursor.inner <= 4)
	{
		// feature shall also work with empty effects
		//if (!effectMacros[slot].effect && !effectMacros[slot].operand)
		//	goto writeNothing;			
		patternTools.setEffect(0, effectMacros[slot].effect, effectMacros[slot].operand);
	}
	else if (cursor.inner > 4)
	{
		// feature shall also work with empty effects
		//if (!effectMacros[slot+10].effect && !effectMacros[slot+10].operand)
		//	goto writeNothing;
		patternTools.setEffect(1, effectMacros[slot+10].effect, effectMacros[slot+10].operand);
	}

	if (advanceImpl)
		advanceImpl->advance();

//writeNothing:

	finishUndo(LastChangeWriteMacro);
}

void PatternEditor::getMacroOperands(pp_int32 slot, pp_uint8& eff, pp_uint8& op)
{
	if (slot < 0 || slot >= 20)
		return;
		
	eff = effectMacros[slot].effect;
	op = effectMacros[slot].operand;
}

void PatternEditor::setMacroOperands(pp_int32 slot, pp_uint8 eff, pp_uint8 op)
{
	if (slot < 0 || slot >= 20)
		return;
		
	effectMacros[slot].effect = eff;
	effectMacros[slot].operand = op;
}

void PatternEditor::deleteCursorSlotData(PatternAdvanceInterface* advanceImpl/* = NULL*/)
{	
	prepareUndo();
	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	if (cursor.inner == 3 || cursor.inner == 4)
	{
		patternTools.setFirstEffect(0,0);
	}
	else if (cursor.inner == 5 || cursor.inner == 6 || cursor.inner == 7)
	{
		// What have I been thinking?
		//pp_int32 eff, op;
		//patternTools.getEffect(1, eff, op);
		// actually I think delete should really delete
		patternTools.setEffect(1, 0, 0);
	}
	else
	{
		patternTools.setNote(0);
		patternTools.setInstrument(0);
	}
	
	if (advanceImpl)
		advanceImpl->advance();
	
	finishUndo(LastChangeDeleteNote);
}

void PatternEditor::deleteCursorSlotDataEntire(PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	prepareUndo();
	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	patternTools.setNote(0);
	patternTools.setInstrument(0);
	patternTools.setFirstEffect(0,0);
	patternTools.setNextEffect(0,0);

	if (advanceImpl)
		advanceImpl->advance();

	finishUndo(LastChangeDeleteNoteVolumeAndEffect);
}

void PatternEditor::deleteCursorSlotDataVolumeAndEffect(PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	prepareUndo();
	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	patternTools.setFirstEffect(0,0);
	patternTools.setNextEffect(0,0);

	if (advanceImpl)
		advanceImpl->advance();

	finishUndo(LastChangeDeleteVolumeAndEffect);
}

void PatternEditor::deleteCursorSlotDataEffect(PatternAdvanceInterface* advanceImpl/* = NULL*/)
{
	prepareUndo();
	PatternTools patternTools;
	patternTools.setPosition(pattern, cursor.channel, cursor.row);
	pp_int32 eff, op;
	patternTools.getFirstEffect(eff, op);
	patternTools.setFirstEffect(eff,op);
	patternTools.setNextEffect(0,0);

	if (advanceImpl)
		advanceImpl->advance();

	finishUndo(LastChangeDeleteEffect);
}

void PatternEditor::insertNote(pp_int32 channel, pp_int32 row)
{
	if (pattern == NULL || pattern->patternData == NULL)
		return;

	if (channel < 0 || channel >= pattern->channum)
		return;
	
	prepareUndo();
	
	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSize = slotSize*pattern->channum;

	for (mp_sint32 i = pattern->rows - 1; i > row; i--)
	{
		mp_ubyte* src = (i-1)*rowSize + channel*slotSize + pattern->patternData;
		mp_ubyte* dst = i*rowSize + channel*slotSize + pattern->patternData;
	
		memcpy(dst, src, slotSize);
	}
	
	memset(row*rowSize + channel*slotSize + pattern->patternData, 0, slotSize);
	
	finishUndo(LastChangeInsertNote);
}

void PatternEditor::insertLine(pp_int32 row)
{
	if (pattern == NULL || pattern->patternData == NULL)
		return;

	if (row < 0 || row >= pattern->rows)
		return;

	prepareUndo();
	
	mp_sint32 slotSize = pattern->effnum * 2 + 2;

	mp_sint32 rowSize = slotSize*pattern->channum;

	for (mp_sint32 i = pattern->rows - 1; i > row; i--)
	{
		mp_ubyte* src = (i-1)*rowSize + pattern->patternData;
		mp_ubyte* dst = i*rowSize + pattern->patternData;
	
		memcpy(dst, src, rowSize);
	}
	
	memset(row*rowSize + pattern->patternData, 0, rowSize);
	
	finishUndo(LastChangeInsertLine);
}

void PatternEditor::deleteNote(pp_int32 channel, pp_int32 row)
{
	if (pattern == NULL || pattern->patternData == NULL)
		return;

	if (row < 0 || row >= pattern->rows)
		return;

	if (channel < 0 || channel >= pattern->channum)
		return;
	
	prepareUndo();
	
	mp_sint32 slotSize = pattern->effnum * 2 + 2;
	mp_sint32 rowSize = slotSize*pattern->channum;

	for (mp_sint32 i = row; i < pattern->rows-1; i++)
	{
		mp_ubyte* src = (i+1)*rowSize + channel*slotSize + pattern->patternData;
		mp_ubyte* dst = i*rowSize + channel*slotSize + pattern->patternData;
	
		memcpy(dst, src, slotSize);
	}
	
	memset((pattern->rows-1)*rowSize + channel*slotSize + pattern->patternData, 0, slotSize);
	
	finishUndo(LastChangeDeleteNote);
}

void PatternEditor::deleteLine(pp_int32 row)
{
	if (pattern == NULL || pattern->patternData == NULL)
		return;

	if (row < 0 || row >= pattern->rows)
		return;

	prepareUndo();
	
	mp_sint32 slotSize = pattern->effnum * 2 + 2;

	mp_sint32 rowSize = slotSize*pattern->channum;

	for (mp_sint32 i = row; i < pattern->rows-1; i++)
	{
		mp_ubyte* src = (i+1)*rowSize + pattern->patternData;
		mp_ubyte* dst = i*rowSize + pattern->patternData;
	
		memcpy(dst, src, rowSize);
	}
	
	memset((pattern->rows-1)*rowSize + pattern->patternData, 0, rowSize);
	
	finishUndo(LastChangeDeleteLine);
}


void PatternEditor::moveSelection(pp_int32 channels, pp_int32 rows)
{
	PatternEditorTools::Position targetStart = selection.start;
	PatternEditorTools::Position targetEnd = selection.end;
	targetStart.row += rows;
	targetStart.channel += channels;
	targetEnd.row += rows;
	targetEnd.channel += channels;
	
	if (!PatternEditorTools::hasValidSelection(pattern, selection.start, selection.end))
		return;
	if (!PatternEditorTools::hasValidSelection(pattern, targetStart, targetEnd))
		return;
	
	prepareUndo();
	
	PatternEditorTools tools(pattern);
	tools.moveSelection(selection.start, selection.end, channels, rows, true);
	
	selection.start = targetStart;
	selection.end = targetEnd;
	
	finishUndo(LastChangeMoveSelection);
}


void PatternEditor::cloneSelection(pp_int32 channels, pp_int32 rows)
{
	PatternEditorTools::Position targetStart = selection.start;
	PatternEditorTools::Position targetEnd = selection.end;
	targetStart.row += rows;
	targetStart.channel += channels;
	targetEnd.row += rows;
	targetEnd.channel += channels;
	
	if (!PatternEditorTools::hasValidSelection(pattern, selection.start, selection.end))
		return;
	if (!PatternEditorTools::hasValidSelection(pattern, targetStart, targetEnd))
		return;
	
	prepareUndo();
	
	PatternEditorTools tools(pattern);
	tools.moveSelection(selection.start, selection.end, channels, rows, false);  // don't erase source notes
	
	selection.start = targetStart;
	selection.end = targetEnd;
	
	finishUndo(LastChangeCloneSelection);
}

void PatternEditor::triggerButton( pp_uint32 id, PPScreen *s, EventListenerInterface *e ){
  PPEvent event(eCommand);
  PPButton *fakebutton = new PPButton(id, s, e, PPPoint(0,0), PPSize(10, 10));
  e->handleEvent(reinterpret_cast<PPObject*>(fakebutton), &event);
  delete fakebutton;
}
