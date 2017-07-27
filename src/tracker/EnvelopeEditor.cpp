/*
 *  tracker/EnvelopeEditor.cpp
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
 *  EnvelopeEditor.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 25.11.07.
 *
 */

#include "EnvelopeEditor.h"
#include "SimpleVector.h"
#include "FilterParameters.h"

EnvelopeEditor::ClipBoard::ClipBoard() :
	envelopeCopy(NULL)
{
}

EnvelopeEditor::ClipBoard::~ClipBoard()
{
	delete envelopeCopy;
}

void EnvelopeEditor::ClipBoard::makeCopy(TEnvelope& envelope)
{
	if (envelopeCopy == NULL)
		envelopeCopy = new TEnvelope;
	
	*envelopeCopy = envelope;
}

void EnvelopeEditor::ClipBoard::paste(TEnvelope& envelope)
{
	if (envelopeCopy)
		envelope = *envelopeCopy;
}

void EnvelopeEditor::prepareUndo()
{
	if (envelope == NULL) return;
	delete before;
	before = new EnvelopeUndoStackEntry(*envelope);
}

bool EnvelopeEditor::finishUndo()
{
	EnvelopeUndoStackEntry after(*envelope); 
	if (*before != after) 
	{ 		
		if (undoStack) 
		{ 
			undoStack->Push(*before); 
			undoStack->Push(after); 
			undoStack->Pop(); 
		} 
		notifyListener(NotificationChanges);
	} 
	return true;
}

void EnvelopeEditor::revoke(const EnvelopeUndoStackEntry* stackEntry)
{
	stackEntry->GetEnvelope(*envelope);
	
	notifyListener(NotificationUndoRedo);
	notifyListener(NotificationChanges);
}

EnvelopeEditor::EnvelopeEditor() :
	EditorBase(),
	envelope(NULL),
	envelopeType(EnvelopeTypeVolume),
	startSelection(false),
	selectionIndex(-1),
	before(NULL),
	undoStack(NULL)
{
	// Undo history
	undoHistory = new UndoHistory<TEnvelope, EnvelopeUndoStackEntry>(UNDOHISTORYSIZE_ENVELOPEEDITOR);	
}

EnvelopeEditor::~EnvelopeEditor()
{
	delete undoHistory;
	delete undoStack;	
	delete before;
}

void EnvelopeEditor::attachEnvelope(TEnvelope* envelope, XModule* module)
{
	if (envelope != this->envelope)
		resetSelection();

	if (envelope)
	{
		// --------- update undo history information --------------------	
		if (undoStack)
		{	
			// if the undo stack is empty, we don't need to save current undo stack
			if (!undoStack->IsEmpty() || !undoStack->IsTop())
			{	
				undoStack = undoHistory->getUndoStack(envelope, this->envelope, undoStack);
			}
			// delete it if it's empty
			else
			{
				delete undoStack;
				undoStack = NULL;
				
				undoStack = undoHistory->getUndoStack(envelope, NULL, NULL);
			}
		}
		
		// couldn't get any from history, create new one
		if (!undoStack)
		{
			undoStack = new PPUndoStack<EnvelopeUndoStackEntry>(UNDODEPTH_ENVELOPEEDITOR);
		}
	}
	
	attachModule(module);
	this->envelope = envelope;
	
	notifyListener(NotificationReload);
}

bool EnvelopeEditor::isEmptyEnvelope() const
{
	return (envelope == NULL || envelope->num == 0);
}

void EnvelopeEditor::reset()
{
	if (undoHistory)
		delete undoHistory;
			
	if (undoStack)
	{
		delete undoStack;
		undoStack = NULL;
		undoStack = new PPUndoStack<EnvelopeUndoStackEntry>(UNDODEPTH_ENVELOPEEDITOR);
	}
	
	undoHistory = new UndoHistory<TEnvelope, EnvelopeUndoStackEntry>(UNDOHISTORYSIZE_ENVELOPEEDITOR);
}

void EnvelopeEditor::makeCopy()
{
	if (!canCopy())
		return;

	ClipBoard::getInstance()->makeCopy(*envelope);
}

void EnvelopeEditor::pasteCopy()
{
	prepareUndo();
	
	ClipBoard::getInstance()->paste(*envelope);
		
	finishUndo();
}

void EnvelopeEditor::pasteOther(const TEnvelope& env)
{
	prepareUndo();
	
	*envelope = env;
	resetSelection();
	
	finishUndo();
}

bool EnvelopeEditor::canCopy() const
{
	return (envelope && envelope->num >= 2);
}

void EnvelopeEditor::undo()
{
	if (undoStack->IsEmpty()) return;

	if (undoStack)
	{
		revoke(undoStack->Pop());
	}	
}

