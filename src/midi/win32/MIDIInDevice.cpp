/*******************************************************************************
 * MIDIInDevice.cpp - Implementation for CMIDIInDevice and related 
 *                    classes.
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Contact: jabberdabber@hotmail.com
 *
 * Last modified: 09/02/2003
 ******************************************************************************/


//--------------------------------------------------------------------
// Dependencies
//--------------------------------------------------------------------


#include "MIDIInDevice.h"
#include "midi.h"


//--------------------------------------------------------------------
// Using declarations
//--------------------------------------------------------------------


using midi::CMIDIInDevice;
using midi::CMIDIReceiver;
using midi::CMIDIInException;


//--------------------------------------------------------------------
// CMIDIInHeader implementation
//--------------------------------------------------------------------


// Constructor
CMIDIInDevice::CMIDIInHeader::CMIDIInHeader(HMIDIIN DevHandle,
                                            LPSTR Buffer, 
                                            DWORD BufferLength) :
m_DevHandle(DevHandle)
{
    // Initialize header
    m_MIDIHdr.lpData         = Buffer;
    m_MIDIHdr.dwBufferLength = BufferLength;
    m_MIDIHdr.dwFlags        = 0;

    // Prepare header
    MMRESULT Result = ::midiInPrepareHeader(DevHandle, &m_MIDIHdr,
                                            sizeof m_MIDIHdr);

    // If an error occurred, throw exception
    if(Result != MMSYSERR_NOERROR)
    {
        throw CMIDIInException(Result);
    }
}


// Destructor
CMIDIInDevice::CMIDIInHeader::~CMIDIInHeader()
{
    ::midiInUnprepareHeader(m_DevHandle, &m_MIDIHdr, 
                            sizeof m_MIDIHdr);
}


// Add system exclusive buffer to queue
void CMIDIInDevice::CMIDIInHeader::AddSysExBuffer()
{
    MMRESULT Result = ::midiInAddBuffer(m_DevHandle, &m_MIDIHdr,
                                        sizeof m_MIDIHdr);

    // If an error occurred, throw exception
    if(Result != MMSYSERR_NOERROR)
    {
        throw CMIDIInException(Result);
    }
}


//--------------------------------------------------------------------
// CHeaderQueue implementation
//--------------------------------------------------------------------


// Constructor
CMIDIInDevice::CHeaderQueue::CHeaderQueue()
{
    ::InitializeCriticalSection(&m_CriticalSection);
}


// Destructor
CMIDIInDevice::CHeaderQueue::~CHeaderQueue()
{
    RemoveAll();

    ::DeleteCriticalSection(&m_CriticalSection);
}


// Add header to queue
void CMIDIInDevice::CHeaderQueue::AddHeader(
                                 CMIDIInDevice::CMIDIInHeader *Header)
{
    ::EnterCriticalSection(&m_CriticalSection);

    m_HdrQueue.push(Header);

    ::LeaveCriticalSection(&m_CriticalSection);
}


// Remove header from queue
void CMIDIInDevice::CHeaderQueue::RemoveHeader()
{
    ::EnterCriticalSection(&m_CriticalSection);

    if(!m_HdrQueue.empty())
    {
        delete m_HdrQueue.front();
        m_HdrQueue.pop();
    }

    ::LeaveCriticalSection(&m_CriticalSection);
}


// Empty header queue
void CMIDIInDevice::CHeaderQueue::RemoveAll()
{
    ::EnterCriticalSection(&m_CriticalSection);

    while(!m_HdrQueue.empty())
    {
        delete m_HdrQueue.front();
        m_HdrQueue.pop();
    }

    ::LeaveCriticalSection(&m_CriticalSection);
}


// Determines if the header queue is empty
bool CMIDIInDevice::CHeaderQueue::IsEmpty()
{
    bool Result;

    ::EnterCriticalSection(&m_CriticalSection);

    Result = m_HdrQueue.empty();

    ::LeaveCriticalSection(&m_CriticalSection);

    return Result;
}


//--------------------------------------------------------------------
// CMIDIInDevice implementation
//--------------------------------------------------------------------


// Constructs CMIDIInDevice object in an closed state
CMIDIInDevice::CMIDIInDevice() :
m_hThread(NULL),
m_threadID(0),
m_Receiver(0),
m_State(CLOSED)
{
    // If we are unable to create signalling event, throw exception
    if(!CreateEvent())
    {
        throw CMIDIInEventFailure();
    }
}


