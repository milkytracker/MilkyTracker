/*
 *  tracker/SampleEditor.cpp
 *
 *  tool_vocode(): GPL /Copyright 2008-2011 David Robillard <http://drobilla.net>
 *  tool_vocode(): GPL /Copyright 1999-2000 Paul Kellett (Maxim Digital Audio)
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
 *  SampleEditor.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 22.11.07.
 *
 */

#include "SampleEditor.h"
#include "SimpleVector.h"
#include "XModule.h"
#include "VRand.h"
#include "FilterParameters.h"
#include "SampleEditorResampler.h"
#include "PlayerMaster.h"
#include "Addon.h"

#define ZEROCROSS(a,b) (a > 0.0 && b <= 0.0 || a < 0.0 && b >= 0.0)

SampleEditor::ClipBoard::ClipBoard() :
		buffer(NULL)
{
}

SampleEditor::ClipBoard::~ClipBoard()
{
	delete[] buffer;
}
		
void SampleEditor::ClipBoard::makeCopy(TXMSample& sample, XModule& module, pp_int32 selectionStart, pp_int32 selectionEnd, bool cut/* = false*/)
{
	if (selectionEnd < 0)
		return;
	
	if (selectionStart < 0)
		selectionStart = 0;
		
	if (selectionEnd > (signed)sample.samplen)
		selectionEnd = sample.samplen;

	if (selectionEnd < selectionStart)
	{
		pp_int32 s = selectionEnd; selectionEnd = selectionStart; selectionStart = s;
	}
	
	this->selectionStart = selectionStart;
	this->selectionEnd = selectionEnd;
		
	this->selectionWidth = abs(selectionEnd - selectionStart); 
		
	if (selectionWidth == 0)
		return;

	if (buffer)
		delete[] buffer;	
		
	numBits = (sample.type & 16) ? 16 : 8;
	
	// 16 bit sample
	if (numBits == 16)
	{
		buffer = (mp_sbyte*)(new mp_sword[selectionWidth+1]);
		
		mp_sword* dstptr = (mp_sword*)buffer;
		for (pp_int32 i = selectionStart; i <= selectionEnd; i++)
			*dstptr++ = sample.getSampleValue(i);
	}
	// 8 bit sample
	else if (numBits == 8)
	{
		buffer = new mp_sbyte[selectionWidth+1];

		mp_sbyte* dstptr = (mp_sbyte*)buffer;
		for (pp_int32 i = selectionStart; i <= selectionEnd; i++)
			*dstptr++ = sample.getSampleValue(i);
	}
	else ASSERT(false);
}

void SampleEditor::ClipBoard::paste(TXMSample& sample, XModule& module, pp_int32 pos)
{
	if (pos < 0)
		pos = 0;

	if (sample.sample == NULL)
	{
		sample.samplen = 0;
		pos = 0;
	}

	pp_int32 newSampleSize = sample.samplen + selectionWidth;
	pp_int32 i;
	
	// 16 bit sample
	if (sample.type & 16)
	{
		mp_sword* newBuffer = (mp_sword*)module.allocSampleMem(newSampleSize*2);
		
		// copy stuff before insert start point
		for (i = 0;  i < pos; i++)
			sample.setSampleValue((mp_ubyte*)newBuffer, i, sample.getSampleValue(i));
		
		// copy selection to start point
		for (i = 0; i < selectionWidth; i++)
			sample.setSampleValue((mp_ubyte*)newBuffer, i+pos, getSampleWord(i));
			
		// copy stuff after insert start point
		for (i = 0;  i < ((signed)sample.samplen - pos); i++)
			sample.setSampleValue((mp_ubyte*)newBuffer, i+pos+selectionWidth, sample.getSampleValue(i+pos));
	
		if (sample.sample)
			module.freeSampleMem((mp_ubyte*)sample.sample);
		
		sample.sample = (mp_sbyte*)newBuffer;
	}
	else
	{
		mp_sbyte* newBuffer = (mp_sbyte*)module.allocSampleMem(newSampleSize);
		
		// copy stuff before insert start point
		for (i = 0;  i < pos; i++)
			sample.setSampleValue((mp_ubyte*)newBuffer, i, sample.getSampleValue(i));
		
		// copy selection to start point
		for (i = 0; i < selectionWidth; i++)
			sample.setSampleValue((mp_ubyte*)newBuffer, i+pos, getSampleByte(i));
			
		// copy stuff after insert start point
		for (i = 0;  i < ((signed)sample.samplen - pos); i++)
			sample.setSampleValue((mp_ubyte*)newBuffer, i+pos+selectionWidth, sample.getSampleValue(i+pos));

		if (sample.sample)
			module.freeSampleMem((mp_ubyte*)sample.sample);

		sample.sample = newBuffer;
	}

	pp_int32 loopend = sample.loopstart + sample.looplen;

	if ((signed)sample.loopstart < pos && loopend > pos)
		loopend+=selectionWidth;
	else if ((signed)sample.loopstart > pos && loopend > pos)
	{
		sample.loopstart+=selectionWidth;
		loopend+=selectionWidth;
	}

	sample.samplen = newSampleSize;

	sample.looplen = loopend - sample.loopstart;

}

void SampleEditor::prepareUndo()
{
	delete before; 
	before = NULL; 
		
	if (undoStackEnabled && undoStackActivated && undoStack) 
	{
		undoUserData.clear();
		notifyListener(NotificationFeedUndoData);

		before = new SampleUndoStackEntry(*sample, 
										  getSelectionStart(), 
										  getSelectionEnd(), 
										  &undoUserData);
	}
}

void SampleEditor::finishUndo()
{
	if (undoStackEnabled && undoStackActivated && undoStack) 
	{ 
		// first of all the listener should get the chance to adjust
		// user data according to our new changes BEFORE we actually save
		// the new state in the undo stack for redo
		lastOperationDidChangeSize = (sample->samplen != before->getSampLen());					
		notifyListener(NotificationChangesValidate);
		
		undoUserData.clear();
		// we want some user data now
		notifyListener(NotificationFeedUndoData);

		SampleUndoStackEntry after(SampleUndoStackEntry(*sample, 
										 getSelectionStart(), 
										 getSelectionEnd(), 
										 &undoUserData)); 
		if (*before != after) 
		{ 
			if (undoStack) 
			{ 
				undoStack->Push(*before); 
				undoStack->Push(after); 
				undoStack->Pop(); 
			} 
		} 
	} 
	
	// we're done, client might want to refresh the screen or whatever
	notifyListener(NotificationChanges);			
}
	
bool SampleEditor::revoke(const SampleUndoStackEntry* stackEntry)
{
	if (sample == NULL)
		return false;
	 if (undoStack == NULL || !undoStackEnabled)
		return false;
		
	sample->samplen = stackEntry->getSampLen();
	sample->loopstart = stackEntry->getLoopStart(); 
	sample->looplen = stackEntry->getLoopLen(); 
	sample->relnote = stackEntry->getRelNote(); 
	sample->finetune = stackEntry->getFineTune(); 
	sample->type = (mp_ubyte)stackEntry->getFlags();
	
	setSelectionStart(stackEntry->getSelectionStart());
	setSelectionEnd(stackEntry->getSelectionEnd());
	
	enterCriticalSection();
	
	// free old sample memory
	if (sample->sample)
	{
		module->freeSampleMem((mp_ubyte*)sample->sample);
		sample->sample = NULL;
	}
	
	if (stackEntry->getBuffer())
	{			
		if (sample->type & 16)
		{
			sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen*2);
			TXMSample::copyPaddedMem(sample->sample, stackEntry->getBuffer(), sample->samplen*2);
		}
		else
		{
			sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen);
			TXMSample::copyPaddedMem(sample->sample, stackEntry->getBuffer(), sample->samplen);
		}
	}
	
	leaveCriticalSection();
	undoUserData = stackEntry->getUserData();
	notifyListener(NotificationFetchUndoData);
	notifyListener(NotificationChanges);
	return true;
}

void SampleEditor::notifyChanges(bool condition, bool lazy/* = true*/)
{
	lastOperation = OperationRegular;	
	lastOperationDidChangeSize = false;
	if (!lazy)
	{
		setLazyUpdateNotifications(false);
		notifyListener(NotificationChangesValidate);
		notifyListener(NotificationChanges);
	}
	else
	{
		setLazyUpdateNotifications(true);
		notifyListener(NotificationChanges);
		setLazyUpdateNotifications(false);
	}
}

SampleEditor::SampleEditor() :
	EditorBase(),
	sample(NULL),
	undoStackEnabled(true), 
	undoStackActivated(true),	
	before(NULL),
	undoStack(NULL),
	lastOperationDidChangeSize(false),
	lastOperation(OperationRegular),
	drawing(false),
	lastSamplePos(-1),
	lastParameters(NULL),
	lastFilterFunc(NULL)
{
	// Undo history
	undoHistory = new UndoHistory<TXMSample, SampleUndoStackEntry>(UNDOHISTORYSIZE_SAMPLEEDITOR);
	
	resetSelection();

	memset(&lastSample, 0, sizeof(lastSample));

	// sampleRate is not perfect (ideally get this from settingsDatabase)
  synth = new Synth(PlayerMaster::getPreferredSampleRate());
}

SampleEditor::~SampleEditor()
{
	delete lastParameters;
	delete undoHistory;
	delete undoStack;
	delete before;
  delete synth;
}

void SampleEditor::attachSample(TXMSample* sample, XModule* module) 
{
	// only return if the sample data really equals what we already have
	if (sample->equals(lastSample) && sample == this->sample)
		return;

	lastSample = *sample;

	// --------- update undo history information --------------------	
	if (undoStackEnabled && undoStackActivated)
	{
		if (undoStack)
		{	
			// if the undo stack is empty, we don't need to save current undo stack
			if (!undoStack->IsEmpty() || !undoStack->IsTop())
			{	
				undoStack = undoHistory->getUndoStack(sample, this->sample, undoStack);
			}
			// delete it if it's empty
			else
			{
				delete undoStack;
				undoStack = NULL;
				
				undoStack = undoHistory->getUndoStack(sample, NULL, NULL);
			}
		}
		
		// couldn't get any from history, create new one
		if (!undoStack)
		{
			undoStack = new PPUndoStack<SampleUndoStackEntry>(UNDODEPTH_SAMPLEEDITOR);
		}
	}

	this->sample = sample;
	attachModule(module);

	resetSelection();
	
	notifyListener(NotificationReload);
}

void SampleEditor::reset()
{
	if (undoStackEnabled)
	{
		if (undoHistory)
			delete undoHistory;
		
		if (undoStack)
		{
			delete undoStack;
			undoStack = NULL;
			undoStack = new PPUndoStack<SampleUndoStackEntry>(UNDODEPTH_SAMPLEEDITOR);
		}
		
		undoHistory = new UndoHistory<TXMSample, SampleUndoStackEntry>(UNDOHISTORYSIZE_SAMPLEEDITOR);
	}
	else
	{
		if (undoHistory)
		{
			delete undoHistory;
			undoHistory = NULL;	
		}
		
		if (undoStack)
		{
			delete undoStack;
			undoStack = NULL;
		}
	}
}

bool SampleEditor::isEmptySample() const  
{
	if (!isValidSample())
		return true;
	
	return (sample->sample == NULL);
}

bool SampleEditor::canMinimize() const
{
	if (!isValidSample())
		return false;
	
	return sample->samplen && sample->sample && (sample->type & 3);
}

bool SampleEditor::isEditableSample() const
{
	if (!isValidSample())
		return false;
	
	return (sample->sample != NULL) && (sample->samplen != 0);
}

void SampleEditor::enableUndoStack(bool enable)
{
	undoStackEnabled = enable;
	reset();
}

bool SampleEditor::undo()
{
	if (!undoStackEnabled || undoStack == NULL) return false;
	if (undoStack->IsEmpty()) return false;
	return revoke(undoStack->Pop());
}

