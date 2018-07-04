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
 *  PlayerSTD.cpp
 *  MilkyPlay standard player
 *
 *
 */
#include "PlayerSTD.h"

#define CHANNEL_FLAGS_DVS				0x10000
#define CHANNEL_FLAGS_DFS				0x20000
#define CHANNEL_FLAGS_FORCE_FORWARD		0x00001
#define CHANNEL_FLAGS_FORCE_BACKWARD	0x00002
#define CHANNEL_FLAGS_FORCE_BILOOP		0x00004
#define CHANNEL_FLAGS_UPDATE_IGNORE		0x00100

//#define MINPERIOD (113*4)

#define FREEMEMORY \

#define CLEAROLDOPERANDS \
{ \
	for (mp_sint32 i = 0; i < numChannels; i++) { \
		chninfo[i].ins = 0; \
		/*memset(&chninfo[i].old,0,sizeof(TLastOperands));*/ \
	} \
}

// must be called after the poscnt has been properly set
#define RESETLOOPING \
{ \
	chnInf->loopstart=chnInf->loopcounter=chnInf->execloop=0; \
	chnInf->isLooping = false; \
	chnInf->loopingValidPosition = poscnt; \
} 	

#define RESET_ALL_LOOPING \
{ \
	for (mp_sint32 c = 0; c < module->header.channum; c++) \
	{ \
		TModuleChannel *chnInf = &chninfo[c]; \
		RESETLOOPING \
	} \
} 	

const mp_sint32	PlayerSTD::vibtab[32] = {0,24,49,74,97,120,141,161,
										 180,197,212,224,235,244,250,253,
										 255,253,250,244,235,224,212,197,
									     180,161,141,120,97,74,49,24};

const mp_uword PlayerSTD::lintab[768+1] = {
		16726,16741,16756,16771,16786,16801,16816,16832,16847,16862,16877,16892,16908,16923,16938,16953,
		16969,16984,16999,17015,17030,17046,17061,17076,17092,17107,17123,17138,17154,17169,17185,17200,
		17216,17231,17247,17262,17278,17293,17309,17325,17340,17356,17372,17387,17403,17419,17435,17450,
		17466,17482,17498,17513,17529,17545,17561,17577,17593,17608,17624,17640,17656,17672,17688,17704,
		17720,17736,17752,17768,17784,17800,17816,17832,17848,17865,17881,17897,17913,17929,17945,17962,
		17978,17994,18010,18027,18043,18059,18075,18092,18108,18124,18141,18157,18174,18190,18206,18223,
		18239,18256,18272,18289,18305,18322,18338,18355,18372,18388,18405,18421,18438,18455,18471,18488,
		18505,18521,18538,18555,18572,18588,18605,18622,18639,18656,18672,18689,18706,18723,18740,18757,
		18774,18791,18808,18825,18842,18859,18876,18893,18910,18927,18944,18961,18978,18995,19013,19030,
		19047,19064,19081,19099,19116,19133,19150,19168,19185,19202,19220,19237,19254,19272,19289,19306,
		19324,19341,19359,19376,19394,19411,19429,19446,19464,19482,19499,19517,19534,19552,19570,19587,
		19605,19623,19640,19658,19676,19694,19711,19729,19747,19765,19783,19801,19819,19836,19854,19872,
		19890,19908,19926,19944,19962,19980,19998,20016,20034,20052,20071,20089,20107,20125,20143,20161,
		20179,20198,20216,20234,20252,20271,20289,20307,20326,20344,20362,20381,20399,20418,20436,20455,
		20473,20492,20510,20529,20547,20566,20584,20603,20621,20640,20659,20677,20696,20715,20733,20752,
		20771,20790,20808,20827,20846,20865,20884,20902,20921,20940,20959,20978,20997,21016,21035,21054,
		21073,21092,21111,21130,21149,21168,21187,21206,21226,21245,21264,21283,21302,21322,21341,21360,
		21379,21399,21418,21437,21457,21476,21496,21515,21534,21554,21573,21593,21612,21632,21651,21671,
		21690,21710,21730,21749,21769,21789,21808,21828,21848,21867,21887,21907,21927,21946,21966,21986,
		22006,22026,22046,22066,22086,22105,22125,22145,22165,22185,22205,22226,22246,22266,22286,22306,
		22326,22346,22366,22387,22407,22427,22447,22468,22488,22508,22528,22549,22569,22590,22610,22630,
		22651,22671,22692,22712,22733,22753,22774,22794,22815,22836,22856,22877,22897,22918,22939,22960,
		22980,23001,23022,23043,23063,23084,23105,23126,23147,23168,23189,23210,23230,23251,23272,23293,
		23315,23336,23357,23378,23399,23420,23441,23462,23483,23505,23526,23547,23568,23590,23611,23632,
		23654,23675,23696,23718,23739,23761,23782,23804,23825,23847,23868,23890,23911,23933,23954,23976,
		23998,24019,24041,24063,24084,24106,24128,24150,24172,24193,24215,24237,24259,24281,24303,24325,
		24347,24369,24391,24413,24435,24457,24479,24501,24523,24545,24567,24590,24612,24634,24656,24679,
		24701,24723,24746,24768,24790,24813,24835,24857,24880,24902,24925,24947,24970,24992,25015,25038,
		25060,25083,25105,25128,25151,25174,25196,25219,25242,25265,25287,25310,25333,25356,25379,25402,
		25425,25448,25471,25494,25517,25540,25563,25586,25609,25632,25655,25678,25702,25725,25748,25771,
		25795,25818,25841,25864,25888,25911,25935,25958,25981,26005,26028,26052,26075,26099,26123,26146,
		26170,26193,26217,26241,26264,26288,26312,26336,26359,26383,26407,26431,26455,26479,26502,26526,
		26550,26574,26598,26622,26646,26670,26695,26719,26743,26767,26791,26815,26839,26864,26888,26912,
		26937,26961,26985,27010,27034,27058,27083,27107,27132,27156,27181,27205,27230,27254,27279,27304,
		27328,27353,27378,27402,27427,27452,27477,27502,27526,27551,27576,27601,27626,27651,27676,27701,
		27726,27751,27776,27801,27826,27851,27876,27902,27927,27952,27977,28003,28028,28053,28078,28104,
		28129,28155,28180,28205,28231,28256,28282,28307,28333,28359,28384,28410,28435,28461,28487,28513,
		28538,28564,28590,28616,28642,28667,28693,28719,28745,28771,28797,28823,28849,28875,28901,28927,
		28953,28980,29006,29032,29058,29084,29111,29137,29163,29190,29216,29242,29269,29295,29322,29348,
		29375,29401,29428,29454,29481,29507,29534,29561,29587,29614,29641,29668,29694,29721,29748,29775,
		29802,29829,29856,29883,29910,29937,29964,29991,30018,30045,30072,30099,30126,30154,30181,30208,
		30235,30263,30290,30317,30345,30372,30400,30427,30454,30482,30509,30537,30565,30592,30620,30647,
		30675,30703,30731,30758,30786,30814,30842,30870,30897,30925,30953,30981,31009,31037,31065,31093,
		31121,31149,31178,31206,31234,31262,31290,31319,31347,31375,31403,31432,31460,31489,31517,31546,
		31574,31602,31631,31660,31688,31717,31745,31774,31803,31832,31860,31889,31918,31947,31975,32004,
		32033,32062,32091,32120,32149,32178,32207,32236,32265,32295,32324,32353,32382,32411,32441,32470,
		32499,32529,32558,32587,32617,32646,32676,32705,32735,32764,32794,32823,32853,32883,32912,32942,
		32972,33002,33031,33061,33091,33121,33151,33181,33211,33241,33271,33301,33331,33361,33391,33421,
		33451 // one more value because of linear interpolation
};

#define LOGFAC 2*16

const mp_uint32 PlayerSTD::logtab[13*8+1] = {
		LOGFAC*907,LOGFAC*900,LOGFAC*894,LOGFAC*887,LOGFAC*881,LOGFAC*875,LOGFAC*868,LOGFAC*862,
		LOGFAC*856,LOGFAC*850,LOGFAC*844,LOGFAC*838,LOGFAC*832,LOGFAC*826,LOGFAC*820,LOGFAC*814,
		LOGFAC*808,LOGFAC*802,LOGFAC*796,LOGFAC*791,LOGFAC*785,LOGFAC*779,LOGFAC*774,LOGFAC*768,
		LOGFAC*762,LOGFAC*757,LOGFAC*752,LOGFAC*746,LOGFAC*741,LOGFAC*736,LOGFAC*730,LOGFAC*725,
		LOGFAC*720,LOGFAC*715,LOGFAC*709,LOGFAC*704,LOGFAC*699,LOGFAC*694,LOGFAC*689,LOGFAC*684,
		LOGFAC*678,LOGFAC*675,LOGFAC*670,LOGFAC*665,LOGFAC*660,LOGFAC*655,LOGFAC*651,LOGFAC*646,
		LOGFAC*640,LOGFAC*636,LOGFAC*632,LOGFAC*628,LOGFAC*623,LOGFAC*619,LOGFAC*614,LOGFAC*610,
		LOGFAC*604,LOGFAC*601,LOGFAC*597,LOGFAC*592,LOGFAC*588,LOGFAC*584,LOGFAC*580,LOGFAC*575,
		LOGFAC*570,LOGFAC*567,LOGFAC*563,LOGFAC*559,LOGFAC*555,LOGFAC*551,LOGFAC*547,LOGFAC*543,
		LOGFAC*538,LOGFAC*535,LOGFAC*532,LOGFAC*528,LOGFAC*524,LOGFAC*520,LOGFAC*516,LOGFAC*513,
		LOGFAC*508,LOGFAC*505,LOGFAC*502,LOGFAC*498,LOGFAC*494,LOGFAC*491,LOGFAC*487,LOGFAC*484,
		LOGFAC*480,LOGFAC*477,LOGFAC*474,LOGFAC*470,LOGFAC*467,LOGFAC*463,LOGFAC*460,LOGFAC*457,
		LOGFAC*453,LOGFAC*450,LOGFAC*447,LOGFAC*443,LOGFAC*440,LOGFAC*437,LOGFAC*434,LOGFAC*431,
		LOGFAC*428 // one more value because of linear interpolation
};

// This takes the period with 8 bit fractional part
mp_sint32	PlayerSTD::getlinfreq(mp_sint32 per)
{
	if (per<0) per=0;
	if (per>7680*256) per=7680*256;
	
	mp_sint32 t = (7680*256-per)/(768*256);
	mp_sint32 r = (7680*256-per)%(768*256);
	
	// Linear interpolation seems to be wrong here
	/*mp_sint32 frac = r & 255;
	
	mp_sint32 r1 = ((lintab[r>>8])<<t)>>5;
	mp_sint32 r2 = ((lintab[(r>>8)+1])<<t)>>5;

	return ((255-frac)*r1 + frac*r2) >> 8;*/

	return ((lintab[r>>8])<<t)>>5;
}

// This takes the period with 8 bit fractional part
mp_sint32	PlayerSTD::getlogfreq(mp_sint32 per) 
{ 
	return fixeddiv(14317056, per)>>8; 
}

mp_sint32	PlayerSTD::getlinperiod(mp_sint32 note,mp_sint32 relnote,mp_sint32 finetune) 
{
	note+=relnote;
	
	if (note<1) note=1;
	if (note>XModule::NOTE_LAST) note=XModule::NOTE_LAST;
	
	return ((7680-((note-1)<<6)-(finetune/2)));	
}

mp_sint32	PlayerSTD::interpolate(mp_sint32 eax,mp_sint32 ebx,mp_sint32 ecx,mp_sint32 edi,mp_sint32 esi)
{
	if (ebx==ecx) return edi;
	mp_sint32 di = ((eax-ebx)*(esi-edi))/(ecx-ebx)+edi;
	return (mp_sint32)di;
}

mp_sint32	PlayerSTD::getlogperiod(mp_sint32 note,mp_sint32 relnote,mp_sint32 finetune)
{
	note+=relnote;

	if (note<1) note=1;
	if (note>XModule::NOTE_LAST) note=XModule::NOTE_LAST;
	
	mp_sint32 ft = finetune;
	ft+=128;
	mp_sint32 octave = (note-1)/12;
	mp_sint32 n = ((note-1)%12)<<3;
	mp_sint32 pi = (ft>>4)+n;
	mp_sint32 v1 = logtab[pi];
	mp_sint32 v2 = logtab[pi+1];
	mp_sint32 t = (ft>>4)-8;
	//mp_sint32 t = (ft>>4);
	return interpolate(t,0,15,v1,v2)>>octave;
}


PlayerSTD::PlayerSTD(mp_uint32 frequency,
					 StatusEventListener* statusEventListener/* = NULL*/) : 
	PlayerBase(frequency),
	statusEventListener(statusEventListener),
	chninfo(NULL),
	lastNumAllocatedChannels(-1)
{
	smpoffs = NULL;
	attick	= NULL;	

	// fill in some default values, don't know if this is necessary
	tickSpeed		= 6;				// our tickspeed
	bpm				= 125;				// BPM speed
	ticker			= tickSpeed-1;		// runs from 0 to tickspeed-1
	patternIndex	= 0;				// holds current pattern index
	numEffects		= 0;				// current number of effects
	numChannels		= 0;				// current number of channels
	
	patDelay = false;
	patDelayCount = 0;
	haltFlag = false;

	options[PlayModeOptionPanning8xx] = true;
	options[PlayModeOptionPanningE8x] = false;
	options[PlayModeOptionForcePTPitchLimit] = true;
}

