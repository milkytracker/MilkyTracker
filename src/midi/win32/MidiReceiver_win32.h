/*
 *  midi/win32/MidiReceiver_win32.h
 *
 *  Copyright 2008 Peter Barth
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

#ifndef __MIDIRECEIVER_H__
#define __MIDIRECEIVER_H__

#include "MIDIInDevice.h"

class Tracker;
class PPMutex;

class MidiReceiver : public midi::CMIDIReceiver
{
private:
	Tracker& tracker;
	PPMutex& criticalSection;
	bool recordVelocity;
	int velocityAmplify;

public:
	MidiReceiver(Tracker& theTracker, PPMutex& mutex);

	void setRecordVelocity(bool recVelocity) { recordVelocity = recVelocity; }
	void setVelocityAmplify(int amplify) { velocityAmplify = amplify; } 

    // Receives short messages
    virtual void ReceiveMsg(DWORD Msg, DWORD TimeStamp);

    // Receives long messages
    virtual void ReceiveMsg(LPSTR Msg, DWORD BytesRecorded, DWORD TimeStamp);

    // Called when an invalid short message is received
    virtual void OnError(DWORD Msg, DWORD TimeStamp);

    // Called when an invalid long message is received
    virtual void OnError(LPSTR Msg, DWORD BytesRecorded, DWORD TimeStamp);
};

#endif

