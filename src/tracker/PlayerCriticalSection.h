/*
 *  PlayerCriticalSection.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 12.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
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
