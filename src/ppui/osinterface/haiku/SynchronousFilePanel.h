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

#ifndef __HAIKU_SYNCHRONOUSFILEPANEL_H__
#define __HAIKU_SYNCHRONOUSFILEPANEL_H__

#include <Looper.h>
#include <FilePanel.h>
#include <String.h>


class SynchronousFilePanel : public BLooper
{
public:
									SynchronousFilePanel(file_panel_mode mode,
										char* caption);
									~SynchronousFilePanel();

	virtual	void					MessageReceived(BMessage* message);
	virtual	void					Quit();

			void					SetSaveText(const char* saveText);
			BString					Go();

private:
	static	BFilePanel*				fOpenPanel;
	static	BFilePanel*				fSavePanel;

			BFilePanel*				fActivePanel;

			BString					fSelectedPath;

			sem_id					fPanelClosedSemaphore;
};

#endif // __HAIKU_SYNCHRONOUSFILEPANEL_H__
