/*
 *  tracker/SamplePlayer.h
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
 *  SamplePlayer.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 13.12.07.
 *
 */

#ifndef __SAMPLEPLAYER_H__
#define __SAMPLEPLAYER_H__

#include "BasicTypes.h"

struct TXMSample;

class SamplePlayer
{
private:
	class ModuleEditor& moduleEditor;
	class PlayerController& playerController;
	
public:
	SamplePlayer(ModuleEditor& moduleEditor, PlayerController& playerController) :
		moduleEditor(moduleEditor),
		playerController(playerController)
	{
	}
	
	void playSample(const TXMSample& smp, pp_uint32 note, pp_int32 rangeStart = -1, pp_int32 rangeEnd = -1);
	void playSample(pp_int32 insIndex, pp_int32 smpIndex, pp_uint32 note);
	void playSample(pp_int32 insIndex, pp_uint32 note);
	void playCurrentSample(pp_uint32 note);
	void playCurrentSampleFromOffset(pp_uint32 offset, pp_uint32 note);
	void playCurrentSampleSelectionRange(pp_uint32 note);
	
	void stopSamplePlayback();
};

#endif
