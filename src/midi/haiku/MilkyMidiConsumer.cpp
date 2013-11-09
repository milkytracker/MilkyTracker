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
#include "MilkyMidiConsumer.h"
#include "MidiTools.h"
#include "Tracker.h"

#include <MidiProducer.h>
#include <MidiRoster.h>
#include <OS.h>


MilkyMidiConsumer::MilkyMidiConsumer(Tracker* tracker, sem_id trackerLock)
	:
	BMidiLocalConsumer("Milky MIDI consumer"),
	fTracker(tracker),
	fTrackerLock(trackerLock),
	fRecordVelocity(false),
	fVelocityAmplify(100),
	fConnectedProducer(NULL)
{
}


MilkyMidiConsumer::~MilkyMidiConsumer()
{
}


void
MilkyMidiConsumer::NoteOn(uchar channel, uchar note, uchar velocity,
	bigtime_t time)
{
	snooze_until(time, B_SYSTEM_TIMEBASE);

	if (velocity == 0) {
		NoteOff(channel, note, velocity, time);
		return;
	}

	acquire_sem(fTrackerLock);
	fTracker->sendNoteDown(note,
		fRecordVelocity ? vol126to255(velocity - 1, fVelocityAmplify) : - 1);
	release_sem(fTrackerLock);
}


void
MilkyMidiConsumer::NoteOff(uchar channel, uchar note, uchar velocity,
	bigtime_t time)
{
	snooze_until(time, B_SYSTEM_TIMEBASE);

	acquire_sem(fTrackerLock);
	fTracker->sendNoteUp(note);
	release_sem(fTrackerLock);
}


void
MilkyMidiConsumer::ConnectProducer(int32 id)
{
	// Disconnect from previously connected producer, if there is one
	if (fConnectedProducer != NULL) {
		fConnectedProducer->Disconnect(this);
		fConnectedProducer->Release();
		fConnectedProducer = NULL;
	}

	if (id == -1)
		return;

	BMidiProducer* producer = BMidiRoster::FindProducer(id);
	if (producer == NULL) {
		fprintf(stderr, "MilkyMidiConsumer::ConnectProducer - error: could "
		                "not find producer id=%d\n", id);
		return;
	}

	producer->Connect(this);
	fConnectedProducer = producer;
}
