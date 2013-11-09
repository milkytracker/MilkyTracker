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

#include "MilkyApplication.h"

#include "KeyCodeMap.h"
#include "MidiSetup.h"
#include "MilkyWindow.h"
#include "Screen.h"
#include "Tracker.h"
#include "MilkySettings/SettingsMessages.h"

#include <Deskbar.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <String.h>

#include <stdio.h>
#include <string.h>


const char*	kMilkySignature	= "application/x-vnd.Milky-MilkyTracker";


MilkyApplication::MilkyApplication()
	:
	BApplication(kMilkySignature),
	fMilkyWindow(NULL),
	fTracker(NULL),
	fTrackerLock(0),
	fTrackerListening(false),
	fLaunchOpenFile(NULL),
	fClockThread(0)
{
}


MilkyApplication::~MilkyApplication()
{

}


void
MilkyApplication::ReadyToRun()
{
	status_t status;

	// Setup tracker ----------------------------------------------------------
	fTracker = new Tracker();
	PPSize windowSize  = fTracker->getWindowSizeFromDatabase();
	int32  scaleFactor = fTracker->getScreenScaleFactorFromDatabase();
	bool   fullScreen  = fTracker->getFullScreenFlagFromDatabase();
	fTrackerLock = create_sem(1, "MilkyTracker lock");

	if (fTrackerLock < B_OK)
		debugger("Could not create tracker lock");

	// Setup main window ------------------------------------------------------
	fMilkyWindow = new MilkyWindow(BRect(100, 100,
		100 + windowSize.width, 100 + windowSize.height), scaleFactor,
		fullScreen, fTracker, fTrackerLock);
	fMilkyWindow->Show();

	// Start tracker ----------------------------------------------------------
	fTracker->startUp();
	fTrackerListening = true;

	if (fLaunchOpenFile != NULL) {
		_LoadFile(fLaunchOpenFile->String());
		delete fLaunchOpenFile;
		fLaunchOpenFile = NULL;
	}

	// Start main clock -------------------------------------------------------
	fClockThread = spawn_thread(&_ClockThread, "Milky clock",
		B_URGENT_DISPLAY_PRIORITY, this);

	if (fClockThread < B_OK)
		debugger("Could not setup clock thread");

	status = resume_thread(fClockThread);

	if (status < B_OK)
		debugger("Could not start clock thread");

	// Start listening for MIDI input -----------------------------------------
	MidiSetup::StartMidi(fTracker, fTrackerLock);

	// Load platform-specific settings from file ------------------------------
	_LoadPlatformSettings();
}


bool
MilkyApplication::QuitRequested()
{
	// Stop raising events to the tracker
	fTrackerListening = false;

	// Stop clock
	kill_thread(fClockThread);

	// Stop listening to MIDI input
	MidiSetup::StopMidi();

	// Nobody should be using the tracker now anymore
	delete_sem(fTrackerLock);
	delete fTracker;

	return true;
}


// #pragma mark - Platform-specific settings


void
MilkyApplication::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsg_SwitchCommandControlToggled:
			_SetSwapCommandControl(message->FindBool("on"));
			break;

		case kMsg_ChangeFullscreenResolutionToggled:
			fMilkyWindow->SetSwitchFullscreenResolution(
				message->FindBool("on"));
			break;

		case kMsg_MidiInputSelected:
			MidiSetup::ConnectProducer(message->FindInt32("id"));
			break;

		case kMsg_MidiRecordVelocityToggled:
			MidiSetup::SetRecordVelocity(message->FindBool("on"));
			break;

		case kMsg_MidiVelocityAmplificationChanged:
			MidiSetup::SetVelocityAmplify(message->FindInt32("value"));
			break;

		default:
			BApplication::MessageReceived(message);
	}
}


void
MilkyApplication::_LoadPlatformSettings()
{
	// Build path to settings file and check if it exists ---------------------
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append("MilkyTracker/platform_settings");

	BEntry settingsEntry(path.Path());
	if (!settingsEntry.Exists())
		return;

	// Settings file exists, try to open and unflatten into message -----------
	BFile settingsFile(path.Path(), B_READ_ONLY);
	if (settingsFile.InitCheck() != B_OK)
		return;

	BMessage settings;
	status_t status = settings.Unflatten(&settingsFile);
	if (status != B_OK)
		return;

	// Look for known settings and apply them  --------------------------------
	bool switchCommandControl;
	status = settings.FindBool("switch_command_control",
		&switchCommandControl);
	if (status == B_OK)
		_SetSwapCommandControl(switchCommandControl);

	bool changeFullscreenResolution;
	status = settings.FindBool("change_fullscreen_resolution",
		&changeFullscreenResolution);
	if (status == B_OK)
		fMilkyWindow->SetSwitchFullscreenResolution(
			changeFullscreenResolution);

	bool recordVelocity;
	status = settings.FindBool("record_velocity",
		&recordVelocity);
	if (status == B_OK)
		MidiSetup::SetRecordVelocity(recordVelocity);

	int16 velocityAmplify;
	status = settings.FindInt16("velocity_amplify",
		&velocityAmplify);
	if (status == B_OK)
		MidiSetup::SetVelocityAmplify(velocityAmplify);

	BString midiInputName;
	status = settings.FindString("midi_input", &midiInputName);
	if (status == B_OK)
		MidiSetup::ConnectProducer(midiInputName);
}


