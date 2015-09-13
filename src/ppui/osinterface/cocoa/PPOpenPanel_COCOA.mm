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

// Padding to surround accessory view controls
static const float kAccessoryViewPadding = 5.0f;

// Objective-C delegate class with selectors for switching file type filter
@interface MTFileTypeFilter : NSObject

{
	NSOpenPanel* _openPanel;
	NSDictionary* _fileTypes;
}

-(id) initWithOpenPanel:(NSOpenPanel*)openPanel andFileTypes:(NSDictionary*) fileTypes;
-(void) filterAllSupportedFileTypes:(id)sender;
-(void) filterSingleFileType:(id)sender;
-(void) removeFilter:(id)sender;

@end

@implementation MTFileTypeFilter

-(id) initWithOpenPanel:(NSOpenPanel*)openPanel andFileTypes:(NSDictionary*) fileTypes
{
	if (self = [super init])
	{
		_openPanel = openPanel;
		_fileTypes = fileTypes;
	}
	return self;
}

-(void) filterAllSupportedFileTypes:(id)sender
{
	[_openPanel setAllowedFileTypes:[_fileTypes allValues]];
}

-(void) filterSingleFileType:(id)sender
{
	NSPopUpButton* popUpButton = (NSPopUpButton*) sender;
	NSString* extension = [_fileTypes valueForKey:[[popUpButton selectedItem] title]];
	[_openPanel setAllowedFileTypes:@[extension]];
}

-(void) removeFilter:(id)sender
{
	[_openPanel setAllowedFileTypes:nil];
}

@end

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
	NSMutableDictionary* fileTypes = [[NSMutableDictionary alloc] init];
	NSOpenPanel* openPanel = [NSOpenPanel openPanel];
	MTFileTypeFilter* fileTypeFilter = [[MTFileTypeFilter alloc] initWithOpenPanel:openPanel andFileTypes:fileTypes];

	// Add file extensions and descriptions to MutableArrays
	for (pp_int32 i = 0; i < items.size(); i++)
	{
		NSString* ext = [NSString stringWithUTF8String:items.get(i)->extension.getStrBuffer()];
		NSString* desc = [NSString stringWithUTF8String:items.get(i)->description.getStrBuffer()];
		[fileTypes setObject:ext forKey:[NSString stringWithFormat:@"%@ (.%@)", desc, ext]];
	}

	// Custom accessory view holds the file type selector popup button
	NSView* accessoryView = [[NSView alloc] initWithFrame:NSZeroRect];
	NSTextField* label = [[NSTextField alloc] initWithFrame:NSZeroRect];
	NSPopUpButton* popUpButton = [[NSPopUpButton alloc] initWithFrame:NSZeroRect pullsDown:NO];

	// Set label properties
	[label setFont:[NSFont systemFontOfSize:0.0f]];
	[label setStringValue:@"File types:"];
	[label setEditable:NO];
	[label setBordered:NO];
	[label setBezeled:NO];
	[label setDrawsBackground:NO];
	[label sizeToFit];

	// Setup popup menu items and actions for all supported types/all files
	NSMenuItem* allSupportedFilesMenuItem = [[NSMenuItem alloc] initWithTitle:@"All supported types" action:@selector(filterAllSupportedFileTypes:) keyEquivalent:@""];
	NSMenuItem* allFilesMenuItem = [[NSMenuItem alloc] initWithTitle:@"All files" action:@selector(removeFilter:) keyEquivalent:@""];
	[allSupportedFilesMenuItem setTarget:fileTypeFilter];
	[allFilesMenuItem setTarget:fileTypeFilter];

	[[popUpButton menu] addItem:allSupportedFilesMenuItem];
	[[popUpButton menu] addItem:allFilesMenuItem];
	[[popUpButton menu] addItem:[NSMenuItem separatorItem]];

	// Set popup button action for menu items without their own actions (individual file types)
	[popUpButton setTarget:fileTypeFilter];
	[popUpButton setAction:@selector(filterSingleFileType:)];

	// Add file extensions and resize popup button to fit
	[popUpButton addItemsWithTitles:[[fileTypes allKeys] sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)]];
	[popUpButton sizeToFit];

	// Set container view dimensions
	float accViewWidth = 2 * kAccessoryViewPadding + label.frame.size.width + popUpButton.frame.size.width;
	float accViewHeight = 2 * kAccessoryViewPadding + fmax(label.frame.size.height, popUpButton.frame.size.height);
	[accessoryView setFrameSize:NSMakeSize(accViewWidth, accViewHeight)];
	[accessoryView addSubview:label];
	[accessoryView addSubview:popUpButton];

	// Enable autolayout
	[accessoryView setTranslatesAutoresizingMaskIntoConstraints:NO];
	[label setTranslatesAutoresizingMaskIntoConstraints:NO];
	[popUpButton setTranslatesAutoresizingMaskIntoConstraints:NO];

	NSDictionary* viewDic = NSDictionaryOfVariableBindings(label, popUpButton);
	NSDictionary* metricsDic = @{ @"padding":@(kAccessoryViewPadding) };

	// Align popup button to right of label, with padding in between
	[accessoryView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-[label]-padding-[popUpButton]-|"
																		  options:0
																		  metrics:metricsDic
																			views:viewDic]];

	// Centre popup button vertically within container
	[accessoryView addConstraint:[NSLayoutConstraint constraintWithItem:popUpButton
															  attribute:NSLayoutAttributeCenterY
															  relatedBy:NSLayoutRelationEqual
																 toItem:accessoryView
															  attribute:NSLayoutAttributeCenterY
															 multiplier:1.0f
															   constant:0.0f]];

	// Align label's text baseline to popup's text baseline
	[accessoryView addConstraint:[NSLayoutConstraint constraintWithItem:label
															  attribute:NSLayoutAttributeBaseline
															  relatedBy:NSLayoutRelationEqual
																 toItem:popUpButton
															  attribute:NSLayoutAttributeBaseline
															 multiplier:1.0f
															   constant:0.0f]];

	// Set dialog filetype filters and options
	[openPanel setTitle:[NSString stringWithUTF8String:caption]];
	[openPanel setAllowedFileTypes:[fileTypes allValues]];
	[openPanel setCanChooseFiles:YES];
	[openPanel setCanChooseDirectories:NO];
	[openPanel setAllowsMultipleSelection:NO];
	[openPanel setAccessoryView:accessoryView];

	// Open the dialog
	if ([openPanel runModal] == NSFileHandlingPanelOKButton)
	{
		fileName = PPSystemString([[[openPanel URL] path] UTF8String]);
		returnCode = ReturnCodeOK;
	}

	return returnCode;
}
