/*
 *  milkyplay/XIInstrument.h
 *
 *  Copyright 2009 Peter Barth
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

/*
 *  XIInstrument.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 07.03.05.
 *
 */
#ifndef XIINSTRUMENT__H
#define XIINSTRUMENT__H

#include "XModule.h"
#include <stdio.h>

class XIInstrument
{
private:
	bool owner;

public:
	mp_ubyte sig[21];
	mp_ubyte name[22];
	mp_ubyte tracker[20];

	mp_ubyte nbu[96];
	
	TEnvelope venv;
	TEnvelope penv;

	mp_ubyte vibtype, vibsweep, vibdepth, vibrate;
	mp_uword volfade;
	
	mp_uword res;
	
	mp_ubyte extra[20];
	
	mp_uword numsamples;
	TXMSample samples[16];

public:
	XIInstrument();
	XIInstrument(const XIInstrument& src);
	~XIInstrument();
	
	void clean();
	
	// Load instrument, .XI by default, otherwise GUS Patch (.PAT)
	mp_sint32 load(const SYSCHAR* pszFile);

	// Load GUS Patch instrument (.PAT)
	mp_sint32 loadPAT(XMFile& f);

	mp_sint32 save(const SYSCHAR* pszFile);

	// assignment operator
	XIInstrument& operator=(const XIInstrument& src);
};

#endif

