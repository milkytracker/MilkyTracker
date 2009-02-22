/*
 *  tracker/Undo.cpp
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

// Undo.cpp - Undo stack implementation

#include "Undo.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														patterns
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// Pre     : 
// Post    : 
// Globals : 
// I/O     : 
// Task    : Create new stack entry
//---------------------------------------------------------------------------
PatternUndoStackEntry::PatternUndoStackEntry(const TXMPattern& pattern, 
											 const pp_int32 cursorPositionChannel, 
											 const pp_int32 cursorPositionRow, 
											 const pp_int32 cursorPositionInner,
											 const UserData* userData/* = NULL*/) :
	UndoStackEntry(userData)
{
	this->cursorPositionChannel = cursorPositionChannel;
	this->cursorPositionRow = cursorPositionRow;
	this->cursorPositionInner = cursorPositionInner;
	
	if (pattern.patternData)
	{
		this->pattern.rows = pattern.rows;
		this->pattern.channum = pattern.channum;
		this->pattern.effnum = pattern.effnum;
	
		this->pattern.len = pattern.compress(NULL);
		
		this->pattern.patternData = new mp_ubyte[this->pattern.len];

		mp_sint32 len = pattern.compress(this->pattern.patternData);
		
		ASSERT(len == (signed)this->pattern.len);
	}
	else
		memset(&this->pattern, 0, sizeof(pattern));
}

//---------------------------------------------------------------------------
// Pre     : 
// Post    : 
// Globals : 
// I/O     : 
// Task    : Copy constructor
//---------------------------------------------------------------------------
PatternUndoStackEntry::PatternUndoStackEntry(const PatternUndoStackEntry& source) :
	UndoStackEntry(&source.getUserData())
{
	cursorPositionChannel = source.cursorPositionChannel;
	cursorPositionRow = source.cursorPositionRow;
	cursorPositionInner = source.cursorPositionInner;

	pattern.len = source.pattern.len;
	pattern.rows = source.pattern.rows;
	pattern.channum = source.pattern.channum;
	pattern.effnum = source.pattern.effnum;
	
	pattern.patternData = new mp_ubyte[pattern.len];
	memcpy(pattern.patternData, source.pattern.patternData, pattern.len);
}

//---------------------------------------------------------------------------
// Pre     : 
// Post    : 
// Globals : 
// I/O     : 
// Task    : Clean up
//---------------------------------------------------------------------------
PatternUndoStackEntry::~PatternUndoStackEntry()
{
	delete[] pattern.patternData;
}

//---------------------------------------------------------------------------
// Pre     : 
// Post    : 
// Globals : 
// I/O     : 
// Task    : assignment operator
//---------------------------------------------------------------------------
PatternUndoStackEntry& PatternUndoStackEntry::operator=(const PatternUndoStackEntry& source)
{
	// no self-assignment
	if (this != &source)
	{
		copyBasePart(source);
	
		cursorPositionChannel = source.cursorPositionChannel;
		cursorPositionRow = source.cursorPositionRow;
		cursorPositionInner = source.cursorPositionInner;	
		
		if (pattern.patternData)
			delete[] pattern.patternData;
	
		pattern.len = source.pattern.len;
		pattern.rows = source.pattern.rows;
		pattern.channum = source.pattern.channum;
		pattern.effnum = source.pattern.effnum;
		
		pattern.patternData = new mp_ubyte[pattern.len];
		memcpy(pattern.patternData, source.pattern.patternData, pattern.len);
	}

	return *this;
}

//---------------------------------------------------------------------------
// Pre     : 
// Post    : 
// Globals : 
// I/O     : 
// Task    : comparison operators
//---------------------------------------------------------------------------
bool PatternUndoStackEntry::operator==(const PatternUndoStackEntry& source)
{
	ASSERT(source.pattern.patternData);
	ASSERT(pattern.patternData);

	if (pattern.len == source.pattern.len &&
		pattern.rows == source.pattern.rows &&
		pattern.channum == source.pattern.channum &&
		pattern.effnum == source.pattern.effnum)
	{
		if (memcmp(pattern.patternData, source.pattern.patternData, pattern.len) == 0)
			return true;
		else
			return false;
	}
	else
		return false;
}

