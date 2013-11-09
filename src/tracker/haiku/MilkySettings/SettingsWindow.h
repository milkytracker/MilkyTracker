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

#ifndef __HAIKU_SETTINGSWINDOW_H__
#define __HAIKU_SETTINGSWINDOW_H__

#include <Window.h>

class InterfaceSettingsView;
class MidiSettingsView;


class SettingsWindow : public BWindow
{
public:
						SettingsWindow();
	virtual				~SettingsWindow();

	virtual	void		MessageReceived(BMessage* message);

	virtual	bool		QuitRequested();

			BMessage*	GetSettings();
			void		SetSettings(BMessage* settings);

private:
			InterfaceSettingsView*	fInterfaceSettingsView;
			MidiSettingsView*		fMidiSettingsView;
};

#endif // __HAIKU_SETTINGSWINDOW_H__
