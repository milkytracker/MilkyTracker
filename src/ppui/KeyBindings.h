/*
 *  KeyBindings.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 12 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
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
			if (bindings[i].keyCode == keyCode && (bindings[i].keyModifier == 0xFFFF || bindings[i].keyModifier == keyModifier))
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