void
MilkyApplication::_SetSwapCommandControl(bool swap)
{
	gSwapCommandControl = swap;
	if (swap) {
		gModifierDataCommand.modifier      = B_CONTROL_KEY;
		gModifierDataCommand.modifierLeft  = B_LEFT_CONTROL_KEY;
		gModifierDataCommand.modifierRight = B_RIGHT_CONTROL_KEY;
		gModifierDataControl.modifier      = B_COMMAND_KEY;
		gModifierDataControl.modifierLeft  = B_LEFT_COMMAND_KEY;
		gModifierDataControl.modifierRight = B_RIGHT_COMMAND_KEY;
	} else {
		gModifierDataCommand.modifier      = B_COMMAND_KEY;
		gModifierDataCommand.modifierLeft  = B_LEFT_COMMAND_KEY;
		gModifierDataCommand.modifierRight = B_RIGHT_COMMAND_KEY;
		gModifierDataControl.modifier      = B_CONTROL_KEY;
		gModifierDataControl.modifierLeft  = B_LEFT_CONTROL_KEY;
		gModifierDataControl.modifierRight = B_RIGHT_CONTROL_KEY;
	}
}


// #pragma mark - Loading files from argv and icon drop


void
MilkyApplication::ArgvReceived(int32 argc, char** argv)
{
	// We only care for the first command line argument, which can be a file
	// which we're going to try to open
	BEntry entry(argv[1]);
	if (entry.InitCheck() != B_OK)
		return;
	if (!entry.Exists()) {
		fprintf(stderr, "ERROR: file not found '%s'\n", argv[1]);
		return;
	}

	entry_ref ref;
	entry.GetRef(&ref);

	BMessage message(B_REFS_RECEIVED);
	message.AddRef("refs", &ref);
	RefsReceived(&message);
}


void
MilkyApplication::RefsReceived(BMessage* message)
{
	status_t status;

	entry_ref ref;
	status = message->FindRef("refs", &ref);
	if (status != B_OK)
		return;

	BPath path(&ref);
	if (path.InitCheck() != B_OK)
		return;

	// Check that it's a file with a MIME type of the audio group
	BNode node(&ref);
	if (node.InitCheck() != B_OK)
		return;
	BNodeInfo nodeInfo(&node);
	char mimeType[B_MIME_TYPE_LENGTH];
	nodeInfo.GetType(mimeType);
	BString mimeTypeString(mimeType);
	if (mimeTypeString.FindFirst("audio/") != 0)
		return;

	// If we are just launching the application (and thus the Tracker
	// is not initialized yet), defer the file load
	if (IsLaunching())
		fLaunchOpenFile = new BString(path.Path());
	else
		_LoadFile(path.Path());
}


void
MilkyApplication::_LoadFile(const char* path)
{
	fDragDroppedFile = PPSystemString(path);
	PPSystemString* pathStringPointer = &fDragDroppedFile;
	PPEvent event(eFileDragDropped, &pathStringPointer,
		sizeof(PPSystemString*));

	fMilkyWindow->RaiseEvent(&event);
}


// #pragma mark - Clock generator


void
MilkyApplication::GenerateClock()
{
	PPEvent event(eTimer);
	bigtime_t beginTime;

	// Send clock events to the tracker every 15ms
	for(;;) {
		beginTime = system_time();

		if (fTrackerListening) {
			acquire_sem(fTrackerLock);
			fMilkyWindow->TrackerScreen()->raiseEvent(&event);
			release_sem(fTrackerLock);
		}

		snooze_until(beginTime + 20000, B_SYSTEM_TIMEBASE);
	}
}


status_t
MilkyApplication::_ClockThread(void* data)
{
	MilkyApplication* app = (MilkyApplication*)data;
	app->GenerateClock();

	return B_OK;
}
