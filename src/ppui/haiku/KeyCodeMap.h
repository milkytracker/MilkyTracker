/*
 *  Copyright 2012 Julian Harnath
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
#ifndef __HAIKU_KEYCODEMAP_H__
#define __HAIKU_KEYCODEMAP_H__

#include "Event.h"

#include <InterfaceDefs.h>
#include <SupportDefs.h>


struct ModifierData {
	int32 modifier;
	int32 modifierLeft;
	int32 modifierRight;
	KeyModifiers keyModifiers;
	uint32 vk;
	uint32 vkLeft;
	uint32 vkRight;
};

extern uint16 gKeyCodeToSC[];

inline uint16
KeyCodeToSC(int32 keyCode)
{
	return gKeyCodeToSC[keyCode];
}

uint16 KeyCodeToVK(int32 keyCode, char character);


extern bool gSwapCommandControl;

extern ModifierData gModifierDataShift;
extern ModifierData gModifierDataControl;
extern ModifierData gModifierDataCommand;

#endif // __HAIKU_KEYCODEMAP_H__
