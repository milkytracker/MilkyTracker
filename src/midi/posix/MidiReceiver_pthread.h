/*
 *  midi/posix/MidiReceiver_pthread.h
 *
 *  Copyright 2009 Peter Barth
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

/*
 *  MidiReceiver.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 08.04.06.
 *
 */

#ifndef __MIDIRECEIVER_H__
#define __MIDIRECEIVER_H__

#include <vector>

class RtMidiIn;

class MidiReceiver
{
public:
	class MidiEventHandler
	{
	public:
		virtual void keyDown(int note, int volume) = 0;
		virtual void keyUp(int note) = 0;
	};

private:
	MidiEventHandler& midiEventHandler;
	RtMidiIn* midiin;
	bool recordVelocity;
	int velocityAmplify;
	
	std::vector<unsigned char> message;
	
	static void receiverCallback(double deltatime, std::vector<unsigned char>* message, void* userData);
	
	void processMessage(double deltatime, std::vector<unsigned char>* message, int offset);
	
public:
	MidiReceiver(MidiEventHandler& midiEventHandler);
	~MidiReceiver();

	void setRecordVelocity(bool recVelocity) { recordVelocity = recVelocity; }
	void setVelocityAmplify(int amplify) { velocityAmplify = amplify; } 

	bool startRecording(unsigned int deviceID);
	void stopRecording();	
};

#endif