bool SampleEditor::redo()
{
	if (!undoStackEnabled || undoStack == NULL) return false;
	if (undoStack->IsTop()) return false;
	return revoke(undoStack->Advance());
}

void SampleEditor::selectAll()
{
	if (isEmptySample())
		return;

	selectionStart = 0;
	selectionEnd = sample->samplen;
}

void SampleEditor::loopRange()
{
	if (!hasValidSelection())
		return;

	// If a loop type is not enabled, set loop to Forward.
	// - Changes loop type to Forward when loop type is set to One shot
	// 	 and the start of the selection is not at the start of the sample, 
	// 	 but so does dragging the start of the loop. 
	if (!getLoopType())
		setLoopType(1);

	// Once loop is enabled, set the loop start/end points to selection start/end points.
	setRepeatStart(getSelectionStart());
	setRepeatEnd(getSelectionEnd());

	// Doesn't currently have undo or have the sample do the new loop 
	// until it retriggers, but neither does dragging the loop points.
}

bool SampleEditor::validate()
{
	if (isEmptySample())
	{
		resetSelection();
		return false;
	}

	pp_int32 sStart = getSelectionStart();
	pp_int32 sEnd = getSelectionEnd();
	if (sEnd < sStart)
	{
		pp_int32 s = sEnd; sEnd = sStart; sStart = s;
	}
	setSelectionStart(sStart);
	setSelectionEnd(sEnd);

	if (getSelectionEnd() != -1 && getSelectionStart () != -1 &&
		getSelectionEnd() < 0)
	{
		resetSelection();
	}

	if (getSelectionEnd() > 0 && getSelectionStart() < 0)
		setSelectionStart(0);

	if (getSelectionStart() > (signed)sample->samplen)
	{
		resetSelection();
	}
	if (getSelectionEnd() > (signed)sample->samplen)
	{
		setSelectionEnd(sample->samplen);
	}
	
	if (sample->loopstart > sample->samplen)
		sample->loopstart = 0;
	if (sample->loopstart + sample->looplen > sample->samplen)
		sample->looplen -= (sample->loopstart + sample->looplen) - sample->samplen;
	
	// one shot sample only allows loopstart == 0
	if ((sample->type & 32) && sample->loopstart)
	{
		sample->type &= ~32;
	}
	return true;
}

bool SampleEditor::canPaste() const
{ 
	if (selectionEnd == selectionStart &&
		selectionStart == -1 &&
		sample->sample != NULL)
		return false;

	return !ClipBoard::getInstance()->isEmpty(); 
}

pp_uint32 SampleEditor::getRepeatStart() const
{
	return sample == NULL ? 0 : sample->loopstart;
}

pp_uint32 SampleEditor::getRepeatEnd() const
{
	return sample == NULL ? 0 : sample->loopstart + sample->looplen;
}

pp_uint32 SampleEditor::getRepeatLength() const
{
	return sample == NULL ? 0 : sample->looplen;
}

void SampleEditor::setRepeatStart(pp_uint32 start)
{
	if (sample == NULL)
		return;

	mp_uint32 before = sample->loopstart;		
		
	sample->loopstart = start;

	validate();

	notifyChanges(before != sample->loopstart, false);
}

void SampleEditor::setRepeatEnd(pp_uint32 end)
{
	if (sample == NULL)
		return;
		
	mp_uint32 before = sample->looplen;	
		
	sample->looplen = (end - sample->loopstart);
	
	validate();

	notifyChanges(before != sample->looplen, false);
}

void SampleEditor::setRepeatLength(pp_uint32 length)
{
	if (sample == NULL)
		return;

	mp_uint32 before = sample->looplen;	
		
	sample->looplen = length;	

	validate();

	notifyChanges(before != sample->looplen, false);
}

bool SampleEditor::increaseRepeatStart()
{
	if (isEmptySample())
		return false;
	
	mp_uint32 before = sample->loopstart;
	
	pp_int32 loopend = sample->loopstart+sample->looplen;
	pp_int32 loopstart = sample->loopstart+1;
	if (loopstart >= 0 && loopstart < loopend && loopend >= 0 && loopend <= (signed)sample->samplen)
	{
		sample->looplen = loopend - loopstart;
		sample->loopstart = loopstart;
	}
	
	validate();

	notifyChanges(before != sample->loopstart, false);

	return true;
}

bool SampleEditor::decreaseRepeatStart()
{
	if (isEmptySample())
		return false;

	mp_uint32 before = sample->loopstart;

	pp_int32 loopend = sample->loopstart+sample->looplen;
	pp_int32 loopstart = sample->loopstart-1;
	if (loopstart >= 0 && loopstart < loopend && loopend >= 0 && loopend <= (signed)sample->samplen)
	{
		sample->looplen = loopend - loopstart;
		sample->loopstart = loopstart;
	}

	validate();

	notifyChanges(before != sample->loopstart, false);

	return true;
}

bool SampleEditor::increaseRepeatLength()
{
	if (isEmptySample())
		return false;

	mp_uint32 before = sample->looplen;
	
	pp_int32 loopend = sample->loopstart+sample->looplen+1;
	pp_int32 loopstart = sample->loopstart;
	if (loopstart >= 0 && loopstart < loopend && loopend >= 0 && loopend <= (signed)sample->samplen)
	{
		sample->looplen = loopend - loopstart;
		sample->loopstart = loopstart;
	}

	validate();

	notifyChanges(before != sample->looplen, false);

	return true;
}

bool SampleEditor::decreaseRepeatLength()
{
	if (isEmptySample())
		return false;

	mp_uint32 before = sample->looplen;

	pp_int32 loopend = sample->loopstart+sample->looplen-1;
	pp_int32 loopstart = sample->loopstart;
	if (loopstart >= 0 && loopstart < loopend && loopend >= 0 && loopend <= (signed)sample->samplen)
	{
		sample->looplen = loopend - loopstart;
		sample->loopstart = loopstart;
	}
	
	validate();
	
	notifyChanges(before != sample->looplen, false);

	return true;
}

bool SampleEditor::setLoopType(pp_uint8 type)
{
	if (sample == NULL)
		return false;

	mp_ubyte before = sample->type;

	if (type <= 2)
	{
		sample->type &= ~(3+32);
		sample->type |= type;
		
		if (type && 
			sample->loopstart == 0 && 
			sample->looplen == 0)
		{
			sample->loopstart = 0;
			sample->looplen = sample->samplen;
		}
	}
	else if (type == 3)
	{
		sample->type &= ~(3+32);
		sample->type |= (1+32);
		mp_sint32 loopend = sample->loopstart + sample->looplen;
		sample->loopstart = 0;
		sample->looplen = loopend;
	}
	else ASSERT(false);
	
	notifyChanges(before != sample->type);

	return true;
}

pp_uint8 SampleEditor::getLoopType() const
{ 
	if (sample) 
	{
		if ((sample->type & 3) == 1 && (sample->type & 32))
			return 3;
		else
			return sample->type & 3;
	}
	else 
		return 0; 
}

bool SampleEditor::is16Bit() const
{ 
	if (sample) 
		return (sample->type & 16) == 16;
	else 
		return false; 
}

pp_int32 SampleEditor::getRelNoteNum() const
{
	return sample ? sample->relnote : 0;
}

void SampleEditor::increaseRelNoteNum(pp_int32 offset)
{
	if (sample == NULL)
		return;
		
	mp_sbyte before = sample->relnote;
		
	pp_int32 relnote = sample->relnote;
	relnote+=offset;
	if (relnote > 71)
		relnote = 71;
	if (relnote < -48)
		relnote = -48;
	sample->relnote = (mp_sbyte)relnote;

	notifyChanges(sample->relnote != before);
}

pp_int32 SampleEditor::getFinetune() const
{
	return sample ? sample->finetune : 0;
}

void SampleEditor::setFinetune(pp_int32 finetune)
{
	if (sample == NULL)
		return;

	mp_sbyte before = sample->finetune;

	if (finetune < -128)
		finetune = -128;
	if (finetune > 127)
		finetune = 127; 

	sample->finetune = (mp_sbyte)finetune;

	notifyChanges(sample->finetune != before);
}

void SampleEditor::setFT2Volume(pp_int32 vol)
{
	if (sample == NULL)
		return;

	mp_ubyte before = sample->vol;

	sample->vol = XModule::vol64to255(vol);
	
	notifyChanges(sample->vol != before);
}

pp_int32 SampleEditor::getFT2Volume() const
{
	return sample ? XModule::vol255to64(sample->vol) : 0;
}

void SampleEditor::setPanning(pp_int32 pan)
{
	if (sample == NULL)
		return;

	mp_sbyte before = sample->pan;

	if (pan < 0) pan = 0;
	if (pan > 255) pan = 255;
	sample->pan = (mp_sbyte)pan;
	
	notifyChanges(sample->pan != before);
}

pp_int32 SampleEditor::getPanning() const
{
	return sample ? sample->pan : 0;
}

bool SampleEditor::isLastOperationResampling() const {
	return lastFilterFunc == &SampleEditor::tool_resampleSample;
}

const FilterParameters* SampleEditor::getLastParameters() const {
	return lastParameters;
}

const SampleUndoStackEntry* SampleEditor::getUndoSample() const {
	return before;
}

void SampleEditor::startDrawing()
{
	if (sample)
		sample->restoreOriginalState();

	drawing = true;
	lastSamplePos = -1;
	prepareUndo();
}

void SampleEditor::drawSample(pp_int32 sampleIndex, float s)
{
	s*=2.0f;

	pp_int32 from = lastSamplePos;
	pp_int32 to = sampleIndex;
	if (from == -1)
		from = sampleIndex;

	float froms = 0.0f;
	froms = getFloatSampleFromWaveform(from);

	if (from > to)
	{
		pp_int32 h = from; from = to; to = h;
		float fh = froms; froms = s; s = fh;
	}
	
	float step = 0;
	if (to-from)
		step = (s-froms)/(to-from);
	else
		froms = s;
	
	lastSamplePos = sampleIndex;

	for (pp_int32 si = from; si <= to; si++)
	{
		setFloatSampleInWaveform(si, froms);
		froms+=step;
	}	
}

void SampleEditor::endDrawing()
{
	drawing = false;
	lastSamplePos = -1;
	if (!sample || !sample->sample || !sample->samplen)
		return;
	
	lastOperation = OperationRegular;
	finishUndo();
}

void SampleEditor::minimizeSample()
{
	FilterParameters par(0);
	tool_minimizeSample(&par);
}

void SampleEditor::cropSample()
{
	FilterParameters par(0);
	tool_cropSample(&par);
}

void SampleEditor::clearSample()
{
	FilterParameters par(0);
	tool_clearSample(&par);
}

void SampleEditor::mixSpreadPasteSample()
{
	FilterParameters par(1);
	par.setParameter(0, FilterParameters::Parameter(0) ); // spreads selection across sample (changes pitch)
	tool_mixPasteSample(&par);
}

void SampleEditor::mixPasteSample()
{
	FilterParameters par(1);
	par.setParameter(0, FilterParameters::Parameter(1) ); // paste's selection on top new selection start (preserve pitch)
	tool_mixPasteSample(&par);
}

void SampleEditor::mixOverflowPasteSample()
{
	FilterParameters par(1);
	par.setParameter(0, FilterParameters::Parameter(2)); // paste's selection on top new selection start (preserves pitch + overflow) 
	tool_mixPasteSample(&par);
}

void SampleEditor::AMPasteSample()
{
	FilterParameters par(0);
	tool_AMPasteSample(&par);
}

void SampleEditor::FMPasteSample()
{
	FilterParameters par(0);
	tool_FMPasteSample(&par);
}

void SampleEditor::PHPasteSample()
{
	FilterParameters par(0);
	tool_PHPasteSample(&par);
}

void SampleEditor::FLPasteSample()
{
	FilterParameters par(0);
	tool_FLPasteSample(&par);
}

void SampleEditor::convertSampleResolution(bool convert)
{
	FilterParameters par(1);
	par.setParameter(0, FilterParameters::Parameter(convert ? 1 : 0));
	tool_convertSampleResolution(&par);
}

