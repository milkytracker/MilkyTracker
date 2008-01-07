/*
 *  milkyplay/PlayerFAR.h
 *
 *  Copyright 2008 Peter Barth
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
 *  PlayerFAR.h
 *  MilkyPlay core
 *
 *  Created by Peter Barth on Thu Jan 20 2005.
 *
 */
#ifndef __PLAYERFAR_H__
#define __PLAYERFAR_H__

#include "PlayerBase.h"

class PlayerFAR : public PlayerBase
{
private:
	static mp_uword Freqs[];
	static mp_uword VolTab[];
	static mp_sint32 SinTable[16][128];
	static mp_sint32 mTempo[16];

	// from TRAK.C
	mp_uword	OverFlow,OCount,PlayOrder;
	mp_sint32   PitchWheel[16],DestPitch[16],Increment[16],PresPitch[16];
	mp_sint32   VolWheel[16],DestVol[16],VIncrement[16],PresVol[16];
	mp_sint32   RetLeft[16],RetSpc[16],CurSpc[16],RetSmp[16],RetVol[16];
	mp_sint32   OfftCnt[16],RetCnt[16];
	mp_sint32   VibOn[16];
	mp_sint32   VibPtr[16],VibInc[16];
	mp_sint32   CurVols[16];
	mp_ubyte	CurBalance[16];
	mp_sint32   CurFreqs[16];
	mp_sint32	CurNote[16];
	mp_sint32	CurEff[16];
	mp_sint32   VibAmp;
	mp_ubyte	TempoType;
	mp_sint32   PlayTempo,TempoBend,CurVoice,CurPattern;
	mp_sint32   CurChSmp[16];
	mp_ubyte	GrabBuf[64];
	
private:
	void					CalcTempo();
	void					UpdateTempo(mp_sint32 tps);

	void					SetFreq(mp_sint32 chn, mp_sint32 freq);
	void					SetVolume(mp_sint32 chn, mp_sint32 vol);
	void					SetBalance(mp_sint32 chn, mp_ubyte balance);

protected:
	virtual void			clearEffectMemory();

public:
							PlayerFAR(mp_uint32 frequency);
	
	virtual					~PlayerFAR();

	virtual PlayerTypes		getType() const { return PlayerType_FAR; }

	// virtual from mixer class, perform playing here
	virtual void			timerHandler(mp_sint32 currentBeatPacket);
	
	virtual void			restart(mp_uint32 startPosition = 0, mp_uint32 startRow = 0, bool resetMixer = true, const mp_ubyte* customPanningTable = NULL, bool playOneRowOnly = false); 

	virtual bool			grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const;
};

#endif
