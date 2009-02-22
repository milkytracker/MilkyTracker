/*
 *  ppui/osinterface/carbon/PPQuitSaveAlert_CARBON.cpp
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
 *  PPQuitSaveAlert.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
 *
 */

#include <Carbon/Carbon.h>
#include "PPQuitSaveAlert.h"

PPQuitSaveAlert::ReturnCodes PPQuitSaveAlert::runModal()
{
	ReturnCodes res = ReturnCodeCANCEL;
	
	OSStatus err = noErr;
	NavDialogRef theOpenDialog;
	NavDialogCreationOptions dialogOptions;
	
	if ((err = NavGetDefaultDialogCreationOptions(&dialogOptions)) == noErr)
	{
		
		dialogOptions.modality = kWindowModalityAppModal;
		dialogOptions.windowTitle = CFStringCreateWithCString(NULL, "You suck", kCFStringEncodingASCII);
		
		err = NavCreateAskSaveChangesDialog (&dialogOptions,
											 kNavSaveChangesClosingDocument,
											 NULL,
											 NULL,
											 &theOpenDialog);	
		if (theOpenDialog)
		{
			err = NavDialogRun(theOpenDialog);
			
			NavUserAction action = NavDialogGetUserAction (theOpenDialog);
			
			switch (action)
			{
				case kNavUserActionSaveChanges:
					res = ReturnCodeOK;
					break;
					
				case kNavUserActionDontSaveChanges:
					res = ReturnCodeNO;
					break;

				case kNavUserActionCancel:
					res = ReturnCodeCANCEL;
					break;
			}
		
			NavDialogDispose(theOpenDialog);			
		}
	}	
	
	return res;
}
