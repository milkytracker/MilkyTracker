/*
 *  ppui/osinterface/PPMessageBox.h
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
 *  PPMessageBox.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.09.05.
 *
 */

#ifndef PPMESSAGEBOX__H
#define PPMESSAGEBOX__H

#include "PPModalDialog.h"

class PPMessageBox : public PPModalDialog
{
private:
	PPSystemString caption, content;

public:
	PPMessageBox(PPScreen* screen, const PPSystemString& strCaption, const PPSystemString& strContent) :
		PPModalDialog(screen),
		caption(strCaption), content(strContent)
	{
	}
	
	virtual ReturnCodes runModal();
};

#endif

