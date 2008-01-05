/*
 *  PlayerLogic.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 21.12.07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __PLAYERLOGIC_H__
#define __PLAYERLOGIC_H__

#include "BasicTypes.h"

class PlayerLogic 
{
private:
	class Tracker& tracker;

	bool stopBackgroundOnPlay;
	
	bool tracePlay;
	pp_int32 backupRow, backupIndex;
	bool rowPlay;
	
	static void stopPlayer(class PlayerController& playerController);

public:
	PlayerLogic(Tracker& tracker);

	void setStopBackgroundOnPlay(bool stopBackgroundOnPlay) { this->stopBackgroundOnPlay = stopBackgroundOnPlay; }
	bool getStopBackgroundOnPlay() const { return stopBackgroundOnPlay; }

	void playSong(pp_int32 row = 0);
	void playPattern();
	void playPosition(bool rowOnly = false);
	void stopSong();
	void stopAll();

	void ensureSongStopped(bool bResetMainVolume, bool suspend);
	void continuePlayingPattern();
	void continuePlayingSong();
		
	void storePosition();
	void restorePosition();
	
	void playTrace();
	void playRow();
	
	void finishTraceAndRowPlay();
	
	static void playNote(class PlayerController& playerController, pp_uint8 chn, pp_int32 note, pp_int32 i, pp_int32 vol = -1);
	
	friend class Tracker;
};

#endif