bool SampleEditor::cutSampleInternal()
{
	if (sample == NULL)
		return false;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	if (sStart >= 0 && sEnd >= 0)
	{		
		if (sEnd < sStart)
		{
			pp_int32 s = sEnd; sEnd = sStart; sStart = s;
		}
	}
	else return false;
	
	selectionStart = sStart;
	selectionEnd = sEnd;

	if (sStart == sEnd)
		return false;

	// reset loop area double buffer to original sample state
	// (switch buffering off)
	sample->restoreOriginalState();

	for (pp_uint32 i = selectionEnd; i <= sample->samplen; i++)
		sample->setSampleValue((i-selectionEnd)+selectionStart, sample->getSampleValue(i));

	mp_sint32 sLoopStart = sample->loopstart;
	if (sEnd < (signed)sample->loopstart + (signed)sample->looplen)
		sLoopStart-=(sEnd - sStart);
	if (sLoopStart < 0)
		sLoopStart = 0;
	
	sample->loopstart = sLoopStart;
	sample->samplen -= abs(selectionEnd - selectionStart);
	
	return true;
}

void SampleEditor::cut()
{
	if (sample == NULL)
		return;

	if (!hasValidSelection())
		return;

	// we're going to change the sample buffers, better stop
	enterCriticalSection();
	// undo stuff going on
	prepareUndo();

	// store selection into clipboard
	ClipBoard::getInstance()->makeCopy(*sample, *module, getSelectionStart(), getSelectionEnd());

	// just make clear what kind of an operation this is
	if (cutSampleInternal())
		lastOperation = OperationCut;
	
	// selection no longer intact
	resetSelection();
	
	// validate our internal state
	validate();	
	// redo stuff and client notifications
	finishUndo();
	// keep on playing if you did
	leaveCriticalSection();
}

void SampleEditor::copy()
{
	if (sample == NULL)
		return;

	if (!hasValidSelection())
		return;

	ClipBoard::getInstance()->makeCopy(*sample, *module, getSelectionStart(), getSelectionEnd());
	lastRelNote = sample->relnote;
	lastFineTune = sample->finetune;
	notifyListener(NotificationUpdateNoChanges);
}

void SampleEditor::paste()
{
	if (sample == NULL)
		return;

	enterCriticalSection();

	prepareUndo();

	if (hasValidSelection())
	{
		mp_uint32 loopstart = sample->loopstart;
		mp_uint32 looplen = sample->looplen;
		if (cutSampleInternal())
			lastOperation = OperationCut;
		sample->loopstart = loopstart;
		sample->looplen = looplen;
	}

	ClipBoard::getInstance()->paste(*sample, *module, getSelectionStart());

	setSelectionEnd(getSelectionStart() + ClipBoard::getInstance()->getWidth());
	sample->relnote = lastRelNote;
	sample->finetune = lastFineTune;
	validate();	
	finishUndo();

	leaveCriticalSection();
}

SampleEditor::WorkSample* SampleEditor::createWorkSample(pp_uint32 size, pp_uint8 numBits, pp_uint32 sampleRate)
{
	WorkSample* workSample = new WorkSample(*module, size, numBits, sampleRate);
	if (workSample->buffer == NULL)
	{
		delete workSample;
		return NULL;
	}
	
	return workSample;
}

void SampleEditor::pasteOther(WorkSample& src)
{
	enterCriticalSection();

	prepareUndo();

	if (sample->sample)
	{
		module->freeSampleMem((mp_ubyte*)sample->sample);
		sample->sample = NULL;
		sample->samplen = 0;
	}
	
	sample->loopstart = 0;
	sample->looplen = 0;
	sample->type = (src.numBits == 16) ? 16 : 0;
	sample->samplen = src.size;

	mp_sbyte rn, ft;
	XModule::convertc4spd((mp_uint32)src.sampleRate, &ft, &rn);
	sample->relnote = rn;
	sample->finetune = ft;
	
	sample->sample = (mp_sbyte*)src.buffer;
	src.buffer = NULL;
	
	finishUndo();
	
	leaveCriticalSection();
}

static float ppfabs(float f)
{
	return f < 0 ? -f : f;
}

float SampleEditor::getFloatSampleFromWaveform(pp_int32 index, void* src/* = NULL*/, pp_int32 size/* = 0*/)
{
	if (isEmptySample())
		return 0.0f;
		
	if (!src)
	{
		if (index > (signed)sample->samplen)
			index = sample->samplen;
		if (index < 0)
			index = 0;
	}
	else if (size != 0)
	{
		if (index >= size)
			index = size-1;
		if (index < 0)
			index = 0;
	}
			
	if (sample->type & 16)
	{
		mp_sword s = src ? *(((mp_sword*)src)+index) : sample->getSampleValue(index);
		return s > 0 ? (float)s*(1.0f/32767.0f) : (float)s*(1.0f/32768.0f);
	}
	else
	{
		mp_sbyte s = src ? *(((mp_sbyte*)src)+index) : sample->getSampleValue(index);
		return s > 0 ? (float)s*(1.0f/127.0f) : (float)s*(1.0f/128.0f);
	}
}

void SampleEditor::setFloatSampleInWaveform(pp_int32 index, float singleSample, void* src/* = NULL*/)
{
	if (isEmptySample() || index > (signed)sample->samplen)
		return;
	
	if (index < 0)
		index = 0;
				
	if (singleSample > 1.0f)
		singleSample = 1.0f;
	if (singleSample < -1.0f)
		singleSample = -1.0f;

	if (sample->type & 16)
	{
		mp_sword s = singleSample > 0 ? (mp_sword)(singleSample*32767.0f+0.5f) : (mp_sword)(singleSample*32768.0f-0.5f);
		if (src)
			*(((mp_sword*)src)+index) = s;
		else
			sample->setSampleValue(index, s);
	}
	else
	{
		mp_sbyte s = singleSample > 0 ? (mp_sbyte)(singleSample*127.0f+0.5f) : (mp_sbyte)(singleSample*128.0f-0.5f);
		if (src)
			*(((mp_sbyte*)src)+index) = s;
		else
			sample->setSampleValue(index, s);
	}
}

void SampleEditor::preFilter(TFilterFunc filterFuncPtr, const FilterParameters* par)
{
	if (filterFuncPtr)
	{
		if (par != NULL)
		{
			FilterParameters newPar(*par);
			if (lastParameters)
			{
				delete lastParameters;
				lastParameters = NULL;
			}
			lastParameters = new FilterParameters(newPar);			
		}
		else
		{
			if (lastParameters)
			{
				delete lastParameters;
				lastParameters = NULL;
			}
		}
		
		lastFilterFunc = filterFuncPtr;
	}

	enterCriticalSection();
	
	lastOperation = OperationRegular;

	notifyListener(NotificationPrepareLengthy);
}

void SampleEditor::postFilter()
{
	notifyListener(NotificationUnprepareLengthy);

	leaveCriticalSection();
}

void SampleEditor::tool_newSample(const FilterParameters* par)
{
	preFilter(NULL, NULL);
	
	prepareUndo();

	pp_int32 numSamples = par->getParameter(0).intPart, numBits = par->getParameter(1).intPart;

	if (sample->sample)
	{
		module->freeSampleMem((mp_ubyte*)sample->sample);
		sample->sample = NULL;
	}
	
	sample->samplen = numSamples;
	sample->loopstart = 0;
	sample->looplen = sample->samplen;
	
	switch (numBits)
	{
		case 8:
			sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen);
			memset(sample->sample, 0, sample->samplen);
			break;
		case 16:
			sample->type |= 16;
			sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen*2);
			memset(sample->sample, 0, sample->samplen*2);
			break;
		default:
			ASSERT(false);
	}
	
	finishUndo();

	lastOperation = OperationNew;
	postFilter();
}

void SampleEditor::tool_minimizeSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	if (!(sample->type & 3))
		return;

	preFilter(NULL, NULL);

	prepareUndo();

	pp_int32 loopend = sample->loopstart+sample->looplen;
	
	if (loopend > (signed)sample->samplen)
		loopend = sample->samplen;
	
	sample->samplen = loopend;

	finishUndo();
	
	postFilter();
}

void SampleEditor::tool_cropSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	if (sStart >= 0 && sEnd >= 0)
	{		
		if (sEnd < sStart)
		{
			pp_int32 s = sEnd; sEnd = sStart; sStart = s;
		}
	}
	else return;
	
	selectionStart = sStart;
	selectionEnd = sEnd;

	if (sStart == sEnd)
		return;
		
	preFilter(NULL, NULL);
	
	prepareUndo();
	
	if (sample->type & 16)
	{
		mp_sword* buff = (mp_sword*)sample->sample;
		for (pp_int32 i = selectionStart; i < selectionEnd; i++)
			buff[i-selectionStart] = buff[i];
	}
	else
	{
		mp_sbyte* buff = (mp_sbyte*)sample->sample;
		for (pp_int32 i = selectionStart; i < selectionEnd; i++)
			buff[i-selectionStart] = buff[i];
	}
	
	sample->samplen = abs(selectionEnd - selectionStart);
	
	if (sample->loopstart > sample->samplen)
		sample->loopstart = 0;
	
	pp_int32 loopend = sample->loopstart + sample->looplen;
	
	if (loopend > (signed)sample->samplen)
		loopend = sample->samplen;
	
	sample->looplen = loopend - sample->loopstart;
	
	selectionStart = 0;
	selectionEnd = sample->samplen;
	
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_clearSample(const FilterParameters* par)
{
	preFilter(NULL, NULL);
	
	prepareUndo();

	module->freeSampleMem((mp_ubyte*)sample->sample);
	sample->sample = NULL;
	sample->samplen = 0;
	sample->loopstart = 0;
	sample->looplen = 0;
	
	finishUndo();

	postFilter();
}

void SampleEditor::tool_convertSampleResolution(const FilterParameters* par)
{
	preFilter(NULL, NULL);

	prepareUndo();

	bool convert = (par->getParameter(0).intPart != 0);

	if (sample->type & 16)
	{
		
		if (!convert)
		{
			sample->type &= ~16;
			sample->samplen<<=1;
			sample->looplen<<=1;
			sample->loopstart<<=1;	

		}
		else
		{
			mp_sbyte* buffer = new mp_sbyte[sample->samplen];
			
			for (mp_sint32 i = 0; i < (signed)sample->samplen; i++)
				buffer[i] = (mp_sbyte)(sample->getSampleValue(i)>>8);
			
			module->freeSampleMem((mp_ubyte*)sample->sample);
			sample->type &= ~16;
			sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen);
			memcpy(sample->sample, buffer, sample->samplen);
			
			delete[] buffer;
		}
	}
	else
	{
		if (!convert)
		{
			sample->type |= 16;
			sample->samplen>>=1;
			sample->looplen>>=1;
			sample->loopstart>>=1;
		}
		else
		{			
			mp_sword* buff16 = new mp_sword[sample->samplen];
			
			for (mp_sint32 i = 0; i < (signed)sample->samplen; i++)
				buff16[i] = (mp_sword)(sample->getSampleValue(i)<<8);
			
			module->freeSampleMem((mp_ubyte*)sample->sample);
			sample->type |= 16;
			sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen*2);
			memcpy(sample->sample, buff16, sample->samplen*2);
			
			delete[] buff16;
		}
	}

	finishUndo();
	
	postFilter();
}

