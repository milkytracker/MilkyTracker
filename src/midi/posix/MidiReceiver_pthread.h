/*
 *  MidiReceiver.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.04.06.
 *  Copyright (c) 2006 milkytracker.net. All rights reserved.
 *
 */

#ifndef __MIDIRECEIVER_H__
#define __MIDIRECEIVER_H__

#include <vector>

class Tracker;
class PPMutex;
class RtMidiIn;

class MidiReceiver
{
private:
	Tracker& tracker;
	PPMutex& criticalSectionMutex;
	RtMidiIn* midiin;
	bool recordVelocity;
	int velocityAmplify;
	
	std::vector<unsigned char> message;
	
	static void receiverCallback(double deltatime, std::vector<unsigned char>* message, void* userData);
	
	void processMessage(double deltatime, std::vector<unsigned char>* message, int offset);
	
public:
	MidiReceiver(Tracker& theTracker, PPMutex& theCriticalSectionMutex);
	~MidiReceiver();

	void setRecordVelocity(bool recVelocity) { recordVelocity = recVelocity; }
	void setVelocityAmplify(int amplify) { velocityAmplify = amplify; } 

	bool startRecording(unsigned int deviceID);
	void stopRecording();	
};

#endif

