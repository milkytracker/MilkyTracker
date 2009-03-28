/*
 * Copyright (c) 2009, The MilkyTracker Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  PlayerGeneric.h
 *  MilkyPlay
 *
 *	This class is meant to be a wrapper class that handles all 
 *	different sorts of players for different Module types.
 *
 *  Created by Peter Barth on 21.01.05.
 *
 */

#ifndef __PLAYERGENERIC_H__
#define __PLAYERGENERIC_H__

#include "MilkyPlayTypes.h"
#include "XMFile.h"
#include "ChannelMixer.h"
#include "PlayerBase.h"

class XModule;
class AudioDriverInterface;

class PlayerGeneric : public MixerSettings, public PlayModeSettings
{
private:
	class MasterMixer* mixer;
	class MixerNotificationListener* listener;
	
	// the current PlayerBase instance
	PlayerBase*			player;

	// ----------------------------------------------------------
	// Since we are destroying and creating PlayerBase instances 
	// within this class there is no way to keep the settings
	// we set for this special instance, so we need to store them
	// not only in the instance but also in this class:
	// ----------------------------------------------------------
	// remember mixer type
	ChannelMixer::ResamplerTypes	resamplerType;
	// remember mixing frequency
	mp_uint32			frequency;
	// overwrite default audiodriver
	AudioDriverInterface*	audioDriver;
	// remember buffersize
	mp_uint32			bufferSize;
	// remember sample shift
	mp_uint32			sampleShift;
	// this flag indicates if audiodriver tries to compensate for 2^n buffer sizes
	bool				compensateBufferFlag;		
	// This contains the string of the selected audio driver
	char*				audioDriverName;

	// remember paused state
	bool				paused;
	// remember if mixing has been disabled
	bool				disableMixing;
	// remember if filters are allowed
	bool				allowFilters;
	// remember idle state
	bool				idle;
	// remember to play only one row
	bool				playOneRowOnly;
	// remember to repeat the song
	bool				repeat;
	// remember to reset on stop
	bool				resetOnStopFlag;
	// remember to reset main volume on start 
	bool				resetMainVolumeOnStartPlayFlag;
	// remember to auto adjust the peak
	bool				autoAdjustPeak;
	// remember our mixer mastervolume
	mp_sint32			masterVolume;
	// remember our mixer panning separation
	mp_sint32			panningSeparation;
	// remember maximum amount of virtual channels
	mp_sint32			numMaxVirChannels;

	void				adjustSettings();

	/**
	 * Determine the best player type for a given module 
	 * @param  module	the module which should be played
	 * @return			an enum with identifies the player type
	 */
 	static PlayerBase::PlayerTypes getPreferredPlayerType(XModule* module);

	/**
	 * Return an actual player instance for a given module
	 * This instance MUST be deleted after usage
	 * @param  module		the module which should be played
	 * @param  audioDriver	you can specify your own audioDriver instance, if this is NULL the default driver will be selected
	 * @return				a PlayerBase instance is returned
	 */
	PlayerBase*			getPreferredPlayer(XModule* module) const;

public:
	/**
	 * Construct a PlayerGeneric object for a given output frequency
	 * @param  frequency	output frequency for the mixer
	 */
						PlayerGeneric(mp_sint32 frequency, 
									  AudioDriverInterface* audioDriver = NULL);
	/**
	 * Destructor
	 */	
						~PlayerGeneric();

	///////////////////////////////////////////////////////////////////////////////////////////
	// -------------------------- wrapping mixer specific stuff -------------------------------
	///////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Specify mixer type. This basically selects the output quality 
	 * @param  type		mixer type
	 * @see				MixerSettings
	 */
	void				setResamplerType(ResamplerTypes type);

	/**
	 * Specify mixer type with two flags indicating whether to interpolate or to do volume ramping. 
	 * @param  type		mixer type
	 * @see				MixerSettings
	 */
	void				setResamplerType(bool interpolation, bool ramping);

	/**
	 * Get the mixer type
	 * @return			mixer type
	 * @see				setResamplerType
	 */
	ChannelMixer::ResamplerTypes	getResamplerType() const;
	
	/**
	 * Specify the amount of which the sample data is right-shifted before sent to the sound driver
	 * This is an amplify in the opposite direction (shift value of 2 means 25% of the original volume)
	 * @param  shift	sample data shift amount
	 */
	void				setSampleShift(mp_sint32 shift);

