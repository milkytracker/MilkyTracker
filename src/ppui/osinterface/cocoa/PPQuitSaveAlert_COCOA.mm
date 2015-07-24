/*
 *  ppui/osinterface/cocoa/PPQuitSaveAlert_COCOA.mm
 *
 *  Copyright 2014 Dale Whinham
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

#include <Cocoa/Cocoa.h>
#include "PPQuitSaveAlert.h"

PPQuitSaveAlert::ReturnCodes PPQuitSaveAlert::runModal()
{
	ReturnCodes returnCode = ReturnCodeCANCEL;
	NSAlert *messageBox = [[NSAlert alloc] init];

	// Add buttons and message strings
	[messageBox addButtonWithTitle:@"Save"];
	[messageBox addButtonWithTitle:@"Cancel"];
	[messageBox addButtonWithTitle:@"Don't Save"];
	[messageBox setMessageText: @"Do you want to save the changes you made to this document?"];
	[messageBox setInformativeText: @"Your changes will be lost if you don't save them."];
	[messageBox setAlertStyle:NSWarningAlertStyle];

	// Set return code
	switch ([messageBox runModal])
	{
		case NSAlertFirstButtonReturn:
			returnCode = ReturnCodeOK;
			break;
		case NSAlertThirdButtonReturn:
			returnCode = ReturnCodeNO;
	};

	return returnCode;
}
