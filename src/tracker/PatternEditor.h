/*
 *  tracker/PatternEditor.h
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
 *  PatternEditor.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 16.11.07.
 *
 */

#ifndef __PATTERNEDITOR_H__
#define __PATTERNEDITOR_H__

#include "EditorBase.h"
#include "PatternEditorTools.h"
#include "Undo.h"
#include "Button.h"

struct TXMPattern;
class XModule;

class PatternEditor : public EditorBase
{
public:
	// These are the clipboards, FT2 uses different clipboard buffers for
	// copy/cut/paste block/track/pattern operations, so we imitate these too
	enum ClipBoardTypes
	{
		ClipBoardTypeSelection,
		ClipBoardTypeTrack,
		ClipBoardTypePattern,
		// do not use
		ClipBoardTypeLAST			
	};

	class PatternAdvanceInterface
	{
	public:
		virtual void advance() = 0;
	};

	class Selection
	{
	private:
		bool copyValid;
	
	public:
		PatternEditorTools::Position start, end, startCopy, endCopy;
		
		Selection()
		{
			reset();
			startCopy = start;
			endCopy = end;
			copyValid = false;
		}
		
		void reset()
		{
			start.channel = -1;
			start.row = -1;
			start.inner = 0;
			end.channel = -1;
			end.row = -1;	
			end.inner = 7;
		}
		
		bool isValid()
		{
			return (start.channel >= 0 && start.row >= 0 &&
					end.channel >= 0 && end.row >= 0);
		}

		bool isCopyValid()
		{
			return (startCopy.channel >= 0 && startCopy.row >= 0 &&
					endCopy.channel >= 0 && endCopy.row >= 0) && copyValid;
		}
		
		void backup();

		void restore();
	};

	struct TCommand
	{
		pp_uint8 effect;
		pp_uint8 operand;
	};
	
private:
	class ClipBoard
	{
	private:
		mp_ubyte* buffer;
		
		PatternEditorTools::Position selectionStart, selectionEnd;

		pp_int32 selectionWidth;
		pp_int32 selectionHeight;
		
		// FT2 uses different clipboards for track/pattern/block operations
		// so a regular singleton design won't cut it
		static ClipBoard* instances[ClipBoardTypeLAST];
		
		ClipBoard();

	public:
		~ClipBoard();
		
		static ClipBoard* getInstance(ClipBoardTypes type);
		
		void makeCopy(TXMPattern& pattern, 
					  const PatternEditorTools::Position& ss, const PatternEditorTools::Position& se, 
					  bool clear = false);
		void paste(TXMPattern& pattern, pp_int32 sc, pp_int32 sr, bool transparent = false);
		bool isEmpty() const { return buffer == NULL; }
		
		pp_int32 getNumRows() { return selectionHeight; }
		pp_int32 getNumChannels() { return selectionWidth; }
	};

	// operations
	enum LastChanges
	{
		LastChangeNone,
		LastChangeSlotChange,
		LastChangeInsertNote,
		LastChangeInsertLine,
		LastChangeDeleteNote,
		LastChangeDeleteLine,
		LastChangeDeleteNoteVolumeAndEffect,
		LastChangeDeleteVolumeAndEffect,
		LastChangeDeleteEffect,
		LastChangeWriteMacro,
		LastChangeCut,
		LastChangePaste,
		LastChangeDeleteSelection,
		LastChangeMoveSelection,
		LastChangeCloneSelection,
		
		LastChangeExpandPattern,
		LastChangeShrinkPattern,
		LastChangeResizePattern,

		LastChangeLoadXPattern,
		LastChangeLoadXTrack,

		LastChangeInsIncSelection,
		LastChangeInsDecSelection,
		LastChangeInsIncTrack,
		LastChangeInsDecTrack,
		LastChangeInsRemapTrack,
		LastChangeInsRemapPattern,
		LastChangeInsRemapSelection,
		
