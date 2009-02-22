/*
 *  tracker/Undo.h
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

#ifndef __UNDO_H__
#define __UNDO_H__

#include "BasicTypes.h"
#include "UndoStack.h"
#include "XModule.h"

#define UNDODEPTH_ENVELOPEEDITOR		32
#define UNDOHISTORYSIZE_ENVELOPEEDITOR	8

#define UNDODEPTH_PATTERNEDITOR			32
#define UNDOHISTORYSIZE_PATTERNEDITOR	8

#define UNDODEPTH_SAMPLEEDITOR			16
#define UNDOHISTORYSIZE_SAMPLEEDITOR	4

//--- This is what we save --------------------------------------------------
class UndoStackEntry
{
public:
	class UserData
	{
	private:
		pp_uint8* data;
		pp_uint32 dataLen;

	public:
		UserData() :
			data(NULL),
			dataLen(0)
		{
		}

		UserData(const pp_uint8* data, pp_uint32 dataLen) :
			data(NULL),
			dataLen(dataLen)
		{
			this->data = new pp_uint8[dataLen];
			memcpy(this->data, data, dataLen);
		}

		UserData(const UserData& src) :
			data(NULL)
		{
			this->data = new pp_uint8[src.dataLen];
			memcpy(this->data, src.data, src.dataLen);
			this->dataLen = src.dataLen;
		}
		
		~UserData()
		{
			delete[] this->data;
		}
		
		UserData& operator=(const UserData& src)
		{
			if (this != &src)
			{
				delete[] this->data;
				this->data = new pp_uint8[src.dataLen];
				memcpy(this->data, src.data, src.dataLen);
				this->dataLen = src.dataLen;
			}
			
			return *this;
		}
		
		const pp_uint8* getData() const { return data; }
		pp_uint32 getDataLen() const { return dataLen; }
		
		void clear() 
		{
			delete[] this->data;
			this->data = NULL;
			dataLen = 0;
		}
	};

private:
	UserData userData;
	
protected:
	UndoStackEntry(const UserData* userData)
	{
		if (userData != NULL)
			this->userData = *userData;
	}
	
	virtual ~UndoStackEntry() 
	{ 
	}
	
	void copyBasePart(const UndoStackEntry& src)
	{
		this->userData = src.userData;
	}
	
public:
	const UserData& getUserData() const { return userData; }
	
};

// Undo information from pattern editor
class PatternUndoStackEntry : public UndoStackEntry
{
public:
	// Construction (new element)
	PatternUndoStackEntry(const TXMPattern& pattern, 
						  const pp_int32 cursorPositionChannel, 
						  const pp_int32 cursorPositionRow, 
						  const pp_int32 cursorPositionInner,
						  const UserData* userData = NULL);
	// Copy ctor
	PatternUndoStackEntry(const PatternUndoStackEntry& source);

	// dtor
	virtual ~PatternUndoStackEntry();

	// get your pattern
	const TXMPattern& GetPattern() const { return pattern; }

	pp_int32 getCursorPositionChannel() const { return cursorPositionChannel; }
	pp_int32 getCursorPositionRow() const { return cursorPositionRow; }
	pp_int32 getCursorPositionInner() const { return cursorPositionInner; }

	// assignment operator
	PatternUndoStackEntry& operator=(const PatternUndoStackEntry& source);
	
	// comparison is necessary too
	bool operator==(const PatternUndoStackEntry& source);
	bool operator!=(const PatternUndoStackEntry& source);


private:	
	TXMPattern pattern;
	
	pp_int32 cursorPositionChannel;
	pp_int32 cursorPositionRow;
	pp_int32 cursorPositionInner;
};

// Less memory consumption than TEnvelope because XMs can only handle 12 envelope points
struct TSmallEnvelope 
{
	mp_uword	env[12][2];
	mp_ubyte	num,sustain,loops,loope,type;
};

// Undo information from Envelope Editor
class EnvelopeUndoStackEntry : public UndoStackEntry
{
public:
	EnvelopeUndoStackEntry() :
		UndoStackEntry(NULL),
		invalid(false)
	{
	}

	// Construction
	EnvelopeUndoStackEntry(const TEnvelope& env, const UserData* userData = NULL) :
		UndoStackEntry(userData),
		invalid(false)
	{
		ASSERT(&env != NULL);
		ASSERT(env.num <= 12);
		envelope.num = env.num;
		envelope.sustain = env.sustain;
		envelope.loops = env.loops;
		envelope.loope = env.loope;
		envelope.type = env.type;
		for (pp_int32 i = 0; i < 12; i++)
		{
			envelope.env[i][0] = env.env[i][0];
			envelope.env[i][1] = env.env[i][1];
		}
	}

	EnvelopeUndoStackEntry(const EnvelopeUndoStackEntry& src) :
		UndoStackEntry(&src.getUserData())		
	{
		envelope = src.envelope;
		invalid = src.invalid;
	}

	// assignment operator
	EnvelopeUndoStackEntry& operator=(const EnvelopeUndoStackEntry& source);
	
	// comparison is necessary too
	bool operator==(const EnvelopeUndoStackEntry& source);
	bool operator!=(const EnvelopeUndoStackEntry& source);

	// get your envelope
	void GetEnvelope(TEnvelope& env) const;

	void SetInvalid(bool b) { invalid = b; }

private:
	TSmallEnvelope envelope;
	bool invalid;
};

struct TXMSample;

// Undo information from Sample Editor
class SampleUndoStackEntry : public UndoStackEntry
{
public:
	SampleUndoStackEntry() : 
		UndoStackEntry(NULL)
	{
	}

	SampleUndoStackEntry(const TXMSample& sample, 
						 pp_int32 selectionStart, 
						 pp_int32 selectionEnd, 
						 const UserData* userData = NULL);
						 
	SampleUndoStackEntry(const SampleUndoStackEntry& src);
						 
	virtual ~SampleUndoStackEntry();

	// assignment operator
	SampleUndoStackEntry& operator=(const SampleUndoStackEntry& source);
	
	// comparison is necessary too
	bool operator==(const SampleUndoStackEntry& source);
	bool operator!=(const SampleUndoStackEntry& source);

	pp_uint32 getSampLen() const { return samplen; }
	pp_uint32 getLoopStart() const { return loopstart; }
	pp_uint32 getLoopLen() const { return looplen; }
	pp_uint32 getFlags() const { return flags; }
	mp_sbyte getRelNote() const { return relnote; }
	mp_sbyte getFineTune() const { return finetune; }
	
	const pp_uint8* getBuffer() const { return buffer; }
	
	pp_int32 getSelectionStart() const { return selectionStart; }
	pp_int32 getSelectionEnd() const { return selectionEnd; }
	
private:
	// from sample
	pp_uint32 samplen, loopstart, looplen;
	mp_sbyte relnote, finetune;
	pp_uint8* buffer;
	pp_uint8 flags;

	// from sample editor
	pp_int32 selectionStart;
	pp_int32 selectionEnd;

	pp_int32 checkSum;
	
	void calcCheckSum();
};

// undo history maintainance
template<class Key, class Type>
struct HistoryEntry
{
	Key* key;
	PPUndoStack<Type>* undoStack;	
};

template<class Key, class Type>
class UndoHistory
{
private:
	// undo/redo information
	PPUndoStack<Type>* currentUndoStack;	

	HistoryEntry<Key, Type>* patternHistory;
	pp_int32 patternHistoryNumEntries;

	pp_int32 size;

public:
	UndoHistory(pp_int32 defaultSize = 8) :
		currentUndoStack(NULL),
		patternHistoryNumEntries(0),
		size(defaultSize)
	{
		patternHistory = new HistoryEntry<Key, Type>[size];
		for (pp_int32 i = 0; i < size; i++)
		{
			patternHistory[i].key = NULL;
			patternHistory[i].undoStack = NULL;
		}
	}
	
	~UndoHistory()
	{
		for (pp_int32 i = 0; i < patternHistoryNumEntries; i++)
			if (patternHistory[i].undoStack && patternHistory[i].undoStack != currentUndoStack)
				delete patternHistory[i].undoStack;
	
		delete[] patternHistory;
	}
	
	PPUndoStack<Type>* getUndoStack(Key* newKey, Key* oldKey, PPUndoStack<Type>* oldUndoStack)
	{
		if (oldUndoStack)
		{
			if (patternHistoryNumEntries < size)
			{
				ASSERT(patternHistory[patternHistoryNumEntries].key == NULL);
				ASSERT(patternHistory[patternHistoryNumEntries].undoStack == NULL);
				patternHistory[patternHistoryNumEntries].key = oldKey;
				patternHistory[patternHistoryNumEntries].undoStack = oldUndoStack;
				patternHistoryNumEntries++;
			}
			else
			{
				ASSERT(patternHistoryNumEntries == size);
				
				delete patternHistory[0].undoStack;
				for (pp_int32 i = 0; i < patternHistoryNumEntries-1; i++)
					patternHistory[i] = patternHistory[i+1];
				
				patternHistory[patternHistoryNumEntries-1].key = oldKey;
				patternHistory[patternHistoryNumEntries-1].undoStack = oldUndoStack;
			}
		}
		
		bool found = false;
		for (pp_int32 i = 0; i < patternHistoryNumEntries; i++)
		{
			if (patternHistory[i].key == newKey)
			{
				oldUndoStack = patternHistory[i].undoStack;
				
				for (pp_int32 j = i; j < patternHistoryNumEntries-1; j++)
					patternHistory[j] = patternHistory[j+1];
				
				patternHistory[patternHistoryNumEntries-1].key = NULL;
				patternHistory[patternHistoryNumEntries-1].undoStack = NULL;
				
				patternHistoryNumEntries--;
				found = true;
				break;
			}
		}
		
		if (found)
		{
			currentUndoStack = oldUndoStack;
			return oldUndoStack;
		}
			
		return NULL;
	}
};

#endif
