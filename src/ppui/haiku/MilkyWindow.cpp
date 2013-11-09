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

#include "MilkyWindow.h"

#include "DisplayDevice_Haiku.h"
#include "MilkyApplication.h"
#include "MilkyView.h"
#include "Screen.h"
#include "Tracker.h"

#include <Application.h>
#include <Bitmap.h>
#include <Screen.h>
#include <String.h>
#include <limits.h>


MilkyWindow::MilkyWindow(BRect frame, int32 scaleFactor, bool fullScreen,
	Tracker* tracker, sem_id trackerLock)
	:
	BWindow(frame, "", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE),
	fTracker(tracker),
	fTrackerLock(trackerLock),
	fEventThread(0),
	fEventPort(0),
	fSwitchFullscreenResolution(false)
{
	SetPulseRate(0);

	status_t status	;

	// Setup view and connect it to the tracker screen ------------------------
	fMilkyView = new MilkyView(Bounds(), scaleFactor, trackerLock);
	fDisplayDevice = new DisplayDevice_Haiku(fMilkyView, scaleFactor);
	// Important to add it _after_ creating the display device to prevent an
	// inter-task deadlock on fBitmapLock and the window's looper
	AddChild(fMilkyView);

	fTrackerScreen = new PPScreen(fDisplayDevice, fTracker);
	fTracker->setScreen(fTrackerScreen);

	// Enter fullscreen mode if requested -------------------------------------
	BScreen screen;
	screen.GetMode(&fOriginalMode);

	if (fullScreen)
		SetFullScreen(true);

	// Start event forwarding -------------------------------------------------
	fEventPort = create_port(64, "Milky event port");

	fEventThread = spawn_thread(&_EventThread, "Milky event forwarder",
		B_DISPLAY_PRIORITY, this);

	if (fEventPort < B_OK || fEventThread < B_OK)
		debugger("Could not setup event thread");

	status = resume_thread(fEventThread);

	if (status < B_OK)
		debugger("Could not start event thread");
}


MilkyWindow::~MilkyWindow()
{
	delete fTrackerScreen;
	delete fDisplayDevice;
}


