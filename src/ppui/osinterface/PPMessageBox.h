/*
 *  PPMessageBox.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 27.09.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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

