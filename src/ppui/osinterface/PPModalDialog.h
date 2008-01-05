/*
 *  PPModalDialog.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef PPMODALDIALOG__H
#define PPMODALDIALOG__H

#include "BasicTypes.h"

class PPScreen;

class PPModalDialog
{
public:
	enum ReturnCodes
	{
		ReturnCodeOK,
		ReturnCodeCANCEL,
		ReturnCodeNO
	};
	
protected:
	PPScreen* screen;

public:
	PPModalDialog(PPScreen* theScreen) :
		screen(theScreen)
	{ 
	}

	virtual ~PPModalDialog() 
	{ 
	}
	
	virtual ReturnCodes runModal() = 0;
};

#endif
