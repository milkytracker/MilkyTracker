/*
 *  tracker/DialogEQ.h
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
 *  DialogEQ.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 03.04.07.
 *
 */

#ifndef __DIALOGEQ_H__
#define __DIALOGEQ_H__

#include "DialogBase.h"

class DialogEQ : public PPDialogBase
{
public:
	enum EQNumBands
	{
		EQ3Bands,
		EQ10Bands
	};

private:
	EQNumBands numBands;
	pp_uint32 numSliders;

	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);	
	
	void resetSliders();
	void update();

public:
	DialogEQ(PPScreen* screen, DialogResponder* responder,
			 pp_int32 id,
			 EQNumBands numBands);

	void setBandParam(pp_uint32 index, float param);
	float getBandParam(pp_uint32 index) const;
	
	EQNumBands getNumBands() const { return numBands; }
	pp_uint32 getNumBandsAsInt() const { return numSliders; }
};


#endif
