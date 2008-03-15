#ifndef LONG_MSG_H
#define LONG_MSG_H


/*

  LongMsg.h

  CLongMsg class declaration. This class represents a system exclusive 
  MIDI message.


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


#include "MIDIMsg.h"    // For CMIDIMsg base class
#include <exception>    // For std::exception


namespace midi
{
    //-----------------------------------------------------------------
    // CLongMsgIndexOutOfBounds class
    //
    // An exception class. Thrown when an index to a CLongMsg object is
    // out of bounds.
    //-----------------------------------------------------------------

    class CLongMsgIndexOutOfBounds : public std::exception
    {
    public:
        const char *what() const throw()
        { return "Index to CLongMsg object is out of bounds."; } 
    };


    //-----------------------------------------------------------------
    // CLongMsg class
    //
    // This class represents system exclusive messages.
    //-----------------------------------------------------------------

    class CLongMsg : public CMIDIMsg
    {
    public:
        // Constructors/Destructor
        CLongMsg();
        CLongMsg(const char *Msg, DWORD Length);
        CLongMsg(const CLongMsg &Msg);
        virtual ~CLongMsg();

        // Assignment
        CLongMsg &operator = (const CLongMsg &Msg);
        
        // Accessors/Mutators
        DWORD GetLength() const { return m_Length; }
        const char *GetMsg() const { return m_Msg;}
        void SetMsg(const char *Msg, DWORD Length);

    protected:
        // Subscript access. This is for derived classes to use in order
        // to access the individual bytes within a CLongMsg object.
        char &operator [] (int i);

    private:
        char *m_Msg;
        DWORD m_Length;        
    };
}


#endif