	/**
	 * Get the shift amount
	 * @return			shift amount
	 * @see				setSampleShift
	 */
	mp_sint32			getSampleShift() const;
	
	/**
	 * Doesn't work. Don't call.
	 * @param  b		true or false
	 */
	void				setPeakAutoAdjust(bool b);
	
	/**
	 * Set the desired output frequency
	 * It's up the the driver if the wanted frequency is possible or not
	 * Note that there is no fallback, if the driver can't handle the frequency
	 * your song will probably sound to fast or to slow
	 * @param  frequency	output frequency for the mixer
	 */
	mp_sint32			adjustFrequency(mp_uint32 frequency);

	/**
	 * Get the current mixer output frequency
	 * @return			output frequency 
	 * @see				adjustFrequency
	 */
	mp_sint32			getMixFrequency() const;
	
	/**
	 * Convert number of beat backets to buffer size:
	 * The ChannelMixer class uses a fixed 250Hz timer so the mixer size is
	 * always a multiple of CurrentOutputFrequency / 250
	 * E.g. if you're mixing at 44100Hz the buffer size is always a multiple
	 * of 176 samples. 
	 * Thus specifying a value of 10 will result in a buffer of 1760 samples size.
	 * Important: Some sound device drivers don't allow buffer sizes which are
	 * not multiple of 2^n that's why on some systems setting up the sound 
	 * device with such a buffer size might not work. Use adjustBufferSize with
	 * a 2^n buffer size or call setPowerOfTwoCompensationFlag(true).	 
	 *
	 * @param  numBeats     The number of beat "packets"
	 */
	mp_sint32			beatPacketsToBufferSize(mp_uint32 numBeats);

	/**
	 * Here for legacy reasons, sets the buffer size according to the number
	 * of beat packets (see function above)
	 */
	mp_sint32			adjustBufferSize(mp_uint32 numBeats);

	/**
	 * Set the desired buffer size
	 * Important: Some sound device drivers don't allow buffer sizes which are
	 * not multiple of 2^n that's why on some systems you might call 
	 * setPowerOfTwoCompensationFlag(true) to always round to the next 2^n buffer size
	 *
	 * @param  bufferSize	buffer size description 
	 */
	mp_sint32			setBufferSize(mp_uint32 bufferSize);

	/**
	 * Tell the sound driver to force forcing 2^n buffer blocks if possible
	 * @param  b		true or false
	 * @see				getPowerOfTwoCompensationFlag, audioDriverSupportsPowerOfTwoCompensation
	 */
	mp_sint32			setPowerOfTwoCompensationFlag(bool b);
	
	/**
	 * Query if the sound driver is currently forcing 2^n buffer blocks
	 * @return			true or false
	 * @see				setPowerOfTwoCompensationFlag, audioDriverSupportsPowerOfTwoCompensation
	 */
	bool				getPowerOfTwoCompensationFlag() const;

	/**
	 * Query the the sound drivers name
	 * @return			Name of the audio driver. Do NOT save this value. It's only temporary and may change.
	 */
	const char*			getCurrentAudioDriverName() const;
	
	/**
	 * Select an audio driver from the available list of audio drivers
	 * specified by the given name
	 * If the return value is false the select has failed but
	 * the default driver has been selected instead so playback
	 * is still possible
	 * @param	name	Full qualified name of the audio driver
	 * @return  true or false indicating if the selection has succeeded
	 */
	bool				setCurrentAudioDriverByName(const char* name);

	/**
	 * Get state information about the mixer: Check if the mixer has been initialized
	 * @return			true or false
	 */
	bool				isInitialized() const;

	/**
	 * Get state information about the player: Check if we're playing a song
	 * @return			true or false
	 */
	bool				isPlaying() const;
	
	/**
	 * Get how many samples have been played since the player has started playing
	 * this is only 32 bit and might overflow quickly depending on your output frequency
	 * @return			number of samples played
	 */
	mp_int64			getSampleCounter() const;

	/**
	 * Reset the sample counter
	 * @see				getSampleCounter
	 */
	void				resetSampleCounter();
	
	/**
	 * For hi latency audio drivers this might return 
	 * a pointer to the sample which is currently played
	 * in the mixing BUFFER!!! Don't depend on this function
	 * it might not work on your system
	 * @return			the position of the driver within the sample buffer
	 */
	mp_sint32			getCurrentSamplePosition() const;

	mp_sint32			getCurrentBeatIndex();
	
