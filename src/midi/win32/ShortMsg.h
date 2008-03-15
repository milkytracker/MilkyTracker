#ifndef SHORT_MSG_H
#define SHORT_MSG_H


/*

  ShortMsg.h

  CShortMsg class declaration. This class represents short MIDI messages.


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

  Last modified: 08/19/2003

*/


//---------------------------------------------------------------------
// Dependencies
//---------------------------------------------------------------------


#include "MIDIMsg.h"    // For CMIDIMsg base class


namespace midi
{
    // Constants
    const DWORD SHORT_MSG_LENGTH = 3;


    //-----------------------------------------------------------------
    // CShortMsg class
    //
    // This class represents short MIDI messages.
    //-----------------------------------------------------------------

    class CShortMsg : public CMIDIMsg
    {
    public:
        // Constructors
        explicit CShortMsg(DWORD TimeStamp = 0);
        CShortMsg(DWORD Msg, DWORD TimeStamp = 0);
        CShortMsg(unsigned char Status, unsigned char Data1,
            unsigned char Data2, DWORD TimeStamp);
        CShortMsg(unsigned char Command, unsigned char Channel,
            unsigned char Data1, unsigned char Data2, 
            DWORD TimeStamp);

        //
        // Accessors
        // 

        DWORD GetLength() const
        { return midi::SHORT_MSG_LENGTH; }
        const char *GetMsg() const;
        unsigned char GetStatus() const;
        unsigned char GetChannel() const;
        unsigned char GetCommand() const;
        unsigned char GetData1() const;
        unsigned char GetData2() const;

        //
        // Mutators
        //

        void SetMsg(unsigned char Status, unsigned char Data1,
            unsigned char Data2);
        void SetMsg(unsigned char Command, unsigned char Channel,
            unsigned char Data1, unsigned char Data2);

        //
        // Class methods
        //


        // Packs short messages without status byte
        static DWORD PackShortMsg(unsigned char DataByte1,
                                  unsigned char DataByte2);


        // Packs short messages with status byte
        static DWORD PackShortMsg(unsigned char Status,
                                  unsigned char DataByte1,
                                  unsigned char DataByte2);

        // Packs short channel messages
        static DWORD PackShortMsg(unsigned char Command,
                                  unsigned char Channel,
                                  unsigned char DataByte1,
                                  unsigned char DataByte2);

        // Unpacks short messages
        static void UnpackShortMsg(DWORD Msg, unsigned char &Status,
                                   unsigned char &DataByte1,
                                   unsigned char &DataByte2);

        // Unpacks short channel messages
        static void UnpackShortMsg(DWORD Msg, unsigned char &Command,
                                   unsigned char &Channel,
                                   unsigned char &DataByte1,
                                   unsigned char &DataByte2);

    private:
        DWORD m_Msg;
        DWORD m_MsgNoStatus;
    };
}


#endif
