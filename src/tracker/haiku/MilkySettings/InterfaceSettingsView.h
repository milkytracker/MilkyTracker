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

#ifndef __HAIKU_INTERFACESETTINGSVIEW_H__
#define __HAIKU_INTERFACESETTINGSVIEW_H__

#include <View.h>

class BCheckBox;


class InterfaceSettingsView : public BView
{
public:
						InterfaceSettingsView(BRect frame);
						~InterfaceSettingsView();

	virtual	void		AttachedToWindow();
	virtual	void		MessageReceived(BMessage* message);

			void		AddSettings(BMessage* settings);
			void		SetSettings(BMessage* settings);

private:
			BCheckBox*	fSwitchCommandControlCheckBox;
			BCheckBox*	fChangeFullscreenResolutionCheckBox;

			bool		fFirstAttach;
};

#endif // __HAIKU_INTERFACESETTINGSVIEW_H__