PlayerSTD::~PlayerSTD()
{
	freeMemory();
}

mp_sint32 PlayerSTD::adjustFrequency(mp_uint32 frequency)
{
	mp_uint32 lastNumBeatPackets = getNumBeatPackets()+1;

	mp_sint32 res = PlayerBase::adjustFrequency(frequency);
	
	if (res < 0)
		return res;
		
	// nothing has changed
	if (lastNumBeatPackets == getNumBeatPackets()+1)
		return MP_OK;

	res = allocateStructures();
	
	return res;
}

mp_sint32 PlayerSTD::setBufferSize(mp_uint32 bufferSize)
{
	mp_uint32 lastNumBeatPackets = getNumBeatPackets()+1;

	mp_sint32 res = PlayerBase::setBufferSize(bufferSize);
	
	if (res < 0)
		return res;
		
	// nothing has changed
	if (lastNumBeatPackets == getNumBeatPackets()+1)
		return MP_OK;

	res = allocateStructures();
	
	return res;
}

void PlayerSTD::timerHandler(mp_sint32 currentBeatPacket)
{
	PlayerBase::timerHandler(currentBeatPacket);

	if (paused)
		return;

	if (statusEventListener)
		statusEventListener->timerTickStarted(*this, *module);

	mp_int64 dummy = (mp_int64)BPMCounter;
	dummy+=(mp_int64)adder;
	BPMCounter=(mp_sint32)dummy;
	
	// check overflow-carry 
	if ((dummy>>32)) 
	{
#ifdef MILKYTRACKER
		setActiveChannels(initialNumChannels);
#else
		setActiveChannels(module->header.channum);
#endif

		if (statusEventListener)
			statusEventListener->playerTickStarted(*this, *module);

		tickhandler();

		if (statusEventListener)
			statusEventListener->playerTickEnded(*this, *module);
	}
	
	if (module->header.flags & XModule::MODULE_AMSENVELOPES)
		updateBPMIndependent();

#ifdef MILKYTRACKER
	TModuleChannel* chnInf = chninfo;
	for (mp_sint32 i = 0; i < initialNumChannels; i++)
	{
		// track envelope position state
		TPrEnv* env = &chnInf->venv;		
		if ((env->envstruc && (env->envstruc->type & 1) && env->envstruc->num) &&
			(env->step < env->envstruc->env[env->envstruc->num-1][0]))
		{
			env->timeRecord[currentBeatPacket].pos = env->step;
			env->timeRecord[currentBeatPacket].envstruc = env->envstruc;
		}
		else
		{
			env->timeRecord[currentBeatPacket].pos = 0;
			env->timeRecord[currentBeatPacket].envstruc = NULL;
		}
		
		env = &chnInf->penv;		
		if ((env->envstruc && (env->envstruc->type & 1) && env->envstruc->num) &&
			(env->step < env->envstruc->env[env->envstruc->num-1][0]))
		{
			env->timeRecord[currentBeatPacket].pos = env->step;
			env->timeRecord[currentBeatPacket].envstruc = env->envstruc;
		}
		else
		{
			env->timeRecord[currentBeatPacket].pos = 0;
			env->timeRecord[currentBeatPacket].envstruc = NULL;
		}
								
		chnInf++;
	}
#endif

	if (statusEventListener)
		statusEventListener->timerTickStarted(*this, *module);
}

void PlayerSTD::restart(mp_uint32 startPosition/* = 0*/, mp_uint32 startRow/* = 0*/, bool resetMixer/* = true*/, const mp_ubyte* customPanningTable/* = NULL*/, bool playOneRowOnly/* = false*/)
{
	if (chninfo == NULL) 
		return;

	bpm	= module->header.speed;
	tickSpeed = module->header.tempo;
	ticker = 0;

	// after the speed has been assigned, it's time to call PlayerBase::restart
	PlayerBase::restart(startPosition, startRow, resetMixer, customPanningTable, playOneRowOnly);

	this->adder = getbpmrate(this->bpm);

	mp_sint32 i,j;

	// clean up player specific variables
	patternIndex	= 0;
	numEffects		= 0;
	numChannels		= 0;

	patDelay		= false;
	patDelayCount   = 0;
	haltFlag		= false;

	startNextRow	= -1;
	
	reset();

	for (i = 0; i < initialNumChannels; i++)
		chninfo[i].masterVol = 0xff;
	
	for (i = 0; i < module->header.channum; i++)
		chninfo[i].pan = customPanningTable ? customPanningTable[i] : module->header.pan[i];

	memset(rowHits, 0, sizeof(rowHits));
	
	for (i = 0; i < (signed)startPosition; i++)
		for (j = 0; j < 256; j++)
			visitRow(i*256+j);
	
	for (i = 0; i < (signed)startRow; i++)
		visitRow(startPosition*256+i);
}

void PlayerSTD::reset()
{
	for (mp_sint32 i = 0; i < initialNumChannels; i++)
		chninfo[i].clear();

	RESET_ALL_LOOPING
}

void PlayerSTD::resetAllSpeed()
{
	bpm	= module->header.speed;
	tickSpeed = module->header.tempo;
	ticker = 0;

	this->adder = getbpmrate(this->bpm);	
}

mp_sint32 PlayerSTD::allocateStructures() 
{
	if (lastNumAllocatedChannels != initialNumChannels)
	{
		freeMemory();
		
		chninfo			= new TModuleChannel[initialNumChannels];
		
		smpoffs			= new mp_uint32[initialNumChannels];
		attick			= new mp_ubyte[initialNumChannels];
		
		lastNumAllocatedChannels = initialNumChannels;
	}
	
#ifdef MILKYTRACKER
	for (mp_sint32 i = 0; i < initialNumChannels; i++)
		chninfo[i].reallocTimeRecord(getNumBeatPackets()+1);
#endif	
	
	return MP_OK;
}

void PlayerSTD::freeMemory() 
{
	if (chninfo) 
	{ 
		delete[] chninfo; 
		chninfo = NULL; 
		lastNumAllocatedChannels = -1; 
	} 
	if (smpoffs) 
	{ 
		delete[] smpoffs; 
		smpoffs = NULL; 
	} 
	if (attick) 
	{ 
		delete[] attick; 
		attick = NULL; 
	}
}

///////////////////////////////////////////////////////////////////////////////////
//					 controlling current song position                           //
///////////////////////////////////////////////////////////////////////////////////
void PlayerSTD::clearEffectMemory()
{
	if (!module || !chninfo) 
		return;
	
	ticker = 0;
	
	//loopstart = execloop = loopcounter=0;
	mp_sint32 i;
	for (i = 0; i < module->header.channum; i++)
	{
		TModuleChannel *chnInf = &chninfo[i]; 
		RESETLOOPING 
	}
	
	patDelay = false;
	patDelayCount = 0;
	haltFlag = false;

	startNextRow = -1;

	memset(rowHits, 0, sizeof(rowHits));

	for (i = 0; i < poscnt; i++)
		for (mp_sint32 j = 0; j < 256; j++)
			visitRow(i*256+j);
	
	for (i = 0; i < (signed)rowcnt; i++)
		visitRow(poscnt*256+i);
}

void PlayerSTD::prenvelope(mp_sint32 c,TPrEnv *env,mp_sint32 keyon)
{
	if (env->envstruc!=NULL && (env->envstruc->type&1)) {
		// if we're sitting on a sustain point and key is on, we don't advance further
		if ((env->envstruc->type&2) && (env->a==env->envstruc->sustain) && 
			(env->step == env->envstruc->env[env->a][0]) && keyon) 
			return;
		
		if ((env->step != env->envstruc->env[env->b][0]) && (env->b < env->envstruc->num)) 
			env->step++;

		if (env->step == env->envstruc->env[env->b][0]) {
			
			if ((env->envstruc->type&4))
			{
				if ((!(env->envstruc->type&8) || keyon) &&
					!(!keyon && (env->envstruc->type&2) && env->envstruc->sustain == env->envstruc->loope)) // Break envelope if sustain pt == loop end point AND sustain is enabled AND key off is send
				{
					if (env->b==env->envstruc->loope) {
						env->a=env->envstruc->loops;
						env->b=env->envstruc->loops+1;
						env->step=env->envstruc->env[env->a][0];
						return;
					}
				}
			}	
			
			// Increase envelope position if there are more points to come
			if (env->b < env->envstruc->num - 1) {
				env->a++;
				env->b++;
			}
		}
		
	}

}

mp_sint32 PlayerSTD::getenvval(mp_sint32 c,TPrEnv *env,mp_sint32 n)
{
	if (env->envstruc==NULL) return n;
	
	if ((env->envstruc->type&1)) 
	{
		
		mp_sint32 dx = (env->envstruc->env[env->b][0]-env->envstruc->env[env->a][0]);
		if (dx==0) dx=1;
		mp_sint32 t = (env->envstruc->env[env->b][0]-env->step)*65536/dx;
		mp_sint32 y0 = env->envstruc->env[env->a][1];
		mp_sint32 y1 = env->envstruc->env[env->b][1];
		
		mp_sint32 y = (y0*t)+(y1*(65536-t));
		
		return y>>16;
		
	}
	
	else return n;
}

mp_sint32 PlayerSTD::getfinalperiod(mp_sint32 c,mp_sint32 p) 
{
	mp_sint32 envVib = 0;
	p<<=8;

	if (chninfo[c].vibenv.envstruc != NULL &&
		(chninfo[c].vibenv.envstruc->type&1))
	{
		mp_sint32 eval = (getenvval(c,&chninfo[c].vibenv,128)-128) << (chninfo[c].vibenv.envstruc->type>>6);
		// AMS doc says vibrato with amplify set to 8 equals vibrato 0xF
		// => alright
		envVib = (eval*61408)>>(3+16-8);
	}

	if (chninfo[c].avibused) {
		mp_ubyte vp = chninfo[c].avibcnt>>2;
		mp_ubyte vd = chninfo[c].avibdepth;
		
		mp_sint32 vm = 0;
		
		mp_sint32 vl = 0;
		switch (chninfo[c].avibused) {
			// sine (we must invert the phase here)
			case 1 : vl=~vibtab[vp&31]; break;
			// square
			case 2 : vl=255; break;
			// ramp down (down being the period here - so ramp frequency up ;)
			case 3 : {
						vl=((vp&31)*539087)>>16;
						if ((vp&63)>31) vl=255-vl;
					 }; break;
			// ramp up (up being the period here - so ramp frequency down ;)
			case 4 : {
						vl=((vp&31)*539087)>>16;
						if ((vp&63)>31) vl=255-vl;
						vl=-vl;
					 }; break;
		}
		
		vm = ((vl*vd)>>1);
		if (chninfo[c].avibsweep) {
			vm*=(mp_sint32)chninfo[c].avibswcnt * 256;
			vm/=chninfo[c].avibsweep;
			vm>>=8;
		}
		
		if ((vp&63)>31) vm=-vm;
		
		return (p+vm+envVib);
	}
	else return (p+envVib);
}

void PlayerSTD::playInstrument(mp_sint32 chn, TModuleChannel* chnInf, bool bNoRestart/* = false*/)
{
	if (chnInf->ins && chnInf->ins <= module->header.insnum) {
		if (module->instr[chnInf->ins-1].samp && chnInf->smp != -1)
		{
			chnInf->flags &= ~CHANNEL_FLAGS_UPDATE_IGNORE;
			
			mp_sint32 i = chnInf->smp;
			
			// start out with the flags for 16bit sample
			mp_sint32 flags = ((module->smp[i].type&16)>>4)<<2;
			// add looping + backward flags
			flags |= module->smp[i].type&(3+128);
			// one shot forward looping?
			flags |= module->smp[i].type & 32;
			
			// force forward playing
			if (chnInf->flags & CHANNEL_FLAGS_FORCE_FORWARD)
				flags &= ~128;
			
			// force backward playing
			if (chnInf->flags & CHANNEL_FLAGS_FORCE_BACKWARD)
				flags |= 128;
			
			if (flags&3) 
			{
				
				if (chnInf->flags & CHANNEL_FLAGS_FORCE_BILOOP)
					flags = (flags & ~3) | 2;
				
				// bNoRestart = false means play new sample from beginning or sample offset
				if (!bNoRestart)
				{
					playSample(chn,
							   (mp_sbyte*)module->smp[i].sample,
							   module->smp[i].samplen,											   
							   smpoffs[chn],
							   0, // sample offset fraction
							   !playModeChopSampleOffset,
							   module->smp[i].loopstart,
							   module->smp[i].loopstart+module->smp[i].looplen,
							   flags);
				}
				// bNoRestart = true means play new sample from beginning of the last sample
				else
				{
					mp_sint32 smpoffset = smpoffs[chn] ? smpoffs[chn] : getSamplePos(chn);
					mp_sint32 smpoffsetfrac = smpoffs[chn] ? 0 : getSamplePosFrac(chn);
				
					playSample(chn,
							   (mp_sbyte*)module->smp[i].sample,
							   module->smp[i].samplen,											   
							   smpoffset,
							   smpoffsetfrac, // sample offset fraction
							   true,
							   module->smp[i].loopstart,
							   module->smp[i].loopstart+module->smp[i].looplen,
							   flags);
				}
			}
			else
			{
				
				// bNoRestart = false means play new sample from beginning or sample offset
				if (!bNoRestart)
				{
					playSample(chn,(mp_sbyte*)module->smp[i].sample,
							   module->smp[i].samplen,
							   smpoffs[chn],
							   0, // sample offset fraction
							   !playModeChopSampleOffset,
							   0,
							   module->smp[i].samplen,
							   flags);
				}
				// bNoRestart = true means play new sample from beginning of the last sample AND don't ramp volume up
				else
				{
					mp_sint32 smpoffset = smpoffs[chn] ? smpoffs[chn] : getSamplePos(chn);
					mp_sint32 smpoffsetfrac = smpoffs[chn] ? 0 : getSamplePosFrac(chn);

					playSample(chn,(mp_sbyte*)module->smp[i].sample,
							   module->smp[i].samplen,
							   smpoffset,
							   smpoffsetfrac, // sample offset fraction
							   true,
							   0,
							   module->smp[i].samplen,
							   flags);
				}
			}
			
		}
		else
		{
			stopSample(chn);
		}
	}
}

