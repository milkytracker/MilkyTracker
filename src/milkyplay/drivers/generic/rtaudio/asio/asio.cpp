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
extern "C"
{

long main()
{
	return 'ASIO';
}

#elif WINDOWS

#include "windows.h"
#include "iasiodrv.h"
#include "asiodrivers.h"

IASIO *theAsioDriver = 0;
extern AsioDrivers *asioDrivers;

#elif SGI || SUN || BEOS || LINUX
#include "asiodrvr.h"
static AsioDriver *theAsioDriver = 0;
#endif

//-----------------------------------------------------------------------------------------------------
ASIOError ASIOInit(ASIODriverInfo *info)
{
#if MAC || SGI || SUN || BEOS || LINUX
	if(theAsioDriver)
	{
		delete theAsioDriver;
		theAsioDriver = 0;
	}		
	info->driverVersion = 0;
	strcpy(info->name, "No ASIO Driver");
	theAsioDriver = getDriver();
	if(!theAsioDriver)
	{
		strcpy(info->errorMessage, "Not enough memory for the ASIO driver!"); 
		return ASE_NotPresent;
	}
	if(!theAsioDriver->init(info->sysRef))
	{
		theAsioDriver->getErrorMessage(info->errorMessage);
		delete theAsioDriver;
		theAsioDriver = 0;
		return ASE_NotPresent;
	}
	strcpy(info->errorMessage, "No ASIO Driver Error");
	theAsioDriver->getDriverName(info->name);
	info->driverVersion = theAsioDriver->getDriverVersion();
	return ASE_OK;

#else

	info->driverVersion = 0;
	strcpy(info->name, "No ASIO Driver");
	if(theAsioDriver)	// must be loaded!
	{
		if(!theAsioDriver->init(info->sysRef))
		{
			theAsioDriver->getErrorMessage(info->errorMessage);
			theAsioDriver = 0;
			return ASE_NotPresent;
		}		

		strcpy(info->errorMessage, "No ASIO Driver Error");
		theAsioDriver->getDriverName(info->name);
		info->driverVersion = theAsioDriver->getDriverVersion();
		return ASE_OK;
	}
	return ASE_NotPresent;

#endif	// !MAC
}

ASIOError ASIOExit(void)
{
	if(theAsioDriver)
	{
#if WINDOWS
		asioDrivers->removeCurrentDriver();
#else
		delete theAsioDriver;
#endif
	}		
	theAsioDriver = 0;
	return ASE_OK;
}

ASIOError ASIOStart(void)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->start();
}

ASIOError ASIOStop(void)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->stop();
}

ASIOError ASIOGetChannels(long *numInputChannels, long *numOutputChannels)
{
	if(!theAsioDriver)
	{
		*numInputChannels = *numOutputChannels = 0;
		return ASE_NotPresent;
	}
	return theAsioDriver->getChannels(numInputChannels, numOutputChannels);
}

ASIOError ASIOGetLatencies(long *inputLatency, long *outputLatency)
{
	if(!theAsioDriver)
	{
		*inputLatency = *outputLatency = 0;
		return ASE_NotPresent;
	}
	return theAsioDriver->getLatencies(inputLatency, outputLatency);
}

ASIOError ASIOGetBufferSize(long *minSize, long *maxSize, long *preferredSize, long *granularity)
{
	if(!theAsioDriver)
	{
		*minSize = *maxSize = *preferredSize = *granularity = 0;
		return ASE_NotPresent;
	}
	return theAsioDriver->getBufferSize(minSize, maxSize, preferredSize, granularity);
}

ASIOError ASIOCanSampleRate(ASIOSampleRate sampleRate)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->canSampleRate(sampleRate);
}

ASIOError ASIOGetSampleRate(ASIOSampleRate *currentRate)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->getSampleRate(currentRate);
}

ASIOError ASIOSetSampleRate(ASIOSampleRate sampleRate)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->setSampleRate(sampleRate);
}

ASIOError ASIOGetClockSources(ASIOClockSource *clocks, long *numSources)
{
	if(!theAsioDriver)
	{
		*numSources = 0;
		return ASE_NotPresent;
	}
	return theAsioDriver->getClockSources(clocks, numSources);
}

ASIOError ASIOSetClockSource(long reference)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->setClockSource(reference);
}

ASIOError ASIOGetSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->getSamplePosition(sPos, tStamp);
}

ASIOError ASIOGetChannelInfo(ASIOChannelInfo *info)
{
	if(!theAsioDriver)
	{
		info->channelGroup = -1;
		info->type = ASIOSTInt16MSB;
		strcpy(info->name, "None");
		return ASE_NotPresent;
	}
	return theAsioDriver->getChannelInfo(info);
}

ASIOError ASIOCreateBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
	long bufferSize, ASIOCallbacks *callbacks)
{
	if(!theAsioDriver)
	{
		ASIOBufferInfo *info = bufferInfos;
		for(long i = 0; i < numChannels; i++, info++)
			info->buffers[0] = info->buffers[1] = 0;
		return ASE_NotPresent;
	}
	return theAsioDriver->createBuffers(bufferInfos, numChannels, bufferSize, callbacks);
}

ASIOError ASIODisposeBuffers(void)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->disposeBuffers();
}

ASIOError ASIOControlPanel(void)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->controlPanel();
}

ASIOError ASIOFuture(long selector, void *opt)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->future(selector, opt);
}

ASIOError ASIOOutputReady(void)
{
	if(!theAsioDriver)
		return ASE_NotPresent;
	return theAsioDriver->outputReady();
}

#if MAC
}	// extern "C"
#pragma export off
#endif


