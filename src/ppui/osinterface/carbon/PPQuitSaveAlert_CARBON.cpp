/*
 *  PPQuitSaveAlert.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.03.05.
 *  Copyright 2005 milkytracker.net. All rights reserved.
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
			
			NavReplyRecord reply;
			
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