void PlayerSTD::updatePlayModeFlags()
{
	// the following flags are exclusive
	newInsPTFlag = (module->header.flags & XModule::MODULE_PTNEWINSTRUMENT);
	newInsST3Flag = (module->header.flags & XModule::MODULE_ST3NEWINSTRUMENT);
	oldPTInsChangeFlag = (module->header.flags & XModule::MODULE_OLDPTINSTRUMENTCHANGE);

	// 4-channel Protracker module = EXACT PTK replay should be applied
	playModePT = ((module->header.flags & XModule::MODULE_PTNEWINSTRUMENT) && (module->header.channum == 4) && playMode == PlayMode_Auto) ||
					   (playMode == PlayMode_ProTracker2) || (playMode == PlayMode_ProTracker3);

	// This is a module with PTK limits
	playModePTPitchLimit = ((module->header.flags & XModule::MODULE_PTNEWINSTRUMENT) && playMode == PlayMode_Auto) || (playMode == PlayMode_ProTracker2) || (playMode == PlayMode_ProTracker3);

	// Override module playmode settings
	switch (playMode)
	{
		case PlayMode_ProTracker2:
			newInsPTFlag = true;
			newInsST3Flag = false;
			oldPTInsChangeFlag = true;
			break;
		case PlayMode_ProTracker3:
			newInsPTFlag = true;
			newInsST3Flag = false;
			oldPTInsChangeFlag = false;
			break;
		case PlayMode_ScreamTracker3:
			newInsPTFlag = false;
			newInsST3Flag = true;
			oldPTInsChangeFlag = false;
			break;
		case PlayMode_FastTracker2:
			newInsPTFlag = false;
			newInsST3Flag = false;
			oldPTInsChangeFlag = false;
			break;
		case PlayMode_ImpulseTracker:
		case PlayMode_Auto:
			break;
	}

	playModeFT2 = (playMode == PlayMode_FastTracker2 ? true : false);
	if (playMode == PlayMode_Auto && (module->header.flags & XModule::MODULE_XMARPEGGIO))
		playModeFT2 = true;

	// Chop off samples which sample offsets greater sample length?
	playModeChopSampleOffset = playModeFT2 || (playMode == PlayMode_ProTracker3);
}

static inline mp_sint32 myMod(mp_sint32 a, mp_sint32 b)
{
	mp_sint32 r = a % b;
	return r < 0 ? b + r : r;
}

mp_sint32 PlayerSTD::calcVibrato(TModuleChannel* chnInf, mp_sint32 effcnt)
{
	mp_sint32 vp = chnInf->vibpos[effcnt];
	mp_sint32 vd = chnInf->vibdepth[effcnt];
	
	mp_sint32 vm = ((vibtab[vp&31]*vd)>>(7-2));
	if ((vp&63)>31) vm=-vm;
	return vm;
}

