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
#include "MidiSetup.h"
#include "MilkyMidiConsumer.h"

#include <MidiProducer.h>
#include <MidiRoster.h>

#include <assert.h>
#include <stdio.h>

MilkyMidiConsumer* MidiSetup::sMilkyMidiConsumer = NULL;


void
MidiSetup::StartMidi(Tracker* tracker, sem_id eventLock)
{
	assert(sMilkyMidiConsumer == NULL);

	sMilkyMidiConsumer = new MilkyMidiConsumer(tracker, eventLock);
	sMilkyMidiConsumer->Register();
}


void
MidiSetup::StopMidi()
{
	sMilkyMidiConsumer->ConnectProducer(-1);
	sMilkyMidiConsumer->Unregister();
	sMilkyMidiConsumer->Release();
	// sMilkyMidiConsumer is automatically destructed when reference
	// count reaches zero
}


void
MidiSetup::ConnectProducer(int32 id)
{
	if (sMilkyMidiConsumer != NULL)
		sMilkyMidiConsumer->ConnectProducer(id);
}


void
MidiSetup::ConnectProducer(BString name)
{
	if (sMilkyMidiConsumer != NULL) {
		int32 id = 0;
		BMidiProducer* midiProducer = BMidiRoster::NextProducer(&id);

		while (midiProducer != NULL) {
			if (name == midiProducer->Name()) {
				midiProducer->Release();
				break;
			}

			midiProducer->Release();
			midiProducer = BMidiRoster::NextProducer(&id);
		}

		sMilkyMidiConsumer->ConnectProducer(id);
	}
}


void
MidiSetup::SetRecordVelocity(bool record)
{
	if (sMilkyMidiConsumer != NULL)
		sMilkyMidiConsumer->SetRecordVelocity(record);
}


void
MidiSetup::SetVelocityAmplify(int32 amplification)
{
	if (sMilkyMidiConsumer != NULL)
		sMilkyMidiConsumer->SetVelocityAmplify(amplification);
}