		LastChangeNoteTransposeTrack,
		LastChangeNoteTransposePattern,
		LastChangeNoteTransposeSelection,
		
		LastChangeNoteInterpolate,
		LastChangeSplitTrack,
		LastChangeSwapChannels,
		LastChangeScaleVolume,

		LastChangeZeroOperandsTrack,
		LastChangeZeroOperandsPattern,
		LastChangeZeroOperandsSelection,

		LastChangeFillOperandsTrack,
		LastChangeFillOperandsPattern,
		LastChangeFillOperandsSelection,

		LastChangeRelocateCommandsTrack,
		LastChangeRelocateCommandsPattern,
		LastChangeRelocateCommandsSelection
	};
	
private:
	TXMPattern* pattern;
	
	// Current cursor position
	PatternEditorTools::Position cursor;
	// Current selection
	Selection selection;
	
	pp_int32 numVisibleChannels;
	bool autoResize;
	pp_int32 currentInstrument;
	bool instrumentEnabled;
	bool instrumentBackTrace;
	pp_int32 currentOctave;
		
	// undo/redo information
	UndoStackEntry::UserData undoUserData;
	PatternUndoStackEntry* before;
	PPUndoStack<PatternUndoStackEntry>* undoStack;	
	UndoHistory<TXMPattern, PatternUndoStackEntry>* undoHistory;
	LastChanges lastChange;	
	bool lastOperationDidChangeRows;
	bool lastOperationDidChangeCursor;

	TCommand effectMacros[20];

	void prepareUndo();
	bool finishUndo(LastChanges lastChange, bool nonRepeat = false);
	
	bool revoke(const PatternUndoStackEntry* stackEntry);

	void cut(ClipBoard& clipBoard);
	void copy(ClipBoard& clipBoard);
	void paste(ClipBoard& clipBoard, bool transparent = false, pp_int32 fromChannel = -1);

	void clearRange(const PatternEditorTools::Position& rangeStart, const PatternEditorTools::Position& rangeEnd);
	
public:
	PatternEditor();
	virtual ~PatternEditor();

	// query status
	bool getLastOperationDidChangeRows() const { return lastOperationDidChangeRows; }
	bool getLastOperationDidChangeCursor() const { return lastOperationDidChangeCursor; }

	void attachPattern(TXMPattern* pattern, XModule* module);
	TXMPattern* getPattern() { return pattern; }
	
	void reset();
	
	pp_int32 getNumChannels() const;
	pp_int32 getNumRows() const;

	void setNumVisibleChannels(pp_int32 numVisibleChannels) { this->numVisibleChannels = numVisibleChannels; }
	void setAutoResize(bool autoResize) { this->autoResize = autoResize; }
	bool getAutoResize() const { return autoResize; }

	// dealing with current cursor
	void setCursor(const PatternEditorTools::Position& cursor) { this->cursor = cursor; }
	PatternEditorTools::Position& getCursor() { return cursor; }

	void resetCursor() { cursor.row = cursor.channel = cursor.inner = 0; }

	// dealing with the selection
	void setSelection(const Selection& selection) { this->selection = selection; }
	Selection& getSelection() { return selection; }
	void setSelectionStart(const PatternEditorTools::Position& pos) { selection.start = pos; }
	void setSelectionEnd(const PatternEditorTools::Position& pos) { selection.end = pos; }
	void resetSelection() { selection.reset(); }
	bool hasValidSelection();
	bool canMoveSelection(pp_int32 channels, pp_int32 rows);
	bool selectionContains(const PatternEditorTools::Position& pos);
	void selectChannel(pp_int32 channel);
	void selectAll();