void PlayerSTD::doTickEffect(mp_sint32 chn, TModuleChannel* chnInf, mp_sint32 effcnt)
{
	mp_ubyte x,y;
	mp_ubyte vp,vd;
	mp_sint32 vm;

	// IN PTK playmode, we've got a bunch of tick 0 effects 
	// which are repeated as long as the pattern delay applies
	// ONLY valid for PTK playmode & effects, for other effects this leads to undefined results
	if (playModePT)
	{
		if (patDelay && ticker &&
			// Those effects are NOT executed
			chnInf->eff[effcnt] > 0x09 &&
			chnInf->eff[effcnt] != 0x33 &&
			chnInf->eff[effcnt] != 0x34 && 
			chnInf->eff[effcnt] != 0x35 &&
			chnInf->eff[effcnt] != 0x36 &&
			chnInf->eff[effcnt] != 0x37 &&
			chnInf->eff[effcnt] != 0x38 &&
			chnInf->eff[effcnt] < 0x3C)
		{
			if (!(ticker % tickSpeed))
				doEffect(chn, chnInf, effcnt);
		}
	}

	switch (chnInf->eff[effcnt]) {
		// portamento up
		case 0x01:
			if (ticker) {
				chnInf->per-=chnInf->old[effcnt].portaup*4;
				handlePeriodUnderflow(chn);
				chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
			}
			break;

		// portamento down
		case 0x02:
			if (ticker) {
				chnInf->per+=chnInf->old[effcnt].portadown*4;
				handlePeriodOverflow(chn);
				chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
			}
			break;

		// note portamento
		case 0x03:
		{
			if (ticker&&chnInf->destnote) {
				// If this is an XM module we need to store the last portamento operand always in the buffer for the second effect
				mp_sint32 op = ((module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER)  && numEffects == 2) ? chnInf->old[1].portanote : chnInf->old[effcnt].portanote;
				if (chnInf->destper>chnInf->per) {
					chnInf->per+=op*4;
					if (chnInf->per>chnInf->destper) chnInf->per=chnInf->destper;
				}
				else if (chnInf->destper<chnInf->per) {
					chnInf->per-=op*4;
					if (chnInf->per<chnInf->destper) chnInf->per=chnInf->destper;
				}
				chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
			}
			break;
		}
			
		// vibrato (applying extra hacks for XM compatibility)
		// In FT2 the vibrato contained in the volume column works a bit different
		// than the vibrato in the effect column: 
		// After the vibrato has occured in the volumn column the pitch of the last
		// vibrato calculation stays on until the next pitch effect occurs
		case 0x04:
		{
			x = chnInf->eop[effcnt]>>4;
			y = chnInf->eop[effcnt]&0xf;

			mp_sint32 effNum = effcnt;
			// in FT2 play mode the last vibrato
			// value comes always from the effect column (index 1)
			if ((module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER)  && numEffects == 2) 
			{
				effNum = 1;
			}
		
			if (x) chnInf->vibspeed[effNum]=x;
			if (y) chnInf->vibdepth[effNum]=y;
		
			mp_sint32 vmp = chnInf->per;
					
			vm = calcVibrato(chnInf, effNum);
			
			if (ticker) 
				chnInf->vibpos[effNum]+=chnInf->vibspeed[effNum];

			vmp+=vm;

			mp_sint32 maxTicks = patDelay ? patDelayCount : tickSpeed;

			// the vibrato in the volumn volumn (index 0) works differently 
			// before applying that, we assure that this is an XM module by checking
			// the module header (checking for FT2 replay mode is not enough,
			// since it won't guarantee that there is volume and effect column)
			if ((module->header.flags & XModule::MODULE_XMVOLCOLUMNVIBRATO) &&
				ticker == maxTicks - 1)
			{
				// if this is a volume column vibrato we keep the pitch
				if (!effcnt)
				{
					chnInf->finalVibratoPer = vmp;
					chnInf->hasVibrato = true;
				}
				// if not we reset that
				else
				{
					chnInf->finalVibratoPer = 0;
					chnInf->hasVibrato = false;
				}
			}

			setFreq(chn,getfreq(chn,getfinalperiod(chn,vmp),chnInf->freqadjust));
			break;
		} 
			
		// note porta + volume slide
		case 0x05: 
		{
			if (ticker&&chnInf->destnote) {
				// If this is an XM module we need to store the last portamento operand always in the buffer for the second effect
				mp_sint32 op = (module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER) ? chnInf->old[1].portanote : chnInf->old[effcnt].portanote;
				if (chnInf->destper>chnInf->per) {
					chnInf->per+=op*4;
					if (chnInf->per>chnInf->destper) chnInf->per=chnInf->destper;
				}
				else if (chnInf->destper<chnInf->per) {
					chnInf->per-=op*4;
					if (chnInf->per<chnInf->destper) chnInf->per=chnInf->destper;
				}
				chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
			}
			
			x = chnInf->old[effcnt].volslide>>4;
			y = chnInf->old[effcnt].volslide&0xf;
			
			if (x&&y) y = 0;
			
			// i'm not completely sure about this
			// that case isn't mentioned mp_sint32 the S3M docs
			if (!(module->header.flags & XModule::MODULE_OLDS3MVOLSLIDES) && 
				ticker == 0)
				break;
			
			if (x) {
				chnInf->vol+=x*4;
				if (chnInf->vol>255) chnInf->vol=255;
			}
			if (y) {
				chnInf->vol-=y*4;
				if (chnInf->vol<0) chnInf->vol=0;
			}					
			chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
			chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
			break;
		}
		
		// vibrato + volume slide
		case 0x06:	
		{
			vm = calcVibrato(chnInf, effcnt);
			
			if (ticker) 
				chnInf->vibpos[effcnt]+=chnInf->vibspeed[effcnt];

			setFreq(chn,getfreq(chn,getfinalperiod(chn,chnInf->per+vm),chnInf->freqadjust));
			
			x = chnInf->old[effcnt].volslide>>4;
			y = chnInf->old[effcnt].volslide&0xf;
			
			if (x&&y) y = 0;
			
			// i'm not completely sure about this
			// that case isn't mentioned in the S3M docs
			if (!(module->header.flags & XModule::MODULE_OLDS3MVOLSLIDES) && 
				ticker == 0)
				break;
			
			if (x) {
				chnInf->vol+=x*4;
				if (chnInf->vol>255) chnInf->vol=255;
			}
			if (y) {
				chnInf->vol-=y*4;
				if (chnInf->vol<0) chnInf->vol=0;
			}
			chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;					
			chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
			break;
		} 

		// tremolo, this is not the exact FT2 way. FT2 way doesn't make sense at all, fuck it
		// (applying extra hacks for XM compatibility)
		case 0x07: 
		{
			x = chnInf->eop[effcnt]>>4;
			y = chnInf->eop[effcnt]&0xf;
			if (x) chnInf->trmspeed[effcnt]=x;
			if (y) chnInf->trmdepth[effcnt]=y;
			
			vp = chnInf->trmpos[effcnt];
			vd = chnInf->trmdepth[effcnt];
			
			mp_sint32 vmp = (ticker == 0 ? (chnInf->hasTremolo ? chnInf->finalTremoloVol : chnInf->vol) : chnInf->tremoloVol);
			
			if (ticker) {
				vm = ((vibtab[vp&31]*vd)>>(6-2));
				if ((vp&63)>31) vm=-vm;
				vmp+=vm;
				if (vmp<0) vmp=0;
				if (vmp>255) vmp=255;
				chnInf->trmpos[effcnt]+=chnInf->trmspeed[effcnt];
			}
			
			if (ticker == tickSpeed - 1)
			{
				chnInf->finalTremoloVol = vmp;
				chnInf->hasTremolo = true;
			}

			setVol(chn,getvolume(chn,vmp));
			break;
		}
		
		// volume slide
		case 0x0A: 
		{
			x = chnInf->old[effcnt].volslide>>4;
			y = chnInf->old[effcnt].volslide&0xf;
			
			// 08/31/04: fixed...
			// don't reject volume slide if both operands are set
			// instead, slide up
			// see other volume slides as well
			if (x&&y) y = 0;
			
			if (ticker) {
				if (x) {
					chnInf->vol+=x*4;
					
					if (chnInf->vol>255) chnInf->vol=255;
				}
				if (y) {
					chnInf->vol-=y*4;
					if (chnInf->vol<0) chnInf->vol=0;
				}
				chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;					
				chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
			}
			
			break;
		} 
		
		// global volume slide
		case 0x11: 
		{
			x = chnInf->old[effcnt].gvolslide>>4;
			y = chnInf->old[effcnt].gvolslide&0xf;
			
			if (x&&y) y = 0;
			
			if (ticker) {
				if (x) {
					mainVolume+=x*4;
					
					if (mainVolume>255) mainVolume=255;
				}
				if (y) {
					mainVolume-=y*4;
					if (mainVolume<0) mainVolume=0;
				}
			}
			
			break;
		}
			
		// panning slide
		case 0x19: 
		{
			x = chnInf->old[effcnt].panslide>>4;
			y = chnInf->old[effcnt].panslide&0xf;
			
			if (x&&y) y = 0;
			
			if (ticker) {
				if (x) {
					chnInf->pan+=x;
					if (chnInf->pan>255) chnInf->pan=255;
				}
				if (y) {
					chnInf->pan-=y;
					if (chnInf->pan<0) chnInf->pan=0;
				}
			}
			
			break;
		}
			
		// retrig + volslide (I worked my ass OFF on this fucking shit)
		// A few notes about FT2 playback:
		// Rxx Retrig doesn't restart envelopes, even with instrument set
		// It only retrigs if the last note has been been within valid range: 1 <= note <= XModule::NOTE_LAST
		case 0x1B: 
		{
			if ((chnInf->old[effcnt].retrig&0xf)) {
				if (chnInf->retrigcounterRxx[effcnt] >= chnInf->retrigmaxRxx[effcnt])
				{
					chnInf->retrigcounterRxx[effcnt] = 0;
					chnInf->retrigmaxRxx[effcnt] = chnInf->old[effcnt].retrig&0xf;
					
					switch (chnInf->old[effcnt].retrig>>4) {
						case 0x1 : 
							chnInf->vol-=4;
							if (chnInf->vol<0) chnInf->vol=0;
							break;
						case 0x2 : 
							chnInf->vol-=8;
							if (chnInf->vol<0) chnInf->vol=0;
							break;
						case 0x3 : 
							chnInf->vol-=16;
							if (chnInf->vol<0) chnInf->vol=0;
							break;
						case 0x4 : 
							chnInf->vol-=32;
							if (chnInf->vol<0) chnInf->vol=0;
							break;
						case 0x5 : 
							chnInf->vol-=64;
							if (chnInf->vol<0) chnInf->vol=0;
							break;
						case 0x6 : 
							chnInf->vol=(chnInf->vol*2)/3; 
							break;
						case 0x7 : 
							chnInf->vol>>=1; 
							break;
						case 0x9 : 
							chnInf->vol+=4;
							if (chnInf->vol>255) chnInf->vol=255;
							break;
						case 0xA : 
							chnInf->vol+=8;
							if (chnInf->vol>255) chnInf->vol=255;
							break;
						case 0xB : 
							chnInf->vol+=16;
							if (chnInf->vol>255) chnInf->vol=255;
							break;
						case 0xC : 
							chnInf->vol+=32;
							if (chnInf->vol>255) chnInf->vol=255;
							break;
						case 0xD : 
							chnInf->vol+=64;
							if (chnInf->vol>255) chnInf->vol=255;
							break;
						case 0xE : 
							chnInf->vol=(chnInf->vol*3)>>1;
							if (chnInf->vol>255) chnInf->vol=255;
							break;
						case 0xF : 
							chnInf->vol<<=1;
							if (chnInf->vol>255) chnInf->vol=255;
							break;
					}
					
					chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;					
					chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;

					if (chnInf->validnote)
						playInstrument(chn, chnInf);
				}
				
				chnInf->retrigcounterRxx[effcnt]++;
			}
			break;
		}

		// tremor (I worked my ass OFF on this fucking shit)
		case 0x1D:
		{
			x = (chnInf->old[effcnt].tremor>>4) + 1;
			y = (chnInf->old[effcnt].tremor&0xf) + 1;

			mp_sint32 v = (ticker == 0 ? chnInf->vol : chnInf->tremorVol);
			
			if (ticker && chnInf->tremorcnt[effcnt] % (x+y) >= x)
				v = 0;
			
			if (ticker) 
				chnInf->tremorcnt[effcnt]++;
			
			if (ticker == tickSpeed - 1)
			{
				chnInf->vol = v;
				chnInf->tremoloVol = v;
				chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
			}
			
			setVol(chn,getvolume(chn,v));
			break;
		}
								
		// MDL Subcommands
		case 0x1E: 
		{
			mp_ubyte eff = chnInf->eop[effcnt]>>4;
			mp_ubyte eop = chnInf->eop[effcnt]&0xf;
			switch (eff) {
				case 0x1 : 
					if (ticker) {
						chnInf->pan-=eop;
						if (chnInf->pan<0) chnInf->pan=0;
					}
					break;
				case 0x2 : 
					if (ticker) {
						chnInf->pan+=eop;
						if (chnInf->pan>255) chnInf->pan=255;
					}
					break;
				case 0xA : 
					if (ticker) {
						mainVolume+=eop;
						if (mainVolume>255) mainVolume=255;
					}
					break;
				case 0xB : 
					if (ticker) {
						mainVolume-=eop;
						if (mainVolume<0) mainVolume=0;
					}
					break;
			}
			break;
		} 
			
		// arpeggio
		case 0x20: 
		{
			if (chnInf->note)
			{
				mp_sint32 r = 0;
				mp_sint32 note = 0, onote = chnInf->note;
				//mp_sint32 c4spd = chnInf->c4spd;
				mp_sint32 relnote = chnInf->relnote;
				mp_sint32 finetune = chnInf->finetune;
				mp_sint32 per,nper;
				
				mp_ubyte eop = chnInf->old[effcnt].arpeg;
				
				mp_sint32 x = eop>>4;
				mp_sint32 y = eop&0xf;
				
				if (playModeFT2)
				{
					// Comment from Saga Musix/OpenMPT:
					// FT2 arpeggio is really screwed because the arpeggio note offset is computed using a LUT
					// instead of simply doing tick%3 or whatever. The problem is... this LUT only has 16 elements
					// so with more than 16 ticks/row, it overflows. The vibrato table happens to be placed right
					// after the arpeggio table in FT2, so it reads values from that table. The first value in the
					// vibrato table is 0, all values after that are greater than 1, which is interpreted as the
					// second arpeggio note.
					// Since FT2 counts ticks backwards, this also explains why arpeggio is "upside down" in FT2
					// compared to e.g. MOD, S3M, IT - the LUT is read back to front.
					// Arpeggio at speed 0 is not emulated properly.
					if (ticker == 0)
						r = 0;
					else
					{
						// Emulate FT2 behaviour (thanks Saga_Musix/OpenMPT)
						r = tickSpeed - ticker;
						if(r > 16) r = 2;
						else if(r == 16) r = 0;
					    else r %= 3;
					}
				}
				else
				{
					r = (ticker)%3;
				}
				
				if (r == 0)
				{
					note=chnInf->note; 
				}
				else if (r == 1)
				{
					note=chnInf->note+x; 
				}
				else if (r == 2)
				{
					note=chnInf->note+y; 
				}
				
				
				// Perform note clipping for XM note range if necessary
				if ((r != 0) && // Only done for arpeggio tick 1 & 2
					(module->header.flags & XModule::MODULE_XMNOTECLIPPING) && // Only when enabled
					(note + relnote > 96)) // Only when exceeding range
				{
					note-=((note+relnote) - 97);
				}
										
				// special case for STM arpeggio (thanks to Skaven/FC)
				// Will not work in combination with other period
				// related effects 
				if (module->header.flags & XModule::MODULE_STMARPEGGIO)
				{
					chnInf->per = getperiod(note,relnote,finetune);
					setFreq(chn,getfreq(chn,getfinalperiod(chn,chnInf->per),chnInf->freqadjust));
				}
				else
				{						
					nper=getperiod(note,relnote,finetune);
					per=getperiod(onote,relnote,finetune);
					
					//nper = (8363*periods[(note-1)%12]*16>>(((note-1)/12)))/c4spd;
					//per = (8363*periods[(onote-1)%12]*16>>(((onote-1)/12)))/c4spd;
					
					nper-=per;
					nper+=chnInf->per;
					
					setFreq(chn,getfreq(chn,getfinalperiod(chn,nper),chnInf->freqadjust));
				}
			}
			break;
		}
			
		// normal retrig
		// A few notes about FT2 playback:
		// E9x Retrig does!!! (while Rxx doesn't) restart envelopes, even without instrument set
		// It only retrigs if the last note has been been within valid range: 1 <= note <= XModule::NOTE_LAST
		case 0x39: 
		{
			if ((chnInf->eop[effcnt]&0xf) && ticker) {
				if (chnInf->retrigcounterE9x[effcnt] >= chnInf->retrigmaxE9x[effcnt])
				{							
					chnInf->retrigcounterE9x[effcnt] = 0;
					chnInf->retrigmaxE9x[effcnt] = chnInf->eop[effcnt]&0xf;
					// trigger envelopes ALWAYS
					triggerInstrumentFX(chnInf);
					chnInf->keyon = true;
					// trigger replay only when last note has been valid
					if (chnInf->validnote)
						playInstrument(chn, chnInf);
				}
				chnInf->retrigcounterE9x[effcnt]++;
			}
			break;
		}
				
		// note cut
		case 0x3C: 
			// S3M ignores tick 0 note cut
			if ((module->header.flags & XModule::MODULE_ST3NOTECUT) &&
				!chnInf->eop[effcnt])
				break;
			
			// Fasttracker cuts note at tick 0
			//if (chnInf->eop[effcnt]) {
				if (ticker == chnInf->eop[effcnt]) 
				{
					chnInf->vol=0;
					chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;					
					chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
				}
			//}
			break;
			
		// MDL porta up
		case 0x43: 
			if (ticker) {
				if (chnInf->old[effcnt].portaup<=0xDF) {
					chnInf->per-=chnInf->old[effcnt].portaup*4;
					handlePeriodUnderflow(chn);
					chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
				}
			}
			break;
			
		// MDL porta down
		case 0x44: 
			if (ticker) {
				if (chnInf->old[effcnt].portaup<=0xDF) {
					chnInf->per+=chnInf->old[effcnt].portaup*4;
					handlePeriodOverflow(chn);
					chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
				}
			}
			break;
				
		// MDL volslide up
		case 0x45: 
			if (ticker) {
				if (chnInf->old[effcnt].volslide<=0xDF) {
					chnInf->vol+=chnInf->old[effcnt].volslide;
					if (chnInf->vol>255) chnInf->vol=255;
				}
				chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;					
				chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
			}
			break;
				
		// MDL volslide down
		case 0x46: 
			if (ticker) {
				if (chnInf->old[effcnt].volslide<=0xDF) {
					chnInf->vol-=chnInf->old[effcnt].volslide;
					if (chnInf->vol<0) chnInf->vol=0;
				}
				chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;					
				chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
			}
			break;
				
		// S3M porta up
		case 0x47: 
			if (ticker) {
				if (chnInf->old[effcnt].portaup<=0xDF) {
					chnInf->per-=chnInf->old[effcnt].portaup*4;
					// Special for ST3
					if (chnInf->per <= 0) 
						stopSample(chn);
					chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
				}
			}
			break;
				
		// S3M porta down
		case 0x48: 
			if (ticker) {
				if (chnInf->old[effcnt].portaup<=0xDF) {
					chnInf->per+=chnInf->old[effcnt].portaup*4;
					chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
				}
			}
			break;
				
		// S3M volslide
		case 0x49: 
		{
			if (!(module->header.flags & XModule::MODULE_OLDS3MVOLSLIDES) && 
				ticker == 0)
				break;
			
			x = chnInf->old[effcnt].volslide>>4;
			y = chnInf->old[effcnt].volslide&0xf;
			
			if (x == 0xF && y) break;
			if (y == 0xF && x) break;
			
			if (x && y) y = 0;
			
			if (x) {
				chnInf->vol+=x*4;
				
				if (chnInf->vol>255) chnInf->vol=255;
			}
			if (y) {
				chnInf->vol-=y*4;
				if (chnInf->vol<0) chnInf->vol=0;
			}
			chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;					
			chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
			break;
		} 
			
		// fine vibrato
		case 0x4A:
		{
			x = chnInf->eop[effcnt]>>4;
			y = chnInf->eop[effcnt]&0xf;

			if (x) chnInf->vibspeed[effcnt]=x;
			if (y) chnInf->vibdepth[effcnt]=y;
		
			mp_sint32 vmp = chnInf->per;
					
			vp = chnInf->vibpos[effcnt];
			vd = chnInf->vibdepth[effcnt];
			
			vm = ((vibtab[vp&31]*vd)>>7);
			if ((vp&63)>31) vm=-vm;
			vmp+=vm;
			
			if (ticker) 
				chnInf->vibpos[effcnt]+=chnInf->vibspeed[effcnt];

			setFreq(chn,getfreq(chn,getfinalperiod(chn,vmp),chnInf->freqadjust));
			break;
		} 
			
		// high precision portamento up
		case 0x4D:
			if (ticker) {
				chnInf->per-=chnInf->old[effcnt].portaup;
				handlePeriodUnderflow(chn);
				chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
			}
			break;
			
		// high precision portamento down
		case 0x4E:
			if (ticker) {
				chnInf->per+=chnInf->old[effcnt].portaup;
				handlePeriodOverflow(chn);
				chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
			}
			break;
				
		// XM: Key off at tick
		case 0x14:
			// not at tick 0
			if (!ticker)
				break;
		// AMS: Key off at tick
		case 0x51:
			if (ticker == chnInf->eop[effcnt])
			{
				if (chnInf->venv.envstruc!=NULL) {
					if (!(chnInf->venv.envstruc->type&1)) 
						chnInf->vol = 0;
				}
				else 
					chnInf->vol=0;

				chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;												
				chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
				chnInf->keyon = false;
			}
			break;
				
		// Oktalyzer: Note slide down
		/*case 0x52:
			if (ticker && chnInf->note) {
				if (chnInf->note > chnInf->eop[effcnt])
				{
					chnInf->note-=chnInf->eop[effcnt];
					if (chnInf->note < 3*12)
						chnInf->note = 3*12;
					chnInf->per = getperiod(chnInf->note,chnInf->relnote,chnInf->finetune);
				}
			} 
			break;
				
		// Oktalyzer: Note slide up
		case 0x53:
			if (ticker && chnInf->note) {
				if (chnInf->note < 120-chnInf->eop[effcnt])
				{
					chnInf->note+=chnInf->eop[effcnt];
					if (chnInf->note > 6*12)
						chnInf->note = 6*12;
					chnInf->per = getperiod(chnInf->note,chnInf->relnote,chnInf->finetune);
				}
			} 
			break;*/
			
		// Oktalyzer arpeggio I, II, III
		case 0x56: 
		case 0x57: 
		case 0x58: 
		{
			if (chnInf->note)
			{
				mp_sint32 eff = chnInf->eff[effcnt]-0x56;
				mp_sint32 r;
				
				if (eff == 1)
					r = (ticker)&3;
				else 
					r = (ticker)%3;
					
				mp_sint32 note = 0,onote = chnInf->note;
				mp_sint32 relnote = chnInf->relnote;
				mp_sint32 finetune = chnInf->finetune;
				mp_sint32 per,nper;
				
				mp_ubyte eop = chnInf->eop[effcnt];
				
				mp_sint32 x = eop>>4;
				mp_sint32 y = eop&0xf;
				
				switch (eff)
				{
					case 0x00:
					{
						switch (r) {
							case 0 : note=chnInf->note-x; break;
							case 1 : note=chnInf->note; break;
							case 2 : note=chnInf->note+y; break;
						}
						break;
					}
					
					case 0x01:
					{
						switch (r) {
							case 0 : note=chnInf->note; break;
							case 1 : note=chnInf->note+y; break;
							case 2 : note=chnInf->note; break;
							case 3 : note=chnInf->note-x; break;
						}
						break;
					}

					case 0x02:
					{
						switch (r) {
							case 0 : note=chnInf->note+y; break;
							case 1 : note=chnInf->note+y; break;
							case 2 : note=chnInf->note; break;
						}
						break;
					}
				}
				
				nper=getperiod(note,relnote,finetune);
				per=getperiod(onote,relnote,finetune);
				
				nper-=per;
				nper+=chnInf->per;
				
				setFreq(chn,getfreq(chn,getfinalperiod(chn,nper),chnInf->freqadjust));
			}
			break;
		}
		
		// Global volslide
		case 0x59: 
		{
			if (ticker == 0)
				break;
			
			x = chnInf->old[effcnt].gvolslide>>4;
			y = chnInf->old[effcnt].gvolslide&0xf;
			
			if (x == 0xF && y) break;
			if (y == 0xF && x) break;
			
			if (x && y) y = 0;
			
			if (x) {
				mainVolume+=x*4;
				
				if (mainVolume>255) mainVolume=255;
			}
			if (y) {
				mainVolume-=y*4;
				if (mainVolume<0) mainVolume=0;
			}
			break;
		} 
			
	}
}