void SampleEditor::tool_mixPasteSample(const FilterParameters* par)
{
	ClipBoard* clipBoard = ClipBoard::getInstance();

	bool preservePitch  = par->getParameter(0).intPart > 0;
	bool overflow       = par->getParameter(0).intPart > 1;
	if (isEmptySample())
		return;

	if (ClipBoard::getInstance()->isEmpty())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = preservePitch ? sStart : 0;
		sEnd = sample->samplen;
	}
	if (preservePitch) sEnd = sStart + clipBoard->getWidth();

	preFilter(NULL, NULL);
	
	prepareUndo();
	
	// preserve pitch (otherwise stretch clipboard to selection)
	float step = preservePitch ? 1 : (float)clipBoard->getWidth() / (float)(sEnd - sStart);
	
	float j = 0.0f;
	for (pp_int32 i = sStart; i < sEnd; i++)
	{
		float frac = j - (float)floor(j);
		pp_int16 s = clipBoard->getSampleWord((pp_int32)j);
		float f1 = s < 0 ? (s/32768.0f) : (s/32767.0f);
		s = clipBoard->getSampleWord( ((pp_int32)j+ 1) % sample->samplen );
		float f2 = s < 0 ? (s/32768.0f) : (s/32767.0f);

		float f = (1.0f-frac)*f1 + frac*f2;
		
		setFloatSampleInWaveform(i % sample->samplen, f + getFloatSampleFromWaveform(i % sample->samplen));
		j+=step;
		if (!overflow && i == sample->samplen) break;
	}
				
	finishUndo();	
	
	postFilter();

}

void SampleEditor::tool_AMPasteSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	if (ClipBoard::getInstance()->isEmpty())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(NULL, NULL);
	
	prepareUndo();
	
	ClipBoard* clipBoard = ClipBoard::getInstance();
	
	float step = (float)clipBoard->getWidth() / (float)(sEnd-sStart);
	
	float j = 0.0f;
	for (pp_int32 i = sStart; i < sEnd; i++)
	{
		float frac = j - (float)floor(j);
	
		pp_int16 s = clipBoard->getSampleWord((pp_int32)j);
		float f1 = s < 0 ? (s/32768.0f) : (s/32767.0f);
		s = clipBoard->getSampleWord((pp_int32)j+1);
		float f2 = s < 0 ? (s/32768.0f) : (s/32767.0f);

		float f = (1.0f-frac)*f1 + frac*f2;
		
		setFloatSampleInWaveform(i, f * getFloatSampleFromWaveform(i));
		j+=step;
	}
				
	finishUndo();	
	
	postFilter();

}

void SampleEditor::tool_FMPasteSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	if (ClipBoard::getInstance()->isEmpty())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(NULL, NULL);
	
	prepareUndo();
	
	ClipBoard* clipBoard = ClipBoard::getInstance();
	
	float step;
	
	float j = 0.0f;
	for (pp_int32 i = sStart; i < sEnd; i++)
	{
		float frac = j - (float)floor(j);
	
		pp_int16 s = clipBoard->getSampleWord(((pp_int32)j)%clipBoard->getWidth());
		float f1 = s < 0 ? (s/32768.0f) : (s/32767.0f);
		s = clipBoard->getSampleWord(((pp_int32)j+1)%clipBoard->getWidth());
		float f2 = s < 0 ? (s/32768.0f) : (s/32767.0f);

		float f = (1.0f-frac)*f1 + frac*f2;
		
		step = powf(16.0f,getFloatSampleFromWaveform(i));
		setFloatSampleInWaveform(i, f);
		j+=step;
		while (j>clipBoard->getWidth()) j-=clipBoard->getWidth();
	}
				
	finishUndo();	
	
	postFilter();

}

void SampleEditor::tool_PHPasteSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	if (ClipBoard::getInstance()->isEmpty())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(NULL, NULL);

	prepareUndo();

	ClipBoard* clipBoard = ClipBoard::getInstance();

	// this filter changes the ratio between above zero to below
	// zero values by stretching the above half wave and shrinking
	// the below zero half wave (or the other way around)
	// the frequency of the sample stays constant, only
	// if the initial ratio is 1/1
	// If it's not, the frequency shifts.
	// To work with non synthetic or already distorted samples,
	// this ratio needs to be calculated and compensated.
	// The frequency will still shift if the ratio is not constant
	// during a longer sample. Ce la vie.
	pp_int32 ups=0,downs=0,zeros=0;
	for (pp_int32 i = 0; i < clipBoard->getWidth(); i++)
	{
		if (clipBoard->getSampleWord(i)<0)
		{
			downs++;
		}
		else if (clipBoard->getSampleWord(i)>0)
		{
			ups++;
		}
		else
		{
			zeros++;
		}
	}
	if (!downs && !zeros)
	{
		downs++; // div by zero prevention
	}
	float phaseRatio = ((float)ups+(0.5f*(float)zeros))/((float)downs+(0.5f*(float)zeros));
	float step;
	float j = 0.0f;
	for (pp_int32 i = sStart; i < sEnd; i++)
	{
		float f;
		float fi = getFloatSampleFromWaveform(i);
		// we need to oversample at a much shorter step size to
		// track the zero crossing with sufficient accuracy
		for (pp_int32 oversample = 0; oversample<0x80; oversample++)
		{
			float frac = j - (float)floor(j);

			pp_int16 s = clipBoard->getSampleWord(((pp_int32)j)%clipBoard->getWidth());
			float f1 = s < 0 ? (s/32768.0f) : (s/32767.0f);
			s = clipBoard->getSampleWord(((pp_int32)j+1)%clipBoard->getWidth());
			float f2 = s < 0 ? (s/32768.0f) : (s/32767.0f);

			f = (1.0f-frac)*f1 + frac*f2;

			step = powf(16.0f,fabsf(fi));
			// the lower half wave is matched
			// to keep the frequency constant
			if (f*fi<0.0f)
			{
				step = 1.0f / (1.0f + (1.0f-(1.0f/step)));
			}
			// which needs to be compensated for a nonzero
			// initial half wave ratio
			if (f<0.0f)
			{
				step = step * (1.0f/phaseRatio);
			}
			// we advance by a fraction due to oversampling
			j+=step*(1.0f/0x80);
		}
		while (j>clipBoard->getWidth()) j-=clipBoard->getWidth();
		setFloatSampleInWaveform(i, f);
	}

	finishUndo();

	postFilter();

}

void SampleEditor::tool_FLPasteSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	if (ClipBoard::getInstance()->isEmpty())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(NULL, NULL);

	prepareUndo();

	ClipBoard* clipBoard = ClipBoard::getInstance();

	float step = (float)clipBoard->getWidth() / (float)(sEnd-sStart);

	// This filter mixes the current sample selection
	// with a copy of itself, which is phase-shifted based on the
	// envelope in the clipboard.
	// For best traditional "flangy" results,
	// the envelope should be a smoothly increasing or decreasing
	// line:
	// 0.25 period of a triangle wave at 25% volume works well,
	// and so do  any other smooth ramps up or down to aprox 25%
	// volume.
	// Interesting effects can be achieved with hand drawn envelopes
	float j = 0.0f;
	for (pp_int32 i = sStart; i < sEnd; i++)
	{
		float frac = j - (float)floor(j);

		pp_int16 s = clipBoard->getSampleWord((pp_int32)j);
		float f1 = s < 0 ? (s/32768.0f) : (s/32767.0f);
		s = clipBoard->getSampleWord((pp_int32)j+1);
		float f2 = s < 0 ? (s/32768.0f) : (s/32767.0f);

		float f = (1.0f-frac)*f1 + frac*f2;

		float g0 = getFloatSampleFromWaveform(i);

		float h = (float)i+f*256.0f;
		frac = h - (float)floor(h);
		pp_int32 hi = (pp_int32)h;
		f1 = getFloatSampleFromWaveform(hi);
		f2 = getFloatSampleFromWaveform(hi+1);
		float g1 = (1.0f-frac)*f1 + frac*f2;
		setFloatSampleInWaveform(i, 0.5f*(g0+g1));
		j+=step;
	}

	finishUndo();

	postFilter();

}

void SampleEditor::tool_foldSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = 0;
	pp_int32 sEnd = sample->samplen/2;

	preFilter(&SampleEditor::tool_foldSample, par);
	
	prepareUndo();
	
	pp_int32 i;
	bool is16Bit = (sample->type & 16);

	// mix first half with second half
	for (i = 0;  i < sEnd; i++){
		mp_sint32 mix = is16Bit ? sample->getSampleValue(i)*0.5 + sample->getSampleValue(i+sEnd)*0.5
		                        : sample->getSampleValue(i)*0.5 + sample->getSampleValue(i+sEnd)*0.5;
		sample->setSampleValue( i, mix);
	}

	finishUndo();	
	
	postFilter();
	// store 1st half in clipboard
	setSelectionStart(0);
	setSelectionEnd(sEnd);
	cropSample();
	setRepeatStart(0);
	setRepeatEnd(sEnd);
	setLoopType(1);
}

void SampleEditor::tool_scaleSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_scaleSample, par);
	
	prepareUndo();

	float startScale = par->getParameter(0).floatPart / 100.0f;
	float endScale = par->getNumParameters() == 1 ? startScale : par->getParameter(1).floatPart / 100.0f;
	
	float step = (endScale - startScale) / (float)(sEnd - sStart);
	
	for (pp_int32 i = sStart; i < sEnd; i++)
	{
		float f = getFloatSampleFromWaveform(i);
		setFloatSampleInWaveform(i, f*startScale);
		startScale+=step;
	}
				
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_normalizeSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_normalizeSample, par);
	
	prepareUndo();
	
	float maxLevel = ((par == NULL)? 1.0f : par->getParameter(0).floatPart);
	float peak = 0.0f;

	pp_int32 i;

	// find peak value
	for (i = sStart; i < sEnd; i++)
	{
		float f = getFloatSampleFromWaveform(i);
		if (ppfabs(f) > peak) peak = ppfabs(f);
	}
	
	float scale = maxLevel / peak;
	
	for (i = sStart; i < sEnd; i++)
	{
		float f = getFloatSampleFromWaveform(i);
		setFloatSampleInWaveform(i, f*scale);
	}
				
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_compressSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(&SampleEditor::tool_compressSample, par);

	prepareUndo();

	pp_int32 i;
	float peak = 0.0f;

	// find peak value (pre)
	for (i = sStart; i < sEnd; i++)
	{
		float f = getFloatSampleFromWaveform(i);
		if (ppfabs(f) > peak) peak = ppfabs(f);
	}

	float max = 0.0f;
	float compress = peak * 0.66;
	float last  = 0.0;
	float wpeak = 0.0;
	int zerocross[2];
	zerocross[0] = -1;
	zerocross[1] = -1;
	float treshold = 0.8;
	float peakTreshold = peak * treshold;

	// scaling limiter inspired by awesome 'TAP scaling limiter'
	for (i = sStart; i < sEnd; i++) {
		float f = getFloatSampleFromWaveform(i);
		if (ZEROCROSS(f, last)) {
			zerocross[0] = zerocross[1];
			zerocross[1] = i;
			if (zerocross[0] >= 0 && zerocross[1] > 0) {                   // detected waveset 
				wpeak = 0;
				for (int j = zerocross[0]; j < zerocross[1]; j++) {        // get peak from waveset
					float w = getFloatSampleFromWaveform(j);
					if (ppfabs(w) > wpeak) wpeak = ppfabs(w);
				}
				if (wpeak > peakTreshold) {                                    // scale down waveset if wpeak exceeds treshold
					for (int j = zerocross[0]; j < zerocross[1]; j++) {
						float b = getFloatSampleFromWaveform(j) * (peakTreshold / wpeak);
						this->setFloatSampleInWaveform(j,b );
					}
				}
			}
		}
		last = f;
	}

	// post-compensate amplitudes 
	float scale = (peak/peakTreshold);
	for (i = sStart; i < sEnd; i++)
	{
		float f = getFloatSampleFromWaveform(i);
		setFloatSampleInWaveform(i, f * scale);
	}

	finishUndo();

	postFilter();
}


