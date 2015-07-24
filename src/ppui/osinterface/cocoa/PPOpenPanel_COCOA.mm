/*
 *  ppui/osinterface/cocoa/PPOpenPanel_COCOA.mm
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
#include "PPOpenPanel.h"

PPOpenPanel::PPOpenPanel(PPScreen* screen, const char* caption) :
PPModalDialog(screen)
{
	this->caption = new char[strlen(caption)+1];
	strcpy(this->caption, caption);
}

PPOpenPanel::~PPOpenPanel()
{
	if (caption)
		delete caption;
}

void PPOpenPanel::addExtension(const PPString& ext, const PPString& desc)
{
	Descriptor* d = new Descriptor(ext, desc);

	items.add(d);
}

PPOpenPanel::ReturnCodes PPOpenPanel::runModal()
{
	ReturnCodes returnCode = ReturnCodeCANCEL;
	NSOpenPanel* openDialog = [NSOpenPanel openPanel];
	NSMutableArray* fileExtensions = [[NSMutableArray alloc] init];

	// Add file extensions and descriptions to MutableArray
	for (pp_int32 i = 0; i < items.size(); i++)
	{
		[fileExtensions addObject: [NSString stringWithUTF8String:items.get(i)->extension.getStrBuffer()]];
		[fileExtensions addObject: [NSString stringWithUTF8String:items.get(i)->description.getStrBuffer()]];
	}

	// Set filters and options
	[openDialog setTitle: [NSString stringWithUTF8String:caption]];
	[openDialog setAllowedFileTypes: fileExtensions];
	[openDialog setCanChooseFiles: YES];
	[openDialog setCanChooseDirectories: NO];
	[openDialog setAllowsMultipleSelection: NO];

	// Open the dialog
	if ([openDialog runModal] == NSFileHandlingPanelOKButton)
	{
		fileName = PPSystemString([[[openDialog URL] path] UTF8String]);
		returnCode = ReturnCodeOK;
	}

	return returnCode;
}
