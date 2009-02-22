/*
 *  tracker/SectionHDRecorder.h
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
 *  SectionHDRecorder.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 26.10.05.
 *
 */

#ifndef SECTIONHDRECORDER__H
#define SECTIONHDRECORDER__H

#include "BasicTypes.h"
#include "Event.h"
#include "SectionUpperLeft.h"

class PPControl;
class Tracker;
class PPCheckBox;

class SectionHDRecorder : public SectionUpperLeft
{
private:
	enum RecorderModes
	{
		RecorderModeToFile,
		RecorderModeToSample
	};

	RecorderModes recorderMode;
	pp_int32 fromOrder;
	pp_int32 toOrder;
	pp_int32 mixerVolume;
	pp_uint32 resampler;

	pp_int32 insIndex;
	pp_int32 smpIndex;
	
	PPSystemString currentFileName;

	bool getSettingsRamping();
	void setSettingsRamping(bool b);
	
	pp_uint32 getSettingsResampler();
	void setSettingsResampler(pp_uint32 resampler);
	
	bool getSettingsAllowMuting();
	void setSettingsAllowMuting(bool b);
	
	pp_int32 getSettingsFrequency();
	void setSettingsFrequency(pp_int32 freq);
	
	pp_int32 getSettingsMixerVolume() { return mixerVolume; }
	void setSettingsMixerVolume(pp_int32 vol) { mixerVolume = vol; }
	
	pp_int32 getSettingsMixerShift();
	void setSettingsMixerShift(pp_int32 shift);

	void validate();

public:
	SectionHDRecorder(Tracker& tracker);
	virtual ~SectionHDRecorder();

	// Derived from SectionAbstract
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);
	
	virtual void init() { SectionUpperLeft::init(); }
	virtual void init(pp_int32 x, pp_int32 y);
	virtual void show(bool bShow); 
	virtual void update(bool repaint = true);

	virtual void notifyInstrumentSelect(pp_int32 index);
	virtual void notifySampleSelect(pp_int32 index);
	
	void exportWAVAs(const PPSystemString& fileName);
	void exportWAVWithPanel(const PPSystemString& defaultFileName);
	void exportWAVAsFileName(const PPSystemString& fileName);
	
	void exportWAVAsSample();
	
	void getPeakLevel();
	
	void resetCurrentFileName();
	void setCurrentFileName(const PPSystemString& fileName);
	
	void adjustOrders();
	
	void selectFileOutput() { recorderMode = RecorderModeToFile; }
	void selectSampleOutput() { recorderMode = RecorderModeToSample; }

private:
	// Message box with list of resampler
	void showResamplerMessageBox();

	void storeResampler(pp_uint32 resampler);

	// Responder should be friend
	friend class DialogResponderHDRec;	
	
	friend class Tracker;
};

#endif
