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
 *  SampleLoaderGeneric.cpp
 *  MilkyPlay
 *
 *  Created by Peter Barth on 16.09.05.
 *
 */

#include "SampleLoaderGeneric.h"
#include "SampleLoaderWAV.h"
#include "SampleLoaderIFF.h"
#include "SampleLoaderAIFF.h"
#include "SampleLoaderALL.h"

SampleLoaderGeneric::SampleLoaderGeneric(const SYSCHAR* fileName, XModule& module, bool supportRawLoading/* = true*/) 
	: SampleLoaderAbstract(fileName, module)
	, supportRawLoading(supportRawLoading)
{
}

mp_sint32 SampleLoaderGeneric::getNumChannels()
{
	SampleLoaderAbstract* loader = getSuitableLoader();
	
	if (loader)
	{
		mp_sint32 res = loader->getNumChannels();
		delete loader;
		return res;
	}
	
	return 0;
}
	
const char* SampleLoaderGeneric::getChannelName(mp_sint32 channelIndex)
{
	SampleLoaderAbstract* loader = getSuitableLoader();
	
	if (loader)
	{
		const char* res = loader->getChannelName(channelIndex);
		delete loader;
		return res;
	}
	
	return SampleLoaderAbstract::getChannelName(channelIndex);
}

bool SampleLoaderGeneric::identifySample()
{
	SampleLoaderAbstract* loader = getSuitableLoader();
	
	if (loader)
	{
		delete loader;
		return true;
	}
	
	return false;
}

mp_sint32 SampleLoaderGeneric::loadSample(mp_sint32 index, mp_sint32 channelIndex)
{
	SampleLoaderAbstract* loader = getSuitableLoader();

	if (loader)
	{
		loader->setPreferredDefaultName(this->preferredDefaultName);

		mp_sint32 res = loader->loadSample(index, channelIndex);
		delete loader;
		return res;
	}
	
	return MP_UNSUPPORTED_FORMAT;
}

mp_sint32 SampleLoaderGeneric::saveSample(const SYSCHAR* fileName, mp_sint32 index, OutputFiletypes type)
{
	SampleLoaderAbstract* loader = NULL;	

	switch (type)
	{
		case OutputFiletypeWAV:
			loader = new SampleLoaderWAV(theFileName, theModule);
			break;

		case OutputFiletypeIFF:
			loader = new SampleLoaderIFF(theFileName, theModule);
			break;

		case OutputFiletypeAIFF:
			loader = new SampleLoaderAIFF(theFileName, theModule);
			break;

		case OutputFiletypeRAW:
			if (!supportRawLoading)
				break;
			loader = new SampleLoaderALL(theFileName, theModule);
			break;
	}
	
	if (loader)
	{
		mp_sint32 res = loader->saveSample(fileName, index);
		delete loader;
		return res;
	}
	
	return MP_UNSUPPORTED_FORMAT;
}

SampleLoaderAbstract* SampleLoaderGeneric::getSuitableLoader()
{
	// Try to find WAV first
	SampleLoaderAbstract* loader = new SampleLoaderWAV(theFileName, theModule);
	if (loader && loader->identifySample())
		return loader;
	
	delete loader;

	// Try to find IFF then
	loader = new SampleLoaderIFF(theFileName, theModule);
	if (loader && loader->identifySample())
		return loader;
	
	delete loader;

	// Try to find AIFF then
	loader = new SampleLoaderAIFF(theFileName, theModule);
	if (loader && loader->identifySample())
		return loader;
	
	delete loader;
	
	if (supportRawLoading)
	{
		// Try to find something else
		loader = new SampleLoaderALL(theFileName, theModule);
		if (loader && loader->identifySample())
			return loader;
	}

	delete loader;
	return NULL;
}