void EnvelopeEditor::redo()
{
	if (undoStack->IsTop()) return;
	
	if (undoStack)
	{
		revoke(undoStack->Advance());
	}
}

void EnvelopeEditor::startSelectionDragging(pp_int32 index)
{
	if (index != -1)
	{
		prepareUndo();
		selectionIndex = index;
		startSelection = true;
	}
}

void EnvelopeEditor::endSelectionDragging()
{
	if (startSelection)
	{
		startSelection = false;
		finishUndo();
	}
}

pp_int32 EnvelopeEditor::getHorizontalExtent() const
{
	if (envelope == NULL)
		return -1;

	if (envelope->num == 0)
		return -1;

	pp_int32 max = envelope->env[0][0];

	for (pp_int32 i = 1; i < envelope->num; i++)
		if (envelope->env[i][0] > max)
			max = envelope->env[i][0];
			
	return max;
}

void EnvelopeEditor::setEnvelopePoint(pp_int32 index, pp_int32 x, pp_int32 y)
{	
	if (envelope == NULL)
		return;

	if (envelope->num == 0)
		return;

	if (index == -1)
		return;

	if (index == 0)
	{
		if (x > 0)
			x = 0;
	}

	if (index > 0)
	{
		if (x < envelope->env[index - 1][0] + 1)
			x = envelope->env[index - 1][0] + 1;
		
		if (index < envelope->num - 1)
		{			
			if (x > envelope->env[index + 1][0] - 1)
				x = envelope->env[index + 1][0] - 1;	
		}
		
	}

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (y > 256)
		y = 256;

	envelope->env[index][0] = (pp_uint16)x;
	envelope->env[index][1] = (pp_uint16)y;
}

void EnvelopeEditor::enableEnvelope(bool b)
{
	prepareUndo();

	if (envelope)
	{
		// clear old flag
		envelope->type &= ~1;
		// set new flag
		if (b) envelope->type |= 1;
	}
	
	finishUndo();
}

bool EnvelopeEditor::isEnvelopeEnabled() const
{
	if (envelope == NULL)
		return false;
	return (envelope->type & 1) != 0;
}

void EnvelopeEditor::enableSustain(bool b)
{
	prepareUndo();

	if (envelope)
	{
		// clear old flag
		envelope->type &= ~2;
		// set new flag
		if (b) envelope->type |= 2;
	}

	finishUndo();
}

bool EnvelopeEditor::isSustainEnabled() const
{
	if (envelope == NULL)
		return false;
	return (envelope->type & 2) != 0;
}

pp_int32 EnvelopeEditor::getSustainPtIndex() const
{
	if (envelope == NULL)
		return 0;
	return envelope->sustain;
}

void EnvelopeEditor::enableLoop(bool b)
{
	prepareUndo();

	if (envelope)
	{
		// clear old flag
		envelope->type &= ~4;
		// set new flag
		if (b) envelope->type |= 4;
	}

	finishUndo();
}

bool EnvelopeEditor::isLoopEnabled() const
{
	if (envelope == NULL)
		return false;
	return (envelope->type & 4) != 0;
}

pp_int32 EnvelopeEditor::getLoopStartPtIndex() const
{
	if (envelope == NULL)
		return 0;
	return envelope->loops;
}

pp_int32 EnvelopeEditor::getLoopEndPtIndex() const
{
	if (envelope == NULL)
		return 0;
	return envelope->loope;
}

void EnvelopeEditor::selectNextSustainPoint()
{
	prepareUndo();

	if (envelope && envelope->sustain < envelope->num - 1)
		envelope->sustain++;

	finishUndo();
}

void EnvelopeEditor::selectPreviousSustainPoint()
{
	prepareUndo();

	if (envelope && envelope->sustain > 0)
		envelope->sustain--;

	finishUndo();
}

void EnvelopeEditor::selectNextLoopStartPoint()
{
	prepareUndo();

	if (envelope && envelope->loops < envelope->num - 1)
		envelope->loops++;

	if (envelope->loops>envelope->loope)
		envelope->loops = envelope->loope;

	finishUndo();
}

void EnvelopeEditor::selectPreviousLoopStartPoint()
{
	prepareUndo();

	if (envelope && envelope->loops > 0)
		envelope->loops--;

	if (envelope->loops>envelope->loope)
		envelope->loops = envelope->loope;

	finishUndo();
}

void EnvelopeEditor::selectNextLoopEndPoint()
{
	prepareUndo();

	if (envelope && envelope->loope < envelope->num - 1)
		envelope->loope++;

	if (envelope->loope<envelope->loops)
		envelope->loope = envelope->loops;
		
	finishUndo();
}

