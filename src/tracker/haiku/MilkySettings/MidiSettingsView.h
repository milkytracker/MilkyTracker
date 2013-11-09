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

#ifndef __HAIKU_MIDISETTINGSVIEW_H__
#define __HAIKU_MIDISETTINGSVIEW_H__

#include <View.h>

#include <Locker.h>
#include <String.h>

class BCheckBox;
class BList;
class BMenuField;
class BMenuItem;
class BPopUpMenu;
class BSlider;

struct MidiProducer;


class MidiSettingsView : public BView
{
public:
							MidiSettingsView(BRect frame);
							~MidiSettingsView();

	virtual	void			AttachedToWindow();
	virtual	void			MessageReceived(BMessage* message);

			void			AddSettings(BMessage* settings);
			void			SetSettings(BMessage* settings);

private:
			void			_HandleMidiEvent(BMessage* message);
			void			_SetVelocityBoxEnabled(bool enable);
			MidiProducer*	_FindMidiProducer(int32 id);
			MidiProducer*	_FindMidiProducer(BMenuItem* item);
			MidiProducer*	_FindMidiProducer(BString name);

			BCheckBox*		fRecordVelocityCheckBox;
			BSlider*		fVelocityAmplificationSlider;
			BPopUpMenu*		fMidiInputMenu;
			BMenuField*		fMidiInputMenuField;

			BList*			fMidiProducers;
			BLocker			fMidiProducersLock;
			MidiProducer* 	fSelectedMidiProducer;

			bool			fFirstAttach;
};

#endif // __HAIKU_INTERFACESETTINGSVIEW_H__
