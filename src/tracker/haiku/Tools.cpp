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

#include "BasicTypes.h"
#include "PPUI.h"

#include <InterfaceDefs.h>
#include <OS.h>


extern bool gSwapCommandControl;


pp_uint32
PPGetTickCount()
{
	return (system_time() / 1000);
}


void
QueryKeyModifiers()
{
	uint32 modifiersState = modifiers();

	if (modifiersState & B_SHIFT_KEY)
		setKeyModifier(KeyModifierSHIFT);
	else
		clearKeyModifier(KeyModifierSHIFT);

	int32 commandModifier =
		gSwapCommandControl ? B_CONTROL_KEY : B_COMMAND_KEY;
	int32 controlModifier =
		gSwapCommandControl ? B_COMMAND_KEY : B_CONTROL_KEY;

	if (modifiersState & commandModifier)
		setKeyModifier(KeyModifierALT);
	else
		clearKeyModifier(KeyModifierALT);

	if (modifiersState & controlModifier)
		setKeyModifier(KeyModifierCTRL);
	else
		clearKeyModifier(KeyModifierCTRL);
}
