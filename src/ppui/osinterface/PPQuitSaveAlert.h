/*
 *  PPQuitSaveAlert.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef PPQUITSAVEALERT__H
#define PPQUITSAVEALERT__H

#include "PPModalDialog.h"

class PPQuitSaveAlert : public PPModalDialog
{
public:
	PPQuitSaveAlert(PPScreen* screen) :
		PPModalDialog(screen)
	{
	}

	virtual ReturnCodes runModal();
};

#endif
