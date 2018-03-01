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
 *  RecorderLogic.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 09.04.08.
 *
 */

#ifndef __RECORDERLOGIC_H__
#define __RECORDERLOGIC_H__

#include "BasicTypes.h"

class PPEvent;
class PatternEditorControl;

class RecorderLogic
{
private:
	class Tracker& tracker;
	
	struct TKeyInfo
	{
		pp_int32 note, ins;
		pp_int32 channel, pos, row;
		class PlayerController* playerController;
	};
	
	TKeyInfo* keys;
	 
	pp_int32 keyVolume;

	bool recordMode;
	bool recordKeyOff;
	bool recordNoteDelay;

public:
	RecorderLogic(Tracker& tracker);
	~RecorderLogic();
	
	void setKeyVolume(pp_int32 keyVolume) { this->keyVolume = keyVolume; }

	void setRecordMode(bool recordMode) { this->recordMode = recordMode; }
	bool getRecordMode() const { return recordMode; }
	
	void setRecordKeyOff(bool recordKeyOff) { this->recordKeyOff = recordKeyOff; }
	bool getRecordKeyOff() const { return recordKeyOff; }

	void setRecordNoteDelay(bool recordNoteDelay) { this->recordNoteDelay = recordNoteDelay; }
	bool setRecordNoteDelay() const { return recordNoteDelay; }
	
	void reset();
	
	void sendNoteDownToPatternEditor(PPEvent* event, pp_int32 note, PatternEditorControl* patternEditorControl);
	void sendNoteUpToPatternEditor(PPEvent* event, pp_int32 note, PatternEditorControl* patternEditorControl);
	
	void init();
	void initToggleEdit();
};

#endif
