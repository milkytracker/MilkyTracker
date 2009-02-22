/*
 *  ppui/osinterface/carbon/PPSavePanel_CARBON.cpp
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
 *  PPSavePanel.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on Sat Mar 05 2005.
 *
 */
 
#include <Carbon/Carbon.h>
#include "PPSavePanel.h"

PPSavePanel::ReturnCodes PPSavePanel::runModal()
{
	ReturnCodes result = ReturnCodeCANCEL;
	
	OSStatus err = noErr;
	NavDialogRef theSaveDialog;
	NavDialogCreationOptions dialogOptions;
	
	if ((err = NavGetDefaultDialogCreationOptions(&dialogOptions)) == noErr)
	{
		
		dialogOptions.modality = kWindowModalityAppModal;
		dialogOptions.windowTitle = CFStringCreateWithCString(NULL, caption, kCFStringEncodingASCII);
		if (defaultFileName.length())
			dialogOptions.saveFileName = CFStringCreateWithCString(NULL, defaultFileName, kCFStringEncodingASCII);
		
		err = NavCreatePutFileDialog(&dialogOptions, 0, 0, NULL, NULL, &theSaveDialog);
	
		if (theSaveDialog)
		{
			err = NavDialogRun(theSaveDialog);
			
			NavReplyRecord reply;
			
			err = NavDialogGetReply (theSaveDialog, &reply);
			
			if (err == noErr)
			{
				
				// retrieve filename
				AEDesc actualDesc;
				FSRef fileToSave;
				//HFSUniStr255 theFileName;
				//CFStringRef fileNameCFString;
				
				err = AECoerceDesc(&reply.selection, typeFSRef, &actualDesc);
				
				err = AEGetDescData(&actualDesc, reinterpret_cast<void*>(&fileToSave), sizeof(FSRef));
				
				// gib ihm
				int len = 4096;
				char* buffer = new char[len+1];
				
				FSRefMakePath (&fileToSave, (UInt8*)buffer, len);			

				int len2 = CFStringGetLength(reply.saveFileName);
				char* buffer2 = new char[len2+1];
				
				CFStringGetCString(reply.saveFileName, buffer2, len2+1, kCFStringEncodingASCII);
								
				fileName = buffer;
					
				fileName.append("/");
				fileName.append(buffer2);
				
				delete[] buffer;
				
				delete[] buffer2;
				
				result = ReturnCodeOK;
				
				NavDisposeReply(&reply);
			}
			
			NavDialogDispose(theSaveDialog);
			
		}
		
		if (dialogOptions.windowTitle)
			CFRelease(dialogOptions.windowTitle);
		if (dialogOptions.saveFileName)
			CFRelease(dialogOptions.saveFileName);
			
				
	}
	
	return result;

}
