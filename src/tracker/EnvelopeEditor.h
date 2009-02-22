/*
 *  tracker/EnvelopeEditor.h
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
 *  EnvelopeEditor.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.11.07.
 *
 */

#ifndef __ENVELOPEEDITOR_H__
#define __ENVELOPEEDITOR_H__

#include "EditorBase.h"
#include "Undo.h"
#include "Singleton.h"

struct TEnvelope;

class FilterParameters;

class EnvelopeEditor : public EditorBase
{
public:
	enum EnvelopeTypes
	{
		EnvelopeTypeVolume = 0,
		EnvelopeTypePanning = 1
	};

private:
	class ClipBoard : public PPSingleton<ClipBoard>
	{
	private:
		TEnvelope* envelopeCopy;
		
		ClipBoard();

	public:
		~ClipBoard();

		void makeCopy(TEnvelope& envelope);
		void paste(TEnvelope& envelope);
		bool isEmpty() const { return envelopeCopy == NULL; }
		
		friend class PPSingleton<ClipBoard>;
	};

	TEnvelope* envelope;
	
	EnvelopeTypes envelopeType;

	// selection
	bool startSelection;
	pp_int32 selectionIndex;

	// undo/redo information
	EnvelopeUndoStackEntry* before;
	PPUndoStack<EnvelopeUndoStackEntry>* undoStack;	
	UndoHistory<TEnvelope, EnvelopeUndoStackEntry>* undoHistory;
	
	void prepareUndo();
	bool finishUndo();
	
	// revoke changes
	void revoke(const EnvelopeUndoStackEntry* stackEntry);
	
public:
	EnvelopeEditor();
	virtual ~EnvelopeEditor();

	void attachEnvelope(TEnvelope* envelope, XModule* module);
	const TEnvelope* getEnvelope() const { return envelope; }

	bool isValidEnvelope() const { return envelope != NULL; }
	bool isEmptyEnvelope() const;
	
	void setEnvelopeType(EnvelopeTypes envelopeType) { this->envelopeType = envelopeType; }
	EnvelopeTypes getEnvelopeType() const { return envelopeType; }
	
	void reset();
	
	void setSelectionIndex(pp_int32 selectionIndex) { this->selectionIndex = selectionIndex; }
	pp_int32& getSelectionIndex() { return selectionIndex; }
	void resetSelection() { selectionIndex = -1; }

	// --- kinda clip board --------------------------------------------------
	void makeCopy();
	void pasteCopy();
	void pasteOther(const TEnvelope& env);

	bool canCopy() const;
	bool canPaste() const { return !ClipBoard::getInstance()->isEmpty(); }

	// --- Multilevel UNDO / REDO --------------------------------------------
	// undo last changes
	void undo();
	// redo last changes
	void redo();

	bool canUndo() const { if (envelope && undoStack) return !undoStack->IsEmpty(); else return false; }
	bool canRedo() const { if (envelope && undoStack) return !undoStack->IsTop(); else return false; }

	void startSelectionDragging(pp_int32 index);
	bool isSelectionDragging() const { return startSelection; }
	void endSelectionDragging();
	
	pp_int32 getHorizontalExtent() const;

	// --- manipulate envelope -----------------------------------------------
	void setEnvelopePoint(pp_int32 index, pp_int32 x, pp_int32 y);

	void enableEnvelope(bool b);
	bool isEnvelopeEnabled() const;

	void enableSustain(bool b);
	bool isSustainEnabled() const;
	pp_int32 getSustainPtIndex() const;

	void enableLoop(bool b);
	bool isLoopEnabled() const;
	pp_int32 getLoopStartPtIndex() const;
	pp_int32 getLoopEndPtIndex() const;

	void selectNextSustainPoint();
	void selectPreviousSustainPoint();
	void selectNextLoopStartPoint();
	void selectPreviousLoopStartPoint();
	void selectNextLoopEndPoint();
	void selectPreviousLoopEndPoint();

	void deletePoint();
	void addPoint();	

	// filters (need the same signature)
	void tool_xScaleEnvelope(const FilterParameters* par);
	void tool_yScaleEnvelope(const FilterParameters* par);
};

#endif