void EnvelopeEditor::selectPreviousLoopEndPoint()
{
	prepareUndo();

	if (envelope && envelope->loope > 0)
		envelope->loope--;

	if (envelope->loope<envelope->loops)
		envelope->loope = envelope->loops;

	finishUndo();
}

void EnvelopeEditor::deletePoint()
{
	if (envelope == NULL)
		return;

	if (envelope->num <= 2)
		return;

	if (selectionIndex == -1)
		return;

	prepareUndo();
	
	for (pp_int32 i = selectionIndex; i < envelope->num - 1; i++)
	{
		envelope->env[i][0] = envelope->env[i+1][0];
		envelope->env[i][1] = envelope->env[i+1][1];
	}

	envelope->num--;

	if (envelope->loops > selectionIndex)
		envelope->loops--;
	if (envelope->loops >= envelope->num)
		envelope->loops = envelope->num-1;
	
	if (envelope->loope > selectionIndex)
		envelope->loope--;
	if (envelope->loope >= envelope->num)
		envelope->loope = envelope->num-1;

	if (envelope->sustain > selectionIndex)
		envelope->sustain--;
	if (envelope->sustain >= envelope->num)
		envelope->sustain = envelope->num-1;

	if (selectionIndex >= envelope->num)
		selectionIndex = envelope->num-1;

	finishUndo();
}

void EnvelopeEditor::addPoint()
{
	if (envelope == NULL)
		return;

	if (envelope->num >= 12)
		return;

	pp_int16 points[12][2];

	if (selectionIndex >= 0 && selectionIndex < envelope->num - 1)
	{
		if (envelope->env[selectionIndex + 1][0] - envelope->env[selectionIndex][0] < 2)
			return; // Insufficient space on x-axis

		prepareUndo();

		pp_int32 i = 0;
		for (i = 0; i <= selectionIndex; i++)
		{
			points[i][0] = envelope->env[i][0];
			points[i][1] = envelope->env[i][1];
		}

		if (envelope->loops > selectionIndex)
			envelope->loops++;
		if (envelope->loope > selectionIndex)
			envelope->loope++;
		if (envelope->sustain > selectionIndex)
			envelope->sustain++;

		for (i = selectionIndex + 2; i <= envelope->num; i++)
		{
			points[i][0] = envelope->env[i-1][0];
			points[i][1] = envelope->env[i-1][1];
		}
		selectionIndex++;
		envelope->num++;

		points[selectionIndex][0] = (envelope->env[selectionIndex-1][0] + envelope->env[selectionIndex][0]) / 2;
		points[selectionIndex][1] = (envelope->env[selectionIndex-1][1] + envelope->env[selectionIndex][1]) / 2;

		for (i = 0; i < envelope->num; i++)
		{
			envelope->env[i][0] = points[i][0];
			envelope->env[i][1] = points[i][1];
		}

	}
	else 
	{
		prepareUndo();

		if (envelope->num)
		{
			envelope->env[envelope->num][0] = envelope->env[envelope->num-1][0] + 10;
			envelope->env[envelope->num][1] = envelope->env[envelope->num-1][1];
		}
		else
		{
			envelope->env[0][0] = 0;
			envelope->env[0][1] = 128;
			
			envelope->env[1][0] = 10;
			envelope->env[1][1] = 128;
			
			envelope->num++;
		}
	
		envelope->num++;
	}

	finishUndo();
}

void EnvelopeEditor::tool_xScaleEnvelope(const FilterParameters* par)
{
	prepareUndo();

	float scale = par->getParameter(0).floatPart;

	for (pp_int32 i = 0; i < envelope->num; i++)
	{
		envelope->env[i][0] = (pp_int16)(envelope->env[i][0]*scale);
	}
	
	finishUndo();
}

void EnvelopeEditor::tool_yScaleEnvelope(const FilterParameters* par)
{
	prepareUndo();

	float scale = par->getParameter(0).floatPart;

	pp_int32 i;
	
	switch (getEnvelopeType())
	{
		case EnvelopeEditor::EnvelopeTypeVolume:
			for (i = 0; i < envelope->num; i++)
			{
				pp_int16 fy = (pp_int16)(envelope->env[i][1]*scale);
				if (fy < 0) fy = 0;
				if (fy > 256) fy = 256;
				envelope->env[i][1] = fy;
			}
			break;
			
		case EnvelopeEditor::EnvelopeTypePanning:
			for (i = 0; i < envelope->num; i++)
			{
				pp_int16 fy = (pp_int16)((envelope->env[i][1]-128)*scale) + 128;
				if (fy < 0) fy = 0;
				if (fy > 256) fy = 256;
				envelope->env[i][1] = fy;
			}
			break;
	}
	
	finishUndo();
}