bool
MilkyWindow::QuitRequested()
{
	// Do not allow quit while the tracker is still starting up
	// (that would crash it)
	if (be_app->IsLaunching())
		return false;

	// Ask tracker to shutdown
	if (!fTracker->shutDown())
		return false;

	PPEvent event(eAppQuit);
	RaiseEvent(&event);

	MilkyApplication* milkyApplication = (MilkyApplication*)be_app;
	milkyApplication->TrackerListening() = false;

	// Kill event forwarder
	kill_thread(fEventThread);
	close_port(fEventPort);

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


// #pragma mark - Event forwarding


void
MilkyWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_MOUSE_WHEEL_CHANGED: {
			float deltaX = message->FindFloat("be:wheel_delta_x");
			float deltaY = message->FindFloat("be:wheel_delta_y");
			fMilkyView->MouseWheelChanged(deltaX, deltaY);
		}	break;

		case B_MODIFIERS_CHANGED: {
			int32 modifiers = message->FindInt32("modifiers");
			int32 oldModifiers = message->FindInt32("be:old_modifiers");
			fMilkyView->ModifiersChanged(oldModifiers, modifiers);
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
MilkyWindow::DispatchMessage(BMessage* message, BHandler* target)
{
	// Special treatment for B_KEY_DOWN messages when command or control
	// are pressed: forward them to the Tracker which handles shortcuts itself
	if (message->what == B_KEY_DOWN) {
		int32 modifiers = message->FindInt32("modifiers");
		if (modifiers & (B_COMMAND_KEY | B_CONTROL_KEY)) {
			int32 rawChar = message->FindInt32("raw_char");
			fMilkyView->KeyDown((const char*)&rawChar, 1);
			return;
		}
	}

	BWindow::DispatchMessage(message, target);
}


void
MilkyWindow::ForwardEvents()
{
	int32 messageCode;
	PPEvent event;
	ssize_t size;
	MilkyApplication* milkyApplication = (MilkyApplication*)be_app;

	// Forward events received through the event port to the tracker
	for (;;) {
		size = read_port(fEventPort, &messageCode, &event, sizeof(PPEvent));
		if (size <= 0)
			debugger("Reading event port failed");

		if (milkyApplication->TrackerListening()) {
			acquire_sem(fTrackerLock);
			fTrackerScreen->raiseEvent(&event);
			release_sem(fTrackerLock);
		}
	}
}


status_t
MilkyWindow::_EventThread(void* data)
{
	MilkyWindow* milkyWindow = (MilkyWindow*)data;
	milkyWindow->ForwardEvents();

	return B_OK;
}


// #pragma mark - Fullscreen mode setting


bool
MilkyWindow::SetFullScreen(bool fullscreen)
{
	BScreen screen;

	int milkyWidth   = fMilkyView->Bitmap()->Bounds().IntegerWidth();
	int milkyHeight  = fMilkyView->Bitmap()->Bounds().IntegerHeight();

	if (fullscreen) {
		// Switch to fullscreen mode ------------------------------------------
		screen.GetMode(&fOriginalMode);

		if (   fOriginalMode.virtual_width  == milkyWidth
		    && fOriginalMode.virtual_height == milkyHeight) {
			MoveTo(0, 0);
			ResizeTo(milkyWidth, milkyHeight);
			return true;
		}

		display_mode* bestMatch = NULL;
		display_mode* modeList = NULL;

		if (fSwitchFullscreenResolution) {
			// MilkyPlayer's screen resolution is different from the physical
			// screen resolution. Try to find a fitting mode.
			uint32 modeCount;
			screen.GetModeList(&modeList, &modeCount);

			int tempWidth  = INT_MAX;
			int tempHeight = INT_MAX;
			for (int i = 0; i < modeCount; i++) {
				display_mode* mode = &modeList[i];

				// Choose a colour space that is either identical to the
				// current one or our preferred one, 32 bit
				if (   mode->space != fOriginalMode.space
				    && mode->space != B_RGBA32)
					continue;

				// Choose a mode with identical refresh rate than current mode
				if (_ModeRefreshRate(&fOriginalMode) != _ModeRefreshRate(mode))
					continue;

				// If the mode is an exact match in resolution, take it
				if (   mode->virtual_width  == milkyWidth
				    && mode->virtual_height == milkyHeight) {
				    bestMatch = mode;
				    break;
				}

				// Second best choice: a mode which is larger than necessary
				// Out of all larger modes, we want the smallest
				// (i.e. the tightest fit)
				if (   mode->virtual_width  > milkyWidth
				    && mode->virtual_height > milkyHeight
				    && mode->virtual_width  < tempWidth
				    && mode->virtual_height < tempHeight) {
				    bestMatch = mode;
				    tempWidth = mode->virtual_width;
				    tempHeight = mode->virtual_height;
				}
			}

			if (bestMatch == NULL) {
				free(modeList);
				return false;
			}

			screen.SetMode(bestMatch);
		} else
			bestMatch = &fOriginalMode;

		// If no exact match was found, center milkyview on screen with a
		// black border around it
		if (   bestMatch->virtual_width  > milkyWidth
		    || bestMatch->virtual_height > milkyHeight) {
			fMilkyView->SetTopLeft(
				(bestMatch->virtual_width  - milkyWidth) / 2,
				(bestMatch->virtual_height - milkyHeight) / 2);
		}

		if (modeList != NULL)
			free(modeList);

		MoveTo(0, 0);
		ResizeTo(bestMatch->virtual_width, bestMatch->virtual_height);
		fMilkyView->Invalidate();

		return true;
	} else {
		// Switch to window mode ----------------------------------------------
		if (fSwitchFullscreenResolution)
			screen.SetMode(&fOriginalMode);
		MoveTo(100, 100);
		ResizeTo(milkyWidth, milkyHeight);
		fMilkyView->SetTopLeft(0, 0);
		fMilkyView->Invalidate();
	}
}


uint32
MilkyWindow::_ModeRefreshRate(display_mode* mode)
{
	return (1000 * (uint32)mode->timing.pixel_clock) /
		((uint32)mode->timing.h_total * (uint32)mode->timing.v_total);
}
