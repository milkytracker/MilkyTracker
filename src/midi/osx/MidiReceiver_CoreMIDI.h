/*
 *  midi/osx/MidiReceiver_CoreMIDI.h
 *
 *  Copyright 2015 Dale Whinham
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

#ifndef __MIDIRECEIVER_COREMIDI_H__
#define __MIDIRECEIVER_COREMIDI_H__

#include <CoreMIDI/CoreMIDI.h>
#include "BasicTypes.h"

class Tracker;
class PPMutex;

class MidiReceiver
{
private:
	struct MidiEndpoint
	{
		MIDIEndpointRef ref;
		PPString name;
	};

	Tracker& tracker;
	PPMutex& criticalSection;
	bool recordVelocity;
	int velocityAmplify;

	MIDIClientRef clientRef;
	MIDIPortRef inputPortRef;
	ItemCount numSourceEndpoints;
	MidiEndpoint* sourceEndpoints;

	static void notifyProc(const MIDINotification* message, void* refCon);
	static void readProc(const MIDIPacketList* pktlist, void* readProcRefCon, void* srcConnRefCon);

	static PPString getMidiDeviceName(MIDIEndpointRef endpointRef);

public:
	MidiReceiver(Tracker& tracker, PPMutex& criticalSection);
	~MidiReceiver();

	bool init();
	bool close();

	void setRecordVelocity(bool recVelocity) { recordVelocity = recVelocity; }
	void setVelocityAmplify(int amplify) { velocityAmplify = amplify; }
};

#endif