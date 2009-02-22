/*
 *  ppui/UndoStack.h
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
 *  UndoStack.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 14.03.06.
 *
 */
#ifndef __UNDOSTACK_H__
#define __UNDOSTACK_H__

#include "BasicTypes.h"

//--- Undo-stack ------------------------------------------------------------
// The undo stack works like this:
//
//	  /--<--< New Entry pushed: Stack is full? => bottom element will be removed
//    |											  and all elements will be moved
//	  |											  one step down and the new element
//	|###|									      is placed on top
//	|###|  |
//	|###|  |
//	|###| \!/
//	|###|
//	  |
//    \-->--> /dev/null :) (bottom element will be deleted)

template<class type>
class PPUndoStack
{
private:
	enum 
	{
		DEFAULTSTACKSIZE = 1024
	};
	
	// Our stacksize
	pp_int32 m_nStackSize;
	
	// our stack
	type** m_pUndoStack;

	// index of current stack entry
	pp_int32	m_nCurIndex;

	// index of last valid stack entry
	pp_int32 m_nTopIndex;

	// stack has overflowed and bottom elements have been removed
	bool m_bOverflow;

public:
	//---------------------------------------------------------------------------
	// Pre     : 
	// Post    : 
	// Globals : 
	// I/O     : 
	// Task    : Construction: Create clean empty stack
	//---------------------------------------------------------------------------
	PPUndoStack(pp_int32 nStackSize = DEFAULTSTACKSIZE) 
	{
		// Remember size
		m_nStackSize = nStackSize;
		
		// create stack containing empty entries
		m_pUndoStack = new type*[nStackSize+1];
		
		for (pp_int32 i = 0; i < nStackSize+1; i++)
			m_pUndoStack[i] = NULL;

		// empty stack
		m_nCurIndex = -1;
		m_nTopIndex = 0;
		
		// hasn't overflowed yet
		m_bOverflow = false;
	}
	
	//---------------------------------------------------------------------------
	// Pre     : 
	// Post    : 
	// Globals : 
	// I/O     : 
	// Task    : Destruction
	//---------------------------------------------------------------------------
	~PPUndoStack()
	{
		for (pp_int32 i = 0; i < m_nStackSize; i++)
			if (m_pUndoStack[i])
				delete m_pUndoStack[i];
		
		delete[] m_pUndoStack;
	}

	//---------------------------------------------------------------------------
	// Pre     : 
	// Post    : 
	// Globals : 
	// I/O     : 
	// Task    : Save entry on stack
	//---------------------------------------------------------------------------
	void Push(const type& stackEntry)
	{
		// does new entry fit onto stack?
		if (m_nCurIndex < m_nStackSize-1) 
		{
			// current index always points to the last entry
			m_nCurIndex++;			
		}
		// nope, kill bottom entry and move content
		else
		{
			// delete first entry
			delete m_pUndoStack[0];
			
			// move references
			for (pp_int32 i = 0; i <= m_nCurIndex; i++)
				m_pUndoStack[i] = m_pUndoStack[i+1];
			
			m_bOverflow = true;
		}
		
		// make sure we don't leak something here
		if (m_pUndoStack[m_nCurIndex])
			delete m_pUndoStack[m_nCurIndex];
		
		// new entry 
		m_pUndoStack[m_nCurIndex] = new type(stackEntry);
		
		m_nTopIndex = m_nCurIndex;
	}

	//---------------------------------------------------------------------------
	// Pre     : 
	// Post    : 
	// Globals : 
	// I/O     : 
	// Task    : Get entry from stack (only by pointer)
	//---------------------------------------------------------------------------
	const type* Pop()
	{
		// anything on stack yet?
		if (m_nCurIndex>=0)
		{
			// get entry
			return m_pUndoStack[m_nCurIndex--];

			/*type* pStackEntry = m_pUndoStack[m_nCurIndex];
			
			// decrease stackpointer
			m_nCurIndex--;
			return pStackEntry;*/
		}
		
		else 
		{
			return NULL;
		}
	}

	//---------------------------------------------------------------------------
	// Pre     : 
	// Post    : 
	// Globals : 
	// I/O     : 
	// Task    : Get superimposed entry if possible (Redo)
	//---------------------------------------------------------------------------
	const type* Advance()
	{
		// Nothing on stack
		if (m_nTopIndex == 0)
		{
			return NULL;
		}
		
		// Redo possible?
		if (m_nCurIndex<m_nTopIndex-1)
		{
			// advance
			m_nCurIndex++;
		}
		
		// get superimposed entry
		return m_pUndoStack[m_nCurIndex+1];
	}

	bool IsEmpty() const { return (m_nCurIndex == -1); }

	bool IsTop() const { return ((m_nTopIndex-1)==m_nCurIndex); }

	bool IsOverflowed() const { return m_bOverflow; }
};

#endif
