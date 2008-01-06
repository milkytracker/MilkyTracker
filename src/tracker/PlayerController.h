/*
 *  PlayerController.h
 *  MilkyTracker
 *
 *  Created by Peter Barth on Tue Mar 15 2005.
 *  Copyright (c) 2005 milkytracker.net, All rights reserved.
 *
 */

#ifndef __PLAYERCONTROLLER_H__
#define __PLAYERCONTROLLER_H__

#include "MilkyPlayCommon.h"
#include "TrackerConfig.h"

class XModule;
struct TXMSample;
struct TEnvelope;

class PlayerController
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
		PlayModeOptionPanning8xx = 1,
		PlayModeOptionPanningE8x = 2,
		// Only affects PTK playback mode
		PlayModeOptionForcePTPitchLimit = 4
	};

private:
	class MasterMixer* mixer;
	class PlayerSTD* player;
	XModule* module;
	class PlayerCriticalSection* criticalSection;
			
	bool patternPlay;
	bool playRowOnly;
	mp_sint32 patternIndex;

	mp_sint32 lastPosition, lastRow;
	bool wasPlayingPattern;
	bool suspended;
	
	mp_ubyte panning[TrackerConfig::MAXCHANNELS];

	bool muteChannels[TrackerConfig::MAXCHANNELS];
	bool recordChannels[TrackerConfig::MAXCHANNELS];
	bool firstRecordChannelCall;
	
	mp_sint32 currentPlayingChannel;

	mp_sint32 numPlayerChannels;
	mp_sint32 numVirtualChannels;
	mp_sint32 totalPlayerChannels;
	bool useVirtualChannels;
	bool multiChannelKeyJazz;
	bool multiChannelRecord;

	mp_sint32 mixerDataCacheSize;
	mp_sint32* mixerDataCache;

	void assureNotSuspended();
	void continuePlaying(bool assureNotSuspended);
	
	// no construction outside
	PlayerController(class MasterMixer* mixer, bool fakeScopes);
	
public:
	~PlayerController();
	
	void attachModule(XModule* module);
	
	PlayerCriticalSection* getCriticalSection() { return criticalSection; }
	
	void playSong(mp_sint32 startIndex, mp_sint32 rowPosition, mp_ubyte* muteChannels);
	void playPattern(mp_sint32 index, mp_sint32 songPosition, mp_sint32 rowPosition, mp_ubyte* muteChannels, bool playRowOnly = false);
	void setCurrentPatternIndex(mp_sint32 index);
	void stop(bool bResetMainVolume = true);
	
	void continuePlaying();
	void restartPlaying();

	bool isPlaying() const;
	bool isPlayingRowOnly() const;
	bool isActive() const;
	bool isPlayingPattern() const { return patternPlay; }
	bool isReallyPlayingPattern() const { return patternPlay && !playRowOnly; }
	bool isSuspended() const { return suspended; }

	void pause();
	void unpause();
	bool isPaused() const;

	void getSpeed(mp_sint32& BPM, mp_sint32& speed);
	void setSpeed(mp_sint32 BPM, mp_sint32 speed, bool adjustModuleHeader = true);
	
	void readjustSpeed(bool adjustModuleHeader = true);
	
	void playSample(TXMSample* sample, mp_sint32 currentSamplePlayNote, mp_sint32 rangeStart = -1, mp_sint32 rangeEnd = -1);
	void stopSample();
	
	void stopInstrument(mp_sint32 insIndex);

	void playNote(mp_ubyte chn, mp_sint32 note, mp_sint32 i, mp_sint32 vol = -1);

	void suspendPlayer(bool bResetMainVolume = true, bool stopPlaying = true);	
	void resumePlayer(bool continuePlaying);

	void muteChannel(mp_sint32 c, bool m);
	bool isChannelMuted(mp_sint32 c);

	void recordChannel(mp_sint32 c, bool m);
	bool isChannelRecording(mp_sint32 c);

private:
	bool reallocChannels();
	void reallocateChannels(mp_sint32 moduleChannels = 32, mp_sint32 virtualChannels = 0);
	void setUseVirtualChannels(bool bUseVirtualChannels);

	void setMultiChannelKeyJazz(bool b) { multiChannelKeyJazz = b; }
	void setMultiChannelRecord(bool b) { multiChannelRecord = b; }

public:
	void resetFirstPlayingChannel();
	mp_sint32 getNextPlayingChannel(mp_sint32 currentChannel);
	
	void initRecording();
	mp_sint32 getNextRecordingChannel(mp_sint32 currentChannel);
	
	mp_sint32 getSongMainVolume();
	void resetMainVolume();
	
	// in seconds
	void resetPlayTimeCounter();
	
	mp_int64 getPlayTime();

	mp_ubyte getPanning(mp_ubyte chn) { return panning[chn]; }
	void setPanning(mp_ubyte chn, mp_ubyte pan);
	const pp_uint8* getPanningTable() { return (const pp_uint8*)&panning; }

	void getPosition(mp_sint32& pos, mp_sint32& row);
	void getPosition(mp_sint32& order, mp_sint32& row, mp_sint32& ticker);
	void setPatternPos(mp_sint32 pos, mp_sint32 row);
	
	// change playmode
	void switchPlayMode(PlayModes playMode, bool exactSwitch = true);
	PlayModes getPlayMode();
	
	void enablePlayModeOption(PlayModeOptions option, bool b);
	bool isPlayModeOptionEnabled(PlayModeOptions option);	
	
	// queries on the mixer
	mp_sint32 getAllNumPlayingChannels();
	mp_sint32 getPlayerNumPlayingChannels();

private:
	mp_sint32 getCurrentSamplePosition();
	mp_sint32 getCurrentBeatIndex();

public:
	bool isSamplePlaying(const TXMSample* smp, mp_sint32 channel, mp_sint32& pos, mp_sint32& vol, mp_sint32& pan);
	bool isEnvelopePlaying(const TEnvelope* envelope, mp_sint32 envelopeType, mp_sint32 channel, mp_sint32& pos);
	bool isNotePlaying(mp_sint32 ins, mp_sint32 channel, mp_sint32& note);
	
	class SampleDataFetcher
	{
	public:
		virtual void fetchSampleData(mp_sint32 sample) = 0;
	};
	void grabSampleData(mp_uint32 chnIndex, mp_sint32 count, mp_sint32 fMul, SampleDataFetcher& fetcher);
	
	bool hasSampleData(mp_uint32 chnIndex);
	
	friend class PlayerMaster;
};

#endif