void SampleEditor::tool_reverseSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_reverseSample, par);
	
	prepareUndo();
	
	pp_int32 i;
	for (i = 0; i < (sEnd-sStart)>>1; i++)
	{
		float f1 = getFloatSampleFromWaveform(sStart + i);
		float f2 = getFloatSampleFromWaveform(sEnd - 1 - i);
		float h = f2;
		f2 = f1; f1 = h;
		setFloatSampleInWaveform(sStart + i, f1);
		setFloatSampleInWaveform(sEnd - 1 - i, f2);
	}
				
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_PTboostSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_PTboostSample, par);
	
	prepareUndo();
	
	pp_int32 i;
	
	float d0 = 0.0f, d1, d2;
	for (i = sStart; i < sEnd; i++)
	{
		d1 = d2 = getFloatSampleFromWaveform(i);
		d1 -= d0;
		d0 = d2;
		
		if (d1 < 0.0f)
		{
			d1 = -d1;
			d1*= 0.25f;
			d2 -= d1;
		}
		else
		{
			d1*= 0.25f;
			d2 += d1;
		}
		
		if (d2 > 1.0f)
			d2 = 1.0f;
		
		if (d2 < -1.0f)
			d2 = -1.0f;
		
		setFloatSampleInWaveform(i, d2);
	}
	
	finishUndo();	
	
	postFilter();
}

bool SampleEditor::isValidxFadeSelection()
{
	if (isEmptySample() || !hasValidSelection() || !(sample->type & 3))
		return false;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (sStart >= 0 && sEnd >= 0)
	{		
		if (sEnd < sStart)
		{
			pp_int32 s = sEnd; sEnd = sStart; sStart = s;
		}
	}
	
	pp_uint32 loopend = sample->loopstart + sample->looplen;
		
	if (((unsigned)sStart <= sample->loopstart && (unsigned)sEnd >= loopend) ||
		((unsigned)sStart > sample->loopstart && (unsigned)sEnd < loopend) ||
		((unsigned)sStart < sample->loopstart && (unsigned)sEnd < sample->loopstart) || 
		((unsigned)sStart > loopend && (unsigned)sEnd > loopend))
		return false;
		
	return true;
}

void SampleEditor::tool_xFadeSample(const FilterParameters* par)
{
	if (!isValidxFadeSelection())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (sStart >= 0 && sEnd >= 0)
	{		
		if (sEnd < sStart)
		{
			pp_int32 s = sEnd; sEnd = sStart; sStart = s;
		}
	}
	
	if (!(sample->type & 3) || sEnd < (signed)sample->loopstart || sStart > (signed)(sample->loopstart + sample->looplen))
		return;

	pp_int32 loopend = sample->loopstart + sample->looplen;
		
	preFilter(&SampleEditor::tool_xFadeSample, par);
	
	if (sStart <= (signed)sample->loopstart && sEnd >= loopend)
		return;
		
	if (sStart >= (signed)sample->loopstart && sEnd >= loopend)
	{
		sStart-=loopend;
		sStart+=sample->loopstart;
		sEnd-=loopend;
		sEnd+=sample->loopstart;
	}
	
	mp_ubyte* buffer = new mp_ubyte[(sample->type & 16) ? sample->samplen*2 : sample->samplen];
	if (!buffer)
		return;

	memcpy(buffer, sample->sample, (sample->type & 16) ? sample->samplen*2 : sample->samplen);

	prepareUndo();	
	
	pp_int32 i = 0;
	
	// loop start
	if ((sample->type & 3) == 1)
	{	
		for (i = sStart; i < (signed)sample->loopstart; i++)
		{
			float t = (((float)i - sStart) / (float)(sample->loopstart - sStart))*0.5f;
			
			float f1 = getFloatSampleFromWaveform(i, buffer, sample->samplen);
			float f2 = getFloatSampleFromWaveform(loopend - (sample->loopstart - sStart) + (i - sStart), buffer, sample->samplen);		
			
			float f = f1*(1.0f-t) + f2*t;
			setFloatSampleInWaveform(i, f);
		}
		
		for (i = sample->loopstart; i < sEnd; i++)
		{
			float t = 0.5f - ((((float)i - sample->loopstart) / (float)(sEnd-sample->loopstart))*0.5f);
			
			float f1 = getFloatSampleFromWaveform(i, buffer, sample->samplen);
			float f2 = getFloatSampleFromWaveform(loopend + (i - sample->loopstart), buffer, sample->samplen);		
			
			float f = f1*(1.0f-t) + f2*t;
			setFloatSampleInWaveform(i, f);
		}
		
		// loop end
		sStart-=sample->loopstart;
		sStart+=loopend;
		sEnd-=sample->loopstart;
		sEnd+=loopend;	
		
		for (i = sStart; i < loopend; i++)
		{
			float t = (((float)i - sStart) / (float)(loopend - sStart))*0.5f;
			
			float f1 = getFloatSampleFromWaveform(i, buffer, sample->samplen);
			float f2 = getFloatSampleFromWaveform(sample->loopstart - (loopend - sStart) + (i - sStart), buffer, sample->samplen);		
			
			float f = f1*(1.0f-t) + f2*t;
			setFloatSampleInWaveform(i, f);
		}	
		
		for (i = loopend; i < sEnd; i++)
		{
			float t = 0.5f - ((((float)i - loopend) / (float)(sEnd-loopend))*0.5f);
			
			float f1 = getFloatSampleFromWaveform(i, buffer, sample->samplen);
			float f2 = getFloatSampleFromWaveform(sample->loopstart + (i - loopend), buffer, sample->samplen);		
			
			float f = f1*(1.0f-t) + f2*t;
			setFloatSampleInWaveform(i, f);
		}
		
	}
	else if ((sample->type & 3) == 2)
	{
		for (i = sStart; i < (signed)sample->loopstart; i++)
		{
			float t = (((float)i - sStart) / (float)(sample->loopstart - sStart))*0.5f;
			
			float f1 = getFloatSampleFromWaveform(i, buffer, sample->samplen);
			float f2 = getFloatSampleFromWaveform(sample->loopstart + (i - sStart), buffer, sample->samplen);		
			
			float f = f1*(1.0f-t) + f2*t;
			setFloatSampleInWaveform(i, f);
		}
		
		for (i = sample->loopstart; i < sEnd; i++)
		{
			float t = 0.5f - ((((float)i - sample->loopstart) / (float)(sEnd-sample->loopstart))*0.5f);
			
			float f1 = getFloatSampleFromWaveform(i, buffer, sample->samplen);
			float f2 = getFloatSampleFromWaveform(sample->loopstart - (i - sample->loopstart), buffer, sample->samplen);		
			
			float f = f1*(1.0f-t) + f2*t;
			setFloatSampleInWaveform(i, f);
		}
	}
	
	delete[] buffer;
	
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_changeSignSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 ignorebits = par->getParameter(0).intPart;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_changeSignSample, par);
	
	prepareUndo();
	
	pp_int32 i;
	pp_uint32 mask;
	if (sample->type & 16)
	{
		mask = 0xffff >> ignorebits;
	}
	else
	{
		mask = 0xff >> ignorebits;
	}
	// lazyness follows
	for (i = sStart; i < sEnd; i++)
	{
		if (sample->type & 16)
		{
			mp_uword* smp = (mp_uword*)sample->sample;
			smp[i] ^= mask;
		}
		else
		{
			mp_ubyte* smp = (mp_ubyte*)sample->sample;
			smp[i] ^= mask;
		}
	}
	
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_swapByteOrderSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	if (!(sample->type & 16))
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_swapByteOrderSample, par);
	
	prepareUndo();
	
	pp_int32 i;

	mp_uword* smp = (mp_uword*)sample->sample;		
	for (i = sStart; i < sEnd; i++)
	{
		mp_uword s = (smp[i] >> 8) | ((smp[i] & 0xFF) << 8);
		smp[i] = s;
	}
	
	finishUndo();	
	
	postFilter();
}

float getc4spd(mp_sint32 relnote,mp_sint32 finetune);

void SampleEditor::tool_resampleSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	preFilter(&SampleEditor::tool_resampleSample, par);
	
	prepareUndo();

	float c4spd = getc4spd(sample->relnote, sample->finetune);

	pp_uint32 resamplerType = par->getParameter(1).intPart;

	SampleEditorResampler resampler(*module, *sample, resamplerType);
	
	bool res = resampler.resample(c4spd, par->getParameter(0).floatPart);
	
	float step = c4spd / par->getParameter(0).floatPart;

	if (res)
	{
		sample->loopstart = (mp_sint32)(sample->loopstart/step);
		sample->looplen = (mp_sint32)(sample->looplen/step);
	
		if (par->getParameter(2).intPart)
		{
			pp_uint32 c4spdi = (mp_uint32)par->getParameter(0).floatPart;
			mp_sbyte rn, ft;
			XModule::convertc4spd((mp_uint32)c4spdi, &ft, &rn);
			sample->relnote = rn;
			sample->finetune = ft;
		}
	}
	
	lastOperation = OperationCut;
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_DCNormalizeSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_DCNormalizeSample, par);
	
	prepareUndo();
	
	pp_int32 i;

	float DC = 0.0f;
	for (i = sStart; i < sEnd; i++)
	{
		DC += getFloatSampleFromWaveform(i);		
	}
	DC = DC / (float)(sEnd-sStart);
	for (i = sStart; i < sEnd; i++)
	{
		setFloatSampleInWaveform(i, getFloatSampleFromWaveform(i) - DC);
	}
	
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_DCOffsetSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_DCOffsetSample, par);
	
	prepareUndo();
	
	pp_int32 i;

	float DC = par->getParameter(0).floatPart;
	for (i = sStart; i < sEnd; i++)
	{
		setFloatSampleInWaveform(i, getFloatSampleFromWaveform(i) + DC);
	}
	
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_rectangularSmoothSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_rectangularSmoothSample, par);
	
	mp_sint32 sLen = sEnd - sStart;
	
	mp_ubyte* buffer = new mp_ubyte[(sample->type & 16) ? sLen*2 : sLen];
	if (!buffer)
		return;

	memcpy(buffer, sample->sample + sStart, (sample->type & 16) ? sLen*2 : sLen);

	prepareUndo();	
	
	pp_int32 i;

	for (i = sStart; i < sEnd; i++)
	{
		float f = (getFloatSampleFromWaveform(i - sStart - 1, buffer, sLen) +
				  getFloatSampleFromWaveform(i - sStart, buffer, sLen) +
				  getFloatSampleFromWaveform(i - sStart + 1, buffer, sLen)) * (1.0f/3.0f);
				  
		setFloatSampleInWaveform(i, f);		
	}
	
	delete[] buffer;
	
	finishUndo();	
	
	postFilter();
}

void SampleEditor::tool_triangularSmoothSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_triangularSmoothSample, par);
	
	mp_sint32 sLen = sEnd - sStart;
	
	mp_ubyte* buffer = new mp_ubyte[(sample->type & 16) ? sLen*2 : sLen];
	if (!buffer)
		return;

	memcpy(buffer, sample->sample + sStart, (sample->type & 16) ? sLen*2 : sLen);

	prepareUndo();	
	
	pp_int32 i;

	for (i = sStart; i < sEnd; i++)
	{
		float f = (getFloatSampleFromWaveform(i - sStart - 2, buffer, sLen) +
				   getFloatSampleFromWaveform(i - sStart - 1, buffer, sLen)*2.0f +
				   getFloatSampleFromWaveform(i - sStart, buffer, sLen)*3.0f +
				   getFloatSampleFromWaveform(i - sStart + 1, buffer, sLen)*2.0f +
				   getFloatSampleFromWaveform(i - sStart + 2, buffer, sLen)) * (1.0f/9.0f);
				  
		setFloatSampleInWaveform(i, f);		
	}
	
	delete[] buffer;
	
	finishUndo();	

	postFilter();
}

void SampleEditor::tool_eqSample(const FilterParameters* par)
{
	tool_eqSample(par,false);
}

