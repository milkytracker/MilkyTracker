/*
 *  tracker/PlayerLogic.h
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
 *  PlayerLogic.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 21.12.07.
 *
 */

#ifndef __PLAYERLOGIC_H__
#define __PLAYERLOGIC_H__

#include "BasicTypes.h"

class PlayerLogic 
{
private:
	class Tracker& tracker;

	bool liveSwitch;
	
	bool stopBackgroundOnPlay;
	
	bool tracePlay;
	pp_int32 backupRow, backupIndex;
	bool rowPlay;
	
	static void stopPlayer(class PlayerController& playerController);

public:
	PlayerLogic(Tracker& tracker);

	void setLiveSwitch(bool liveSwitch);
	bool getLiveSwitch() const { return liveSwitch; }	

	void setStopBackgroundOnPlay(bool stopBackgroundOnPlay) { this->stopBackgroundOnPlay = stopBackgroundOnPlay; }
	bool getStopBackgroundOnPlay() const { return stopBackgroundOnPlay; }

	void playSong(pp_int32 row = 0);
	void playPattern();
	void playPosition(bool rowOnly = false);
	void stopSong();
	void stopAll();

	void ensureSongStopped(bool bResetMainVolume, bool suspend);
	void ensureSongPlaying(bool continuePlaying);
	void continuePlayingPattern();
	void continuePlayingSong();
		
	void storePosition();
	void restorePosition();
	
	void playTrace();
	void playRow();
	
	void finishTraceAndRowPlay();
	
	static void playNote(class PlayerController& playerController, 
						 pp_uint8 chn, 
						 pp_int32 note, pp_int32 i, pp_int32 vol = -1);
	
	friend class Tracker;
};

#endif
