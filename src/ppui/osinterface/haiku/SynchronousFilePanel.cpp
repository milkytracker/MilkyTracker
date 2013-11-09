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

#include "SynchronousFilePanel.h"

#include "MilkyApplication.h"
#include "MilkyView.h"

#include <FilePanel.h>
#include <Messenger.h>
#include <Path.h>
#include <Window.h>

BFilePanel* SynchronousFilePanel::fOpenPanel = NULL;
BFilePanel* SynchronousFilePanel::fSavePanel = NULL;


SynchronousFilePanel::SynchronousFilePanel(file_panel_mode mode, char* caption)
	:
	BLooper("File panel looper"),
	fSelectedPath("")
{
	// Use static instances for open and save panel so file panels retain the
	// last opened directory. Otherwise, the user would have to start with the
	// home directory again every time he opens or saves something.
	if (mode == B_OPEN_PANEL) {
		if (fOpenPanel == NULL) {
			fOpenPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE,
				false, NULL, NULL, true);
		}
		fActivePanel = fOpenPanel;
	} else if (mode == B_SAVE_PANEL) {
		if (fSavePanel == NULL) {
			fSavePanel = new BFilePanel(B_SAVE_PANEL, NULL, NULL, B_FILE_NODE,
				false, NULL, NULL, true);
		}
		fActivePanel = fSavePanel;
	}

	fActivePanel->SetTarget(BMessenger(this));
	fActivePanel->Window()->SetTitle(caption);
	fPanelClosedSemaphore = create_sem(0, "File panel message wait");

	// Start looper thread
	BLooper::Run();
}


SynchronousFilePanel::~SynchronousFilePanel()
{
}


void
SynchronousFilePanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			message->FindRef("refs", &ref);

			BPath path(&ref);
			if (path.InitCheck() == B_OK)
				fSelectedPath = path.Path();

			release_sem(fPanelClosedSemaphore);
		} break;

		case B_SAVE_REQUESTED:
		{
			entry_ref directoryRef;
			message->FindRef("directory", &directoryRef);

			BString fileName;
			message->FindString("name", &fileName);

			BPath path(&directoryRef);
			if (path.InitCheck() == B_OK) {
				fSelectedPath = path.Path();
				fSelectedPath << "/" << fileName;
			}

			release_sem(fPanelClosedSemaphore);
		}	break;

		case B_CANCEL:
			release_sem(fPanelClosedSemaphore);
			break;

		default:
			BLooper::MessageReceived(message);
			break;
	}
}


void
SynchronousFilePanel::Quit()
{
	delete_sem(fPanelClosedSemaphore);

	// Stop looper thread
	BLooper::Lock();
	BLooper::Quit();
}


void
SynchronousFilePanel::SetSaveText(const char* saveText)
{
	fActivePanel->SetSaveText(saveText);
}


BString
SynchronousFilePanel::Go()
{
	MilkyApplication* milkyApplication = (MilkyApplication*)be_app;

	fSelectedPath = "";

	milkyApplication->TrackerListening() = false;

	fActivePanel->Show();
	acquire_sem(fPanelClosedSemaphore);

	milkyApplication->TrackerListening() = true;

	return fSelectedPath;
}
