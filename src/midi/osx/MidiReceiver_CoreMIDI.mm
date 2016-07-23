/*
 *  midi/osx/MidiReceiver_CoreMIDI.mm
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

#include <Foundation/Foundation.h>
#include "MidiReceiver_CoreMIDI.h"
#include "PPMutex.h"
#include "Tracker.h"
#include "MidiTools.h"

// Error handling macro
#define CORE_MIDI_CHECK(RES, MSG)							\
do															\
{															\
	if (RES != noErr)										\
	{														\
		NSLog(@"Core MIDI: %s (Error code: %d)", MSG, RES); \
		return false;										\
	}														\
} while (0);												\

MidiReceiver::MidiReceiver(Tracker& tracker, PPMutex& criticalSection) :
tracker(tracker),
criticalSection(criticalSection),
recordVelocity(true),
velocityAmplify(100),
clientRef(0),
inputPortRef(0),
numSourceEndpoints(0),
sourceEndpoints(NULL)
{
}

MidiReceiver::~MidiReceiver()
{
}

// Handler for notifications of changes to the system
void MidiReceiver::notifyProc(const MIDINotification* message, void* refCon)
{
	MidiReceiver& receiver = *(MidiReceiver*) refCon;
	NSUserNotification* notification = [[NSUserNotification alloc] init];

	switch (message->messageID)
	{
		case kMIDIMsgObjectAdded:
		case kMIDIMsgObjectRemoved:
		{
			ItemCount numSourceEndpoints = MIDIGetNumberOfSources();
			notification.title = @"Changed number of MIDI inputs";
			notification.informativeText = [NSString stringWithFormat:@"Now %@ available.",
											numSourceEndpoints  > 1 ? [NSString stringWithFormat:@"%lu inputs are", numSourceEndpoints] :
											numSourceEndpoints == 1 ? @"one input is" : @"no inputs are"];

			// Re-init the midi receiver
			receiver.close();
			receiver.init();
			break;
		}
	}

	// Send pop-up message to notification centre
	[[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
}

// Handler for MIDI packet reception
void MidiReceiver::readProc(const MIDIPacketList* pktlist, void* readProcRefCon, void* srcConnRefCon)
{
	MidiReceiver& receiver = *(MidiReceiver*) readProcRefCon;
	Tracker& tracker = receiver.tracker;
	PPMutex& mutex = receiver.criticalSection;

	for (int i = 0; i < pktlist->numPackets; i++)
	{
		MIDIPacket packet = pktlist->packet[i];
		Byte status = packet.data[0];
		Byte command = status >> 4;

		if (command == 0x09 || command == 0x08)
		{
			Byte note = packet.data[1] & 0x7F;
			Byte velocity = packet.data[2] & 0x7F;

			mutex.lock();
			if (command == 0x09 && velocity)
				tracker.sendNoteDown(note, receiver.recordVelocity ? vol126to255(velocity, receiver.velocityAmplify) : -1);
			else
				tracker.sendNoteUp(note);
			mutex.unlock();
		}
	}
}

PPString MidiReceiver::getMidiDeviceName(MIDIEndpointRef endpointRef)
{
	CFStringRef nameStringRef = NULL;
	MIDIObjectGetStringProperty(endpointRef, kMIDIPropertyName, &nameStringRef);
	CFIndex strLen = CFStringGetLength(nameStringRef);
	CFIndex bufSize = CFStringGetMaximumSizeForEncoding(strLen, kCFStringEncodingUTF8);
	char strBuf[bufSize];
	CFStringGetCString(nameStringRef, strBuf, bufSize, kCFStringEncodingUTF8);
	CFRelease(nameStringRef);
	return PPString(strBuf);
}

bool MidiReceiver::init()
{
	// Create a client
	CORE_MIDI_CHECK(MIDIClientCreate(CFSTR("MilkyTracker"), notifyProc, this, &clientRef), "Failed to create MIDI client!");

	// Create port
	CORE_MIDI_CHECK(MIDIInputPortCreate(clientRef, CFSTR("Input port"), readProc, this, &inputPortRef), "Failed to create input port!");

	// Look for sources
	numSourceEndpoints = MIDIGetNumberOfSources();
	if (!numSourceEndpoints)
	{
		NSLog(@"Core MIDI: Error while trying to get sources!");
		return false;
	}

	if (sourceEndpoints)
	{
		delete[] sourceEndpoints;
		sourceEndpoints = NULL;
	}

	sourceEndpoints = new MidiEndpoint[numSourceEndpoints];

	NSLog(@"Core MIDI: Detected %lu source(s).", numSourceEndpoints);

	for (ItemCount i = 0; i < numSourceEndpoints; i++)
	{
		MIDIEndpointRef endpointRef = MIDIGetSource(i);
		if (!endpointRef)
		{
			NSLog(@"Core MIDI: Error while trying to get source ID %lu!", i);
			return false;
		}

		sourceEndpoints[i].ref = MIDIEndpointRef(endpointRef);
		sourceEndpoints[i].name = getMidiDeviceName(endpointRef);

		NSLog(@"Core MIDI: Endpoint ID: %d, name: %s", endpointRef, sourceEndpoints[i].name.getStrBuffer());

		// Attach endpoint to our input port
		CORE_MIDI_CHECK(MIDIPortConnectSource(inputPortRef, endpointRef, NULL), "Error while attaching endpoint to input port!");
	}

	return true;
}

bool MidiReceiver::close()
{
	if (sourceEndpoints)
	{
		delete[] sourceEndpoints;
		sourceEndpoints = NULL;
	}

	// Release port
	if (inputPortRef)
		CORE_MIDI_CHECK(MIDIPortDispose(inputPortRef), "Failed to dispose of MIDI port!");

	// Release client
	if (clientRef)
		CORE_MIDI_CHECK(MIDIClientDispose(clientRef), "Failed to dispose of MIDI client!");

	return true;
}
