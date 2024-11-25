/*
 *  tracker/RecPosProvider.cpp
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
 
#include "RecPosProvider.h"
#include "PlayerController.h"
#include "ModuleEditor.h"

void RecPosProvider::getPosition(pp_int32& order, pp_int32& row)
{
	playerController.getPosition((mp_sint32&)order, (mp_sint32&)row);
	if (playerController.isPlayingPattern())
		order = -1;
}

bool RecPosProvider::getPosition(pp_int32& order, pp_int32& row, pp_int32& ticker)
{
	playerController.getPosition((mp_sint32&)order, (mp_sint32&)row, (mp_sint32&)ticker);
	if (playerController.isPlayingPattern())
		order = -1;
	
	bool rounded = false;
	if (roundToClosestRow)
	{
		mp_sint32 speed, bpm;
		playerController.getSpeed(bpm, speed);
		// snap position to the closest row
		if (ticker >= speed / 2)
		{
			rounded = true;
			ticker = 0;

			incrementRow(order, row);		
		}
	}

	return rounded;
}

void RecPosProvider::incrementRow(pp_int32& order, pp_int32& row)
{
    ModuleEditor* moduleEditor = playerController.getModuleEditor();
    row++;
    if (order != -1)
    {
        pp_int32 patIndex = moduleEditor->getOrderPosition(order);
        if (row >= moduleEditor->getPattern(patIndex)->rows)
        {
            row = 0;
            order++;
            if (order >= moduleEditor->getNumOrders())
                order = 0;
        }
    }
    else
    {
        pp_int32 patIndex = moduleEditor->getCurrentPatternIndex();
        if (row >= moduleEditor->getPattern(patIndex)->rows)
            row = 0;
    }
}
