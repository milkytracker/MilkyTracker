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
#ifndef __HAIKU_MILKYWINDOW_H__
#define __HAIKU_MILKYWINDOW_H__

#include <Window.h>

#include <Screen.h>

#include "Event.h"

class DisplayDevice_Haiku;
class MilkyView;
class PPScreen;
class Tracker;


class MilkyWindow : public BWindow
{
public:
							MilkyWindow(BRect frame, int32 scaleFactor,
								bool fullScreen, Tracker* tracker,
								sem_id trackerLock);
	virtual					~MilkyWindow();

	virtual	bool			QuitRequested();

			PPScreen*		TrackerScreen()
								{ return fTrackerScreen; }
private:
			MilkyView*		fMilkyView;
			DisplayDevice_Haiku* fDisplayDevice;
			PPScreen*		fTrackerScreen;
			Tracker*		fTracker;
			sem_id			fTrackerLock;

// --- Event forwarding -------------------------------------------------------
public:
	virtual	void			MessageReceived(BMessage* message);
	virtual	void			DispatchMessage(BMessage* message,
								BHandler* target);

			void			ForwardEvents();
			status_t		RaiseEvent(PPEvent* event);
private:
	static	status_t		_EventThread(void* data);

			thread_id		fEventThread;
			port_id			fEventPort;

// --- Fullscreen mode setting ------------------------------------------------
public:
			bool			SetFullScreen(bool fullscreen);

			void			SetSwitchFullscreenResolution(bool enabled)
								{ fSwitchFullscreenResolution = enabled; }
private:
			uint32			_ModeRefreshRate(display_mode* mode);

			display_mode    fOriginalMode;
			bool			fSwitchFullscreenResolution;
};


inline status_t
MilkyWindow::RaiseEvent(PPEvent* event)
{
	status_t status = write_port_etc(fEventPort, 0, event, sizeof(PPEvent),
		B_TIMEOUT, 0);

	if (status == B_BAD_PORT_ID)
		debugger("Event port write failed: bad port ID");
}


#endif // __HAIKU_MILKYWINDOW_H__
