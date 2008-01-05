/*
 *  XIInstrument.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 07.03.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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

