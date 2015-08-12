/*
 *  tracker/SamplePlayer.cpp
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
 *  SamplePlayer.cpp
 *  MilkyTracker
 *
 *  Created by Peter Barth on 13.12.07.
 *
 */

#include "SamplePlayer.h"
#include "ModuleEditor.h"
#include "SampleEditor.h"
#include "PlayerController.h"

void SamplePlayer::playSample(const TXMSample& smp, pp_uint32 note, pp_int32 rangeStart/* = -1*/, pp_int32 rangeEnd/* = -1*/)
{
	playerController.playSample(smp, note, rangeStart, rangeEnd);	
}

void SamplePlayer::playSample(pp_int32 insIndex, pp_int32 smpIndex, pp_uint32 note)
{
	playSample(*moduleEditor.getSampleInfo(insIndex, smpIndex), note);
}

void SamplePlayer::playSample(pp_int32 insIndex, pp_uint32 note)
{
	const mp_ubyte* nbu = moduleEditor.getSampleTable(insIndex);
	pp_int32 smpIndex = nbu[note];
	playSample(insIndex, smpIndex, note);
}

void SamplePlayer::playCurrentSample(pp_uint32 note)
{
	playSample(*moduleEditor.getSampleEditor()->getSample(), note);
}

void SamplePlayer::playCurrentSampleFromOffset(pp_uint32 offset, pp_uint32 note)
{
	SampleEditor* sampleEditor = moduleEditor.getSampleEditor();

	if (offset != -1)
	{
		playSample(*sampleEditor->getSample(),
				   note,
				   offset,
				   sampleEditor->getSampleLen());
	}
}

void SamplePlayer::playCurrentSampleSelectionRange(pp_uint32 note)
{
	SampleEditor* sampleEditor = moduleEditor.getSampleEditor();
	
	if (sampleEditor->getLogicalSelectionStart() != -1 &&
		sampleEditor->getLogicalSelectionEnd() != -1)
	{
		playSample(*sampleEditor->getSample(), 
				   note, 
				   sampleEditor->getLogicalSelectionStart(), 
				   sampleEditor->getLogicalSelectionEnd());
	}
	
}

void SamplePlayer::stopSamplePlayback()
{
	playerController.stopSample();
}

