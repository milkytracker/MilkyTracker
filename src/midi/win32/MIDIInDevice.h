#ifndef MIDI_IN_DEVICE_H
#define MIDI_IN_DEVICE_H


/*******************************************************************************
 * MIDIInDevice.h - Interface for CMIDIInDevice and related classes.
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
 * Last modified: 07/30/2003
 *
 * Note: You must link to the winmm.lib to use these classes.
 ******************************************************************************/


#pragma warning(disable:4786) // Disable annoying template warnings


//--------------------------------------------------------------------
// Dependencies
//--------------------------------------------------------------------


// Necessary for Windows data types
#include <windows.h>
#include <mmsystem.h>
#ifdef _UNICODE
#include <tchar.h>
#endif

// Necessary for exception classes derived from std::exception
#include <exception> 

// Necessary for header queue used by CHeaderQueue
#include <queue>


namespace midi
{
	//----------------------------------------------------------------
    // UnicodeToAnsi converts the Unicode string pszW to an ANSI string
	// and returns the ANSI string through ppszA. Space for the
    // the converted string is allocated by UnicodeToAnsi.
	//----------------------------------------------------------------

	static HRESULT __fastcall UnicodeToAnsi(LPCWSTR pszW, LPSTR pszA)
	{

		ULONG cbAnsi, cCharacters;
		DWORD dwError;

		// If input is null then just return the same.
		if (pszW == NULL)
		{
			strcpy(pszA, "");
			return NOERROR;
		}

		cCharacters = wcslen(pszW)+1;
		// Determine number of bytes to be allocated for ANSI string. An
		// ANSI string can have at most 2 bytes per character (for Double
		// Byte Character Strings.)
		cbAnsi = cCharacters*2;

		// Convert to ANSI.
		if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, pszA,
			cbAnsi, NULL, NULL))
		{
			dwError = GetLastError();
			strcpy(pszA, "");
			return HRESULT_FROM_WIN32(dwError);
		}

		return NOERROR;
	}    
	
	//----------------------------------------------------------------
    // Class declarations
    //----------------------------------------------------------------


    //----------------------------------------------------------------
    // CMIDIReceiver
    //
    // An abstract class representing objects that receive messages
    // from CMIDIInDevice objects. A CMIDIReceiver object registers
    // itself with a CMIDIInDevice object. It will then receive all
    // messages that are recorded.
    //----------------------------------------------------------------


    class CMIDIReceiver
    {
    public:
        virtual ~CMIDIReceiver() {}

        // Receives short messages
        virtual void ReceiveMsg(DWORD Msg, DWORD TimeStamp) = 0;

        // Receives long messages
        virtual void ReceiveMsg(LPSTR Msg, DWORD BytesRecorded,
                                DWORD TimeStamp) = 0;

        // Called when an invalid short message is received
        virtual void OnError(DWORD Msg, DWORD TimeStamp) = 0;

        // Called when an invalid long message is received
        virtual void OnError(LPSTR Msg, DWORD BytesRecorded,
                             DWORD TimeStamp) = 0;
    };


    //----------------------------------------------------------------
    // CMIDIInDevice exception classes
    //----------------------------------------------------------------


    // Encapsulates the midiInGetErrorText messages
    class CMIDIInException : public std::exception
    {
    public:
        CMIDIInException(MMRESULT ErrCode) throw()
        { 
#ifdef _UNICODE
			::midiInGetErrorText(ErrCode, m_ErrMsgW, sizeof m_ErrMsgW); 
			UnicodeToAnsi(m_ErrMsgW, m_ErrMsg);
#else
			::midiInGetErrorText(ErrCode, m_ErrMsg, sizeof m_ErrMsg); 
#endif
		}

        const char *what() const throw() { return m_ErrMsg; }

    private:
#ifdef _UNICODE
		WCHAR m_ErrMsgW[128];
        char m_ErrMsg[256];
#else
        char m_ErrMsg[128];
#endif
	};


    // Thrown when memory allocation fails within a CMIDIInDevice 
    // object
    class CMIDIInMemFailure : public std::bad_alloc
    {
    public:
        const char *what() const throw()
        { return "Memory allocation within a CMIDIInDevice object "
                 "failed."; }
    };


    // Thrown when a CMIDIInDevice is unable to create a signalling 
    // event
    class CMIDIInEventFailure : public std::exception
    {
    public:
        const char *what() const throw()
        { return "Unable to create a signalling event for "
                 "CMIDIInDevice object."; }
    };


    // Thrown when a CMIDIInDevice is unable to create a worker thread
    class CMIDIInThreadFailure : public std::exception
    {
    public:
        const char *what() const throw()
        { return "Unable to create worker thread for CMIDIInDevice "
                 "object."; }
    };


    //----------------------------------------------------------------
    // CMIDIInDevice
    //
    // This class represents MIDI input devices. 
    //----------------------------------------------------------------


    class CMIDIInDevice
    {
    public:
		enum ThreadPriority
		{
			MIDI_THREAD_PRIORITY_LOWEST			= 0,
			MIDI_THREAD_PRIORITY_BELOW_NORMAL	= 1,
			MIDI_THREAD_PRIORITY_NORMAL			= 2,
			MIDI_THREAD_PRIORITY_ABOVE_NORMAL	= 3,
			MIDI_THREAD_PRIORITY_HIGHEST		= 4
		};

        // For constructing a CMIDIInDevice object in an closed state
        CMIDIInDevice();

        // For constructing a CMIDIInDevice object in an closed state and
        // initializing the MIDI receiver
        CMIDIInDevice(CMIDIReceiver &Receiver);

        // For constructing a CMIDIInDevice object in an opened state
        CMIDIInDevice(UINT DeviceId, CMIDIReceiver &Receiver);

        // Destruction
        ~CMIDIInDevice();

        // Opens the MIDI input device
        void Open(UINT DeviceId);

        // Closes the MIDI input device
        void Close();

        // Adds a buffer to receive system exclusive messages
        void AddSysExBuffer(LPSTR Buffer, DWORD BufferLength);

        // Starts the recording process
        void StartRecording(ThreadPriority RecordingThreadPriority = MIDI_THREAD_PRIORITY_NORMAL);

        // Stops the recording process
        void StopRecording();

        // Sets the current MIDI receiver. Returns the previous 
        // receiver.
        CMIDIReceiver *SetReceiver(CMIDIReceiver &Receiver);

        // Returns true if the device is open
        bool IsOpen() const;

        // Returns true if the device is recording
        bool IsRecording() const;

        // Gets Id for this device
        UINT GetDevID() const;

        // Gets the number of MIDI input devices on this system
        static UINT GetNumDevs() { return midiInGetNumDevs(); }

        // Gets the capabilities of a particular MIDI input device
        // The results are stored in the MIDIINCAPS parameter.
        static void GetDevCaps(UINT DeviceId, MIDIINCAPS &Caps);

    // Private methods
    private:
        // Copying and assignment not allowed
        CMIDIInDevice(const CMIDIInDevice &);
        CMIDIInDevice &operator = (const CMIDIInDevice &);

        // Creates an event for signalling the header thread
        bool CreateEvent();

        // Called by Windows when a MIDI input event occurs
        static void CALLBACK MidiInProc(HMIDIIN MidiIn, UINT Msg,
                                        DWORD_PTR Instance, DWORD_PTR Param1, 
                                        DWORD_PTR Param2);

        // Thread function for managing headers
        static DWORD WINAPI HeaderProc(LPVOID Parameter);

    // Private class declarations
    private:
        // Encapsulates the MIDIHDR structure for MIDI input
        class CMIDIInHeader
        {
        public:
            CMIDIInHeader(HMIDIIN DevHandle, LPSTR Buffer, 
                          DWORD BufferLength);
            ~CMIDIInHeader();

            // Add the buffer for receiving system exclusive messages
            void AddSysExBuffer();

        private:
            HMIDIIN m_DevHandle;
            MIDIHDR m_MIDIHdr;
        };


        // Thread safe queue for storing CMIDIInHeader objects
        class CHeaderQueue
        {
        public:
            CHeaderQueue();
            ~CHeaderQueue();

            void AddHeader(CMIDIInHeader *Header);
            void RemoveHeader();
            void RemoveAll();
            bool IsEmpty();

        private:
            std::queue<CMIDIInHeader *> m_HdrQueue;
            CRITICAL_SECTION m_CriticalSection;
        };

    // Private attributes and constants
    private:
        HMIDIIN        m_DevHandle;
        HANDLE         m_Event;
		//CWinThread    *m_Thread;
        
		HANDLE			m_hThread;
		DWORD			m_threadID;		
		
		CMIDIReceiver *m_Receiver;
        CHeaderQueue   m_HdrQueue;
        enum State { CLOSED, OPENED, RECORDING } m_State;
    };
}


#endif
