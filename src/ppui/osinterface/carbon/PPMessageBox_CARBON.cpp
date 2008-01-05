/*
 *  PPMessageBox_CARBON.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.09.05.
 *  Copyright (c) 2005 milkytracker.net. All rights reserved.
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
