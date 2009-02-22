/*
 *  tracker/EnvelopeContainer.h
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
 *  EnvelopeContainer.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 20.06.05.
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