void SampleEditor::tool_eqSample(const FilterParameters* par, bool selective)
{
	if (isEmptySample())
		return;

	if (selective && ClipBoard::getInstance()->isEmpty())
		return;

		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	if (selective) {
		preFilter(NULL,NULL);
	} else {	
		preFilter(&SampleEditor::tool_eqSample, par);
	}
	
	prepareUndo();	
	
	ClipBoard* clipBoard;
	float step;
	float j2 = 0.0f;
	if (selective) {
		clipBoard = ClipBoard::getInstance();
		step = (float)clipBoard->getWidth() / (float)(sEnd-sStart);
	}
	
	float c4spd = 8363; // there really should be a global constant for this
	
	Equalizer** eqs = new Equalizer*[par->getNumParameters()];
	
	// three band EQ
	if (par->getNumParameters() == 3)
	{
		for (pp_int32 i = 0; i < par->getNumParameters(); i++)
		{
			eqs[i] = new Equalizer();
			eqs[i]->CalcCoeffs(EQConstants::EQ3bands[i], EQConstants::EQ3bandwidths[i], c4spd, Equalizer::CalcGain(par->getParameter(i).floatPart));
		}
	}
	// ten band EQ
	else if (par->getNumParameters() == 10)
	{
		for (pp_int32 i = 0; i < par->getNumParameters(); i++)
		{
			eqs[i] = new Equalizer();
			eqs[i]->CalcCoeffs(EQConstants::EQ10bands[i], EQConstants::EQ10bandwidths[i], c4spd, Equalizer::CalcGain(par->getParameter(i).floatPart));
		}
	}
	else
	{
		delete[] eqs;
		finishUndo();
		return;
	}
	
	// apply EQ here
	pp_int32 i;

	for (i = sStart; i < sEnd; i++)
	{
		// Fetch a stereo signal
		double xL = getFloatSampleFromWaveform(i);
		double xR = xL;
		float x = (float)xL;
			
		for (pp_int32 j = 0; j < par->getNumParameters(); j++)
		{
			double yL, yR;
			// Pass the stereo input
			eqs[j]->Filter(xL, xR, yL, yR);
			
			xL = yL;
			xR = yR;
		}
		if (selective)
		{
			float frac = j2 - (float)floor(j2);
		
			pp_int16 s = clipBoard->getSampleWord((pp_int32)j2);
			float f1 = s < 0 ? (s/32768.0f) : (s/32767.0f);
			s = clipBoard->getSampleWord((pp_int32)j2+1);
			float f2 = s < 0 ? (s/32768.0f) : (s/32767.0f);

			float f = (1.0f-frac)*f1 + frac*f2;

			if (f>=0) {
				x = f * ((float)xL) + (1.0f-f) * x;
			} else {
				x = -f * (x-(float)xL) + (1.0+f) * x; 
			}
			j2+=step;
		} else {
			x = (float)xL;
		}
		setFloatSampleInWaveform(i, x);
	}
	
	for (i = 0; i < par->getNumParameters(); i++)
		delete eqs[i];
	
	delete[] eqs;
	finishUndo();	

	postFilter();
}

void SampleEditor::tool_generateSilence(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (sStart >= 0 && sEnd >= 0)
	{
		if (sEnd < sStart)
		{
			pp_int32 s = sEnd; sEnd = sStart; sStart = s;
		}
	}
	else
	{
		sStart = 0;
		sEnd = 0;
	}
	
	preFilter(&SampleEditor::tool_generateSilence, par);
	
	mp_sint32 sLen = sEnd - sStart;
	
	prepareUndo();	
	
	pp_int32 i, j;

	pp_int32 size = par->getParameter(0).intPart;

	pp_int32 newSampleSize = (sample->samplen - sLen) + size;
	
	if (sample->type & 16)
	{		
		mp_sword* dst = new mp_sword[newSampleSize];

		j = 0;
		for (i = 0; i < sStart; i++, j++)
			dst[j] = sample->getSampleValue(i);

		for (i = 0; i < size; i++, j++)
			dst[j] = 0;

		for (i = sEnd; i < (signed)sample->samplen; i++, j++)
			dst[j] = sample->getSampleValue(i);

		module->freeSampleMem((mp_ubyte*)sample->sample);
		sample->sample = (mp_sbyte*)module->allocSampleMem(newSampleSize*2);
		memcpy(sample->sample, dst, newSampleSize*2);

		sample->samplen = newSampleSize;

		delete[] dst;
	}
	else
	{
		mp_sbyte* dst = new mp_sbyte[newSampleSize];

		j = 0;
		for (i = 0; i < sStart; i++, j++)
			dst[j] = sample->getSampleValue(i);

		for (i = 0; i < size; i++, j++)
			dst[j] = 0;

		for (i = sEnd; i < (signed)sample->samplen; i++, j++)
			dst[j] = sample->getSampleValue(i);

		module->freeSampleMem((mp_ubyte*)sample->sample);
		sample->sample = (mp_sbyte*)module->allocSampleMem(newSampleSize);
		memcpy(sample->sample, dst, newSampleSize);

		sample->samplen = newSampleSize;

		delete[] dst;
	}

	// show everything
	lastOperation = OperationCut;
	finishUndo();	

	postFilter();
}

void SampleEditor::tool_generateNoise(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_generateNoise, par);
	
	prepareUndo();	
	
	pp_int32 i;

	float    amp  = par->getParameter(0).floatPart;
	pp_int32 type = par->getParameter(1).intPart;

	VRand rand;
	rand.seed();

	switch (type)
	{
		case 0:
			for (i = sStart; i < sEnd; i++){
				float x = getFloatSampleFromWaveform(i);
				setFloatSampleInWaveform(i, x + (rand.white()*2.0f)*amp );		
			}
			break;
		case 1:
			for (i = sStart; i < sEnd; i++){
				float x = getFloatSampleFromWaveform(i);
				setFloatSampleInWaveform(i, x + (rand.pink()*2.0f)*amp );		
			}
			break;
		case 2:
			for (i = sStart; i < sEnd; i++){
				float x = getFloatSampleFromWaveform(i);
				setFloatSampleInWaveform(i, x + (rand.brown()*2.0f)*amp );		
			}
			break;
	}
	
	finishUndo();	

	postFilter();
}

void SampleEditor::tool_generateSine(const FilterParameters* par)
{
	if (isEmptySample())
		return;
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_generateSine, par);
	
	mp_sint32 sLen = sEnd - sStart;
	
	prepareUndo();	
	
	pp_int32 i;

	const float numPeriods = (float)(6.283185307179586476925286766559 * par->getParameter(1).floatPart);
	const float amplify = par->getParameter(0).floatPart;

	// generate sine wave here
	for (i = sStart; i < sEnd; i++)
	{
		float v = getFloatSampleFromWaveform(i);
		float per = (i-sStart)/(float)sLen * numPeriods;
		setFloatSampleInWaveform(i, v + (float)sin(per)*amplify);	
	}

	finishUndo();	

	postFilter();
}

void SampleEditor::tool_generateSquare(const FilterParameters* par)
{
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_generateSquare, par);
	
	mp_sint32 sLen = sEnd - sStart;
	
	prepareUndo();	
	
	pp_int32 i;

	const float numPeriods = par->getParameter(1).floatPart;
	const float amplify = par->getParameter(0).floatPart;

	// generate square wave here
	for (i = sStart; i < sEnd; i++)
	{
    float v   = getFloatSampleFromWaveform(i);
		float per = (i-sStart)/(float)sLen * numPeriods;
		float frac = per-(float)floor(per);
		setFloatSampleInWaveform(i, v + (frac < 0.5f ? amplify : -amplify) );	
	}

	finishUndo();	

	postFilter();
}

void SampleEditor::tool_generateTriangle(const FilterParameters* par)
{
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_generateTriangle, par);
	
	mp_sint32 sLen = sEnd - sStart;
	
	prepareUndo();	
	
	pp_int32 i;

	const float numPeriods = par->getParameter(1).floatPart;
	const float amplify = par->getParameter(0).floatPart;

	// generate triangle wave here
	for (i = sStart; i < sEnd; i++)
	{
    float v   = getFloatSampleFromWaveform(i);
		float per = (i-sStart)/(float)sLen * numPeriods;
		float frac = per-(float)floor(per);
		if (frac < 0.25f)
			setFloatSampleInWaveform(i, v + (frac*4.0f)*amplify);	
		else if (frac < 0.75f)
			setFloatSampleInWaveform(i, v + (1.0f-(frac-0.25f)*4.0f)*amplify);	
		else	
			setFloatSampleInWaveform(i, v + (-1.0f+(frac-0.75f)*4.0f)*amplify);	
	}

	finishUndo();	

	postFilter();
}

void SampleEditor::tool_generateSawtooth(const FilterParameters* par)
{
		
	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_generateSawtooth, par);
	
	mp_sint32 sLen = sEnd - sStart;
	
	prepareUndo();	
	
	pp_int32 i;

	const float numPeriods = par->getParameter(1).floatPart;
	const float amplify = par->getParameter(0).floatPart;

	// generate saw-tooth wave here
	for (i = sStart; i < sEnd; i++)
	{
		float per = (i-sStart)/(float)sLen * numPeriods;
		float frac = per-(float)floor(per);
    float v   = getFloatSampleFromWaveform(i);
		setFloatSampleInWaveform(i, v + (frac < 0.5f ? (frac*2.0f)*amplify : (-1.0f+((frac-0.5f)*2.0f))*amplify) );	
	}

	finishUndo();	

	postFilter();
}

void SampleEditor::tool_generateHalfSine(const FilterParameters* par)
{

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(&SampleEditor::tool_generateHalfSine, par);

	mp_sint32 sLen = sEnd - sStart;

	prepareUndo();

	pp_int32 i;

	const float numPeriods = (float)(6.283185307179586476925286766559 * par->getParameter(1).floatPart);
	const float amplify = par->getParameter(0).floatPart;

	// generate half sine wave here
	for (i = sStart; i < sStart + sLen / 2; i++)
	{
		float v   = getFloatSampleFromWaveform(i);
		float per = (i - sStart) / (float)sLen * numPeriods;
		setFloatSampleInWaveform(i, v + ((float)sin(per) * amplify) );
	}
	for (; i < sEnd; i++)
	{
		float v   = getFloatSampleFromWaveform(i);
		setFloatSampleInWaveform(i, 0 + v);
	}

	finishUndo();

	postFilter();
}

void SampleEditor::tool_generateAbsoluteSine(const FilterParameters* par)
{

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(&SampleEditor::tool_generateAbsoluteSine, par);

	mp_sint32 sLen = sEnd - sStart;

	prepareUndo();

	pp_int32 i;

	const float numPeriods = (float)(6.283185307179586476925286766559 * par->getParameter(1).floatPart);
	const float amplify = par->getParameter(0).floatPart;

	// generate absolute sine wave here
	for (i = sStart; i < sEnd; i++)
	{
    float v   = getFloatSampleFromWaveform(i);
		float per = (i - sStart) / (float)sLen * numPeriods;
		setFloatSampleInWaveform(i, v + (fabs((float)sin(per) * amplify)) );
	}

	finishUndo();

	postFilter();
}

void SampleEditor::tool_generateQuarterSine(const FilterParameters* par)
{

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(&SampleEditor::tool_generateQuarterSine, par);

	mp_sint32 sLen = sEnd - sStart;

	prepareUndo();

	pp_int32 i;

	const float numPeriods = (float)(6.283185307179586476925286766559 * par->getParameter(1).floatPart);
	const float amplify = par->getParameter(0).floatPart;

	// generate quarter sine wave in first and third quarters
	for (i = sStart; i < sStart + sLen / 4; i++)
	{
    float v   = getFloatSampleFromWaveform(i);
		float per = (i - sStart) / (float)sLen * numPeriods;
		setFloatSampleInWaveform(i, v + ((float)sin(per) * amplify) );
	}
	for (; i < sStart + sLen / 2; i++)
	{
    float v   = getFloatSampleFromWaveform(i);
		setFloatSampleInWaveform(i, v+0);
	}
	for (; i < sStart + sLen * 3 / 4; i++)
	{
    float v   = getFloatSampleFromWaveform(i);
		float per = (i - (sStart + sLen / 2)) / (float)sLen * numPeriods;
		setFloatSampleInWaveform(i, v + ((float)sin(per) * amplify) );
	}
	for (; i < sEnd; i++)
	{
    float v   = getFloatSampleFromWaveform(i);
		setFloatSampleInWaveform(i, v+0);
	}

	finishUndo();

	postFilter();
}

