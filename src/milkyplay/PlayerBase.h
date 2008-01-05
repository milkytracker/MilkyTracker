/*
 *  PlayerBase.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Thu Jan 20 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef __PLAYERBASE_H__
#define __PLAYERBASE_H__

#include "ChannelMixer.h"

class XModule;

struct TPlayerChannelInfo
{
	mp_ubyte note;
	mp_ubyte instrument;
	mp_ubyte volume;
	mp_ubyte panning;
	mp_ubyte numeffects;
	mp_ubyte effects[MP_NUMEFFECTS];
	mp_ubyte operands[MP_NUMEFFECTS];
	
	void clear()
	{
		memset(this, 0, sizeof(TPlayerChannelInfo));
	}
};

class PlayModeSettings
{
public:
	enum PlayModes
	{
		PlayMode_Auto,
		PlayMode_ProTracker2,
		PlayMode_ProTracker3,
		PlayMode_ScreamTracker3,
		PlayMode_FastTracker2,
		PlayMode_ImpulseTracker
	};

	enum PlayModeOptions
	{
		PlayModeOptionFirst = 0,
		PlayModeOptionPanning8xx = 0,
		PlayModeOptionPanningE8x = 1,
		// Only affects PTK playback mode
		PlayModeOptionForcePTPitchLimit = 2,
		PlayModeOptionLast
	};

protected:
	bool			options[PlayModeOptionLast];

	PlayModes		playMode;
};

class PlayerBase : public ChannelMixer, public PlayModeSettings
{
private:
	struct TimeRecord
	{
		enum BITPOSITIONS
		{
			BITPOS_POS = 0,
			BITPOS_ROW = 8,
			BITPOS_TEMPO = 16,
			BITPOS_SPEED = 24,
			
			BITPOS_MAINVOL = 0,
			BITPOS_TICKER = 8
		};
		
		mp_uint32 posRowTempoSpeed;
		mp_sint32 mainVolumeTicker;
		
		TimeRecord()
		{
		}
		
		TimeRecord(mp_uint32 pos, mp_uint32 row, mp_uint32 tempo, mp_uint32 speed, mp_sint32 mainVol, mp_sint32 ticker) :
			posRowTempoSpeed((pos << BITPOS_POS) + 
							 (row << BITPOS_ROW) +
							 (tempo << BITPOS_TEMPO) +
							 (speed << BITPOS_SPEED)),
			mainVolumeTicker((mainVol << BITPOS_MAINVOL) +
							 (ticker << BITPOS_TICKER))
		{
		}
	};

	TimeRecord		timeRecordTable[TIMESAMPLEBUFFERSIZE];
	
public:
	enum PlayerTypes
	{
		PlayerType_Generic,		// generic module player, can play most of the Protracker style formats
		PlayerType_FAR,			// Farandole composer player
		PlayerType_IT,			// Supposed to be a compatible IT replayer 
		PlayerType_INVALID = -1	// NULL player :D
	};
	
protected:
	XModule*		module;

	bool			paused;					// Player is paused
	bool			stopped;				// Playing has been stopped (song is over)
	bool			repeat;					// Player will repeat song
	bool			idle;					// Player is mixing, but not processing song
	bool			playOneRowOnly;			// Player will only play one row and not advance to the next row (used for milkytracker)
	bool			resetOnStopFlag;
	bool			resetMainVolumeOnStartPlayFlag;

	mp_sint32		initialNumChannels;		// Fixed number of channels, can be set manually in StartPlaying
											// otherwise it will be module->header.channum

	mp_sint32		mainVolume;				// current song's main volume 

	mp_sint32		tickSpeed;				// our tickspeed
	mp_sint32		baseBpm;				// Support digibooster REAL BPM value
	mp_sint32		bpm;					// BPM speed
	mp_sint32		ticker;					// runs from 0 to tickspeed-1

	mp_sint32		rowcnt;					// counts through each row in a pattern
	mp_sint32		poscnt;					// counts through each index the pattern index table
	mp_int64		synccnt;				// will increment 250 times per mixed second
	mp_sint32		lastUnvisitedPos;		// the last order we visited before a new order has been set

	mp_uint32		adder, BPMCounter;		

	mp_sint32		patternIndexToPlay;		// Play special pattern, -1 = Play entire song

protected:
	mp_sint32		kick();

	virtual mp_sint32 allocateStructures() { return 0; }

	virtual void clearEffectMemory() {}	

	void updateTimeRecord()
	{
		for (mp_uint32 i = 0; i < TIMESAMPLEBUFFERSIZE; i++)
		{
			timeRecordTable[i] = TimeRecord(poscnt, 
											rowcnt, 
											bpm, 
											tickSpeed, 
											mainVolume, 
											ticker);
		}
	}
	
public:
	PlayerBase(mp_uint32 frequency);

	virtual ~PlayerBase();
	
	void setPlayMode(PlayModes mode) { playMode = mode; }

	PlayModes getPlayMode() const { return playMode; }

	void			enable(PlayModeOptions option, bool b)
	{
		ASSERT(option>=PlayModeOptionFirst && option<PlayModeOptionLast);
		options[option] = b;
	}

	bool			isEnabled(PlayModeOptions option) const
	{
		ASSERT(option>=PlayModeOptionFirst && option<PlayModeOptionLast);
		return options[option];
	}
	
	// runtime type identification
	virtual PlayerTypes getType() const = 0;

	// virtual from mixer class, perform playing here
	virtual void timerHandler(mp_sint32 currentBeatPacket);

	virtual void restart(mp_uint32 startPosition = 0, 
						 mp_uint32 startRow = 0, 
						 bool resetMixer = true, 
						 const mp_ubyte* customPanningTable = NULL, 
						 bool playOneRowOnly = false);
	
	virtual void reset() {}

	virtual void resetAllSpeed() {}	
	
	virtual mp_sint32   startPlaying(XModule* module, 
									 bool repeat = false, 
									 mp_uint32 startPosition = 0, 
									 mp_uint32 startRow = 0,
									 mp_sint32 numChannels = -1, 
									 const mp_ubyte* customPanningTable = NULL,
									 bool idle = false,
									 mp_sint32 patternIndex = -1,
									 bool playOneRowOnly = false);
	
	void			setPattern(mp_sint32 patternIndex) { patternIndexToPlay = patternIndex; }

	mp_sint32		stopPlaying();
	
	bool			hasStopped() const { return stopped; }

	void			setIdle(bool idle) { this->idle = idle; }
	bool			isIdle() const { return idle; }

	virtual void	setRepeat(bool repeat) { this->repeat = repeat; }
	bool			isRepeating() const { return repeat; }
		
	mp_sint32		pausePlaying();
	mp_sint32		resumePlaying(bool unpause = true);

	bool			isPaused() const { return paused; }	
	
	// Set song main volume
	void			setSongMainVolume(mp_ubyte volume) { mainVolume = volume; }
	mp_sint32		getSongMainVolume(mp_uint32 i = 0) const
	{
		return (timeRecordTable[i].mainVolumeTicker >> TimeRecord::BITPOS_MAINVOL) & 255;
	}
	
	// Reset sound mixer on song stop
	void			resetOnStop(bool b) { resetOnStopFlag = b; }
	// Reset main volume when song is started
	void			resetMainVolumeOnStartPlay(bool b) { resetMainVolumeOnStartPlayFlag = b; }
	
	virtual mp_sint32		getOrder(mp_uint32 i = 0) const
	{ 
		return (timeRecordTable[i].posRowTempoSpeed >> TimeRecord::BITPOS_POS) & 255;
	}
	
	virtual mp_sint32		getRow(mp_uint32 i = 0) const
	{ 
		return (timeRecordTable[i].posRowTempoSpeed >> TimeRecord::BITPOS_ROW) & 255;
	}

	virtual void		getPosition(mp_sint32& order, mp_sint32& row, mp_uint32 i = 0) const
	{ 
		order = (timeRecordTable[i].posRowTempoSpeed >> TimeRecord::BITPOS_POS) & 255;
		row = (timeRecordTable[i].posRowTempoSpeed >> TimeRecord::BITPOS_ROW) & 255;
	}

	virtual mp_sint32	getLastUnvisitedPosition() const { return lastUnvisitedPos; }

	virtual void		getPosition(mp_sint32& order, mp_sint32& row, mp_sint32& ticker, mp_uint32 i = 0) const
	{ 
		order = (timeRecordTable[i].posRowTempoSpeed >> TimeRecord::BITPOS_POS) & 255;
		row = (timeRecordTable[i].posRowTempoSpeed >> TimeRecord::BITPOS_ROW) & 255;
		ticker = (timeRecordTable[i].mainVolumeTicker >> TimeRecord::BITPOS_TICKER) & 255;
	}
	
	virtual mp_int64		getSyncCount() const { return synccnt; }
	
	virtual void			nextPattern();
	virtual void			lastPattern();
	virtual void			setPatternPos(mp_uint32 pos, mp_uint32 row = 0, bool resetChannels = true, bool resetFXMemory = true);

	virtual void			setTempo(mp_sint32 tempo) 
	{ 
		bpm = tempo;
		updateTimeRecord();
	}
	
	virtual void			setSpeed(mp_sint32 speed) 
	{ 
		if (speed != tickSpeed)
		{
			tickSpeed = speed;
			if (ticker >= speed)
				ticker = 0;
			updateTimeRecord();
		}
	}

	virtual mp_sint32		getTempo(mp_uint32 i = 0) const
	{ 
		if (isPlaying() && !isIdle())
		{
			return (timeRecordTable[i].posRowTempoSpeed >> TimeRecord::BITPOS_TEMPO) & 255;
		}
		return bpm;
	}
	
	virtual mp_sint32		getSpeed(mp_uint32 i = 0) const
	{ 
		if (isPlaying() && !isIdle())
		{
			return (timeRecordTable[i].posRowTempoSpeed >> TimeRecord::BITPOS_SPEED) & 255;;
		}
		return tickSpeed;
	}
	
	virtual bool			grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const
	{
		channelInfo.clear();
		return false;
	}
	
	// milkytracker
	virtual void			playNote(mp_ubyte chn, mp_sint32 note, mp_sint32 i, mp_sint32 vol = -1) {}
	
	virtual void			setPanning(mp_ubyte chn, mp_ubyte pan) {}
};

#endif
