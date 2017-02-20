/*
 *  tracker/ScopesControl.h
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
 *  ScopesControl.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.10.05.
 *
 */

#ifndef SCOPESCONTROL__H
#define SCOPESCONTROL__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"
#include "TrackerConfig.h"

// Forwards
class PPButton;
class PPGraphicsAbstract;
class PPFont;
class PPControl;
class PlayerController;

class ScopesControl : public PPControl
{
public:
	enum ClickTypes
	{
		ClickTypeMute,
		ClickTypeSolo,
		ClickTypeRec,
		ClickTypeSingleRec
	};

	enum AppearanceTypes
	{
		AppearanceTypeNormal,
		AppearanceTypeSolid,
		AppearanceTypeLines
	};

private:
	PPColor color;

	bool border;
	PPColor ourOwnBorderColor;
	const PPColor* borderColor;

	PPButton* backgroundButton;

	// extent
	pp_int32 visibleWidth;
	pp_int32 visibleHeight;

	PlayerController* playerController;

	pp_int32 numChannels;
	pp_int32 channelWidthTable[TrackerConfig::MAXCHANNELS];
	bool onOffState[TrackerConfig::MAXCHANNELS];
	bool zeroVolumeState[TrackerConfig::MAXCHANNELS];
	pp_uint8 muteChannels[TrackerConfig::MAXCHANNELS];
	pp_uint8 lastMuteChannels[TrackerConfig::MAXCHANNELS];
	pp_uint8 recChannels[TrackerConfig::MAXCHANNELS];
	pp_uint8 lastRecChannels[TrackerConfig::MAXCHANNELS];
	pp_int32 lastNumChannels;
	PPFont* font;
	PPFont* smallFont;

	pp_int32 lMouseDownInChannel, rMouseDownInChannel;
	bool didSoloChannel;

	bool enabled;

	AppearanceTypes appearance;

	ClickTypes currentClickType;

public:
	enum ChangeValueTypes
	{
		ChangeValueMuting,
		ChangeValueRecording
	};

	ScopesControl(pp_int32 id,
				  PPScreen* parentScreen,
				  EventListenerInterface* eventListener,
				  const PPPoint& location, const PPSize& size,
				  bool border = true);

	virtual ~ScopesControl();

	void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { color.r = r; color.g = g; color.b = b; }
	void setColor(PPColor color) { this->color = color; }

	void setBorderColor(const PPColor& color) { this->borderColor = &color; }

	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);

	virtual pp_int32 dispatchEvent(PPEvent* event);

	void attachSource(PlayerController* playerController);

	void setNumChannels(pp_int32 numChannels) { this->numChannels = numChannels; }

	bool needsUpdate();
	void enable(bool b) { enabled = b; }

	void muteChannel(pp_int32 index, bool b) { muteChannels[index] = (b ? 1 : 0); }
	void recordChannel(pp_int32 index, bool b) { recChannels[index] = (b ? 1 : 0); }

	bool isSoloChannel(pp_int32 c) const;
	bool isSingleRecChannel(pp_int32 c) const;

	void handleMute(pp_int32 channel);
	void handleSolo(pp_int32 channel);
	void handleRec(pp_int32 channel);
	void handleSingleRec(pp_int32 channel);

	void handleUnmuteAll();

	void setCurrentClickType(ClickTypes type) { currentClickType = type; }
	ClickTypes getCurrentClickType() const { return currentClickType; }

	void setAppearance(AppearanceTypes appearance) { this->appearance = appearance; }
	AppearanceTypes getAppearance() const { return appearance; }

private:
	pp_int32 pointToChannel(const PPPoint& pt);

	pp_int32 WRAPCHANNELS();
};


#endif
