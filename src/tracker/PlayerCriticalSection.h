/*
 *  tracker/PlayerCriticalSection.h
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
 *  PlayerCriticalSection.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 12.12.07.
 *
 */

#ifndef __PLAYERCRITICALSECTION_H__
#define __PLAYERCRITICALSECTION_H__

#include "PlayerController.h"

class PlayerCriticalSection
{
private:
	PlayerController& playerController;
	bool enabled;

public:
	PlayerCriticalSection(PlayerController& playerController) :
		playerController(playerController),
		enabled(false)
	{
	}
	
	void enter(bool stopPlaying = true)
	{
		if (enabled)
			return;
		playerController.suspendPlayer(false, stopPlaying);
		enabled = true;
	}
	
	void leave(bool continuePlaying = true)
	{
		if (!enabled)
			return;
		playerController.resumePlayer(continuePlaying);
		enabled = false;
	}
};

#endif
