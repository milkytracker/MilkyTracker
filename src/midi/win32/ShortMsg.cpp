/*

  ShortMsg.cpp

  Implementation for the CShortMsg class


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


#include "midi.h"
#include "ShortMsg.h"
#include "MIDIInDevice.h"


// Using declaration
using midi::CShortMsg;


//---------------------------------------------------------------------
// CShortMsg implementation
//---------------------------------------------------------------------


// Constructor
CShortMsg::CShortMsg(DWORD TimeStamp) :
m_Msg(0),
m_MsgNoStatus(0)
{
    SetTimeStamp(TimeStamp);
}


// Constructor
CShortMsg::CShortMsg(DWORD Msg, DWORD TimeStamp) : 
m_Msg(Msg)
{
    unsigned char DataByte1 = GetData1();
    unsigned char DataByte2 = GetData2();

    m_MsgNoStatus = PackShortMsg(DataByte1, DataByte2);

    SetTimeStamp(TimeStamp);
}


// Constructor
CShortMsg::CShortMsg(unsigned char Status, unsigned char Data1, 
                     unsigned char Data2, DWORD TimeStamp)
{
    SetMsg(Status, Data1, Data2);
    m_MsgNoStatus = PackShortMsg(Data1, Data2);

    SetTimeStamp(TimeStamp);
}


// Constructor
CShortMsg::CShortMsg(unsigned char Command, unsigned char Channel, 
                     unsigned char Data1, unsigned char Data2,
                     DWORD TimeStamp)
{
    SetMsg(Command, Channel, Data1, Data2);
    m_MsgNoStatus = PackShortMsg(Data1, Data2);

    SetTimeStamp(TimeStamp);
}


// Gets message
const char *CShortMsg::GetMsg() const
{
    return reinterpret_cast<const char *>(&m_Msg);
}


// Gets status byte
unsigned char CShortMsg::GetStatus() const
{
    unsigned char Status;
    unsigned char Dummy;

    UnpackShortMsg(m_Msg, Status, Dummy, Dummy);

    return Status;
}


// Gets MIDI channel
unsigned char CShortMsg::GetChannel() const
{
    unsigned char Channel;
    unsigned char Dummy;

    UnpackShortMsg(m_Msg, Dummy, Channel, Dummy, Dummy);

    return Channel;
}


// Gets command code
unsigned char CShortMsg::GetCommand() const
{
    unsigned char Command;
    unsigned char Dummy;

    UnpackShortMsg(m_Msg, Command, Dummy, Dummy, Dummy);

    return Command;
}


// Gets data byte 1
unsigned char CShortMsg::GetData1() const
{
    unsigned char Data1;
    unsigned char Dummy;

    UnpackShortMsg(m_Msg, Dummy, Dummy, Data1, Dummy);

    return Data1;
}


// Gets data byte 2
unsigned char CShortMsg::GetData2() const
{
    unsigned char Data2;
    unsigned char Dummy;

    UnpackShortMsg(m_Msg, Dummy, Dummy, Dummy, Data2);

    return Data2;
}


// Sets message
void CShortMsg::SetMsg(unsigned char Status, unsigned char Data1,
                       unsigned char Data2)
{
    m_Msg = PackShortMsg(Status, Data1, Data2);
    m_MsgNoStatus = PackShortMsg(Data1, Data2);
}


// Sets message
void CShortMsg::SetMsg(unsigned char Command, unsigned char Channel,
                       unsigned char Data1, unsigned char Data2)
{
    m_Msg = PackShortMsg(Command, Channel, Data1, Data2);
    m_MsgNoStatus = PackShortMsg(Data1, Data2);
}


// Packs data into short message without status byte
DWORD CShortMsg::PackShortMsg(unsigned char DataByte1,
                              unsigned char DataByte2)
{
    DWORD Msg = DataByte1;
    Msg |= DataByte2 << midi::SHORT_MSG_SHIFT;

    return Msg;
}


// Packs data into short message with status byte
DWORD CShortMsg::PackShortMsg(unsigned char Status,
                              unsigned char DataByte1,
                              unsigned char DataByte2)
{
    DWORD Msg = Status;
    Msg |= DataByte1 << midi::SHORT_MSG_SHIFT;
    Msg |= DataByte2 << midi::SHORT_MSG_SHIFT * 2;

    return Msg;
}


// Packs data into short channel message
DWORD CShortMsg::PackShortMsg(unsigned char Command,
                              unsigned char Channel,
                              unsigned char DataByte1,
                              unsigned char DataByte2)
{
    DWORD Msg = Command | Channel;
    Msg |= DataByte1 << midi::SHORT_MSG_SHIFT;
    Msg |= DataByte2 << midi::SHORT_MSG_SHIFT * 2;

    return Msg;
}


// Unpacks short message
void CShortMsg::UnpackShortMsg(DWORD Msg, unsigned char &Status,
                               unsigned char &DataByte1,
                               unsigned char &DataByte2)
{
    Status = static_cast<unsigned char>(Msg);
    DataByte1 = static_cast<unsigned char>
                                   (Msg >> midi::SHORT_MSG_SHIFT);
    DataByte2 = static_cast<unsigned char>
                                   (Msg >> midi::SHORT_MSG_SHIFT * 2);
}


// Unpacks short channel message
void CShortMsg::UnpackShortMsg(DWORD Msg, unsigned char &Command,
                               unsigned char &Channel,
                               unsigned char &DataByte1,
                               unsigned char &DataByte2)
{
    Command = static_cast<unsigned char>(Msg & ~midi::SHORT_MSG_MASK);
    Channel = static_cast<unsigned char>(Msg & midi::SHORT_MSG_MASK);
    DataByte1 = static_cast<unsigned char>
                                   (Msg >> midi::SHORT_MSG_SHIFT);
    DataByte2 = static_cast<unsigned char>
                                   (Msg >> midi::SHORT_MSG_SHIFT * 2);
}
