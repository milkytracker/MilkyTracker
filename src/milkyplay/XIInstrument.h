/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

