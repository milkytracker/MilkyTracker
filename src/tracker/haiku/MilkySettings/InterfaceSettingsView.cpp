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

#include "InterfaceSettingsView.h"

#include "SettingsMessages.h"

#include <Application.h>
#include <CheckBox.h>
#include <StringView.h>


InterfaceSettingsView::InterfaceSettingsView(BRect frame)
	:
	BView(frame, "Interface", B_FOLLOW_NONE, B_WILL_DRAW),
	fFirstAttach(true)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BRect rect = frame;
	rect.bottom = rect.top + 20;

	fSwitchCommandControlCheckBox = new BCheckBox(rect,
		"SwitchCommandControl",
		"Swap control / command key for keyboard shortcuts",
		new BMessage(kMsg_SwitchCommandControlToggled));
	AddChild(fSwitchCommandControlCheckBox);

	rect.OffsetBy(0, 30);
	fChangeFullscreenResolutionCheckBox = new BCheckBox(rect,
		"ChangeFullscreenResolution",
		"Change screen resolution in fullscreen modes",
		new BMessage(kMsg_ChangeFullscreenResolutionToggled));
	AddChild(fChangeFullscreenResolutionCheckBox);

	rect.OffsetBy(0, 30);
	rect.bottom = rect.top + 20;
	BStringView* warningLabel = new BStringView(rect, "WarningLabel",
		"Warning: resolution change might lead to a screenmode");
	AddChild(warningLabel);
	rect.OffsetBy(0, 20);
	warningLabel = new BStringView(rect, "WarningLabel",
		"your monitor can't handle. Use with caution!");
	AddChild(warningLabel);
}


InterfaceSettingsView::~InterfaceSettingsView()
{
}


void
InterfaceSettingsView::AttachedToWindow()
{
	if (fFirstAttach) {
		fSwitchCommandControlCheckBox->SetTarget(this);
		fChangeFullscreenResolutionCheckBox->SetTarget(this);
		fFirstAttach = false;
	}
}


void
InterfaceSettingsView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsg_SwitchCommandControlToggled:
			message->AddBool("on",
				fSwitchCommandControlCheckBox->Value() > 0 ? true : false);
			be_app->PostMessage(message);
			break;

		case kMsg_ChangeFullscreenResolutionToggled:
			message->AddBool("on",
				fChangeFullscreenResolutionCheckBox->Value() > 0 ?
				true : false);
			be_app->PostMessage(message);
			break;

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
InterfaceSettingsView::AddSettings(BMessage* settings)
{
	settings->AddBool("switch_command_control",
		fSwitchCommandControlCheckBox->Value() > 0 ? true : false);
	settings->AddBool("change_fullscreen_resolution",
		fChangeFullscreenResolutionCheckBox->Value() > 0 ? true : false);
}


void
InterfaceSettingsView::SetSettings(BMessage* settings)
{
	status_t status;

	bool switchCommandControl;
	status = settings->FindBool("switch_command_control",
		&switchCommandControl);
	if (status == B_OK)
		fSwitchCommandControlCheckBox->SetValue(switchCommandControl ? 1 : 0);

	bool changeFullscreenResolution;
	status = settings->FindBool("change_fullscreen_resolution",
		&changeFullscreenResolution);
	if (status == B_OK) {
		fChangeFullscreenResolutionCheckBox->SetValue(
			changeFullscreenResolution ? 1 : 0);
	}
}
