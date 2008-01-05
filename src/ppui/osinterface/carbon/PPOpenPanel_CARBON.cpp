/*
 *  PPOpenPanel_CARBON.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sun Feb 27 2005.
 *  Copyright (c) 2005 milkytracker.net. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>
#include "PPOpenPanel.h"

extern SInt32			globalWindowLevel;
extern WindowGroupRef   groupRef;

PPOpenPanel::PPOpenPanel(PPScreen* screen, const char* caption) :
	PPModalDialog(screen)
{
	this->caption = new char[strlen(caption)+1];
	strcpy(this->caption, caption);
}

PPOpenPanel::~PPOpenPanel()
{
	delete[] caption;
}

void PPOpenPanel::addExtension(const PPString& ext, const PPString& desc)
{
	Descriptor* d = new Descriptor(ext, desc);

	items.add(d);
}

PPOpenPanel::ReturnCodes PPOpenPanel::runModal()
{
	ReturnCodes result = ReturnCodeCANCEL;
	
	OSStatus err = noErr;
	NavDialogRef theOpenDialog;
	NavDialogCreationOptions dialogOptions;
	
	if ((err = NavGetDefaultDialogCreationOptions(&dialogOptions)) == noErr)
	{
		
		dialogOptions.modality = kWindowModalityAppModal;
		dialogOptions.windowTitle = CFStringCreateWithCString(NULL, caption, kCFStringEncodingASCII);
		
		err = NavCreateChooseFileDialog(&dialogOptions, NULL, NULL, NULL, NULL, NULL, &theOpenDialog);
	
		if (theOpenDialog)
		{
			err = NavDialogRun(theOpenDialog);
			
			NavReplyRecord reply;
			
			err = NavDialogGetReply (theOpenDialog, &reply);
			
			if (err == noErr)
			{
				
				// retrieve filename
				AEDesc actualDesc;
				FSRef fileToOpen;
				//HFSUniStr255 theFileName;
				//CFStringRef fileNameCFString;
				
				err = AECoerceDesc(&reply.selection, typeFSRef, &actualDesc);
				
				err = AEGetDescData(&actualDesc, reinterpret_cast<void*>(&fileToOpen), sizeof(FSRef));
				
				// gib ihm
				int len = PATH_MAX;
				char* buffer = new char[PATH_MAX+1];
				
				FSRefMakePath (&fileToOpen, (UInt8*)buffer, len);			
				
				fileName = buffer;
					
				delete[] buffer;
				
				result = ReturnCodeOK;
				
				NavDisposeReply(&reply);
			}
			
			NavDialogDispose(theOpenDialog);
			
		}
		
		if (dialogOptions.windowTitle)
			CFRelease(dialogOptions.windowTitle);
				
	}
	
	return result;

}
