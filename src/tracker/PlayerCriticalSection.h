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

public:
	PlayerCriticalSection(PlayerController& playerController) :
		playerController(playerController)
	{
	}
	
	void enter()
	{
		playerController.suspendPlayer(false);
	}
	
	void leave()
	{
		playerController.resumePlayer(true);
	}
};

#endif