bool SampleEditor::tool_canApplyLastFilter() const
{
	return lastFilterFunc != NULL && isValidSample(); 
}

void SampleEditor::tool_applyLastFilter()
{
	if (lastFilterFunc)
	{
		if (lastParameters)
		{
			FilterParameters newPar(*lastParameters);
			(this->*lastFilterFunc)(&newPar);
		}
		else
		{
			(this->*lastFilterFunc)(NULL);
		}
	}
}

pp_uint32 SampleEditor::convertSmpPosToMillis(pp_uint32 pos, pp_int32 relativeNote/* = 0*/)
{
	if (!isValidSample())
		return 0;
			
	relativeNote+=sample->relnote;
	
	double c4spd = XModule::getc4spd(relativeNote, sample->finetune);
	
	return (pp_uint32)(((double)pos / c4spd) * 1000.0);
}

void SampleEditor::tool_reverb(const FilterParameters* par)
{ 
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;
	
	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{		
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}
	
	preFilter(&SampleEditor::tool_reverb, par);
	
	prepareUndo();

	pp_int32 sLength = sEnd - sStart;
	float ratio   = par->getParameter(0).floatPart / 100.0f;
    pp_uint32 verb_size = 700 * (pp_uint32)par->getParameter(1).floatPart;
    pp_int32 newSampleSize = sLength + verb_size;

	// create buffers (smpout will be calloc'ed by reverb)
	float* smpin;
	float* smpout;
	smpin = (float*)calloc(newSampleSize,  sizeof(float));
	for (pp_int32 i = 0; i < newSampleSize; i++) { // copy source (and pad with zeros)
		smpin[i] = i < sLength ? this->getFloatSampleFromWaveform(i+sStart) : 0.0;
	}

	int outlength = Convolver::reverb( smpin, &smpout, newSampleSize, verb_size);

	for (pp_int32 i = sStart; i < sStart+outlength; i++) {
		pp_uint32 pos = i % sEnd;
		if( pos < sStart ) pos += sStart; // fold back reverb tail to beginning (aid seamless looping)
		float dry      = this->getFloatSampleFromWaveform( pos ) * ( i < sEnd ?  (1.0-ratio) : 1.0 );
		float wet      = 1.2 * (smpout[i] * ratio);
		this->setFloatSampleInWaveform(pos, dry+wet );
	}
				
	finishUndo();	
	
	postFilter();

	free(smpin);
	free(smpout);
}

void SampleEditor::tool_MTboostSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	pp_int32 sLength = sEnd - sStart;

	preFilter(&SampleEditor::tool_MTboostSample, par);

	prepareUndo();

	pp_int32 i;

	// instead of only distorting highfreqs (PTboost) we 
	// filter the highfreqs and smear them with a reverb 
	pp_int32 samplerate = 32000;
	filter_t hp;
	Filter::init( (filter_t *)&hp, samplerate );
    // extract and resonate high end 
	hp.cutoff = par->getParameter(0).floatPart; //(samplerate/2);
	hp.q      = 0.01; 

	float* smpin;
	float* smpout;
	smpin = (float*)calloc(sLength,  sizeof(float));
	for (pp_int32 i = 0; i < sLength; i++) { // copy source (and pad with zeros)
		Filter::process( this->getFloatSampleFromWaveform(i+sStart), (filter_t *)&hp );  // apply HP
		smpin[i] = hp.out_hp;
	}
	// smear and smooth with a phasing roomverb
	int outlength = Convolver::reverb( smpin, &smpout, sLength, 100 * (int)par->getParameter(1).floatPart );
	int phase     = (int)( float(samplerate/5000) * par->getParameter(2).floatPart );
	float wet     = ( par->getParameter(3).floatPart / 100.0f) * 5.0f;

	float in  = 0.0;
	float out = 0.0;

	pp_int32 pos = 0;

	for (i = 0; i < sLength; i++)
	{
        out = i >= phase ? smpout[ i-phase ] : 0.0f;
		this->setFloatSampleInWaveform(i, this->getFloatSampleFromWaveform(i+sStart) + (out*wet) );
	}

	finishUndo();

	postFilter();
}

void SampleEditor::tool_saturate(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(&SampleEditor::tool_saturate, par);

	prepareUndo();

	pp_int32 i;
	float in;
	float out;
	float peak = 0.0f;
	float foldback = par->getParameter(0).floatPart / 5.0;
	if( foldback < 1.0 ) foldback = 1.0;
	pp_int32 samplerate = XModule::getc4spd(sample->relnote, sample->finetune);
	float freq = par->getParameter(1).floatPart / 100.0;
	freq = (freq*freq*freq) * float(samplerate/2); // curve
	float dry = fmin( 1.0f - (par->getParameter(3).floatPart / 100.0f  ), 0.5 ) * 2.0f;
	float wet = ( par->getParameter(3).floatPart / 100.0f);
	float compand = ( par->getParameter(2).floatPart / 25.0f);
	float volume  = par->getParameter(4).floatPart / 100.0f;
	float scale;

	// init filter
	multifilter_t filter;
	multifilter_state_t filter0;
	Filter::multifilter_set(&filter,
		samplerate,
	 	freq > 0.05 ? FILTER_BANDPASS: FILTER_NONE,
		freq, // freq 
		0.9,  // res 
		1.5); // gain
	filter0.x1 = filter0.x2 = filter0.y1 = filter0.y2 = 0.0;

	// find peak value 
	for (i = sStart; i < sEnd; i++)
	{
		float f = getFloatSampleFromWaveform(i);
		if (ppfabs(f) > peak) peak = ppfabs(f);
	}
	scale = 1.0f/peak;

	// process 
	for (i = sStart; i < sEnd; i++)
	{
		in  = getFloatSampleFromWaveform(i);                  // normalized amp input
		out = Filter::multifilter(&filter, &filter0, in );    // bandpass
                                                          //
		if( compand >= 1.0 ){                                 // we average with companded version  
		  out = (out + tanh( out * compand ))/2.0;            // https://graphtoy.com/?f1(x,t)=(x%20+%20tanh(%20x%20*%205))/2
		}
		out = sin( (out*scale) * foldback ) / foldback;       // sinusoid foldback & denormalize 
		out = (out*wet)  + (in*dry);					      //
		setFloatSampleInWaveform(i, out * peak * volume );   // full harmonic fold complete
	}

	finishUndo();

	postFilter();
}

void SampleEditor::tool_filter(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(&SampleEditor::tool_filter, par);

	prepareUndo();

	pp_int32 samplerate = 48000;
	// static filter (for highpass/lowpass)
	filter_t lp;
	filter_t hp;
	Filter::init( (filter_t *)&lp, samplerate ); 
	Filter::init( (filter_t *)&hp, samplerate );
	hp.cutoff  = par->getParameter(0).floatPart;
	hp.q       = par->getParameter(2).floatPart / 10.0;
	lp.cutoff  = par->getParameter(1).floatPart;
	lp.q       = hp.q;
	float scale  = par->getParameter(4).floatPart / 100.0f;

	// sweeping filter
	int sweep   = (int)par->getParameter(3).floatPart;
	multifilter_t filter;
	multifilter_state_t filter0;
	multifilter_type_t type;
	filter0.x1 = filter0.x2 = filter0.y1 = filter0.y2 = 0.0;

	float sweepmin = 150.0f;
	float sweepmax = 21000.0f;
	switch( sweep ){
		case 0: type = FILTER_NONE;    break;
		case 1: { type = FILTER_LOWPASS; sweepmax = lp.cutoff; sweepmin = hp.cutoff; break; }
		case 2: { type = FILTER_BANDPASS;sweepmax = lp.cutoff; sweepmin = hp.cutoff; break; }
		case 3: { type = FILTER_NOTCH;   sweepmax = lp.cutoff; sweepmin = hp.cutoff; break; }
	}  
	float sweepadd = sweepmax/float(sample->samplen);

	// lets go
	pp_int32 i;
	float in;
	float out;

	// process 
	for (i = sStart; i < sEnd; i++)
	{
		in = getFloatSampleFromWaveform(i);

		Filter::process( in, (filter_t *)&lp );               // apply LP (+grit)
		out = lp.out_lp;                                      
        if( sweep == 0 ){
			Filter::process( out, (filter_t *)&hp );          // apply HP
			out = hp.out_hp;
        }else{												  
			Filter::multifilter_set(&filter,
					samplerate,
					type,
					sweepmin + (float(i)*sweepadd),            // freq 
					0.1+hp.q,                                  // res 
					1.0);                                      // gain
			out = Filter::multifilter(&filter, &filter0, out );// sweep it!
        }													   //
		setFloatSampleInWaveform(i, sin(out * scale) );        // update 
	}

	finishUndo();

	postFilter();
}

void SampleEditor::tool_timestretch(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	preFilter(&SampleEditor::tool_timestretch, par);

	prepareUndo();

	pp_uint32 i;
  pp_uint32 gi      = 0; // index of grain-window (samples)
  pp_uint32 overlap = 0;
  pp_uint32 pos  = 0;
  pp_uint32 end  = 0;
  float    gin   = 0.0f; // index of grain-window (normalized between 0..1)
  float *buf;
  float scale;
  pp_int32 grain    = (int)par->getParameter(0).floatPart;    // grain size
  pp_int32 stretch  = 1+(int)par->getParameter(1).floatPart;  // stretch factor
  pp_int32 sLength2 = sample->samplen * (2+stretch);

  pp_int32 samplerate = XModule::getc4spd(sample->relnote, sample->finetune);
  buf = (float*)malloc( sLength2 * sizeof(float));
  for( i = 0; i < sLength2; i++ ) buf[i] = 0.0f;

	// 90s akai-style timestretch algo
	for (i = 0; i < sample->samplen; i++) {
    if( gi == 0 ){
      for( pp_int32 s = 0; s < stretch; s++ ){
        overlap += (grain/2);
        for( pp_int32 j = 0; j < grain; j++ ){
          gin   = (1.0f/(float)grain) * (float)j;   // normalize grainposition
          scale = sin(gin/M_PI*9.8664);             // apply fade-in fade-out curve
          pos   = i+j < sample->samplen-1 ? i+j : sample->samplen-1;
          float f = getFloatSampleFromWaveform(pos);
          end = j + overlap < sLength2-1 ? j + overlap : sLength2-1;;
          buf[ end ] = buf[ end ] + (f*scale);
        } 
      }
    }
    gi = (gi+1) % grain;
	}

  // write sample
  mp_ubyte *oldsample = (mp_ubyte*)sample->sample;
  sample->samplen = end;
  if( sample->type & 16 ){
    sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen*2);
    memset(sample->sample, 0, sample->samplen*2);
  }else{
    sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen);
    memset(sample->sample, 0, sample->samplen);
  }

  for( i = 0; i < end; i++ ){
    this->setFloatSampleInWaveform(i, buf[i] );
  }

  // free mem
  free(buf);
  module->freeSampleMem(oldsample);

	finishUndo();

	postFilter();
}

