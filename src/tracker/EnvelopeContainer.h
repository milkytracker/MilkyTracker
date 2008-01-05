/*
 *  EnvelopeContainer.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 20.06.05.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef ENVELOPECONTAINER__H
#define ENVELOPECONTAINER__H

#include "BasicTypes.h"

struct TEnvelope;

class EnvelopeContainer
{
private:
	TEnvelope* envelopes;
	pp_int32 numEnvelopes;

public:
	EnvelopeContainer(pp_int32 num);
	~EnvelopeContainer();
	
	void store(pp_int32 index, const TEnvelope& env);
	const TEnvelope* restore(pp_int32 index);

	static PPString encodeEnvelope(const TEnvelope& env);
	static TEnvelope decodeEnvelope(const PPString& str);
};

#endif
