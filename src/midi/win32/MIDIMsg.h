#ifndef MIDI_MSG_H
#define MIDI_MSG_H


/*

  MIDIMsg.h

  Interface for the CMIDIMsg class. This is the base class for all MIDI
  message classes.


  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 
  USA

  Contact: Leslie Sanford (jabberdabber@hotmail.com)

  Last modified: 12/14/2002

*/


//---------------------------------------------------------------------
// Dependencies
//---------------------------------------------------------------------


#include <windows.h>    // For DWORD data type


//---------------------------------------------------------------------
// Forward declarations
//---------------------------------------------------------------------


namespace midi
{
    class CMIDIOutDevice;
}


namespace midi
{
    //------------------------------------------------------------------
    // CMIDIMsg class
    //
    // This class represents the base class for all MIDI messages.
    //------------------------------------------------------------------

    class CMIDIMsg
    {
    public:
        virtual ~CMIDIMsg() {}

        // Gets the MIDI message length
        virtual DWORD GetLength() const = 0;

        // Gets the MIDI message
        virtual const char *GetMsg() const = 0;

        // Get/Set time stamp
        DWORD GetTimeStamp() const { return m_TimeStamp; }
        void SetTimeStamp(DWORD TimeStamp) { m_TimeStamp = TimeStamp; }

    private:
        DWORD m_TimeStamp;
    };
}


#endif
