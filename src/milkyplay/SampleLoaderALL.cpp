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
 *  SampleLoaderALL.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 14.09.05.
 *
 */

#include "SampleLoaderALL.h"
#include "XMFile.h"
#include "XModule.h"

SampleLoaderALL::SampleLoaderALL(const SYSCHAR* fileName, XModule& theModule) :
	SampleLoaderAbstract(fileName, theModule)
{
}

bool SampleLoaderALL::identifySample()
{
	return getNumChannels() != 0;
}

mp_sint32 SampleLoaderALL::getNumChannels()
{
	return 1;
}
	
mp_sint32 SampleLoaderALL::loadSample(mp_sint32 index, mp_sint32 channelIndex)
{
	XMFile f(theFileName);
				
	TXMSample* smp = &theModule.smp[index];

	if (smp->sample)
	{
		theModule.freeSampleMem((mp_ubyte*)smp->sample);
		smp->sample = NULL;
	}
	
	smp->samplen = f.size();
	
	// Bigger than 100mb? Probably someone's kidding here...
	if ((unsigned)smp->samplen > 100*1024*1024)
		smp->samplen = 100*1024*1024;
	
	smp->sample = (mp_sbyte*)theModule.allocSampleMem(smp->samplen);
	
	if (smp->sample == NULL)
		return MP_OUT_OF_MEMORY;

	smp->type = 0;
	smp->flags = 3;
	smp->looplen = smp->loopstart = 0;

	nameToSample(preferredDefaultName, smp);
	
	// assuming signed data
	theModule.loadSample(f, smp->sample, smp->samplen, smp->samplen);
	return MP_OK;
}

mp_sint32 SampleLoaderALL::saveSample(const SYSCHAR* fileName, mp_sint32 index)
{
	TXMSample* smp = &theModule.smp[index];

	XMFile f(fileName, true);

	if (smp->type & 16)
	{
		for (mp_uint32 i = 0; i < smp->samplen; i++)
			f.writeWord(smp->getSampleValue(i));
	}
	else
	{
		for (mp_uint32 i = 0; i < smp->samplen; i++)
			f.writeByte(smp->getSampleValue(i));
	}
	return MP_OK;
}
