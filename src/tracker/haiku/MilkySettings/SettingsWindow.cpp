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

#include "SettingsWindow.h"

#include "InterfaceSettingsView.h"
#include "MidiSettingsView.h"

#include <Application.h>
#include <TabView.h>


SettingsWindow::SettingsWindow()
	:
	BWindow(BRect(100, 100, 500, 300), "MilkyTracker settings",
		B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
	BTabView* tabView = new BTabView(Bounds(), "TabView");

	BRect tabRect = tabView->ContainerView()->Bounds();
	tabRect.InsetBy(5, 5);

	fInterfaceSettingsView = new InterfaceSettingsView(tabRect);
	fMidiSettingsView = new MidiSettingsView(tabRect);

	tabView->AddTab(fInterfaceSettingsView);
	tabView->AddTab(fMidiSettingsView);

	AddChild(tabView);
}


SettingsWindow::~SettingsWindow()
{
}


void
SettingsWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
SettingsWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


BMessage*
SettingsWindow::GetSettings()
{
	BMessage* settings = new BMessage();
	fInterfaceSettingsView->AddSettings(settings);
	fMidiSettingsView->AddSettings(settings);
	return settings;
}


void
SettingsWindow::SetSettings(BMessage* settings)
{
	fInterfaceSettingsView->SetSettings(settings);
	fMidiSettingsView->SetSettings(settings);
}