bool PatternUndoStackEntry::operator!=(const PatternUndoStackEntry& source)
{
	return !(*this==source);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														envelopes
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Envelope stack entry
EnvelopeUndoStackEntry& EnvelopeUndoStackEntry::operator=(const EnvelopeUndoStackEntry& source)
{
	// no self-assignment
	if (this != &source)
	{
		copyBasePart(source);
	
		envelope = source.envelope;
		invalid = source.invalid;
	}

	return *this;
}
	
// comparison is necessary too
bool EnvelopeUndoStackEntry::operator==(const EnvelopeUndoStackEntry& source)
{
	if (source.invalid || invalid)
		return true;

	if (source.envelope.num != envelope.num ||
		source.envelope.sustain != envelope.sustain ||
		source.envelope.loops != envelope.loops ||
		source.envelope.loope != envelope.loope ||
		source.envelope.type != envelope.type)
		return false;
	
	for (pp_int32 i = 0; i < envelope.num; i++)
	{
		if (source.envelope.env[i][0] != envelope.env[i][0] ||
			source.envelope.env[i][1] != envelope.env[i][1])
			return false;
	}
	
	return true;
}

bool EnvelopeUndoStackEntry::operator!=(const EnvelopeUndoStackEntry& source)
{
	return !(*this==source);
}

void EnvelopeUndoStackEntry::GetEnvelope(TEnvelope& env) const
{
	env.num = envelope.num;
	env.sustain = envelope.sustain;
	env.loops = envelope.loops;
	env.loope = envelope.loope;
	env.type = envelope.type;
	env.speed = 0;

	for (pp_int32 i = 0; i < 12; i++)
	{
		env.env[i][0] = envelope.env[i][0];
		env.env[i][1] = envelope.env[i][1];
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														samples
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SampleUndoStackEntry::calcCheckSum()
{
	checkSum = 0;
	if (!buffer)
		return;
	mp_uint32 size = (flags & 16) ? samplen*2 : samplen;
	if (size)
	{
		mp_ubyte* mem = TXMSample::getPadStartAddr(buffer);
		mp_uint32 realSize = TXMSample::getPaddedSize(size);
		for (pp_uint32 i = 0; i < realSize; i++)
			checkSum+=(pp_uint32)mem[i];		
	}
}

SampleUndoStackEntry::SampleUndoStackEntry(const TXMSample& sample, 
										   pp_int32 selectionStart, pp_int32 selectionEnd, 
										   const UserData* userData/* = NULL*/) :
	UndoStackEntry(userData)
{
	samplen = sample.samplen;
	loopstart = sample.loopstart;
	looplen = sample.looplen;
	relnote = sample.relnote;
	finetune = sample.finetune;
	flags = sample.type;
	this->selectionStart = selectionStart;
	this->selectionEnd = selectionEnd;
	
	buffer = NULL;

	checkSum = 0;
	
	if (sample.samplen && sample.sample)
	{
		// 16 bit sample
		if (flags & 16)
		{
			buffer = TXMSample::allocPaddedMem(samplen*2);
			if (buffer)
				TXMSample::copyPaddedMem(buffer, sample.sample, samplen*2);				
		}
		else
		{
			buffer = TXMSample::allocPaddedMem(samplen);
			if (buffer)
				TXMSample::copyPaddedMem(buffer, sample.sample, samplen);
		}
		calcCheckSum();
	}
}

SampleUndoStackEntry::SampleUndoStackEntry(const SampleUndoStackEntry& src)	:
	UndoStackEntry(&src.getUserData())
{
	samplen = src.samplen;
	loopstart = src.loopstart;
	looplen = src.looplen;	
	relnote = src.relnote;
	finetune = src.finetune;
	flags = src.flags;
	checkSum = src.checkSum;
	this->selectionStart = src.selectionStart;
	this->selectionEnd = src.selectionEnd;
	
	buffer = NULL;
	if (src.buffer && samplen)
	{	
		// 16 bit sample
		if (flags & 16)
		{			
			buffer = TXMSample::allocPaddedMem(samplen*2);
			if (buffer)
				TXMSample::copyPaddedMem(buffer, src.buffer, samplen*2);
		}
		else
		{
			buffer = TXMSample::allocPaddedMem(samplen);
			if (buffer)
				TXMSample::copyPaddedMem(buffer, src.buffer, samplen);
		}	
	}
}

SampleUndoStackEntry::~SampleUndoStackEntry()
{
	TXMSample::freePaddedMem(buffer);
}

// assignment operator
SampleUndoStackEntry& SampleUndoStackEntry::operator=(const SampleUndoStackEntry& src)
{
	if (this != &src)
	{
		copyBasePart(src);
	
		samplen = src.samplen;
		loopstart = src.loopstart;
		looplen = src.looplen;	
		relnote = src.relnote;
		finetune = src.finetune;
		flags = src.flags;
		checkSum = src.checkSum;
		
		if (buffer)
		{
			TXMSample::freePaddedMem(buffer);
			buffer = NULL;
		}
		
		if (src.buffer && samplen)
		{	
			// 16 bit sample
			if (flags & 16)
			{
				buffer = TXMSample::allocPaddedMem(samplen*2);
				if (buffer)
					TXMSample::copyPaddedMem(buffer, src.buffer, samplen*2);
			}
			else
			{
				buffer = TXMSample::allocPaddedMem(samplen);
				if (buffer)
					TXMSample::copyPaddedMem(buffer, src.buffer, samplen);
			}	
		}
	}

	return (*this);
}
	
// comparison is necessary too
bool SampleUndoStackEntry::operator==(const SampleUndoStackEntry& src)
{
	if (samplen != src.samplen)
		return false;
		
	if (checkSum != src.checkSum)
		return false;

	if (loopstart != src.loopstart)
		return false;
		
	if (looplen != src.looplen)
		return false;

	if (relnote != src.relnote)
		return false;

	if (finetune != src.finetune)
		return false;
		
	if (flags != src.flags)
		return false;
	
	if (buffer == NULL && src.buffer != NULL)
		return false;

	if (buffer != NULL && src.buffer == NULL)
		return false;
	
	mp_uint32 size = (flags & 16) ? samplen*2 : samplen;
	if (size)
	{
		mp_ubyte* _src = src.buffer;
		mp_ubyte* _dst = buffer;

		mp_ubyte* srcmem = TXMSample::getPadStartAddr(_src);
		mp_ubyte* dstmem = TXMSample::getPadStartAddr(_dst);
		mp_uint32 realSize = TXMSample::getPaddedSize(size);
		for (pp_uint32 i = 0; i < realSize; i++)
			if (srcmem[i] != dstmem[i])
				return false;
	}

	return true;
}

bool SampleUndoStackEntry::operator!=(const SampleUndoStackEntry& source)
{
	return !(*this==source);
}
