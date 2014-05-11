/*
 *  ppui/osinterface/cocoa/PPMessageBox_COCOA.mm
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

/*
 *  PPMessageBox_COCOA.mm
 *  MilkyTracker
 *
 *  Created by Dale Whinham on 11/05/2014.
 *
 */

#include "PPMessageBox.h"
#include <Cocoa/Cocoa.h>

PPMessageBox::ReturnCodes PPMessageBox::runModal()
{
	@autoreleasepool {
		NSAlert *messageBox = [[NSAlert alloc] init];

		// Set message strings and style
		[messageBox setMessageText:[NSString stringWithUTF8String:caption]];
		[messageBox setInformativeText:[NSString stringWithUTF8String:content]];
		[messageBox setAlertStyle:NSCriticalAlertStyle];

		// Open the dialog
		[messageBox runModal];

		return ReturnCodeOK;
	}
}