void SampleEditor::tool_delay(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	preFilter(&SampleEditor::tool_delay, par);

	prepareUndo();

  // setup params and vars
	pp_uint32 i;
  pp_uint32 looptype = getLoopType();
  pp_uint32 iecho = 0;                                    // current echo
  pp_uint32 pos   = 0;                                    // sample position
  pp_int32 delay  = (int)par->getParameter(0).floatPart;  // delay size in samples
  pp_int32 echos  = (int)par->getParameter(1).floatPart;  // number of echo's 
  float dry       = fmin( 1.0f - (par->getParameter(5).floatPart / 100.0f  ), 0.5 ) * 2.0f;
  float wet       = ( par->getParameter(5).floatPart / 100.0f);
  float detune    = (1.0f + (float)par->getParameter(2).floatPart) / 500.0f;
  float bandpass  = (float)par->getParameter(3).floatPart;
  float saturate  = (float)par->getParameter(4).floatPart / 10.0f;

  // only pad sample when not looped, otherwise overflow in case of forward loop
  pp_int32 sLength2 = looptype == 0 || looptype == 3? sample->samplen + (echos*delay) : sample->samplen;
  bool overflow     = looptype == 1;

  // setup bandpass filter (actually this is an brickwall filter [lp+hp]) 
  filter_t lp; 
  filter_t hp;
  Filter::init( (filter_t *)&lp, 48000 ); 
  Filter::init( (filter_t *)&hp, 48000 );
  hp.cutoff    = bandpass;
  lp.cutoff    = hp.cutoff + 500;  // 500hz bpf bandwidth
  lp.q  = hp.q = 0.66;             // with high resonance

  // create temporary sample
  float *buf;
  buf = (float*)malloc( sLength2 * sizeof(float));
  for( i = 0; i < sLength2; i++ ) buf[i] = 0.0f;

	for (i = 0; i < sample->samplen; i++) {
    buf[i] = getFloatSampleFromWaveform(i) * dry;
  }

  // lets go
	for (iecho= 0; iecho < echos; iecho++ ){
    float fi = 0.0f;
    for ( i=0;i < sample->samplen; i++ ){
      fi += 1.0f - detune;
      pos = ((iecho+1) * delay) + i;
      if( pos >= sLength2 &&  overflow ) pos = pos % sLength2;
      if( pos >= sLength2 && !overflow ) break; 
      float amp = 1.0f - ((float)iecho/(0.8*(float)echos));    // calculate echo fadeout
      float out = wet * sin( getFloatSampleFromWaveform( (int)fi ) * (1+saturate) )  * amp; // saturate + apply fadeout
      if( bandpass > 60.0 ){
        Filter::process( out, (filter_t *)&lp );    // apply LP
        out = lp.out_lp;                            //
        Filter::process( out, (filter_t *)&hp );    // apply HP
        out = hp.out_hp;                            //
      }                                             //
      buf[pos] += -out;                             // add phase-inverted so flange effect will be more articulated due to PWM
    }
	}

  // write sample
  mp_ubyte *oldsample = (mp_ubyte*)sample->sample;
  sample->samplen = sLength2; 
  if( sample->type & 16 ){
    sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen*2);
    memset(sample->sample, 0, sample->samplen*2);
  }else{
    sample->sample = (mp_sbyte*)module->allocSampleMem(sample->samplen);
    memset(sample->sample, 0, sample->samplen);
  }
  for( i = 0; i < sLength2; i++ ){
    this->setFloatSampleInWaveform(i, buf[i] );
  }

  // free mem
  free(buf);
  module->freeSampleMem(oldsample);

	finishUndo();

	postFilter();
}

void SampleEditor::tool_synth(const FilterParameters* par)
{
	preFilter(&SampleEditor::tool_synth, par);

	prepareUndo();

  // update controls just to be sure
  for( int i = 0; i < synth->getMaxParam(); i++ ){
    synth->setParam(i, (float)par->getParameter(i).floatPart );
  }
  
  //enableUndoStack(false);
  synth->process( NULL,NULL);
  //enableUndoStack(true);

  // serialize synth to samplename 
  PPString preset = synth->ASCIISynthExport();
  memcpy( sample->name, preset.getStrBuffer(), MP_MAXTEXT );

  finishUndo();

  postFilter();
}


void SampleEditor::tool_vocodeSample(const FilterParameters* par)
{
	if (isEmptySample())
		return;

	pp_int32 sStart = selectionStart;
	pp_int32 sEnd = selectionEnd;

	if (hasValidSelection())
	{
		if (sStart >= 0 && sEnd >= 0)
		{
			if (sEnd < sStart)
			{
				pp_int32 s = sEnd; sEnd = sStart; sStart = s;
			}
		}
	}
	else
	{
		sStart = 0;
		sEnd = sample->samplen;
	}

	preFilter(&SampleEditor::tool_vocodeSample, par);

	prepareUndo();

	ClipBoard* clipBoard = ClipBoard::getInstance();

	pp_int32 cLength = clipBoard->getWidth();
	pp_int32 sLength = sEnd - sStart;
	
	if (ClipBoard::getInstance()->isEmpty())
		return;

	///global internal variables
	pp_int32 i;
	const pp_int32 bands = 8; // 16 is buggy 
	pp_int32  swap;   //input channel swap
	float gain;       //output level
	float thru = 100.0f / par->getParameter(2).floatPart;
	float high = 100.0f / par->getParameter(3).floatPart;
	float q = 100.0f / par->getParameter(4).floatPart;
	float kout; //downsampled output
	pp_int32  kval; //downsample counter
	pp_int32  nbnd; //number of bands

	float param[7];
	// SANE DEFAULTS
	param[0] = 0.0f;   //input select
  param[1] = 0.0;
	param[2] = par->getParameter(2).floatPart / 100.0f; // 0.40f;  //hi thru
	param[3] = par->getParameter(3).floatPart / 50.0f; // 0.40f;  // hi freq 
	param[4] = (1.0f / 9.0)* par->getParameter(0).floatPart; // envelope
	param[5] = par->getParameter(4).floatPart / 100.0f; // 0.5f;   // filter q
	param[6] = 1.0f; 

	//filter coeffs and buffers - seems it's faster to leave this global than make local copy 
	float f[bands][13]; //[0-8][0 1 2 | 0 1 2 3 | 0 1 2 3 | val rate]
                        //  #   reson | carrier |modulator| envelope
	// init 
	double tpofs = 6.2831853 / XModule::getc4spd(sample->relnote, sample->finetune); /* FIXME somehow guess samplerate */
	double rr, th; //, re;
	float sh;

	if( bands == 8){
		nbnd = 8;
		//re=0.003f;
		f[1][2] = 3000.0f;
		f[2][2] = 2200.0f;
		f[3][2] = 1500.0f;
		f[4][2] = 1080.0f;
		f[5][2] = 700.0f;
		f[6][2] = 390.0f;
		f[7][2] = 190.0f;
		param[1] = 0.40f + (param[4] * 0.6);  //output dB
	}
	else
	{
		nbnd = 16;
		//re=0.0015f;
		f[1][2] = 5000.0f; //+1000
		f[2][2] = 4000.0f; //+750
		f[3][2] = 3250.0f; //+500
		f[4][2] = 2750.0f; //+450
		f[5][2] = 2300.0f; //+300
		f[6][2] = 2000.0f; //+250
		f[7][2] = 1750.0f; //+250
		f[8][2] = 1500.0f; //+250
		f[9][2] = 1250.0f; //+250
		f[10][2] = 1000.0f; //+250
		f[11][2] = 750.0f; //+210
		f[12][2] = 540.0f; //+190
		f[13][2] = 350.0f; //+155
		f[14][2] = 195.0f; //+100
		f[15][2] = 95.0f;
		param[1] = 0.40f;  //output dB
	}

	for (i = 0; i < nbnd; i++) for (int j = 3; j < 12; j++) f[i][j] = 0.0f; //zero band filters and envelopes
	kout = 0.0f;
	kval = 0;
	swap = (int)par->getParameter(1).floatPart; 
	gain = (float)pow(10.0f, 2.0f * param[1] - 3.0f * param[5] - 2.0f);

	thru = (float)pow(10.0f, 0.5f + 2.0f * param[1]);
	high = param[3] * param[3] * param[3] * thru;
	thru *= param[2] * param[2] * param[2];

	float a, b, c, d, o = 0.0f, aa, bb, oo = kout, g = gain, ht = thru, hh = high, tmp;
	pp_int32 k = kval, sw = swap, nb = nbnd;

	if (param[4] < 0.05f) //freeze
	{
		for (i = 0; i < nbnd; i++) f[i][12] = 0.0f;
	}
	else
	{
		f[0][12] = (float)pow(10.0, -1.7 - 2.7f * param[4]); //envelope speed

		rr = 0.022f / (float)nbnd; //minimum proportional to frequency to stop distortion
		for (i = 1; i < nbnd; i++)
		{
			f[i][12] = (float)(0.025 - rr * (double)i);
			if (f[0][12] < f[i][12]) f[i][12] = f[0][12];
		}
		f[0][12] = 0.5f * f[0][12]; //only top band is at full rate
	}

	rr = 1.0 - pow(10.0f, -1.0f - 1.2f * param[5]);
	sh = (float)pow(2.0f, 3.0f * param[6] - 1.0f); //filter bank range shift

	for (i = 1; i < nbnd; i++)
	{
		f[i][2] *= sh;
		th = acos((2.0 * rr * cos(tpofs * f[i][2])) / (1.0 + rr * rr));
		f[i][0] = (float)(2.0 * rr * cos(th)); //a0
		f[i][1] = (float)(-rr * rr);           //a1
					//was .98
		f[i][2] *= 0.96f; //shift 2nd stage slightly to stop high resonance peaks
		th = acos((2.0 * rr * cos(tpofs * f[i][2])) / (1.0 + rr * rr));
		f[i][2] = (float)(2.0 * rr * cos(th));
	}

	/* process */
	for (pp_int32 si = 0; si < sLength; si++) {
		pp_int32 j  = si % clipBoard->getWidth();               // repeat carrier
		pp_int16 s  = clipBoard->getSampleWord((pp_int32)j);   // get clipboard sample word
		float fclip = s < 0 ? (s / 32768.0f) : (s / 32767.0f); // convert to float

		a = this->getFloatSampleFromWaveform(si); // carrier/speech
		b = fclip;                                // modulator/synth 
		
		if (sw == 0) { tmp = a; a = b; b = tmp; } //swap channels?

		tmp = a - f[0][7]; //integrate modulator for HF band and filter bank pre-emphasis
		f[0][7] = a;
		a = tmp;

		if (tmp < 0.0f) tmp = -tmp;
		f[0][11] -= f[0][12] * (f[0][11] - tmp);      //high band envelope
		o = f[0][11] * (ht * a + hh * (b - f[0][3])); //high band + high thru

		f[0][3] = b; //integrate carrier for HF band

		if (++k & 0x1) //this block runs at half sample rate
		{
			oo = 0.0f;
			aa = a + f[0][9] - f[0][8] - f[0][8];  //apply zeros here instead of in each reson
			f[0][9] = f[0][8];  f[0][8] = a;
			bb = b + f[0][5] - f[0][4] - f[0][4];
			f[0][5] = f[0][4];  f[0][4] = b;

			for (i = 1; i < nb; i++) //filter bank: 4th-order band pass
			{
				tmp = f[i][0] * f[i][3] + f[i][1] * f[i][4] + bb;
				f[i][4] = f[i][3];
				f[i][3] = tmp;
				tmp += f[i][2] * f[i][5] + f[i][1] * f[i][6];
				f[i][6] = f[i][5];
				f[i][5] = tmp;

				tmp = f[i][0] * f[i][7] + f[i][1] * f[i][8] + aa;
				f[i][8] = f[i][7];
				f[i][7] = tmp;
				tmp += f[i][2] * f[i][9] + f[i][1] * f[i][10];
				f[i][10] = f[i][9];
				f[i][9] = tmp;

				if (tmp < 0.0f) tmp = -tmp;
				f[i][11] -= f[i][12] * (f[i][11] - tmp);
				oo += f[i][5] * f[i][11];
			}
		}
		o += oo * g; //effect of interpolating back up to Fs would be minimal (aliasing >16kHz)

		setFloatSampleInWaveform(si, o * (par->getParameter(5).floatPart / 100.0f) );
	}

	finishUndo();

	postFilter();
}
	
void SampleEditor::tool_addon(const FilterParameters* par)
{
	preFilter(&SampleEditor::tool_addon, par);

	prepareUndo();

	pp_uint8 looptype = getLoopType(); // remember
	int ret = Addon::runMenuItem(par);
	setLoopType(looptype);             // revert back (because new wav-file was imported)

	finishUndo();

	postFilter();
}