void PlayerSTD::doEffect(mp_sint32 chn, TModuleChannel* chnInf, mp_sint32 effcnt)
{
	mp_ubyte x,y;
	mp_sint32 eop=chnInf->eop[effcnt];
	switch (chnInf->eff[effcnt]) {
		case 0x01 : if (eop) chnInf->old[effcnt].portaup=eop; break;
		case 0x02 : if (eop) chnInf->old[effcnt].portadown=eop; break;
		case 0x03 : if (module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER) 
					{
						// XM compatible playing?
						/*if (!newInsPTFlag && 
							!newInsST3Flag &&
							!oldPTInsChangeFlag &&
							!note &&
							chnInf->note)
						{
							chnInf->destper=getperiod(chnInf->note,chnInf->relnote,chnInf->finetune);
						}*/

						ASSERT(numEffects >= 2);
						if (eop) chnInf->old[1].portanote=eop; 
					}
					else
					{
						if (eop) chnInf->old[effcnt].portanote=eop; 
					}
					break;
		case 0x05 : if (eop) chnInf->old[effcnt].volslide=eop; break;
		case 0x06 : if (eop) chnInf->old[effcnt].volslide=eop; break;
		case 0x08 : if (options[PlayModeOptionPanning8xx]) chnInf->pan=eop; break;
		case 0x09 : {
						if (eop) chnInf->old[effcnt].smpoffset = eop; 
						smpoffs[chn] = chnInf->old[effcnt].smpoffset<<8;					
					}; 
					break;
		case 0x0A : if (eop) chnInf->old[effcnt].volslide=eop; break;
		case 0x0B : {
						pjump = 1;
						pjumppos = eop;
						pjumprow = 0;
						pjumpPriority = MP_NUMEFFECTS*chn + effcnt;
					}; 
					break;
		case 0x0C : chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = eop; 
					chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;					
					chnInf->hasSetVolume = true;					
					break;
		case 0x0D : {
						pbreak=1;
						pbreakpos = (eop>>4)*10+(eop&0xf);
						if (pbreakpos > 63)
							pbreakpos = 0;
						pbreakPriority = MP_NUMEFFECTS*chn + effcnt;
					}; break;
		case 0x0F : {
						if (eop) 
						{
							if (eop>=32) {
								bpm=eop;
								this->adder = getbpmrate(eop);
							}
						}
						else
						{
							haltFlag = true;
						}
					}; break;
		case 0x10 : mainVolume=eop; break;
		case 0x11 : if (eop) chnInf->old[effcnt].gvolslide=eop; break;
		// set envelope position
		case 0x15 : {
						chnInf->venv.setToTick(eop);
						if (chnInf->venv.envstruc && chnInf->venv.envstruc->type & 2)
						{
							// FT2 only updates the pan envelope position if the volume envelope's sustain point is enabled
							chnInf->penv.setToTick(eop);
						}
						break;
					}
		// set BPM
		case 0x16 : {
						if (eop) {
							bpm=eop;
							this->adder = getbpmrate(eop);
						}
					}; break;
		
		case 0x19 : if (eop) chnInf->old[effcnt].panslide=eop; break;
		case 0x1B : {
						x = eop & 0xf;
						y = eop & 0xF0;
						
						if (x) 
							chnInf->old[effcnt].retrig = (chnInf->old[effcnt].retrig & 0xF0) | x;
						if (y) 
							chnInf->old[effcnt].retrig = (chnInf->old[effcnt].retrig & 0x0F) | y;
						
						eop = chnInf->old[effcnt].retrig;
									
						chnInf->retrigmaxRxx[effcnt] = eop & 0xF;		
						
						// Simulate really nasty FT2 bug:
						// When a volume is set in the volume column
						// the interval for the first retrig is lengthen by one tick
						if (chnInf->hasSetVolume && playModeFT2)
						{
							chnInf->retrigcounterRxx[effcnt] = -1;
							chnInf->hasSetVolume = false;
						}
						
						// If a note is playing on tick 0, increase counter
						if (chnInf->currentnote && chnInf->validnote)
							chnInf->retrigcounterRxx[effcnt]++;
						break;
					}
		// Tremor
		case 0x1D : if (eop) chnInf->old[effcnt].tremor=eop; break;						
		// MDL set sample offset
		case 0x1F : {
						smpoffs[chn]=((mp_sint32)eop<<8)+((mp_sint32)chnInf->eop[(effcnt+1)%numEffects]<<16);
					}; break;
		case 0x20 : if (eop) chnInf->old[effcnt].arpeg=eop; break;
		// ULT set sample offset
		case 0x21 : {
						smpoffs[chn]=((mp_sint32)eop<<10);
					}; break;
		// ULT Fine set sample offset
		case 0x22 : {
						mp_sint32 op = (((mp_sint32)eop)<<8) + ((mp_sint32)chnInf->eop[(effcnt+1)%numEffects]);
						smpoffs[chn]=op<<2;
					}; break;					
		// ULT special commands
		case 0x23 : {
						if (((eop >> 4) == 1 || (eop&0xF) == 1) ||
							((eop >> 4) == 12 || (eop&0xF) == 12))
						{
							breakLoop(chn);
						}
						if ((eop >> 4) == 2 || (eop&0xF) == 2)
						{
							chnInf->flags &= ~CHANNEL_FLAGS_FORCE_FORWARD;
							chnInf->flags |= CHANNEL_FLAGS_FORCE_BACKWARD;
							
							setBackward(chn);
						}
					}; break;					
		// Far position jump (PLM support)
		case 0x2B : {
						pjump = 1;
						pjumppos = eop;
						pjumprow = chnInf->eop[(effcnt+1)%numEffects];
						pjumpPriority = MP_NUMEFFECTS*chn + effcnt;
					}; break;
		// Fine porta up
		case 0x31 : {
						if (eop) chnInf->old[effcnt].fineportaup=eop;
						chnInf->per-=chnInf->old[effcnt].fineportaup*4;
						handlePeriodUnderflow(chn);
						chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
					}; break;
		// Fine porta down
		case 0x32 : {
						if (eop) chnInf->old[effcnt].fineportadown=eop;
						chnInf->per+=chnInf->old[effcnt].fineportadown*4;
						handlePeriodOverflow(chn);
						chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
					}; break;
		case 0x36 : {
						if (!eop) {
							chnInf->execloop=0;
							chnInf->loopstart=rowcnt;
							chnInf->loopingValidPosition = poscnt;
						}
						else {
							if (chnInf->loopcounter==eop) 
							{
								// Imitate nasty XM bug here:
								if (playModeFT2)
								{
									startNextRow = chnInf->loopstart;
								}
							
								RESETLOOPING
							}
							else {
								chnInf->execloop=1;
								chnInf->loopcounter++;
							}
						}
					}; break;
		case 0x38 : if (options[PlayModeOptionPanningE8x]) chnInf->pan=(mp_ubyte)XModule::pan15to255(eop); break;
		case 0x39 : {
						chnInf->retrigcounterE9x[effcnt] = 0;							
						if (eop)
						{
							chnInf->retrigmaxE9x[effcnt] = eop & 0xF;		

							// If a note is playing on tick 0, increase counter
							if (chnInf->currentnote && chnInf->validnote)
								chnInf->retrigcounterE9x[effcnt]++;
						}
						else if (!chnInf->currentnote)
						{
							// trigger envelopes ALWAYS
							triggerInstrumentFX(chnInf);
							chnInf->keyon = true;
							// trigger replay only when last note has been valid
							if (chnInf->validnote)
								playInstrument(chn, chnInf);
						}						
					}; break;
		case 0x3A : {
						if (eop) chnInf->old[effcnt].finevolslide=eop;
						chnInf->vol+=chnInf->old[effcnt].finevolslide*4;
						if (chnInf->vol>255) chnInf->vol=255;
						chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
						chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
					}; break;
		case 0x3B : {
						if (eop) chnInf->old[effcnt].finevolslide=eop;
						chnInf->vol-=chnInf->old[effcnt].finevolslide*4;
						if (chnInf->vol<0) chnInf->vol=0;
						chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
						chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
					}; break;
		// Note delay triggers envelopes/autovibrato/fade out again
		case 0x3D : {
						triggerInstrumentFX(chnInf);
						chnInf->keyon = true;						
					}; break;
		case 0x3E : {
						patDelay = true;
						patDelayCount = (mp_sint32)tickSpeed*((mp_sint32)eop+1);
					}; break;
		// Xtra fine porta up
		case 0x41 : {
						if (eop) chnInf->old[effcnt].xfineportaup=eop;
						chnInf->per-=chnInf->old[effcnt].xfineportaup;
						handlePeriodUnderflow(chn);
						chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
					}; break;
		case 0x42 : {
						if (eop) chnInf->old[effcnt].xfineportadown=eop;
						chnInf->per+=chnInf->old[effcnt].xfineportadown;
						handlePeriodOverflow(chn);
						chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
					}; break;
		// MDL fine portas up
		case 0x43 : {
						if (eop) chnInf->old[effcnt].portaup=eop; 
						if (chnInf->old[effcnt].portaup>=0xE0) {
							y=chnInf->old[effcnt].portaup>>4;
							x=chnInf->old[effcnt].portaup&0xf;
							switch (y) {
								case 0xF:
									chnInf->per-=x*4;
									handlePeriodUnderflow(chn);
									chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
									break;
								case 0xE: 
									chnInf->per-=(x>>1);
									handlePeriodUnderflow(chn);
									chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
									break;
							}
						}
					}; break;
		case 0x44 : {
						if (eop) chnInf->old[effcnt].portaup=eop; 
						if (chnInf->old[effcnt].portaup>=0xE0) {
							y=chnInf->old[effcnt].portaup>>4;
							x=chnInf->old[effcnt].portaup&0xf;
							switch (y) {
								case 0xF : 
									chnInf->per+=x*4; 
									handlePeriodOverflow(chn);
									chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
									break;
								case 0xE : 
									chnInf->per+=(x>>1); 
									handlePeriodOverflow(chn);
									chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
									break;
							}
						}
					}; break;
		case 0x45 : {
						if (eop) chnInf->old[effcnt].volslide=eop; 
						if (chnInf->old[effcnt].volslide>=0xE0) {
							y=chnInf->old[effcnt].volslide>>4;
							x=chnInf->old[effcnt].volslide&0xf;
							switch (y) {
								case 0xF : 
									chnInf->vol+=x*4;
									if (chnInf->vol>255) chnInf->vol=255;
									break;
								case 0xE : 
									chnInf->vol+=x; 
									if (chnInf->vol>255) chnInf->vol=255;
									break;
							}
							chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
							chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
						}
					}
					break;
		case 0x46 : {
						if (eop) chnInf->old[effcnt].volslide=eop; 
						if (chnInf->old[effcnt].volslide>=0xE0) {
							y=chnInf->old[effcnt].volslide>>4;
							x=chnInf->old[effcnt].volslide&0xf;
							switch (y) {
								case 0xF : 
									chnInf->vol-=x*4;
									if (chnInf->vol<0) chnInf->vol=0;
									break;
								case 0xE : 
									chnInf->vol-=x; 
									if (chnInf->vol<0) chnInf->vol=0;
									break;
							}
							chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
							chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
						}
					}; break;
		// S3M porta up
		case 0x47 : {
						if (eop) chnInf->old[effcnt].portaup=eop; 
						if (chnInf->old[effcnt].portaup>=0xE0) {
							y=chnInf->old[effcnt].portaup>>4;
							x=chnInf->old[effcnt].portaup&0xf;
							switch (y) {
								case 0xF:
									chnInf->per-=x*4;
									// Special for ST3
									if (chnInf->per <= 0) stopSample(chn);
									chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
									break;
								case 0xE: 
									chnInf->per-=x;
									// Special for ST3
									if (chnInf->per <= 0) stopSample(chn);
									chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
									break;
							}
						}
					}; break;
		// S3M porta down
		case 0x48 : {
						if (eop) chnInf->old[effcnt].portaup=eop; 
						if (chnInf->old[effcnt].portaup>=0xE0) {
							y=chnInf->old[effcnt].portaup>>4;
							x=chnInf->old[effcnt].portaup&0xf;
							switch (y) {
								case 0xF : 
									chnInf->per+=x*4; 
									handlePeriodOverflow(chn);
									chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
									break;
								case 0xE : 
									chnInf->per+=x; 
									handlePeriodOverflow(chn);
									chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
									break;
							}
						}
					}; break;
		// S3M volslide
		case 0x49 : {
						if (eop) chnInf->old[effcnt].volslide=eop; 
						
						if (chnInf->old[effcnt].volslide) {
							y=chnInf->old[effcnt].volslide>>4;
							x=chnInf->old[effcnt].volslide&0xf;
							
							if ((x!=0x0F)&&(y!=0x0F)) break;
							if (x==0x0F && !y) break;
							if (y==0x0F && !x) break;
							
							if (x==0x0F)
							{
								chnInf->vol+=y*4;
								if (chnInf->vol>255) chnInf->vol=255;
								chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
								chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
							}
							if (y==0x0F)
							{
								chnInf->vol-=x*4;
								if (chnInf->vol<0) chnInf->vol=0;
								chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
								chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
							}
						}
					}; break;
		// extra fine volslide up (PSM support)
		case 0x4B : {
						if (eop) chnInf->old[effcnt].finevolslide=eop;
						chnInf->vol+=chnInf->old[effcnt].finevolslide;
						if (chnInf->vol>255) chnInf->vol=255;
						chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
						chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
					}; break;
		// extra fine volslide down (PSM support)
		case 0x4C : {
						if (eop) chnInf->old[effcnt].finevolslide=eop;
						chnInf->vol-=chnInf->old[effcnt].finevolslide;
						if (chnInf->vol<0) chnInf->vol=0;
						chnInf->tremorVol = chnInf->tremoloVol = chnInf->vol;
						chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
					}; break;
		// high precision porta up (PSM support)
		// same as 0x01 but 4x more accurate
		case 0x4D : if (eop) chnInf->old[effcnt].portaup=eop; break;
		// high precision porta down (PSM support)
		// same as 0x02 but 4x more accurate
		case 0x4E : if (eop) chnInf->old[effcnt].portaup=eop; break;
		// AMS: Set sampleflags
		case 0x4F : if (!eop)
					{
						chnInf->flags &= ~CHANNEL_FLAGS_FORCE_BACKWARD;
						chnInf->flags |= CHANNEL_FLAGS_FORCE_FORWARD;
						
						setForward(chn);
					}
					else if (eop == 1)
					{
						chnInf->flags &= ~CHANNEL_FLAGS_FORCE_FORWARD;
						chnInf->flags |= CHANNEL_FLAGS_FORCE_BACKWARD;

						setBackward(chn);
					}
					else if (eop == 2)
					{
						chnInf->flags |= CHANNEL_FLAGS_FORCE_BILOOP;
					}
					else if (eop == 3)  // break sampleloop / oktalyzer too
					{
						breakLoop(chn);
					}
					break;
		// AMS: Set channel mastervol
		case 0x50 : chnInf->masterVol=eop; break;
		// Digibooster set real BPM
		case 0x52 : {
						if (eop) 
						{
							baseBpm = eop >= 32 ? eop : 32;
							// Simply recalculate
							this->adder = getbpmrate(bpm);
						}
						break;
					}
		// Oktalyzer: fine note slide down
		case 0x54:  {
						if (chnInf->note > eop)
						{
							chnInf->note-=eop;
							if (chnInf->note < 3*12)
								chnInf->note = 3*12;
							chnInf->per = getperiod(chnInf->note,chnInf->relnote,chnInf->finetune);
							chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
						}
					}
					break;
		// Oktalyzer: fine note slide up
		case 0x55:  {
						if (chnInf->note < XModule::NOTE_LAST-eop)
						{
							chnInf->note+=eop;
							if (chnInf->note > 6*12)
								chnInf->note = 6*12;
							chnInf->per = getperiod(chnInf->note,chnInf->relnote,chnInf->finetune);
							chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
						}
					}		
					break;
		// S3M global volslide
		case 0x59 : {
						if (eop) chnInf->old[effcnt].gvolslide=eop; 
						
						if (chnInf->old[effcnt].gvolslide) {
							y=chnInf->old[effcnt].gvolslide>>4;
							x=chnInf->old[effcnt].gvolslide&0xf;
							
							if ((x!=0x0F)&&(y!=0x0F)) break;
							if (x==0x0F && !y) break;
							if (y==0x0F && !x) break;
							
							if (x==0x0F)
							{
								mainVolume+=y*4;
								if (mainVolume>255) mainVolume=255;
								break;
							}
							if (y==0x0F)
							{
								mainVolume-=x*4;
								if (mainVolume<0) mainVolume=0;
								break;
							}
						}
						break;
					}					
			
	} // switch
}

