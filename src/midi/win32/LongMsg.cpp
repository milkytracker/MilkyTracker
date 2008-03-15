/*

  CLongMsg.cpp

  Implementation for the CLongMsg class.


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


#include "LongMsg.h"


// Using declaration
using midi::CLongMsg;


//---------------------------------------------------------------------
// CLongMsg implementation
//---------------------------------------------------------------------


// Default constructor
CLongMsg::CLongMsg() :
m_Msg(0),
m_Length(0)
{}


// Constructor
CLongMsg::CLongMsg(const char *Msg, DWORD Length) :
m_Msg(0),
m_Length(0)
{
    SetMsg(Msg, Length);
}


// Constructor
CLongMsg::CLongMsg(const CLongMsg &Msg) : CMIDIMsg(Msg)
{
    m_Msg = 0;
    m_Length = 0;

    *this = Msg;
}


// Destructor
CLongMsg::~CLongMsg()
{
    // Release resources for this object if they exist
    if(m_Msg != 0)
    {
        delete [] m_Msg;
    }
}


// Assignment
CLongMsg &CLongMsg::operator = (const CLongMsg &Msg)
{
    // Test for self assignment
    if(this != &Msg)
    {
        SetMsg(Msg.m_Msg, Msg.m_Length);
    }

    return *this;
}


// Sets message
void CLongMsg::SetMsg(const char *Msg, DWORD Length)
{
    // Release old message if it exists
    if(m_Msg != 0)
    {
        delete [] m_Msg;
    }

    // 
    // Allocate and initialize new message
    //

    m_Msg = new char[Length];
    m_Length = Length;

    for(DWORD i = 0; i < m_Length; i++)
    {
        m_Msg[i] = Msg[i];
    }
}


// Subscripting
char &CLongMsg::operator [] (int i)
{
    // Bounds checking
    if((signed)m_Length == 0 || i < 0 || i >= (signed)m_Length)
    {
        throw CLongMsgIndexOutOfBounds();
    }

    return m_Msg[i];
}
