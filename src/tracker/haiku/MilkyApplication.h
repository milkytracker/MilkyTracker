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

#ifndef __MILKYAPPLICATION_H__
#define __MILKYAPPLICATION_H__

#include <Application.h>

#include "BasicTypes.h"
#include "Event.h"

class MilkyWindow;
class Tracker;


class MilkyApplication : public BApplication
{
public:
							MilkyApplication();
	virtual					~MilkyApplication();

	virtual	void			ReadyToRun();
	virtual	bool			QuitRequested();
private:
			MilkyWindow*	fMilkyWindow;
			Tracker*		fTracker;
			sem_id			fTrackerLock;
			bool			fTrackerListening;

// --- Platform-specific settings ---------------------------------------------
public:
	virtual	void			MessageReceived(BMessage* message);
private:
			void			_LoadPlatformSettings();
			void			_SetSwapCommandControl(bool swap);


// --- Loading files from argv and icon drop ----------------------------------
public:
	virtual	void			ArgvReceived(int32 argc, char** argv);
	virtual	void			RefsReceived(BMessage* message);
private:
			void			_LoadFile(const char* path);

			BString*		fLaunchOpenFile;
			PPSystemString	fDragDroppedFile;


// --- Clock generator --------------------------------------------------------
public:
			void			GenerateClock();
			bool&			TrackerListening()
								{ return fTrackerListening; }
private:
	static	status_t		_ClockThread(void* data);

			thread_id		fClockThread;
};

#endif // __MILKYAPPLICATION_H__