void PlayerSTD::doTickeffects()
{
	for (mp_sint32 chn=0;chn<numChannels;chn++) {
	
		TModuleChannel *chnInf = &chninfo[chn];

		for (mp_sint32 effcnt=0;effcnt<numEffects;effcnt++) {
			doTickEffect(chn, chnInf, effcnt);
		}

	}

}

void PlayerSTD::triggerEnvelope(TPrEnv& dstEnv, TEnvelope& srcEnv)
{
	dstEnv.envstruc=&srcEnv;
	dstEnv.a=0;
	dstEnv.b=1;
	dstEnv.step = 0;
	dstEnv.bpmCounter = 0;
	if (dstEnv.envstruc->speed)
		dstEnv.bpmAdder = getbpmrate(dstEnv.envstruc->speed);
}

void PlayerSTD::triggerEnvelopes(TModuleChannel* chnInf)
{
	mp_uword e = module->smp[chnInf->smp].venvnum;
	if (e) {
		triggerEnvelope(chnInf->venv, module->venvs[e-1]);
	}
	else chnInf->venv.envstruc=NULL;
	
	e = module->smp[chnInf->smp].penvnum;
	if (e) {
		triggerEnvelope(chnInf->penv, module->penvs[e-1]);
	}
	else chnInf->penv.envstruc=NULL;
	
	e = module->smp[chnInf->smp].fenvnum;
	if (e) {
		triggerEnvelope(chnInf->fenv, module->fenvs[e-1]);
	}
	else chnInf->fenv.envstruc=NULL;
	
	e = module->smp[chnInf->smp].vibenvnum;
	if (e) {
		triggerEnvelope(chnInf->vibenv, module->vibenvs[e-1]);
	}
	else chnInf->vibenv.envstruc=NULL;				
}

void PlayerSTD::triggerAutovibrato(TModuleChannel* chnInf)
{
	if (module->smp[chnInf->smp].vibdepth&&module->smp[chnInf->smp].vibrate) {
		//chnInf->avibused=1;
		chnInf->avibcnt=0;
		chnInf->avibused=module->smp[chnInf->smp].vibtype+1;
		chnInf->avibdepth=module->smp[chnInf->smp].vibdepth;
		chnInf->avibspd=module->smp[chnInf->smp].vibrate;
		chnInf->avibswcnt=0;
		chnInf->avibsweep=module->smp[chnInf->smp].vibsweep;
	}
	else chnInf->avibused=0;
}

void PlayerSTD::triggerInstrumentFX(TModuleChannel* chnInf)
{
	if (chnInf->smp != -1)
	{
		triggerEnvelopes(chnInf);				
		triggerAutovibrato(chnInf);
		
		chnInf->fadevolstart = 65536;				
		chnInf->fadevolstep = module->smp[chnInf->smp].volfade;		
	}
}


