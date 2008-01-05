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

