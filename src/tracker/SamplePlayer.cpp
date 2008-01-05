/*
 *  SamplePlayer.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 13.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SamplePlayer.h"
#include "ModuleEditor.h"
#include "SampleEditor.h"
#include "PlayerController.h"

void SamplePlayer::playSample(TXMSample* smp, pp_uint32 note, pp_int32 rangeStart/* = -1*/, pp_int32 rangeEnd/* = -1*/)
{
	playerController.playSample(smp, note, rangeStart, rangeEnd);	
}

void SamplePlayer::playSample(pp_int32 insIndex, pp_int32 smpIndex, pp_uint32 note)
{
	playSample(moduleEditor.getSampleInfo(insIndex, smpIndex), note);
}

void SamplePlayer::playSample(pp_int32 insIndex, pp_uint32 note)
{
	const mp_ubyte* nbu = moduleEditor.getSampleTable(insIndex);
	pp_int32 smpIndex = nbu[note];
	playSample(insIndex, smpIndex, note);
}

void SamplePlayer::playCurrentSample(pp_uint32 note)
{
	playSample(moduleEditor.getSampleEditor()->getSample(), note);
}

void SamplePlayer::playCurrentSampleSelectionRange(pp_uint32 note)
{
	SampleEditor* sampleEditor = moduleEditor.getSampleEditor();
	
	if (sampleEditor->getLogicalSelectionStart() != -1 &&
		sampleEditor->getLogicalSelectionStart() != -1)
	{
		playSample(sampleEditor->getSample(), 
				   note, 
				   sampleEditor->getLogicalSelectionStart(), 
				   sampleEditor->getLogicalSelectionEnd());
	}
	
}

void SamplePlayer::stopSamplePlayback()
{
	playerController.stopSample();
}

