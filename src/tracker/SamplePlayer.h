/*
 *  SamplePlayer.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 13.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
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
	
	void playSample(TXMSample* smp, pp_uint32 note, pp_int32 rangeStart = -1, pp_int32 rangeEnd = -1);
	void playSample(pp_int32 insIndex, pp_int32 smpIndex, pp_uint32 note);
	void playSample(pp_int32 insIndex, pp_uint32 note);
	void playCurrentSample(pp_uint32 note);
	void playCurrentSampleSelectionRange(pp_uint32 note);
	
	void stopSamplePlayback();
};

#endif