void PlayerSTD::progressRow()
{
	mp_sint32 slotsize = (numEffects*2)+2;

	TXMPattern* pattern = &module->phead[patternIndex];

	mp_ubyte *row = pattern->patternData+
						 (pattern->channum*slotsize*rowcnt);

	//for (mp_sint32 chn=0;chn<1;chn++) {
	for (mp_sint32 chn=0;chn<numChannels;chn++) {
		
		if ((mp_sint32)attick[chn]==ticker && ticker < tickSpeed) {
			TModuleChannel *chnInf = &chninfo[chn];

			mp_sint32 pp = slotsize*chn;
			mp_sint32 note = chnInf->currentnote = row[pp];
			
			mp_sint32 i    = row[pp+1];

			bool noteporta = false;
			bool notedelay = false;
			
			mp_sint32 oldIns = chnInf->ins;
			mp_sint32 oldSmp = chnInf->smp;
			
			// Effect preprocessor & get effect + operand from interleaved pattern data
			mp_sint32 effcnt, finetune = 0x7FFFFFFF;
			for (effcnt = 0; effcnt < numEffects; effcnt++) {
				chnInf->eff[effcnt] = row[(pp+2)+(effcnt*2)];
				chnInf->eop[effcnt] = row[(pp+2)+(effcnt*2+1)];
				switch (chnInf->eff[effcnt])
				{
					// We need to know if we process the note as new note or or portamento destination period
					case 0x03:
					case 0x05:
						noteporta = true;
						break;
					// XM key off at tick with tick operand == 0 is like normal key off
					case 0x14:
						if (chnInf->eop[effcnt] == 0)
							note = XModule::NOTE_OFF;
						break;
					// set finetune will override the instrument setting
					case 0x35:
						finetune = XModule::modfinetunes[playModeFT2 ? ((chnInf->eop[effcnt] - 8) & 0xF) : (chnInf->eop[effcnt] & 0xF)];
						break;
					// note delay without note retriggers last note
					case 0x3d:
						if (chnInf->eop[effcnt])
						{
							notedelay = true;
							if (!note && playModeFT2)
								note = chnInf->lastnoportanote;
						}
						break;
				}				
			}
			
			// Check new instrument settings only if valid note or no note at all
			if (i && note <= XModule::NOTE_LAST) {
				// valid sample?
				bool invalidIns = true;
				bool invalidSmp = true;
#ifdef MILKYTRACKER
				// ----------------------------------------------------------------
				// When editing in MilkyTracker all instruments have 16 samples 
				// So I need another way of finding out which instruments are
				// invalid ones
				// ----------------------------------------------------------------
				mp_sint32 n = note;
				if (!note) n = chnInf->note;
				// here we have a little problem
				if (!n)
				{ 
					for (mp_sint32 j = 0; j < 120; j++)
					{
						if (module->instr[i-1].snum[j] != -1)
						{
							n = 1;
							break;
						}
					}
				}

				// invalid instrument
				if (i <= module->header.insnum && n && n <= 120)
				{
					mp_sint32 s = module->instr[i-1].snum[n-1];
					if (module->smp[s].sample)
						invalidIns = false;
				}
				// invalid sample
				if (module->instr[i-1].samp && n && n <= 120)
				{
					mp_sint32 s = module->instr[i-1].snum[n-1];
					if (s != -1 && module->smp[s].sample)
						invalidSmp = false;
				}
#else
				// invalid instrument
				if (i <= module->header.insnum && module->instr[i-1].samp)
					invalidIns = false;
				// invalid sample
				if (module->instr[i-1].samp && module->instr[i-1].snum[0] != -1)
					invalidSmp = false;
#endif
				if (!invalidIns) // valid sample
					chnInf->ins = i;
				else if (note) // invalid sample
				{
					// cut means stop sample in FT2
					if (!newInsPTFlag && !newInsST3Flag && !noteporta)
					{
						chnInf->smp = -1;
						chnInf->ins = 0;
						stopSample(chn);
					}
				}
				
				// protracker sample cut when invalid instrument is triggered
				if (newInsPTFlag)
				{
					if (!note)
					{
						if (invalidSmp)
						{
							chnInf->smp = -1;
							chnInf->ins = 0;
							chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = 0;	// cut means: volume to zero (no stop sample)
							chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
							//stopSample(chn);
						}
						else
						{
							chnInf->smp = module->instr[i-1].snum[0];
						}
					}
					else
					{
						if (invalidSmp)
						{
							chnInf->smp = -1;
							chnInf->ins = 0;
							chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = 0;	// cut means: volume to zero (no stop sample)
							chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
							// NOT sure!!!
							//stopSample(chn);
						}
					}
				}
				// screamtracker continues playing when invalid instrument is triggered
				// applies new volume when instrument only is triggered
				else if (newInsST3Flag)
				{
					if (!note)
					{
						if (!invalidSmp)
						{
							chnInf->smp = module->instr[i-1].snum[0];
						}
						// completely invalid instrument without note, does nothing at all
						else 
						{
							i = 0;
						}
					}
				}
			}
			
			chnInf->validnote = true;
			if (note && note < XModule::NOTE_OFF) {			
			
				if (chnInf->ins) {
					chnInf->smp = module->instr[chnInf->ins-1].snum[note-1];
					if ((module->instr[chnInf->ins-1].flags & 1) &&
						module->instr[chnInf->ins-1].notemap[note-1] != 0xFF)
					{
						chnInf->currentnote = note = module->instr[chnInf->ins-1].notemap[note-1] + 1;
					}
					
					// invalid sample entry?
					// Only apply new fintune / relative note number when not doing portamento
					if (chnInf->smp != -1 && !noteporta) {
						mp_sint32 finalNote = note + (mp_sint32)module->smp[chnInf->smp].relnote;
					
						// Within current note range?
						if (finalNote >= 1 && finalNote <= 119)
						{
							chnInf->finetune = (finetune != 0x7FFFFFFF ? finetune : module->smp[chnInf->smp].finetune);
							chnInf->relnote = module->smp[chnInf->smp].relnote;
							chnInf->freqadjust = module->smp[chnInf->smp].freqadjust;
						}
						// This is not a valid note
						else 
						{							
							chnInf->validnote = false;
							note = chnInf->note;
						}
					}
				}
				
				mp_sint32 relnote = chnInf->relnote;
				mp_sint32 finetune = chnInf->finetune;
				
				// If this is not a note portamento
				// and a valid note => keep that note and calculate new period
				if (!noteporta) {
					chnInf->note=chnInf->lastnoportanote=note;
					chnInf->per=getperiod(note,relnote,finetune);
					chnInf->hasVibrato = false; chnInf->finalVibratoPer = 0;
					// if there is a valid note => destroy portamento to note memory when playing an S3M(?)
					if (/*newInsPTFlag||*/newInsST3Flag)
					{
						chnInf->destnote=0;
						chnInf->destper=0;
					}
				}
				// If this is a note portamento keep destination's note + period
				else {
					// if a note delay is happening while the portamento is set, [AND we don't have a note (?)]
					// we restore the original period, but the destination period keeps set
					if (playModeFT2 && notedelay/* && !chnInf->currentnote*/)
					{
						chnInf->per=getperiod(note,relnote,finetune);
						if (chnInf->currentnote)
							chnInf->lastnoportanote = chnInf->currentnote;
						noteporta = false;
					}
					else
					{
						chnInf->lastnoportanote=chnInf->note;
						chnInf->destnote=chnInf->note=note;
						chnInf->destper=getperiod(note,relnote,finetune);
					}
				}

				// If this has not been a valid note, do not trigger it
				if (!chnInf->validnote)
					note = 0;
			}
			
			// man this FT2 bug emulation starts getting on my nerves:
			// only take new instrument of there is no note porta
			if (playModeFT2 && i &&
				(noteporta || !chnInf->validnote))
			{
				i = chnInf->ins = oldIns;
				chnInf->smp = oldSmp;
			}
			
			// when we have a new instrument we apply the settings
			// for this instrument, but in FT2 mode we only do it, when we're not
			// having a note portamento at the same time
			if (i && chnInf->smp != -1 && note < XModule::NOTE_OFF) 
			{				
				if ((module->smp[chnInf->smp].flags&1)) 
				{
					chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = module->smp[chnInf->smp].vol;
					chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
				}
				if (playModeFT2 &&
					(module->smp[chnInf->smp].flags&2)) 
					chnInf->pan = module->smp[chnInf->smp].pan;	
				if ((module->smp[chnInf->smp].flags&4)) 
					chnInf->masterVol = module->smp[chnInf->smp].vol;
				
				triggerInstrumentFX(chnInf);
					
				// reset vibrato/tremolo/tremor/retrig counters
				for (effcnt=0;effcnt<numEffects;effcnt++) 
					chnInf->vibpos[effcnt] = chnInf->tremorcnt[effcnt] = chnInf->trmpos[effcnt] = chnInf->retrigcounterRxx[effcnt] = 0;
					
				if (playModePT)
					smpoffs[chn] = 0;
					
				chnInf->keyon = true;	
			}
			
			// ------ 11/05/05: it seems that note off commands are processed BEFORE effect commands
			// S3M style keyoff:
			// sample is stopped
			if (note == XModule::NOTE_CUT) {
				note=0;
				if (chnInf->venv.envstruc!=NULL) {
					if (!(chnInf->venv.envstruc->type&1))
					{
						chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = 0;
						chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
						stopSample(chn);
					}
				}
				else {
					chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = 0;
					chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
					stopSample(chn);
				}
			}
			// XM style keyoff:
			// volume is set to zero
			else if (note == XModule::NOTE_OFF) {
				note=0;
				// no envelope or no volume envelope
				if (chnInf->venv.envstruc!=NULL) {
					if (!(chnInf->venv.envstruc->type&1)) 
					{
						chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = 0;
						chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
					}
				}
				else 
				{
					chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = 0;
					chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
				}
			
				chnInf->keyon = false;
			}
			
			chnInf->hasSetVolume = false;
			for (effcnt=0;effcnt<numEffects;effcnt++) {	
				// MTM hack
				// sample offset without note seems to trigger last note
				if (chnInf->eff[effcnt] == 0x09 && !note && module->getType() == XModule::ModuleType_MTM)
				{
					note = chnInf->note;
				}
				// FT2 hack
				// when note delay appears, we ignore the portamento operand
				else if (chnInf->eff[effcnt] == 0x03 && notedelay && playModeFT2)
				{
					chnInf->eop[effcnt] = 0;
				}
				doEffect(chn, chnInf, effcnt);
			} // for
	
			if (note) 
			{
				if (note <= XModule::NOTE_OFF)
				{
					if (!noteporta) {					
						playInstrument(chn, chnInf);										
					}
					else if (oldPTInsChangeFlag && newInsPTFlag && noteporta && i && chnInf->smp != -1 && chnInf->note) {						
						playInstrument(chn, chnInf, true);																	
					}
					
				}
				
			} // note
			else if (oldPTInsChangeFlag && newInsPTFlag && i && chnInf->note && chnInf->per) {
				playInstrument(chn, chnInf, true);					
			}
			
		}
	
	}
 
}

void PlayerSTD::update()
{
	mp_sint32 c;
	//for (c=10;c<11;c++) {
#ifdef MILKYTRACKER
	for (c=0;c<initialNumChannels;c++) 
#else
	for (c=0;c<numChannels;c++) 
#endif
	{
		TModuleChannel *chnInf = &chninfo[c];

		if (chnInf->flags & CHANNEL_FLAGS_UPDATE_IGNORE)
			continue;

		mp_sint32 dfs = chnInf->flags & CHANNEL_FLAGS_DFS, dvs = chnInf->flags & CHANNEL_FLAGS_DVS;

		if ((chnInf->per)&&(!dfs)) 
			setFreq(c,getfreq(c,getfinalperiod(c,chnInf->hasVibrato ? chnInf->finalVibratoPer : chnInf->per),chnInf->freqadjust));
		
		if (!dvs) 
			setVol(c,getvolume(c,chnInf-> hasTremolo ? chnInf->finalTremoloVol : chnInf->vol));

		setPan(c,getpanning(c,chnInf->pan));

		if (chnInf->venv.envstruc != NULL &&
			!chnInf->venv.envstruc->speed)
		{
			prenvelope(c,&chnInf->venv,chnInf->keyon);
		}
		
		if (chnInf->penv.envstruc != NULL &&
			!chnInf->penv.envstruc->speed)
		{
			prenvelope(c,&chnInf->penv,chnInf->keyon);
		}

		if (chnInf->fenv.envstruc != NULL &&
			!chnInf->fenv.envstruc->speed)
		{
			prenvelope(c,&chnInf->fenv,chnInf->keyon);
		}
		
		if (chnInf->vibenv.envstruc != NULL &&
			!chnInf->vibenv.envstruc->speed)
		{
			prenvelope(c,&chnInf->vibenv,chnInf->keyon);
		}

		if (!chnInf->keyon)
		{
			chnInf->fadevolstart-=chnInf->fadevolstep;
			if (chnInf->fadevolstart<0) chnInf->fadevolstart=0;
		}

		if (chnInf->avibused) 
		{
			chnInf->avibcnt+=chnInf->avibspd;
			if (chnInf->avibswcnt<chnInf->avibsweep)
				chnInf->avibswcnt++;

		}

	}

}

void PlayerSTD::updateBPMIndependent()
{
	mp_int64 dummy;
	mp_sint32 c;
#ifdef MILKYTRACKER
	for (c=0;c<initialNumChannels;c++) 
#else
	for (c=0;c<numChannels;c++) 
#endif
	{
		TModuleChannel *chnInf = &chninfo[c];
		
		if (chnInf->flags & CHANNEL_FLAGS_UPDATE_IGNORE)
			continue;

		bool dfs = chnInf->flags & CHANNEL_FLAGS_DFS, dvs = chnInf->flags & CHANNEL_FLAGS_DVS;

		// Volume envelope
		if (chnInf->venv.envstruc != NULL &&
			chnInf->venv.envstruc->speed)
		{
			dummy = (mp_int64)chnInf->venv.bpmCounter;
			dummy+=(mp_int64)chnInf->venv.bpmAdder;
			chnInf->venv.bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(c,&chnInf->venv,chnInf->keyon);
				if (!dvs) 
					setVol(c,getvolume(c,chnInf-> hasTremolo ? chnInf->finalTremoloVol : chnInf->vol));
			}
		}
		
		// Panning envelope
		if (chnInf->penv.envstruc != NULL &&
			chnInf->penv.envstruc->speed)
		{
			dummy = (mp_int64)chnInf->penv.bpmCounter;
			dummy+=(mp_int64)chnInf->penv.bpmAdder;
			chnInf->penv.bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(c,&chnInf->penv,chnInf->keyon);
				setPan(c,getpanning(c,chnInf->pan));
			}
		}

		// Frequency envelope: Digitracker MDL
		if (chnInf->fenv.envstruc != NULL &&
			chnInf->fenv.envstruc->speed)
		{
			dummy = (mp_int64)chnInf->fenv.bpmCounter;
			dummy+=(mp_int64)chnInf->fenv.bpmAdder;
			chnInf->fenv.bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(c,&chnInf->fenv,chnInf->keyon);
				if ((chnInf->per)&&(!dfs)) 
					setFreq(c,getfreq(c,getfinalperiod(c,chnInf->per),chnInf->freqadjust));
			}
		}

		// Vibrato envelope: Velvet Studio AMS
		if (chnInf->vibenv.envstruc != NULL &&
			chnInf->vibenv.envstruc->speed)
		{
			dummy = (mp_int64)chnInf->vibenv.bpmCounter;
			dummy+=(mp_int64)chnInf->vibenv.bpmAdder;
			chnInf->vibenv.bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(c,&chnInf->vibenv,chnInf->keyon);
				if ((chnInf->per)&&(!dfs)) 
					setFreq(c,getfreq(c,getfinalperiod(c,chnInf->per),chnInf->freqadjust));
			}
		}

	}

}

void inline PlayerSTD::setNewPosition(mp_sint32 poscnt)
{
	if (statusEventListener)
		statusEventListener->patternEndReached(*this, *module, poscnt);

	if (poscnt == this->poscnt)
		return;

	if (poscnt>=module->header.ordnum) 
		poscnt=module->header.restart;						
	
	// reset looping flags
	RESET_ALL_LOOPING	

	lastUnvisitedPos = this->poscnt;
	
	this->poscnt = poscnt;
}