	/**
	 * Get a sample from the current mixing buffer
	 * a pointer to the sample which is currently played
	 * in the mixing BUFFER!!! Don't depend on this function
	 * it might not work on your system
	 * @param  position	position within the mixing buffer
	 * @param  channel	the channel (0 = left, 1 = right [mixing is always stereo])
	 * @return			a sample between -32768 and 32768
	 */
	mp_sint32			getCurrentSample(mp_sint32 position, mp_sint32 channel);

	/**
	 * Get some peaks around the position "pos" in the current mixing buffer
	 * Just looks good when displayed, is not a perfect peak detector
	 * @param  position in the buffer, needs to be within the valid range
	 * @param  channel the channel (0 = left, 1 = right [mixing is always stereo])
	 */
	mp_sint32			getCurrentSamplePeak(mp_sint32 position, mp_sint32 channel);

	/**
	 * Reset channels. All channels will immediately stop playing after this has been called.
	 */
	void				resetChannels();

	/**
	 * How many active channels do we have?
	 * @return			return the number of channels which are active in the mixing process
	 */
	mp_sint32			getNumActiveChannels() const;

	/**
	 * How many channels have been allocated for playback?
	 * @return			return the number of channels allocated for playback
	 */
	mp_sint32			getNumAllocatedChannels() const;

	///////////////////////////////////////////////////////////////////////////////////////////
	// -------------------------- wrapping player specific stuff ------------------------------
	///////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Set a specified play mode
	 * @param  mode		play mode
	 * @see				PlayModeSettings
	 */
	void				setPlayMode(PlayModes mode);
	
	/**
	 * Get current selected play mode
	 * @return			current play mode
	 * @see				setPlayMode
	 */
	PlayModes			getPlayMode() const;

	/**
	 * Switch special playmode flags, only applies to players
	 * which are able to play the kind of modules of interest
	 * @param  option	option to enable or disable
	 * @param  b		enable or disable option
	 */
	void				enable(PlayModeOptions option, bool b);

	/**
	 * See if a special flags is enabled for the current player
	 * @param  option	option to enable or disable
	 * @return			true if option is enabled, false otherwise
	 */
	bool				isEnabled(PlayModeOptions option) const;
	
	/**
	 * Restart playback
	 * @param  startPosition		start position within the song
	 * @param  startRow				start position within the pattern
	 * @param  resetMixer			whether or not to reset the mixer
	 * @param  customPanningTable	When specifying a custom panning table the panning default from the module is ignored
	 * @param  playOneRowOnly		stop after playing exactly ONE row
	 */
	void				restart(mp_uint32 startPosition = 0, mp_uint32 startRow = 0, bool resetMixer = true, const mp_ubyte* customPanningTable = NULL, bool playOneRowOnly = false);
	
	/**
	 * Reset everything. Includes audio device if there is one.
	 */
	void				reset();
	
	/**
	 * Reset speed settings to the default. 
	 */	
	void				resetAllSpeed();

	/**
	 * Start playback
	 * @param  module				module to play
	 * @param  repeat				repeat song?
	 * @param  startPosition		start position within the song
	 * @param  startRow				start position within the pattern
	 * @param  numChannels			number of channels to be allocated for playback, supply -1 for auto adjust
	 * @param  customPanningTable	When specifying a custom panning table the panning default from the module is ignored
	 * @param  idle					whether or not to start in idle mode
	 * @param  patternIndex			this specifies to play a given pattern only, supply -1 to play entire song instead of just one pattern
	 */
	mp_sint32			startPlaying(XModule* module, 
									 bool repeat = false, 
									 mp_uint32 startPosition = 0, 
									 mp_uint32 startRow = 0,
									 mp_sint32 numChannels = -1, 
									 const mp_ubyte* customPanningTable = NULL,
									 bool idle = false,
									 mp_sint32 patternIndex = -1,
									 bool playOneRowOnly = false);

	/**
	 * Specify a pattern to be played instead of playing the entire song
	 * @param  patternIndex		this specifies to play a given pattern only, supply -1 to play entire song instead of just one pattern
	 */
	void				setPatternToPlay(mp_sint32 patternIndex);

	/**
	 * Stop playing, also stops audio device (easy, huh?)
	 * @return			error code (0 = everything is fine)
	 */
	mp_sint32			stopPlaying();
	