	// dealing with instrument
	void setCurrentInstrument(pp_int32 ins) { currentInstrument = ins; }
	pp_int32 getCurrentActiveInstrument();	
	void enableInstrument(bool instrumentEnabled) { this->instrumentEnabled = instrumentEnabled; }
	bool isInstrumentEnabled() { return instrumentEnabled; }	
	// Intelligent instrument backtrace?
	void setInstrumentBackTrace(bool instrumentBackTrace) { this->instrumentBackTrace = instrumentBackTrace; }

	void setCurrentOctave(pp_int32 octave) { currentOctave = octave; }
	pp_int32 getCurrentOctave() const { return currentOctave; }
	void increaseCurrentOctave() { if (currentOctave < 8) currentOctave++; }
	void decreaseCurrentOctave() { if (currentOctave > 1) currentOctave--; }	

	// --- Multilevel UNDO / REDO --------------------------------------------
	bool canUndo() const { if (undoStack) return !undoStack->IsEmpty(); else return false; }
	bool canRedo() const { if (undoStack) return !undoStack->IsTop(); else return false; }
	// undo last changes
	bool undo();
	// redo last changes
	bool redo();
	void setUndoUserData(const void* data, pp_uint32 dataLen) { this->undoUserData = UndoStackEntry::UserData((pp_uint8*)data, dataLen); }
	pp_uint32 getUndoUserDataLen() const { return undoUserData.getDataLen(); }
	const void* getUndoUserData() const { return (void*)undoUserData.getData(); }
	
	// --- dealing with the pattern data -------------------------------------
	void clearSelection();
	
	void clearPattern();

	bool clipBoardSelectionIsEmpty() const { return ClipBoard::getInstance(ClipBoardTypeSelection)->isEmpty(); }

	void cut(ClipBoardTypes clipBoardType);
	void copy(ClipBoardTypes clipBoardType);
	void paste(ClipBoardTypes clipBoardType, bool transparent = false, pp_int32 fromChannel = -1);
	
	// resize pattern to a new number of rows
	bool resizePattern(pp_int32 newRowNum, bool withUndo = true);

	// insert a blank line after each pattern line
	bool expandPattern();

	// delete every odd pattern line
	bool shrinkPattern();

	// Load extended pattern from file (.XP)
	bool loadExtendedPattern(const PPSystemString& fileName);
	bool saveExtendedPattern(const PPSystemString& fileName);

	// Load extended track from file (.XT)
	bool loadExtendedTrack(const PPSystemString& fileName);
	bool saveExtendedTrack(const PPSystemString& fileName);
	
	// --- increase/decrease/remap instruments -------------------------------------------------
	pp_int32 insIncSelection();
	pp_int32 insDecSelection();
	pp_int32 insIncTrack();
	pp_int32 insDecTrack();
	pp_int32 insRemapTrack(pp_int32 oldIns, pp_int32 newIns);
	pp_int32 insRemapPattern(pp_int32 oldIns, pp_int32 newIns);
	pp_int32 insRemapSelection(pp_int32 oldIns, pp_int32 newIns);

	// --- transpose notes ---------------------------------------------------
	pp_int32 noteTransposeTrackCore(const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate);
	pp_int32 noteTransposePatternCore(const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate);
	pp_int32 noteTransposeSelectionCore(const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate);

	// --- interpolate values in current selection ---------------------------
	pp_int32 interpolateValuesInSelection();

	// --- split track -------------------------------------------------------
	pp_int32 splitTrack(pp_int32 useChannels, bool selectionOnly, bool insertNoteOff);

	// --- swap channels -----------------------------------------------------
	pp_int32 swapChannels(pp_int32 dstChannel, pp_int32 srcChannel);

	// --- FT2 compatible scale volume function ------------------------------
	pp_int32 scaleVolume(const PatternEditorTools::Position& startSelection, const PatternEditorTools::Position& endSelection, float startScale, float endScale);
	pp_int32 scaleVolumeTrack(float startScale, float endScale);
	pp_int32 scaleVolumePattern(float startScale, float endScale);
	pp_int32 scaleVolumeSelection(float startScale, float endScale);

