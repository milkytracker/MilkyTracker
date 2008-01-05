/*
 *  MidiReceiver.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.04.06.
 *  Copyright (c) 2006 milkytracker.net. All rights reserved.
 *
 */

#include "MidiReceiver_pthread.h"
#include "Tracker.h"
#include "PPMutex.h"
#include "RtMidi.h"
#include "RtError.h"
#include "PPSystem.h"
#include <iostream>
#include "MidiTools.h"

MidiReceiver::MidiReceiver(Tracker& theTracker, PPMutex& theCriticalSectionMutex) :
	tracker(theTracker),
	criticalSectionMutex(theCriticalSectionMutex),
	midiin(NULL),
	recordVelocity(false),
	velocityAmplify(100)
{
}

MidiReceiver::~MidiReceiver()
{
	if (midiin)
		delete midiin;
}

bool MidiReceiver::startRecording(unsigned int deviceID)
{
	if (midiin)
	{
		delete midiin;
		midiin = NULL;
	}
	
	try 
	{
		midiin = new RtMidiIn();
		midiin->openPort(deviceID);
	}
	catch (RtError &error) 
	{
		error.printMessage();
		goto cleanup;
	}
	
	// Set our callback function.  This should be done immediately after
	// opening the port to avoid having incoming messages written to the
	// queue instead of sent to the callback function.
	midiin->setCallback(&receiverCallback, this);
	
	// ignore sysex, timing, or active sensing messages.
	midiin->ignoreTypes(true, true, true);
	
	return true;
	
cleanup:
	delete midiin;
	midiin = NULL;	
	return false;
}

void MidiReceiver::stopRecording()
{
	delete midiin;
	midiin = NULL;
}

void MidiReceiver::processMessage(double deltatime, std::vector<unsigned char>* message, int offset)
{
	int command = static_cast<int>(message->at(offset));
	
	if (command >= 128 && command <= 143)
	{
		int note = static_cast<int>(message->at(offset+1));
		// One octave less
		note -= 12;
		if (note < 1) note = 1;
		
		criticalSectionMutex.lock();
		tracker.sendNoteUp(note);
		criticalSectionMutex.unlock();
	}
	else if (command >= 144 && command <= 159)
	{
		int note = static_cast<int>(message->at(offset+1));
		// One octave less
		note -= 12;
		if (note < 1) note = 1;
		
		int velocity = static_cast<int>(message->at(offset+2));
		
		if (velocity == 0)
		{
			criticalSectionMutex.lock();
			tracker.sendNoteUp(note);
			criticalSectionMutex.unlock();
		}
		else
		{
			criticalSectionMutex.lock();
			tracker.sendNoteDown(note, recordVelocity ? vol126to255(velocity-1, velocityAmplify) : -1);
			criticalSectionMutex.unlock();
		}
	}	
}

void MidiReceiver::receiverCallback(double deltatime, std::vector<unsigned char>* message, void* userData)
{
	MidiReceiver* midiReceiver = reinterpret_cast<MidiReceiver*>(userData);
	
	if (message->size() == 3)
	{
		midiReceiver->processMessage(deltatime, message, 0);
	}
}

