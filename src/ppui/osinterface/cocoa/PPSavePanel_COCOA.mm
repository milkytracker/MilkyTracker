/*
 *  ppui/osinterface/cocoa/PPSavePanel_COCOA.mm
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
#include "PPSavePanel.h"

PPSavePanel::ReturnCodes PPSavePanel::runModal()
{
	ReturnCodes returnCode = ReturnCodeCANCEL;
	NSSavePanel* saveDialog = [NSSavePanel savePanel];
	NSMutableArray* fileExtensions = [[NSMutableArray alloc] init];

	// Add file extensions and descriptions to MutableArray
	for (pp_int32 i = 0; i < items.size(); i++)
	{
		[fileExtensions addObject: [NSString stringWithUTF8String:items.get(i)->extension.getStrBuffer()]];
		[fileExtensions addObject: [NSString stringWithUTF8String:items.get(i)->description.getStrBuffer()]];
	}

	// Set filters and options
	[saveDialog setTitle: [NSString stringWithUTF8String:caption]];
	[saveDialog setAllowedFileTypes: fileExtensions];

	// Open the dialog
	if ([saveDialog runModal] == NSFileHandlingPanelOKButton)
	{
		fileName = PPSystemString([[[saveDialog URL] path] UTF8String]);
		returnCode = ReturnCodeOK;
	}

	return returnCode;
}