	/**
	 * Check if song has been stopped (either the song did something stupid or it played once and repeat is false)
	 * NOTE: This will not mean that audio streaming has stopped, it only tells you that the song hit a position 
	 * that was already played and if it's not in repeat mode it will halt. 
	 * If the player is in repeat mode this will most probably always return false and even if it is true you need 
	 * to still call stopPlaying() if you want to play another song.
	 * @return			true or false
	 */
	bool				hasSongHalted() const;

	/**
	 * This is probably only used by MilkyTracker
	 * You can tell the core not to play any song, but the mixer is still active
	 * So if you play samples on the channels manually they will still be mixed 
	 * @param  idle		true = go into idle mode stop song but mixer stays active, false = start playing the song again
	 */
	void				setIdle(bool idle);

	/**
	 * Check if player is in idle state
	 * @return			true or false
	 * @see				setIdle
	 */
	bool				isIdle() const;

	/**
	 * Tell the player to enable/disable repeating
	 * @param  repeat	true or false
	 */
	void				setRepeat(bool repeat);

	/**
	 * Check if player is repeating
	 * @return			true or false
	 * @see				setRepeat
	 */
	bool				isRepeating() const;
		
	/**
	 * Pause the player
	 */
	mp_sint32			pausePlaying();	

	/**
	 * Resume from paused state
	 * @see				pausePlaying
	 */
	mp_sint32			resumePlaying();

	/**
	 * Check if player is in paused state
	 * @return			true or false
	 * @see				pausePlaying
	 */
	bool				isPaused() const;
	
	/**
	 * Turn mixer off, but song is still played
	 * This is used for calculating the song length:
	 * Song is played internally but the entire mixer 
	 * is disabled so playing will be really fast
	 * @param  b		true or false
	 */
	void				setDisableMixing(bool b);

	/**
	 * Allow DSP filters. 
	 * IT uses a low pass resonanance IIR filter
	 * which can be disabled to save CPU power.
	 * @param  b		true or false
	 */
	void				setAllowFilters(bool b);

	/**
	 * Tell if filters are allowed.
	 * @return			true if filters are enabled.
	 * @see				setAllowFilters
	 */
	bool				getAllowFilters() const;
	
	/**
	 * Set master volume for the mixer
	 * @param  vol		Master volume between 0 and 256
	 */
	void				setMasterVolume(mp_sint32 vol);

	/**
	 * Return the master volume for the mixer
	 * @return			master volume (usually a volume between 0 and 256)
	 * @see				setMasterVolume
	 */
	mp_sint32			getMasterVolume() const;

	/**
	 * Set panning separation for the mixer
	 * @param  separation	Panning separation between 0 (none) and 256 (full)
	 */
	void				setPanningSeparation(mp_sint32 separation);

	/**
	 * Return the panning separation for the mixer
	 * @return			Panning separation between 0 (none) and 256 (full)
	 * @see				setPanningSeparation
	 */
	mp_sint32			getPanningSeparation() const;
	
	/**
	 * Return the main volume of the currently played song, this is NOT the mixer master volume
	 * @return			main volume between 0 and 255
	 */
	mp_sint32			getSongMainVolume() const;

	/**
	 * Which row in the current pattern is the player at?
	 * @return			current row in current pattern 
	 */
	mp_sint32			getRow() const;

	/**
	 * Which order position is the player at?
	 * @return			order position from 0 to 255 (might return bigger values in future)
	 */
	mp_sint32			getOrder() const;

	/**
	 * Return both, order and row at the same time, 
	 * might be needed for sync reasons (MilkyTracker)
	 * Retrieving the row requires very low latency output, so it might
	 * not be as precise as you want to have it
	 * @param  order	reference to an 32 bit integer to hold the order
	 * @param  row		reference to an 32 bit integer to hold the row
	 */
	void				getPosition(mp_sint32& order, mp_sint32& row) const;

	mp_sint32			getLastUnvisitedPosition() const;

	/**
	 * Return order, row and ticker at the same time, 
	 * might be needed for sync reasons (MilkyTracker)
	 * Retrieving the ticker/row requires very low latency output, so it might
	 * not be as precise as you want to have it
	 * @param  order	reference to an 32 bit integer to hold the order
	 * @param  row		reference to an 32 bit integer to hold the row
	 * @param  ticker	reference to an 32 bit integer to hold the ticker
	 */
	void				getPosition(mp_sint32& order, mp_sint32& row, mp_sint32& ticker) const;

	/**
	 * Return the number of rows played since the player has started
	 * Might become handy for sync reasons.
	 * @return			number of rows played since start
	 */
	mp_int64			getSyncCount() const;

