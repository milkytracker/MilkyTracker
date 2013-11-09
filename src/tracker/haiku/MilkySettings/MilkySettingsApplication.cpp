/*
 *  Copyright 2012 Julian Harnath
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

#include "MilkySettingsApplication.h"

#include "SettingsMessages.h"
#include "SettingsWindow.h"

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Messenger.h>
#include <Path.h>
#include <Roster.h>
#include <String.h>

#include <stdio.h>

const char* kMilkyTrackerSignature = "application/x-vnd.Milky-MilkyTracker";


MilkySettingsApplication::MilkySettingsApplication()
	:
	BApplication("application/x-vnd.Milky-MilkySettings")
{
	fSettingsWindow = new SettingsWindow();
	fSettingsWindow->Show();

	// Try to open settings file
	BFile* settingsFile = _OpenSettingsFile(B_READ_ONLY);
	if (settingsFile != NULL) {
		BMessage settings;
		status_t status = settings.Unflatten(settingsFile);
		delete settingsFile;

		if (status == B_OK) {
			// Apply settings
			fSettingsWindow->LockLooper();
			fSettingsWindow->SetSettings(&settings);
			fSettingsWindow->UnlockLooper();
		}
	}

	fMilkyTrackerMessenger = new BMessenger(kMilkyTrackerSignature);

	be_roster->StartWatching(BMessenger(this), B_REQUEST_LAUNCHED);
}


MilkySettingsApplication::~MilkySettingsApplication()
{
	delete fMilkyTrackerMessenger;
}


void
MilkySettingsApplication::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_SOME_APP_LAUNCHED: {
			// Watch out for launches of MilkyTracker
			BString signature;
			message->FindString("be:signature", &signature);

			if (signature == kMilkyTrackerSignature) {
				BMessenger* newMessenger =
					new BMessenger(kMilkyTrackerSignature);

				// Swap pointers to make sure that fMilkyTrackerMessenger
				// points to an existing messenger instance at all times
				BMessenger* oldMessenger = fMilkyTrackerMessenger;
				fMilkyTrackerMessenger = newMessenger;
				delete oldMessenger;
			}
		}	break;

		case kMsg_SwitchCommandControlToggled:
		case kMsg_ChangeFullscreenResolutionToggled:
		case kMsg_MidiInputSelected:
		case kMsg_MidiRecordVelocityToggled:
		case kMsg_MidiVelocityAmplificationChanged: {
			// Forward to MilkyTracker if it's running
			if (fMilkyTrackerMessenger->IsValid())
				fMilkyTrackerMessenger->SendMessage(message);

			// Save to settings file
			BFile* settingsFile = _OpenSettingsFile(B_WRITE_ONLY |
				B_CREATE_FILE | B_ERASE_FILE);

			if (settingsFile != NULL) {
				BMessage* settings = fSettingsWindow->GetSettings();
				settings->Flatten(settingsFile);
				delete settings;
				delete settingsFile;
			}
		}	break;

		default:
			BApplication::MessageReceived(message);
			break;
	}
}


BFile*
MilkySettingsApplication::_OpenSettingsFile(uint32 openMode)
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append("MilkyTracker");

	BEntry dirEntry(path.Path());
	if (!dirEntry.Exists()) {
		// MilkyTracker settings dir doesn't exist, create it
		BDirectory temp;
		temp.CreateDirectory(path.Path(), NULL);
	}

	path.Append("platform_settings");
	BFile* file = new BFile(path.Path(), openMode);

	if (file->InitCheck() != B_OK) {
		delete file;
		return NULL;
	}

	return file;
}
