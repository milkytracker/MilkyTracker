/*
 *  ScopesControl.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on 28.10.05.
 *  Copyright 2005 milkytracker.net, All rights reserved.
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
				  PPPoint location, 
				  PPSize size, 
				  bool border = true);

	~ScopesControl();

	void setColor(pp_int32 r,pp_int32 g,pp_int32 b) { color.r = r; color.g = g; color.b = b; }
	void setColor(PPColor color) { this->color = color; }

	void setBorderColor(const PPColor& color) { this->borderColor = &color; }

	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);

	virtual pp_int32 callEventListener(PPEvent* event);

	void attachSource(PlayerController* playerController);
	
	void setNumChannels(pp_int32 numChannels) { this->numChannels = numChannels; }
	
	bool needsUpdate();
	
	void muteChannel(pp_int32 index, bool b) { muteChannels[index] = (b ? 1 : 0); }
	void recordChannel(pp_int32 index, bool b) { recChannels[index] = (b ? 1 : 0); }
	
	bool isSoloChannel(pp_int32 c);
	bool isSingleRecChannel(pp_int32 c);
	
	pp_int32 pointToChannel(const PPPoint& pt);

	void enable(bool b) { enabled = b; }
	
	void handleMute(pp_int32 channel);
	void handleSolo(pp_int32 channel);
	void handleRec(pp_int32 channel);
	void handleSingleRec(pp_int32 channel);

	void setCurrentClickType(ClickTypes type) { currentClickType = type; }
	ClickTypes getCurrentClickType() { return currentClickType; }
	
	void setAppearance(AppearanceTypes appearance) { this->appearance = appearance; }
	AppearanceTypes getAppearance() const { return appearance; }
	
	//void setSolid(bool b) { appearance = b ? AppearanceTypeSolid : AppearanceTypeNormal; }
	//bool getSolid() const { return appearance == AppearanceTypeSolid; }
	
	pp_int32 WRAPCHANNELS();
};


#endif
