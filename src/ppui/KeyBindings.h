/*
 *  ppui/KeyBindings.h
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
 *  KeyBindings.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 12 2005.
 *
 */

#ifndef KEYBINDINGS__H
#define KEYBINDINGS__H

#include "BasicTypes.h"


template<class Type>
class PPKeyBindings
{
private:
	struct TKeyBinding
	{
		pp_uint16 keyCode;
		pp_uint32 keyModifier;
		Type handlerFunc;
	};

	TKeyBinding bindings[256];
	
	pp_uint32 currentBindingIndex;
	
public:
	PPKeyBindings() :
		currentBindingIndex(0)
	{
		memset(bindings, 0, sizeof(bindings));
	}
	
	void addBinding(pp_uint16 keyCode, pp_uint32 keyModifier, Type handlerFunc)
	{
		if (currentBindingIndex < sizeof(bindings)/sizeof(TKeyBinding))
		{
			bindings[currentBindingIndex].keyCode = keyCode;
			bindings[currentBindingIndex].keyModifier = keyModifier;
			bindings[currentBindingIndex].handlerFunc = handlerFunc;	
			currentBindingIndex++;
		}
	}
	
	bool getBinding(pp_uint16 keyCode, pp_uint32 keyModifier, Type& handlerFunc) const
	{
		pp_int32 i = 0;
		while (bindings[i].handlerFunc && i < (signed)currentBindingIndex)
		{
			if (bindings[i].keyCode == keyCode && 
				(bindings[i].keyModifier == 0xFFFF || bindings[i].keyModifier == keyModifier))
			{
				handlerFunc = bindings[i].handlerFunc;
				return true;
			}
			i++;
		}
		return false;
	}
};

#endif