	// --- Zero out unecessary operands --------------------------------------
	pp_int32 zeroOperandsTrack(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate);
	pp_int32 zeroOperandsPattern(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate);
	pp_int32 zeroOperandsSelection(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate);

	// --- Fill in zero operands ---------------------------------------------
	pp_int32 fillOperandsTrack(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate);
	pp_int32 fillOperandsPattern(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate);
	pp_int32 fillOperandsSelection(const PatternEditorTools::OperandOptimizeParameters& optimizeParameters, bool evaluate);

	// --- Relocate FX into volume column if possible ------------------------
	pp_int32 relocateCommandsTrack(const PatternEditorTools::RelocateParameters& relocateParameters, bool evaluate);
	pp_int32 relocateCommandsPattern(const PatternEditorTools::RelocateParameters& relocateParameters, bool evaluate);
	pp_int32 relocateCommandsSelection(const PatternEditorTools::RelocateParameters& relocateParameters, bool evaluate);

	// --- write slot data ---------------------------------------------------
	bool writeNote(pp_int32 note, 
				   bool withUndo = false,
				   PatternAdvanceInterface* advanceImpl = NULL);
				   
	// --- write through, without undo etc. ----------------------------------
	void writeDirectNote(pp_int32 note,
						 pp_int32 track = -1,
						 pp_int32 row = -1,
						 pp_int32 order = -1);
	
	enum NibbleTypes
	{
		NibbleTypeLow,
		NibbleTypeHigh,
		NibbleTypeBoth
	};
	
	bool writeInstrument(NibbleTypes nibleType, pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);
	bool writeFT2Volume(NibbleTypes nibleType, pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);
	bool writeEffectNumber(pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);
	bool writeEffectOperand(NibbleTypes nibleType, pp_uint8 value, bool withUndo = false, PatternAdvanceInterface* advanceImpl = NULL);

	bool writeEffect(pp_int32 effNum, pp_uint8 eff, pp_uint8 op, 
					 bool withUndo = false, 
					 PatternAdvanceInterface* advanceImpl = NULL);
					 	
	// --- write through, without undo etc. ----------------------------------
	void writeDirectEffect(pp_int32 effNum, pp_uint8 eff, pp_uint8 op, 
						   pp_int32 track = -1,
						   pp_int32 row = -1,
						   pp_int32 order = -1);
	
	// --- dealing with FT2 style effect macros ------------------------------
	void storeMacroFromCursor(pp_int32 slot);
	void writeMacroToCursor(pp_int32 slot, PatternAdvanceInterface* advanceImpl = NULL);
	
	void getMacroOperands(pp_int32 slot, pp_uint8& eff, pp_uint8& op);
	void setMacroOperands(pp_int32 slot, pp_uint8 eff, pp_uint8 op);	
	
	// --- deleting slot data ------------------------------------------------
	void deleteCursorSlotData(PatternAdvanceInterface* advanceImpl = NULL);
	void deleteCursorSlotDataEntire(PatternAdvanceInterface* advanceImpl = NULL);
	void deleteCursorSlotDataVolumeAndEffect(PatternAdvanceInterface* advanceImpl = NULL);
	void deleteCursorSlotDataEffect(PatternAdvanceInterface* advanceImpl = NULL);

	// --- inserting/deleting entire rows ------------------------------------
	void insertNote(pp_int32 channel, pp_int32 row);
	void insertLine(pp_int32 row);
	void deleteNote(pp_int32 channel, pp_int32 row);
	void deleteLine(pp_int32 row);

	// --- moving entire selection -------------------------------------------
	void moveSelection(pp_int32 channels, pp_int32 rows);
	void cloneSelection(pp_int32 channels, pp_int32 rows);

  //  -- improved UX by linking context-menu items to button-clicks
  void triggerButton( pp_uint32 id, PPScreen *s, EventListenerInterface *e );

};

#endif