// Constructs CMIDIInDevice object in an closed state and initializes the 
// MIDI receiver
CMIDIInDevice::CMIDIInDevice(CMIDIReceiver &Receiver) :
m_hThread(NULL),
m_threadID(0),
m_Receiver(&Receiver),
m_State(CLOSED)
{
    // If we are unable to create signalling event, throw exception
    if(!CreateEvent())
    {
        throw CMIDIInEventFailure();
    }
}


// Constructs CMIDIInDevice object in an opened state
CMIDIInDevice::CMIDIInDevice(UINT DeviceId, CMIDIReceiver &Receiver) :
m_hThread(NULL),
m_threadID(0),
m_Receiver(&Receiver),
m_State(CLOSED)
{
    // Open device
    Open(DeviceId);

    // If we are unable to create signalling event, throw exception
    if(!CreateEvent())
    {
        Close();
        throw CMIDIInEventFailure();
    }
}


// Destruction
CMIDIInDevice::~CMIDIInDevice()
{
    // Close device
    Close();

    // Close handle to signalling event
    ::CloseHandle(m_Event);
}


// Opens the MIDI input device
void CMIDIInDevice::Open(UINT DeviceId)
{
    // Makes sure the previous device, if any, is closed before 
    // opening another one
    Close();

    // Open MIDI input device
    MMRESULT Result = ::midiInOpen(&m_DevHandle, DeviceId, 
                                  reinterpret_cast<DWORD_PTR>(MidiInProc),
                                  reinterpret_cast<DWORD_PTR>(this),
                                  CALLBACK_FUNCTION);

    // If we are able to open the device, change state
    if(Result == MMSYSERR_NOERROR)
    {
        m_State = OPENED;
    }
    // Else opening failed, throw exception
    else
    {
        throw CMIDIInException(Result);
    }
}


// Closes the MIDI input device
void CMIDIInDevice::Close()
{
    // If the device is recording, stop recording before closing the 
    // device
    if(m_State == RECORDING)
    {
        StopRecording();
    }

    // If the device is opened...
    if(m_State == OPENED)
    {
        // Close the device
        MMRESULT Result = ::midiInClose(m_DevHandle);

        // If a failure occurred, throw exception
        if(Result != MMSYSERR_NOERROR)
        {
            throw CMIDIInException(Result);
        }

        // Change state
        m_State = CLOSED;
    }
}


// Adds a buffer for receiving system exclusive messages
void CMIDIInDevice::AddSysExBuffer(LPSTR Buffer, DWORD BufferLength)
{
    CMIDIInHeader *Header = NULL;

    try
    {
        // Create new header
        Header = new CMIDIInHeader(m_DevHandle, Buffer, BufferLength);
    }
    // If memory allocation failed, throw exception
    catch(const std::bad_alloc &)
    {
        throw CMIDIInMemFailure();
    }
    // If preparation for the header failed, rethrow exception
    catch(const CMIDIInDevice &)
    {
        throw;
    }

    try
    {
        // Add header to queue
        Header->AddSysExBuffer();
        m_HdrQueue.AddHeader(Header);
    }
    // If we are unable to add the buffer to the queue, delete header
    // and throw exception
    catch(const CMIDIInDevice &)
    {
        delete Header;
        throw;
    }
}


// Starts the recording process
void CMIDIInDevice::StartRecording(ThreadPriority RecordingThreadPriority/* = MIDI_THREAD_PRIORITY_NORMAL*/)
{
	static const int WinThreadPriorities[5] = 
	{
		THREAD_PRIORITY_LOWEST,
		THREAD_PRIORITY_BELOW_NORMAL,
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST
	};

    // Only begin recording if the MIDI input device has been opened
    if(m_State == OPENED)
    { 
        // Change state
        m_State = RECORDING;

        //m_Thread = ::AfxBeginThread((AFX_THREADPROC)HeaderProc, this);
		if (!m_hThread)
		{
			m_hThread = CreateThread(NULL, 
								     0, 
									 &HeaderProc, 
								     (void*)this, 
								     0,
								     &m_threadID);

			if (RecordingThreadPriority >= 0 && 
				RecordingThreadPriority < (signed)sizeof(WinThreadPriorities) / (signed)sizeof(int))
				SetThreadPriority(m_hThread, WinThreadPriorities[RecordingThreadPriority]);
		}		

        // Start recording
        MMRESULT Result = ::midiInStart(m_DevHandle);

        // If recording attempt failed...
        if(Result != MMSYSERR_NOERROR)
        {
            // Revert back to opened state
            m_State = OPENED;

            // Signal the worker thread to finish
            ::SetEvent(m_Event);

			::WaitForSingleObject(m_hThread, INFINITE);

            // Throw exception
            throw CMIDIInException(Result);
        }
    }
}


