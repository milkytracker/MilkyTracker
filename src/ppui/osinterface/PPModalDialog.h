/*
 *  ppui/osinterface/PPModalDialog.h
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
 *  PPModalDialog.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
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
