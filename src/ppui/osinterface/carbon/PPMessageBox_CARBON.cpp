/*
 *  ppui/osinterface/carbon/PPMessageBox_CARBON.cpp
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
 *  PPMessageBox_CARBON.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.09.05.
 *
 */

#include <Carbon/Carbon.h>
#include "PPMessageBox.h"

PPMessageBox::ReturnCodes PPMessageBox::runModal()
{
	ReturnCodes res = ReturnCodeOK;
	
	OSStatus err = noErr;

	SInt16 out;

	Str255 caption, content;

	CopyCStringToPascal(this->caption, caption);
	CopyCStringToPascal(this->content, content);

	err = StandardAlert(kAlertNoteAlert,
						caption,
						content,
						NULL,
						&out);	
	return res;
}