void PlayerSTD::tickhandler()
{
	mp_sint32 maxTicks;

	if (!idle)
	{
		// Important! Without this, the different playmodes will not be recognized properly
		updatePlayModeFlags();

		// sanity check 1
		if (patternIndexToPlay == -1 && poscnt >= module->header.ordnum)
		{
			halt();
			return;
		}
		
		// Play special pattern?
		if (patternIndexToPlay == -1)
			patternIndex = module->header.ord[poscnt];
		else
			patternIndex = patternIndexToPlay;

		TXMPattern* pattern = &module->phead[patternIndex];
		
		if (pattern->patternData == NULL)
		{
			halt();
			return;
		}
		
		// sanity check 2 :)
		if (rowcnt >= pattern->rows)
		{
			if (patternIndexToPlay == -1)
			{
				//rowcnt = 0;
				ticker = 0;
				goto nextrow;
			}
			else
			{
				//halt();
				//return;
				rowcnt = 0;
				ticker = 0;
			}
		}

		numEffects = pattern->effnum;
		numChannels = pattern->channum <= module->header.channum ? pattern->channum : module->header.channum;

		mp_sint32 c;
		
		if (ticker == 0) 
		{
		
			// Keep track of visited rows
			mp_sint32 absolutePos = poscnt*256+rowcnt;
			if (isRowVisited(absolutePos) && !repeat)
			{
				// pattern loop active?
				bool b = false;
				for (c=0;c<numChannels;c++) 
				{
					if (chninfo[c].isLooping && chninfo[c].loopingValidPosition == poscnt)
					{
						b = true;
						break;
					}
				}

				if (!b)
				{
					halt();
					return;
				}
			}
			else
			{
				visitRow(absolutePos);
			}
		
			pbreak = pbreakpos = pbreakPriority = pjump = pjumppos = pjumprow = pjumpPriority = 0;
			// sample offset 0
			if (!playModePT)
				memset(smpoffs,0,sizeof(mp_uint32)*numChannels);
			// noteslot will be processed at tick 0   
			memset(attick,0,sizeof(mp_ubyte)*numChannels);
			
			// search for note delays
			mp_sint32 slotsize = (numEffects*2)+2;
			
			mp_ubyte *row = pattern->patternData+(pattern->channum*slotsize*rowcnt);
			
			// process high priority effects in advance to other effects
			mp_ubyte* slot = row;
			
			for (c=0;c<numChannels;c++) 
			{				
				chninfo[c].flags = 0;	
				
				for (mp_sint32 effcnt=0;effcnt<numEffects;effcnt++) 
				{										
					chninfo[c].eff[effcnt] = 0;
					chninfo[c].eop[effcnt] = 0;

					if (slot[2+(effcnt*2)] == 0x04) chninfo[c].flags |= CHANNEL_FLAGS_DFS;
					else if (slot[2+(effcnt*2)] == 0x4A) chninfo[c].flags |= CHANNEL_FLAGS_DFS;
					else if (slot[2+(effcnt*2)] == 0x06) chninfo[c].flags |= CHANNEL_FLAGS_DFS;
					else if (slot[2+(effcnt*2)] == 0x20) chninfo[c].flags |= CHANNEL_FLAGS_DFS; // normal arpeggio
					else if (slot[2+(effcnt*2)] == 0x56) chninfo[c].flags |= CHANNEL_FLAGS_DFS; // oktalyzer arpeggio I
					else if (slot[2+(effcnt*2)] == 0x57) chninfo[c].flags |= CHANNEL_FLAGS_DFS; // oktalyzer arpeggio II
					else if (slot[2+(effcnt*2)] == 0x58) chninfo[c].flags |= CHANNEL_FLAGS_DFS; // oktalyzer arpeggio III
					else if (slot[2+(effcnt*2)] == 0x07) chninfo[c].flags |= CHANNEL_FLAGS_DVS; // Tremolo
					else if (slot[2+(effcnt*2)] == 0x1D) chninfo[c].flags |= CHANNEL_FLAGS_DVS; // Tremor
					
					else if (slot[2+(effcnt*2)] == 0x3D) 
					{
						// found note delay: noteslot will be processed at a later tick
						attick[c] = slot[2+(effcnt*2)+1];
					}
					// set speed in advance also, 
					// in order to correctly implement note delay 
					else if (slot[2+(effcnt*2)] == 0xf &&		// protracker set speed/bpm
						slot[2+(effcnt*2)+1] &&		
						slot[2+(effcnt*2)+1] < 32)		// set tickspeed not BPM
					{
						tickSpeed = slot[2+(effcnt*2)+1];
					}
					else if (slot[2+(effcnt*2)] == 0x1c &&	//  S3M/MDL/... set speed
						slot[2+(effcnt*2)+1])			// valid set speed?
						tickSpeed = slot[2+(effcnt*2)+1];
				}
				
				slot+=slotsize;
			}
			
		}
		
		progressRow();
		
		doTickeffects();	
		
		ticker++;
		
		maxTicks = tickSpeed;
		if (patDelay)
			maxTicks = patDelayCount;
		
		if (ticker>=maxTicks) 
		{
			if (patDelay)
				patDelay = false;
			
			// reset ticker
			ticker=0;
			
			// if we're told to play this row only, we will stop now 
			// and neither process any of those pattern jump/repeat stuff
			if (playOneRowOnly)
			{
				BPMCounter = adder = 0;
				return;
			}
			
			if (patternIndexToPlay == -1)
			{
				
				// break pattern?
				if (pbreak&&(poscnt<(module->header.ordnum-1))) 
				{
					if (!pjump || (pjump && pjumpPriority > pbreakPriority))
						setNewPosition(poscnt+1);
					rowcnt=pbreakpos-1;
					startNextRow = -1;
				}
				else if (pbreak&&(poscnt==(module->header.ordnum-1))) 
				{
					// Pattern break on the last order? Break to restart position
					if (!pjump || (pjump && pjumpPriority > pbreakPriority))
						setNewPosition(module->header.restart);
					rowcnt=pbreakpos-1;
					startNextRow = -1;
				}
				
				// pattern jump?
				if (pjump) 
				{
					if (!pbreak || (pbreak && pjumpPriority > pbreakPriority))
						rowcnt = pjumprow-1;					
					setNewPosition(pjumppos);					
					startNextRow = -1;
				}
				
				// it could be that our position has changed because
				// of position jumps, so make sure we're getting the real position here
				patternIndex = module->header.ord[poscnt];
			}
			// We have one pattern to play
			else
			{
				// Position jump occurred and repeating is allowed, start again
				if (pjump || pbreak)
				{
					rowcnt = -1;
					startNextRow = -1;
					
					if (statusEventListener)
						statusEventListener->patternEndReached(*this, *module, poscnt);
				}
				//RESETLOOPING // macro
			}

			// handle loop
			for (c=0;c<numChannels;c++) 
			{			
				// pattern loop? nesting doesn't work yet
				if (chninfo[c].execloop) 
				{
					rowcnt = chninfo[c].loopstart-1;
					chninfo[c].execloop = 0;
					chninfo[c].isLooping = true;
				}
			}
			
			// next row
			rowcnt++;
nextrow:
			synccnt++;

			// reached end of pattern? 
			if (rowcnt>=module->phead[patternIndex].rows) 
			{
				// start at row 0?
				if (startNextRow != -1)
				{
					rowcnt = startNextRow;
					startNextRow = -1;
				}
				else
				{
					rowcnt = 0;
				}
				
				if (patternIndexToPlay == -1)
				{
					// play next order
					setNewPosition(poscnt+1);
				}
				// We have one pattern to play but repeating isn't allowed, so stop here
				else if (!repeat)
				{
					halt();
					return;
				}
				// We have one pattern to play and repeating is allowed so start again
				else
				{
					rowcnt = 0; 
					// reset looping flags
					RESET_ALL_LOOPING
					
					if (statusEventListener)
						statusEventListener->patternEndReached(*this, *module, poscnt);
				}
				
			}

			// halting has been requested
			if (haltFlag)
			{
				halt();
			}
			
		}
	
	}
	else
	{
		numChannels = module->header.channum;
	}
	
	update();

}

void PlayerSTD::halt()
{
	halted = true;
	BPMCounter = adder = 0;
	if (resetOnStopFlag)
		resetChannelsWithoutMuting();
}

bool PlayerSTD::grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const
{
	channelInfo.note = chninfo[chn].currentnote;
	channelInfo.instrument = chninfo[chn].ins;
	channelInfo.volume = chninfo[chn].vol;
	channelInfo.panning = chninfo[chn].pan;
	channelInfo.numeffects = numEffects;
	memcpy(channelInfo.effects, chninfo[chn].eff, sizeof(chninfo[chn].eff));
	memcpy(channelInfo.operands, chninfo[chn].eop, sizeof(chninfo[chn].eop));
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// for MilkyTracker use
//////////////////////////////////////////////////////////////////////////////////////////
void PlayerSTD::playNote(mp_ubyte chn, mp_sint32 note, mp_sint32 i, mp_sint32 vol/* = -1*/)
{
	if (!i)
		return;

	//stopSample(chn);

	smpoffs[chn] = 0;
	
	TModuleChannel *chnInf = &chninfo[chn];
	
	bool invalidSample = false;
#ifdef MILKYTRACKER
	for (mp_uint32 j = 0; j < 96; j++)
#else
	for (mp_uint32 j = 0; j < 120; j++)
#endif
	{
		mp_sword s = module->instr[i-1].snum[j];
		if (s != -1 && module->smp[s].sample)
		{
			invalidSample = true;
			break;
		}	
	}
	
	if (!invalidSample)
		return;
	
	chnInf->ins = i;

	if (note && note < XModule::NOTE_OFF) 
	{
		if (chn >= module->header.channum)
			chnInf->pan = 0x80;	
	
		if (chnInf->ins) 
		{
			chnInf->smp = module->instr[chnInf->ins-1].snum[note-1];
			
			// invalid sample entry
			if (chnInf->smp != -1) {
				if (module->smp[chnInf->smp].sample == NULL)
				{
					chnInf->ins = 0;
					return;
				}
				
				mp_sint32 finalNote = note + (mp_sint32)module->smp[chnInf->smp].relnote;
					
				// Within current note range?
				if (!(finalNote >= 1 && finalNote <= 119))
					return;
			
				chnInf->finetune=module->smp[chnInf->smp].finetune;
				chnInf->relnote=module->smp[chnInf->smp].relnote;
			}
		}
		
		mp_sint32 relnote = chnInf->relnote;
		mp_sint32 finetune = chnInf->finetune;
		
		chnInf->note=note;
		chnInf->per=getperiod(note,relnote,finetune);
	}
	
	if (i && chnInf->smp != -1 && note < XModule::NOTE_OFF) 
	{
		if ((module->smp[chnInf->smp].flags&1)) 
		{
			chnInf->vol = chnInf->tremorVol = chnInf->tremoloVol = (vol == -1 ? module->smp[chnInf->smp].vol : vol);
			chnInf->hasTremolo = false; chnInf->finalTremoloVol = 0;
		}
		if ((playMode == PlayMode_FastTracker2) &&
			(module->smp[chnInf->smp].flags&2)) 
			chnInf->pan=module->smp[chnInf->smp].pan;	
		if ((module->smp[chnInf->smp].flags&4)) 
			chnInf->masterVol=module->smp[chnInf->smp].vol;
		
		triggerInstrumentFX(chnInf);
		
		// reset vibrato/tremolo/tremor counter
		for (mp_sint32 effcnt=0;effcnt<numEffects;effcnt++) 
			chnInf->vibpos[effcnt] = chnInf->tremorcnt[effcnt] = chnInf->trmpos[effcnt] = 0;
		
		chnInf->keyon = true;	
	}
	
	if (note) 
	{
		// S3M style key-off
		// sample is stopped
		if (note == XModule::NOTE_CUT) {
			note=0;
			if (chnInf->venv.envstruc!=NULL) {
				if (!(chnInf->venv.envstruc->type&1))
				{
					chnInf->vol = 0;
					stopSample(chn);
				}
			}
			else {
				chnInf->vol=0;
				stopSample(chn);
			}
		}
		// XM style-keyoff
		// volume is set to zero
		else if (note == XModule::NOTE_OFF) {
			note=0;
			if (chnInf->venv.envstruc!=NULL) {
				if (!(chnInf->venv.envstruc->type&1)) 
					chnInf->vol = 0;
			}
			else 
				chnInf->vol=0;
			
			chnInf->keyon = false;
		}
		else
		{			
			playInstrument(chn, chnInf);								
		}
		
	}
			
}


void PlayerSTD::TPrEnv::setToTick(mp_uint32 tick)
{
	if (envstruc == NULL)
		return;
		
	bool bSet = false;
	for (mp_sint32 i = 0; i < envstruc->num-1; i++)
	{
		if (tick >= envstruc->env[i][0] &&
			tick < envstruc->env[i+1][0])
		{
			a = i;
			b = i+1;
			step = tick;
			bSet = true;
			break;
		}
	}
	
	if (!bSet)
	{
		// if position is beyond the last envelope point
		// we limit it to the last point and exit
		bool beyond = tick > envstruc->env[envstruc->num-1][0];
		a = envstruc->num-1;
		b = envstruc->num;
		step = envstruc->env[envstruc->num-1][0];
		if (beyond)
			return;
	}

	// check if we set envelope position to a loop end point
	// in that case wrap to the loop start, otherwise the loop
	// end is skipped and the envelope will roll out without 
	// looping
	if ((envstruc->type & 4) && 
		step == envstruc->env[envstruc->loope][0])
	{
		a=envstruc->loops;
		b=envstruc->loops+1;
		step=envstruc->env[a][0];
	}
}
