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
 *  SampleLoaderSF2.h
 *  MilkyPlay
 *
 *  Created by Peter Barth on 14.09.05.
 *
 */

#ifndef SAMPLELOADERSF2__H
#define SAMPLELOADERSF2__H

#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "SampleLoaderAbstract.h"

class SampleLoaderSF2 : public SampleLoaderAbstract
{
private:

	char item[6];
	char string[512];
	int inputchar;
	FILE *inp = NULL, *outp = NULL;

	void getstring();
	unsigned long bit32conv();
	void put32 (unsigned long d);
	unsigned long bit32convl (FILE * inp);
	void getitem ();

public:
	static char lastSF2File[255];
	static int lastSF2FileSamples;

	SampleLoaderSF2(const SYSCHAR* fileName, XModule& theModule);

	virtual bool identifySample();

	virtual mp_sint32 loadSample(mp_sint32 index, mp_sint32 channelIndex);
	mp_sint32 loadSampleIndex(mp_sint32 index, mp_sint32 channelIndex, int sampleIndex);
	mp_sint32 loadSampleIndexToSample( TXMSample *smp, mp_sint32 channelIndex, int sampleIndex);

	~SampleLoaderSF2();
};

#endif

