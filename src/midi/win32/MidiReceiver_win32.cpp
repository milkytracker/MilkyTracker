#include "MidiReceiver_win32.h"
#include <iostream>
#include "ShortMsg.h"
#include "Tracker.h"
#include "PPMutex.h"
#include "MidiTools.h"

using midi::CMIDIInDevice;
using midi::CMIDIReceiver;

MidiReceiver::MidiReceiver(Tracker& theTracker, PPMutex& mutex) :
	tracker(theTracker),
	criticalSection(mutex),
	recordVelocity(false),
	velocityAmplify(100)
{
}

// Function called to receive short messages
void MidiReceiver::ReceiveMsg(DWORD Msg, DWORD TimeStamp)
{
    midi::CShortMsg ShortMsg(Msg, TimeStamp);

	int command = static_cast<int>(ShortMsg.GetCommand());

	if (command >= 128 && command <= 143)
	{
		int note = static_cast<int>(ShortMsg.GetData1());
		// One octave less
		note -= 12;
		if (note < 1) note = 1;

		criticalSection.lock();
		tracker.sendNoteUp(note);
		criticalSection.unlock();
	}
	else if (command >= 144 && command <= 159)
	{
		int note = static_cast<int>(ShortMsg.GetData1());
		// One octave less
		note -= 12;
		if (note < 1) note = 1;

		int velocity = static_cast<int>(ShortMsg.GetData2());

		if (velocity == 0)
		{
			criticalSection.lock();
			tracker.sendNoteUp(note);
			criticalSection.unlock();
		}
		else
		{
			criticalSection.lock();
			tracker.sendNoteDown(note, recordVelocity ? vol126to255(velocity-1, velocityAmplify) : -1);
			criticalSection.unlock();
		}
	}

    /*std::cout << "Command: " << static_cast<int>(ShortMsg.GetCommand());
    std::cout << "\nChannel: " << static_cast<int>(ShortMsg.GetChannel());
    std::cout << "\nDataByte1: " << static_cast<int>(ShortMsg.GetData1());
    std::cout << "\nDataByte2: " << static_cast<int>(ShortMsg.GetData2());
    std::cout << "\nTimeStamp: " << static_cast<int>(ShortMsg.GetTimeStamp());
    std::cout << "\n\n";*/
}

// Receives long messages
void MidiReceiver::ReceiveMsg(LPSTR Msg, DWORD BytesRecorded, DWORD TimeStamp)
{
}

// Called when an invalid short message is received
void MidiReceiver::OnError(DWORD Msg, DWORD TimeStamp)
{
}

// Called when an invalid long message is received
void MidiReceiver::OnError(LPSTR Msg, DWORD BytesRecorded, DWORD TimeStamp)
{
}