// Stops the recording process
void CMIDIInDevice::StopRecording()
{
    // If the device is in fact recording...
    if(m_State == RECORDING)
    {
        // Change state
        m_State = OPENED;

        // Signal the worker thread to finish
        ::SetEvent(m_Event);

		::WaitForSingleObject(m_hThread, INFINITE);

        // Reset the MIDI input device
        ::midiInReset(m_DevHandle);

        // Empty header queue
        m_HdrQueue.RemoveAll();
    }
}


// Registers the MIDI receiver. Returns the previous receiver.
CMIDIReceiver *CMIDIInDevice::SetReceiver(CMIDIReceiver &Receiver)
{
    CMIDIReceiver *PrevReceiver = m_Receiver;

    m_Receiver = &Receiver;

    return PrevReceiver;
}


// Determines if the MIDI input device is opened
bool CMIDIInDevice::IsOpen() const
{
    return ((m_State == OPENED) || (m_State == RECORDING));
}


// Determines if the MIDI input device is recording
bool CMIDIInDevice::IsRecording() const
{
    return (m_State == RECORDING);
}


// Gets Id for this device
UINT CMIDIInDevice::GetDevID() const
{
    UINT DeviceID;
    MMRESULT Result = ::midiInGetID(m_DevHandle, &DeviceID);

    if(Result != MMSYSERR_NOERROR)
    {
        throw CMIDIInException(Result);
    }

    return DeviceID;
}


// Gets the capabilities of a particular MIDI input device
void CMIDIInDevice::GetDevCaps(UINT DeviceId, MIDIINCAPS &Caps)
{
    MMRESULT Result = ::midiInGetDevCaps(DeviceId, &Caps, 
                                         sizeof Caps);

    // If we are not able to retrieve device capabilities, throw 
    // exception
    if(Result != MMSYSERR_NOERROR)
    {
        throw CMIDIInException(Result);
    }
}


// Creates event for signalling header thread
bool CMIDIInDevice::CreateEvent()
{
    bool Result = true;

    m_Event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    // If event creation failed, record failure
    if(m_Event == NULL)
    {
        Result = false;
    }

    return Result;
}


// Called by Windows when a MIDI input event occurs
void CALLBACK CMIDIInDevice::MidiInProc(HMIDIIN MidiIn, UINT Msg,
                                        DWORD_PTR Instance, DWORD_PTR Param1,
                                        DWORD_PTR Param2)
{
    CMIDIInDevice *Device;
    
    Device = reinterpret_cast<CMIDIInDevice *>(Instance);

    if(Device->m_Receiver != 0)
    {
        switch(Msg)
        {
        case MIM_DATA:      // Short message received
            Device->m_Receiver->ReceiveMsg(Param1, Param2);
            break;

        case MIM_ERROR:     // Invalid short message received
            Device->m_Receiver->OnError(Param1, Param2);
            break;

        case MIM_LONGDATA:  // System exclusive message received
            if(Device->m_State == RECORDING)
            {
                // Retrieve data, send it to receiver, and notify header
                // thread that we are done with the system exclusive 
                // message
                MIDIHDR *MidiHdr = reinterpret_cast<MIDIHDR *>(Param1);
                Device->m_Receiver->ReceiveMsg(MidiHdr->lpData, 
                                               MidiHdr->dwBytesRecorded, 
                                               Param2);
                ::SetEvent(Device->m_Event);
            }
            break;

        case MIM_LONGERROR: // Invalid system exclusive message received
            if(Device->m_State == RECORDING)
            {
                // Retrieve data, send it to receiver, and notify header
                // thread that we are done with the system exclusive 
                // message
                MIDIHDR *MidiHdr = reinterpret_cast<MIDIHDR *>(Param1);
                Device->m_Receiver->OnError(MidiHdr->lpData,
                                            MidiHdr->dwBytesRecorded,
                                            Param2);
                ::SetEvent(Device->m_Event);
            }
            break;
        }
    }
}


// Header worker thread
DWORD CMIDIInDevice::HeaderProc(LPVOID Parameter)
{
    CMIDIInDevice *Device; 
    
    Device = reinterpret_cast<CMIDIInDevice *>(Parameter);

    // Continue while the MIDI input device is recording
    while(Device->m_State == RECORDING)
    {
        ::WaitForSingleObject(Device->m_Event, INFINITE);

        // Make sure we are still recording
        if(Device->m_State == RECORDING)
        {
            // Remove the finished header
            Device->m_HdrQueue.RemoveHeader();
        }
    }

    return 0;
}
