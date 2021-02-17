/*
 *  tracker/SynthHarmonica.h
 *
 *  Harmonic waveform editor
 *
 *  Copyright 2011 neoman
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

#ifndef SYNTHHARMONICA__H
#define SYNTHHARMONICA__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

class SynthHarmonica : public PPControl
{
private:
	const PPColor  *borderColor;
	PPColor        ourOwnBorderColor;

	pp_int32       visibleWidth;
	pp_int32       visibleHeight;
	PPColor        color;

	float          wave[128];
	pp_int32       harmonics;

	class DialogSynth* magic;

public:
	SynthHarmonica(pp_int32 id,
		PPScreen* parentScreen,
		EventListenerInterface* eventListener,
		const PPPoint& location,
		const PPSize& size,
		class DialogSynth* magic);

	virtual ~SynthHarmonica();

	void setHarmonics(pp_int32 h);
	void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { color.r = r; color.g = g; color.b = b; }
	void setColor(PPColor color)                    { this->color = color; }
	void setBorderColor(const PPColor& color)       { this->borderColor = &color; }

	// virtuals
	virtual void     paint(PPGraphicsAbstract* graphics);
	virtual pp_int32 dispatchEvent(PPEvent* event);

	friend class DialogSynth;
};


#endif