	/**
	 * Return number of samples played but this time the
	 * audio driver will return this value.
	 * This is also used for VERY accurate synching especially
	 * using MMSYSTEM. The value will be resettet on calling stopPlaying()
	 * @return			number of samples the audiodriver has played since start
	 */
	mp_uint32			getSyncSampleCounter() const;
	
	/**
	 * Jump to next pattern
	 */
	void				nextPattern();
	
	/**
	 * Jump to previous pattern
	 */
	void				lastPattern();
	
	/**
	 * Select a new position within the song
	 * @param  pos				new order position
	 * @param  row				new row
	 * @param  resetChannels	reset channels, yes or no
	 */
	void				setPatternPos(mp_uint32 pos, mp_uint32 row = 0, bool resetChannels = true, bool resetFXMemory = true);
	
	/**
	 * Return the tempo of the song at the current position (in BPM)
	 * When there is no song playing the last active tempo will be returned
	 * @return			current tempo in BPM
	 */
	mp_sint32			getTempo() const;

	/**
	 * Return the speed of the current position in ticks
	 * When there is no song playing the last active tick speed will be returned
	 * @return			current speed in ticks
	 */
	mp_sint32			getSpeed() const;
	
	/**
	 * Tell the player to reset the channels when the song stops (= is played once)
	 * When the player is in repeat mode the channels will never be resettet, not
	 * even when the song loops
	 * @param  b		reset on stop, yes or no
	 */	
	void				resetOnStop(bool b);

	/**
	 * Tell the player to reset the main volume of the player (not the mixer master volume)
	 * when the song is restarted (MilkyTracker does not want the volume to be reset)
	 * @param  b		reset volume on start, yes or no
	 */	
	void				resetMainVolumeOnStartPlay(bool b);
	
	/**
	 * Export the song as WAV file
	 * @param  fileName				the path and the filename to export to
	 * @param  module				the module to export
	 * @param  startOrder			the start position within the order list of the song
	 * @param  endOrder				the last order to be played
	 * @param  mutingArray			optional: an array telling which channels to mute
	 * @param  mutingNumChannels	optional: many channels does the muting array contain?
	 * @param  customPanningTable	When specifying a custom panning table the panning default from the module is ignored
	 * @param  preferredDriver		optional: specify another output audio driver here (NULL = WAV driver)
	 * @param  timingLUT			optional: specify a pointer to a buffer which will hold the 
	 *										  number of samples played up to this position in the orderlist
	 *										  the buffer needs at least module->header.ordnum entries
	 */	
	mp_sint32			exportToWAV(const SYSCHAR* fileName, 
									XModule* module, 
									mp_sint32 startOrder = 0, mp_sint32 endOrder = -1, 
									const mp_ubyte* mutingArray = NULL, mp_uint32 mutingNumChannels = 0,
									const mp_ubyte* customPanningTable = NULL,
									AudioDriverBase* preferredDriver = NULL,
									mp_sint32* timingLUT = NULL);
	
	/**
	 * Grab current channel data from a module channel
	 * @param  chn					the channel index to grab the data from
	 * @param  channelInfo			reference to a channelInfo structure to be filled
	 */
	bool				grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const;
	
	/**
	 * Set the maximum number of virtual channels to be allocated while playing.
	 * This might reduce CPU consumption a lot but only if the current player is using
	 * virtual channels (like PlayerIT for example). If the current player doesn't 
	 * support virtual channels, this function has no effect.
	 *
	 * IMPORTANT: call this only before startPlaying, otherwise it will have no effect.
	 *
	 * @param  max		maximum amount of virtual channels (default is 256)
	 */
	void				setNumMaxVirChannels(mp_sint32 max);

	/**
	 * Return the maximum number of virtual channels to be allocated while playing.
	 * @return			maximum amount of virtual channels (default is 256)
	 */
	mp_sint32			getNumMaxVirChannels() const;

	// ---------------------------- milkytracker ----------------------------
	/**
	 * Change panning of a current playing channel
	 * @param  chn	the channel
	 * @param  pan	new panning value Left(0..255)Right
	 */	
	void				setPanning(mp_ubyte chn, mp_ubyte pan);

	/**
	 * Return the currently used player instance (handle with care)
	 * @return				a PlayerBase instance is returned or NULL if there is none right now
	 */
	PlayerBase*			getPlayerInstance() { return player; }	
	
	friend class MixerNotificationListener;
};

#endif

