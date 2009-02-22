/*
 *  tracker/PeakLevelControl.h
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
 *  PeakLevelControl.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.10.05.
 *
 */

#ifndef PEAKLEVELCONTROL__H
#define PEAKLEVELCONTROL__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"

class PeakLevelControl : public PPControl
{
private:
	PPColor color;

	bool border;
	PPColor ourOwnBorderColor;
	const PPColor* borderColor;

	// extent
	pp_int32 visibleWidth;
	pp_int32 visibleHeight;

	pp_int32 peak[2];
	
	pp_uint8 peakColorLUT[256][3];

public:
	PeakLevelControl(pp_int32 id, 
					 PPScreen* parentScreen, 
					 EventListenerInterface* eventListener, 
					 const PPPoint& location, const PPSize& size, 
					 bool border = true);

	~PeakLevelControl();

	void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { color.r = r; color.g = g; color.b = b; }
	void setColor(PPColor color) { this->color = color; }

	void setBorderColor(const PPColor& color) { this->borderColor = &color; }

	void setPeak(pp_int32 whichPeak, pp_int32 p) { if (p>65536) p = 65536; if (p < 0) p = 0; peak[whichPeak] = p; }
	pp_int32 getPeak(pp_int32 whichPeak) const { return peak[whichPeak]; } 

	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);
	
private:
	void buildColorLUT();
};


#endif
