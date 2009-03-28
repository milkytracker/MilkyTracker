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
#if WINDOWS
#include <windows.h>
#include "combase.h"
#include "iasiodrv.h"
class AsioDriver : public IASIO ,public CUnknown
{
public:
	AsioDriver(LPUNKNOWN pUnk, HRESULT *phr);

	DECLARE_IUNKNOWN
	// Factory method
	static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE NonDelegatingQueryInterface(REFIID riid,void **ppvObject);

#else

class AsioDriver
{
public:
	AsioDriver();
#endif
	virtual ~AsioDriver();

	virtual ASIOBool init(void* sysRef);
	virtual void getDriverName(char *name);	// max 32 bytes incl. terminating zero
	virtual long getDriverVersion();
	virtual void getErrorMessage(char *string);	// max 124 bytes incl.

	virtual ASIOError start();
	virtual ASIOError stop();

	virtual ASIOError getChannels(long *numInputChannels, long *numOutputChannels);
	virtual ASIOError getLatencies(long *inputLatency, long *outputLatency);
	virtual ASIOError getBufferSize(long *minSize, long *maxSize,
		long *preferredSize, long *granularity);

	virtual ASIOError canSampleRate(ASIOSampleRate sampleRate);
	virtual ASIOError getSampleRate(ASIOSampleRate *sampleRate);
	virtual ASIOError setSampleRate(ASIOSampleRate sampleRate);
	virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources);
	virtual ASIOError setClockSource(long reference);

	virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp);
	virtual ASIOError getChannelInfo(ASIOChannelInfo *info);

	virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
		long bufferSize, ASIOCallbacks *callbacks);
	virtual ASIOError disposeBuffers();

	virtual ASIOError controlPanel();
	virtual ASIOError future(long selector, void *opt);
	virtual ASIOError outputReady();
};
#endif
