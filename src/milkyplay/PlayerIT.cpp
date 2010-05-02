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
 *  PlayerIT.cpp
 *  MilkyPlay IT player. Note that this evolved out of the standard player which
 *  aims at correct XM replay, so many things might still be not correct for
 *  impulse tracker. So this code is subject to change. 
 *
 *
 */
#include "PlayerIT.h"

// if we're in background we work on our own state
// if not, we're just going to work on the host state
PlayerIT::TChnState& PlayerIT::TVirtualChannel::chnstat()
{
	return host ? host->state : state;
}

#define CHANNEL_FLAGS_DVS				0x10000
#define CHANNEL_FLAGS_DFS				0x20000
#define CHANNEL_FLAGS_DPS				0x40000
#define CHANNEL_FLAGS_FORCE_FORWARD		0x00001
#define CHANNEL_FLAGS_FORCE_BACKWARD	0x00002
#define CHANNEL_FLAGS_FORCE_BILOOP		0x00004
#define CHANNEL_FLAGS_UPDATE_IGNORE		0x00100

//#define MINPERIOD (113*4)

// must be called after the poscnt has been properly set
#define RESETLOOPING \
{ \
	chnInf->loopstart=chnInf->loopcounter=chnInf->execloop=0; \
	chnInf->isLooping = false; \
	chnInf->loopingValidPosition = poscnt; \
} 	

#define RESET_ALL_LOOPING \
{ \
	for (mp_sint32 c = 0; c < numModuleChannels; c++) \
	{ \
		TModuleChannel *chnInf = &chninfo[c]; \
		RESETLOOPING \
	} \
} 	

static inline mp_sint32 myMod(mp_sint32 a, mp_sint32 b)
{
	mp_sint32 r = a % b;
	return r < 0 ? b + r : r;
}

const mp_sint32	PlayerIT::vibtab[32] = {
	0,24,49,74,97,120,141,161,
	180,197,212,224,235,244,250,253,
	255,253,250,244,235,224,212,197,
	180,161,141,120,97,74,49,24
};

const mp_sint32 PlayerIT::finesintab[256] = {
	0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
	24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
	45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
	59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
	59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
	45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
	24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
	0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
	-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
	-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
	-64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
	-59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
	-45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
	-24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2
};

#define MAXNOTES (16*12)
#define LINEAR_PERIOD_MAX (MAXNOTES*16*4)

const mp_uword PlayerIT::lintab[769] = {
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

const mp_uint32 PlayerIT::logtab[] = {
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

// 2^(SlideValue/768) in 16.16 fixed point
// SlideValue in [-256..256]
const mp_uint32 PlayerIT::powtab[] = {
	52015, 52062, 52109, 52156, 52204, 52251, 52298, 52345, 52392, 52440, 52487, 52534, 52582, 52629, 52677, 52724, 
	52772, 52820, 52867, 52915, 52963, 53011, 53059, 53107, 53154, 53202, 53250, 53299, 53347, 53395, 53443, 53491, 
	53540, 53588, 53636, 53685, 53733, 53782, 53830, 53879, 53928, 53976, 54025, 54074, 54123, 54172, 54220, 54269, 
	54318, 54367, 54417, 54466, 54515, 54564, 54613, 54663, 54712, 54761, 54811, 54860, 54910, 54959, 55009, 55059, 
	55108, 55158, 55208, 55258, 55308, 55358, 55408, 55458, 55508, 55558, 55608, 55658, 55709, 55759, 55809, 55860, 
	55910, 55961, 56011, 56062, 56112, 56163, 56214, 56264, 56315, 56366, 56417, 56468, 56519, 56570, 56621, 56672, 
	56723, 56775, 56826, 56877, 56928, 56980, 57031, 57083, 57134, 57186, 57238, 57289, 57341, 57393, 57445, 57496, 
	57548, 57600, 57652, 57704, 57757, 57809, 57861, 57913, 57965, 58018, 58070, 58123, 58175, 58228, 58280, 58333, 
	58385, 58438, 58491, 58544, 58597, 58650, 58702, 58755, 58809, 58862, 58915, 58968, 59021, 59075, 59128, 59181, 
	59235, 59288, 59342, 59395, 59449, 59503, 59556, 59610, 59664, 59718, 59772, 59826, 59880, 59934, 59988, 60042, 
	60096, 60151, 60205, 60259, 60314, 60368, 60423, 60477, 60532, 60586, 60641, 60696, 60751, 60806, 60860, 60915, 
	60970, 61025, 61081, 61136, 61191, 61246, 61301, 61357, 61412, 61468, 61523, 61579, 61634, 61690, 61746, 61801, 
	61857, 61913, 61969, 62025, 62081, 62137, 62193, 62249, 62305, 62362, 62418, 62474, 62531, 62587, 62644, 62700, 
	62757, 62814, 62870, 62927, 62984, 63041, 63098, 63155, 63212, 63269, 63326, 63383, 63440, 63498, 63555, 63612, 
	63670, 63727, 63785, 63842, 63900, 63958, 64016, 64073, 64131, 64189, 64247, 64305, 64363, 64421, 64479, 64538, 
	64596, 64654, 64713, 64771, 64830, 64888, 64947, 65005, 65064, 65123, 65182, 65240, 65299, 65358, 65417, 65476, 
	65536, 65595, 65654, 65713, 65773, 65832, 65891, 65951, 66010, 66070, 66130, 66189, 66249, 66309, 66369, 66429, 
	66489, 66549, 66609, 66669, 66729, 66789, 66850, 66910, 66971, 67031, 67092, 67152, 67213, 67273, 67334, 67395, 
	67456, 67517, 67578, 67639, 67700, 67761, 67822, 67883, 67945, 68006, 68067, 68129, 68190, 68252, 68314, 68375, 
	68437, 68499, 68561, 68623, 68685, 68747, 68809, 68871, 68933, 68995, 69057, 69120, 69182, 69245, 69307, 69370, 
	69432, 69495, 69558, 69621, 69684, 69747, 69809, 69873, 69936, 69999, 70062, 70125, 70189, 70252, 70315, 70379, 
	70442, 70506, 70570, 70633, 70697, 70761, 70825, 70889, 70953, 71017, 71081, 71145, 71209, 71274, 71338, 71403, 
	71467, 71532, 71596, 71661, 71725, 71790, 71855, 71920, 71985, 72050, 72115, 72180, 72245, 72310, 72376, 72441, 
	72507, 72572, 72638, 72703, 72769, 72834, 72900, 72966, 73032, 73098, 73164, 73230, 73296, 73362, 73429, 73495, 
	73561, 73628, 73694, 73761, 73827, 73894, 73961, 74027, 74094, 74161, 74228, 74295, 74362, 74429, 74497, 74564, 
	74631, 74699, 74766, 74833, 74901, 74969, 75036, 75104, 75172, 75240, 75308, 75376, 75444, 75512, 75580, 75648, 
	75717, 75785, 75853, 75922, 75991, 76059, 76128, 76197, 76265, 76334, 76403, 76472, 76541, 76610, 76679, 76749, 
	76818, 76887, 76957, 77026, 77096, 77165, 77235, 77305, 77375, 77445, 77514, 77584, 77655, 77725, 77795, 77865, 
	77935, 78006, 78076, 78147, 78217, 78288, 78359, 78429, 78500, 78571, 78642, 78713, 78784, 78855, 78926, 78998, 
	79069, 79140, 79212, 79283, 79355, 79427, 79498, 79570, 79642, 79714, 79786, 79858, 79930, 80002, 80074, 80147, 
	80219, 80292, 80364, 80437, 80509, 80582, 80655, 80727, 80800, 80873, 80946, 81019, 81093, 81166, 81239, 81312, 
	81386, 81459, 81533, 81607, 81680, 81754, 81828, 81902, 81976, 82050, 82124, 82198, 82272, 82346, 82421, 82495, 
	82570 // one more value because of linear interpolation
};

mp_sint32	PlayerIT::interpolate(mp_sint32 eax,mp_sint32 ebx,mp_sint32 ecx,mp_sint32 edi,mp_sint32 esi)
{
	if (ebx==ecx) return edi;
	mp_sint32 di = ((eax-ebx)*(esi-edi))/(ecx-ebx)+edi;
	return (mp_sint32)di;
}

// This takes the period with 8 bit fractional part
mp_sint32	PlayerIT::getlinfreq(mp_sint32 per)
{
	if (per<0) per=0;
	if (per>LINEAR_PERIOD_MAX*256) per=LINEAR_PERIOD_MAX*256;
	
	mp_sint32 t = (LINEAR_PERIOD_MAX*256-per)/(768*256);
	mp_sint32 r = myMod(LINEAR_PERIOD_MAX*256-per, 768*256);
	
	// Linear interpolation seems to be wrong here
	/*mp_sint32 frac = r & 255;
	
	mp_sint32 r1 = ((lintab[r>>8])<<t)>>5;
	mp_sint32 r2 = ((lintab[(r>>8)+1])<<t)>>5;

	return ((255-frac)*r1 + frac*r2) >> 8;*/

	return t >= 0 ? (((lintab[r>>8])<<t)>>5) : (((lintab[r>>8])>>(-t))>>5);
}

// This takes the period with 8 bit fractional part
mp_sint32	PlayerIT::getlogfreq(mp_sint32 per) 
{ 
	return fixeddiv(14317056, per)>>8; 
}

mp_sint32	PlayerIT::getlinperiod(mp_sint32 note,mp_sint32 relnote,mp_sint32 finetune) 
{
	note+=relnote+(mp_sint32)module->header.relnote;
	
	//if (note<1) note = 1;
	if (note>MAXNOTES) note = MAXNOTES;
	
	// t=(24L*OCTAVE+2-note)*32L-(fine>>1);
	
	return ((LINEAR_PERIOD_MAX-((note-1)*16*4)-(finetune/2)));	
}

mp_sint32	PlayerIT::getlogperiod(mp_sint32 note,mp_sint32 relnote,mp_sint32 finetune)
{
	note+=relnote+(mp_sint32)module->header.relnote;

	//if (note<1) note = 1;
	if (note>MAXNOTES) note = MAXNOTES;
	
	mp_sint32 ft = finetune;
	ft+=128;
	mp_sint32 octave = (note-1)/12;
	mp_sint32 n = myMod(note-1, 12)<<3;
	mp_sint32 pi = (ft>>4)+n;
	mp_sint32 v1 = logtab[pi];
	mp_sint32 v2 = logtab[pi+1];
	mp_sint32 t = (ft>>4)-8;
	//mp_sint32 t = (ft>>4);
	return octave >= 0 ? (interpolate(t,0,15,v1,v2)>>octave) : (interpolate(t,0,15,v1,v2)<<(-octave));
}


PlayerIT::PlayerIT(mp_uint32 frequency) : 
	PlayerBase(frequency)
{
	chninfo		= NULL;
	vchninfo	= NULL;
	attick		= NULL;	
	// fill in some default values, don't know if this is necessary

	tickSpeed			= 6;				// our tickspeed
	bpm					= 125;				// BPM speed
	ticker				= tickSpeed-1;		// runs from 0 to tickspeed-1
	patternIndex		= 0;				// holds current pattern index
	numEffects			= 0;				// current number of effects
	numChannels			= 0;				// current number of channels
	numMaxVirChannels	= 256;				// maximum amount of virtual channels
	
	//loopstart = execloop = loopcounter=0;

	patDelay = false;
	patDelayCount = 0;
	haltFlag = false;

	options[PlayModeOptionPanning8xx] = true;
	options[PlayModeOptionPanningE8x] = false;
	options[PlayModeOptionForcePTPitchLimit] = true;
}

PlayerIT::~PlayerIT()
{
	freeMemory();
}

#define MYMAX(a, b) (mp_sint32)((a) > (b) ? (a) : (b))

void PlayerIT::timerHandler(mp_sint32 currentBeatPacket)
{
	PlayerBase::timerHandler(currentBeatPacket);

	if (paused)
		return;

	// get current maximum virtual channels
	mp_sint32 oldMaxVirChannels = curMaxVirChannels;

	mp_int64 dummy = (mp_int64)BPMCounter;
	dummy+=(mp_int64)adder;
	BPMCounter=(mp_sint32)dummy;
	
	// check overflow-carry 
	if ((dummy>>32)) 
	{
		tickhandler();
	}
	
	if (module->header.flags & XModule::MODULE_AMSENVELOPES)
		updateBPMIndependent();
	
	// if the new maximum of virtual channels is greater than the old one
	// set it to the new one, else keep the old one, because if some channels were
	// cut by stopSample() the mixer needs to shut these off
	setActiveChannels(MYMAX(curMaxVirChannels, oldMaxVirChannels));
}

mp_sint32 PlayerIT::startPlaying(XModule* module, 
								 bool repeat/* = false*/, 
								 mp_uint32 startPosition/* = 0*/, 
								 mp_uint32 startRow/* = 0*/,
								 mp_sint32 numChannels/* = -1*/, 
								 const mp_ubyte* customPanningTable/* = NULL*/,
								 bool idle/* = false*/,
								 mp_sint32 patternIndex/* = -1*/,
								 bool playOneRowOnly/* = false*/)
{

	/*for (mp_sint32 vmd = -256; vmd < 257; vmd++)
	{
		double fac = pow(2.0, vmd/768.0);
		printf("%d, ", (mp_sint32)(fac * 65536.0));
	}*/

	numModuleChannels = module->header.channum;
	numVirtualChannels = numMaxVirChannels < numModuleChannels ? numModuleChannels : numMaxVirChannels;

	return PlayerBase::startPlaying(module, 
									repeat, 
									startPosition,
									startRow,
									numVirtualChannels,
									customPanningTable,
									idle,
									patternIndex,
									playOneRowOnly);
}


void PlayerIT::restart(mp_uint32 startPosition/* = 0*/, mp_uint32 startRow/* = 0*/, bool resetMixer/* = true*/, const mp_ubyte* customPanningTable/* = NULL*/, bool playOneRowOnly/* = false*/)
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

	for (i = 0; i < numModuleChannels; i++)
	{
		chninfo[i].setMasterVol(0xff);
		chninfo[i].setPan(customPanningTable ? customPanningTable[i] : module->header.pan[i]);
	}

	memset(rowHits, 0, sizeof(rowHits));
	
	for (i = 0; i < (signed)startPosition; i++)
		for (j = 0; j < 256; j++)
			visitRow(i*256+j);
	
	for (i = 0; i < (signed)startRow; i++)
		visitRow(startPosition*256+i);
}

void PlayerIT::reset()
{
	curMaxVirChannels = 0;
	memset(chninfo, 0, sizeof(TModuleChannel)*numModuleChannels);
	memset(vchninfo, 0, sizeof(TVirtualChannel)*numVirtualChannels);
	RESET_ALL_LOOPING
}

void PlayerIT::resetAllSpeed()
{
	bpm	= module->header.speed;
	tickSpeed = module->header.tempo;
	ticker = 0;

	this->adder = getbpmrate(this->bpm);	
}

mp_sint32 PlayerIT::allocateStructures() 
{
	freeMemory();

	chninfo			= new TModuleChannel[numModuleChannels];
	vchninfo		= new TVirtualChannel[numVirtualChannels];
	attick			= new mp_ubyte[numModuleChannels];
	return MP_OK;
}

void PlayerIT::freeMemory() 
{
	if (chninfo) 
	{ 
		delete[] chninfo; 
		chninfo = NULL; 
	} 
	if (vchninfo) \
	{ 
		delete[] vchninfo; 
		vchninfo = NULL; 
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
void PlayerIT::clearEffectMemory()
{
	if (!module || !chninfo) 
		return;
	
	ticker = 0;
	
	//loopstart = execloop = loopcounter=0;
	mp_sint32 i;
	for (i = 0; i < numModuleChannels; i++)
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

PlayerIT::TVirtualChannel* PlayerIT::allocateVirtualChannel()
{
	const mp_sint32 numVirtualChannels = this->numVirtualChannels;
	
	mp_sint32 i;

	TVirtualChannel* vchn = vchninfo;	
	for (i = 0; i < numVirtualChannels; i++, vchn++)
	{
		if (vchn->getBackground())
		{
			if (!vchn->getActive())
			{
				if (i+1 > curMaxVirChannels)
					curMaxVirChannels = i+1;
				vchn->setChannelIndex(i);
				return vchn;
			}
		}
	}
	
	mp_sint32 chnIndex = -1;
	mp_sint32 vol = 0x7FFFFFFF;
	vchn = vchninfo;	
	for (i = 0; i < curMaxVirChannels; i++, vchn++)
	{
		if (vchn->getBackground())
		{
			mp_sint32 newVol = vchn->getResultingVolume();
			if (newVol < vol)
			{
				vol = newVol;
				chnIndex = i;
			}
		}
	}
	
	if (chnIndex != -1)
	{
		vchn = vchninfo + chnIndex;
		vchn->setChannelIndex(chnIndex);
		return vchn;
	}
	
	return NULL;
}

void PlayerIT::handleNoteOFF(TChnState& state)
{
	const mp_sint32 ins = state.getIns();
	if (ins && ins <= module->header.insnum)
	{
		// IT style fadeout also works without active envelope
		if (module->instr[ins-1].flags & TXMInstrument::IF_ITFADEOUT)
		{	
			if (state.getVenv().envstruc!=NULL) 
			{
				// envelope is off or on and looping
				if (!state.getVenv().isEnabled() || 
					(state.getVenv().isEnabled() && (state.getVenv().envstruc->type&4))) 
				{
					state.setFadeout(true);
				}
			}
			// no envelope at all
			else 
			{
				state.setFadeout(true);
			}
		}
		// XM style (envelope is off)
		else if (!state.getVenv().isEnabled()) 
		{
			state.setVol(0);
			state.adjustTremoloTremorVol();
		}
	}
	
	state.setKeyon(false);
}

void PlayerIT::handlePastNoteAction(TModuleChannel* chnInf, mp_ubyte pastNoteActionType)
{
	TVirtualChannel* vchn = vchninfo;	
	const mp_sint32 curMaxVirChannels = this->curMaxVirChannels;
	
	switch (pastNoteActionType)
	{
		case 0:
		{
			for (mp_sint32 i = 0; i < curMaxVirChannels; i++, vchn++)
				if (vchn->getActive() && (vchn->getOldHost() == chnInf))
				{
					mp_sint32 index = vchn->getChannelIndex();
					stopSample(index);
					releaseVirtualChannel(vchn);
				}
			break;
		}
		
		case 1:
		{
			for (mp_sint32 i = 0; i < curMaxVirChannels; i++, vchn++)
				if (vchn->getActive() && (vchn->getOldHost() == chnInf))
					handleNoteOFF(vchn->getRealState());
			break;
		}

		case 2:
		{
			for (mp_sint32 i = 0; i < curMaxVirChannels; i++, vchn++)
				if (vchn->getActive() && (vchn->getOldHost() == chnInf))
					vchn->getRealState().setFadeout(true);
			break;
		}
	}
}

bool PlayerIT::handleDCT(TModuleChannel* chnInf, const TNNATriggerInfo& triggerInfo, mp_ubyte DCT, mp_ubyte DCA)
{
	TVirtualChannel* vchn = vchninfo;	
	const mp_sint32 curMaxVirChannels = this->curMaxVirChannels;
	for (mp_sint32 i = 0; i < curMaxVirChannels; i++, vchn++)
	{
		if (vchn->getActive() && (vchn->getOldHost() == chnInf || vchn->getHost() == chnInf))
		{
			bool matchDCT;
		
			// normal case (instrument supplied with note)
			if (triggerInfo.ins)
			{
				// must always be the same instrument
				matchDCT = (vchn->getIns() == triggerInfo.ins);
				// check for note
				if (DCT == 1)
					matchDCT &= (vchn->getNote() == triggerInfo.note);
				// check for sample
				else if (DCT == 2)
					matchDCT &= (vchn->getSmp() == triggerInfo.smp);
			}
			// no instrument supplied with note
			else 
			{
				matchDCT = true;
				// note check doesn't do anything if instrument is 0
				if (DCT == 1)
					continue;
			}
			
			if (!matchDCT)
				continue;
			
			// cut = keep channel
			if (DCA == 0)
			{
				mp_sint32 index = vchn->getChannelIndex();
				stopSample(index);
				releaseVirtualChannel(vchn);
			}
			// note off
			else if (DCA == 1)
			{
				// virtual channel is no longer linked to host
				if (vchn->getOldHost() == chnInf)
					handleNoteOFF(vchn->getRealState());
				// virtual channel is linked to host, unlink and handle note off
				else
				{
					// important: first set host to NULL
					// THEN set key on flag
					TVirtualChannel* oldvchn = chnInf->unlinkVchn();
					handleNoteOFF(oldvchn->getRealState());
				}
			}
			// note fade
			else if (DCA == 2)
			{
				// virtual channel is no longer linked to host
				if (vchn->getOldHost() == chnInf)
					vchn->getRealState().setFadeout(true);
				// virtual channel is linked to host, unlink and handle fade out
				else
				{
					// important: first set host to NULL
					// THEN set fade out
					chnInf->unlinkVchn()->setFadeout(true);
				}
			}
		}
	}
	
	// deal with no instrument case
	if (!triggerInfo.ins)
	{
		// if DCT is set to note, just continue with playing the note
		if (DCT == 1)
			return true;
		// if DCT is set to ins/smp and DCA is set to cut, cut and don't play the current note
		else if (DCA == 0)
			return false;
	}
	
	return true;
}

bool PlayerIT::handleNNAs(TModuleChannel* chnInf, const TNNATriggerInfo& triggerInfo)
{
	/*if (poscnt == 7 && rowcnt == 00)
	{
		int i=0;
		i++;
		i--;
	}*/

	// sanity checks
	if (triggerInfo.ins > module->header.insnum  ||
		!triggerInfo.note ||
		triggerInfo.smp < 0)
		return true;
	
	TVirtualChannel* newVchn = allocateVirtualChannel();
	if (newVchn == NULL)
		return false;
	
	mp_uword insflags = chnInf->getInsflags();
	mp_ubyte NNA = (insflags>>4) & 3;
	mp_ubyte DCT = (insflags>>6) & 3;
	mp_ubyte DCA = (insflags>>8) & 3;
	
	if (DCT)
	{
		if (!handleDCT(chnInf, triggerInfo, DCT, DCA))
			return false;
	}
	
	// do we have some virtual channel already?
	if (chnInf->hasVchn())
	{
		// NNA = CUT? Use the same virtual channel for playback
		if (NNA == 0)
			return true;
		// NNA = continue
		else if (NNA == 1)
		{
			chnInf->unlinkVchn();			
			chnInf->linkVchn(newVchn);
			return true;
		}
		// NNA = note off
		else if (NNA == 2)
		{
			// important: first set host to NULL
			// THEN set key on flag
			TVirtualChannel* oldvchn = chnInf->unlinkVchn();
			handleNoteOFF(oldvchn->getRealState());
			chnInf->linkVchn(newVchn);
			return true;
		}
		// NNA = note fade
		else if (NNA == 3)
		{
			// important: first set host to NULL
			// THEN set fade out
			chnInf->unlinkVchn()->setFadeout(true);
			chnInf->linkVchn(newVchn);
			return true;
		}
	}
	else
	{
		chnInf->linkVchn(newVchn);
	}	
	
	return true;
}

void PlayerIT::adjustVirtualChannels()
{
	mp_sint32 i;

	TVirtualChannel* vchn = vchninfo;	
	for (i = 0; i < curMaxVirChannels; i++, vchn++)
	{
		if (!vchn->getActive())
			continue;
			
		if (vchn->getBackground())
		{
			if (!isChannelPlaying(i) ||
				!vchn->getVol() ||
				!vchn->getMasterVol() ||
				!vchn->getFadevolstart() ||
				vchn->getVenv().cutted(vchn->getKeyon()))
			{
				//bool cutted = vchn->getVenv().cutted(vchn->getKeyon());
				mp_sint32 index = vchn->getChannelIndex();
				stopSample(index);
				releaseVirtualChannel(vchn);
				continue;
			}
		}
		/*else
		{
			if (!vchn->getFadevolstart() ||
				vchn->getVenv().cutted(vchn->getKeyon()))
			{
				mp_sint32 index = vchn->getChannelIndex();
				stopSample(index);
				continue;
			}
		}*/
	}
}

void PlayerIT::prenvelope(TPrEnv *env, bool keyon, bool timingIT)
{
	if (env->isEnabled()) 
	{
		// if we're sitting on a sustain point and key is on, we don't advance further
		if ((env->envstruc->type&2) && (env->a==env->envstruc->sustain) && 
			(env->step == env->envstruc->env[env->a][0]) && keyon) 
			return;
		
		// IT-style envelopes count differently
		if (timingIT)
		{
			if ((env->step<=env->envstruc->env[env->b][0]) && (env->b < env->envstruc->num)) 
				env->step++;
			
			if (env->step > env->envstruc->env[env->b][0]) {
				
				// normal loop
				if ((env->envstruc->type&4))
				{
					// check for envelope loop break (AMS)
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
				
				// sustain loop (IT)
				if ((env->envstruc->type&16))
				{
					if (keyon) 
					{
						if (env->b==env->envstruc->susloope) {
							env->a=env->envstruc->sustain;
							env->b=env->envstruc->sustain+1;
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
				//else
				//{
				//	// fuck you
				//	printf("fuck");
				//}
			}
			
		}
		else
		{
			if ((env->step!=env->envstruc->env[env->b][0]) && (env->b < env->envstruc->num)) 
				env->step++;
			
			if (env->step == env->envstruc->env[env->b][0]) {
				
				// normal loop
				if ((env->envstruc->type&4))
				{
					// check for envelope loop break (AMS)
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
				
				// sustain loop (IT)
				if ((env->envstruc->type&16))
				{
					if (keyon) // Break envelope if sustain pt == loop end point AND sustain is enabled AND key off is send
					{
						if (env->b==env->envstruc->susloope) {
							env->a=env->envstruc->sustain;
							env->b=env->envstruc->sustain+1;
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

}

mp_sint32 PlayerIT::getenvval(TPrEnv *env,mp_sint32 n)
{
	if (env->isEnabled()) 
	{
		mp_sint32 step = env->step;
		if (step > env->envstruc->env[env->b][0])
			step = env->envstruc->env[env->b][0];
		mp_sint32 dx = (env->envstruc->env[env->b][0]-env->envstruc->env[env->a][0]);
		if (dx==0) dx=1;
		mp_sint32 t = (env->envstruc->env[env->b][0]-step)*65536/dx;
		mp_sint32 y0 = env->envstruc->env[env->a][1];
		mp_sint32 y1 = env->envstruc->env[env->b][1];
		
		mp_sint32 y = (y0*t)+(y1*(65536-t));
		
		return y>>16;		
	}
	return n;
}

mp_sint32 PlayerIT::getFinalPeriod(TChnState& state, mp_sint32 p) 
{
	mp_sint32 envVib = 0;
	p<<=8;

	if (state.vibenv.isEnabled())
	{
		mp_sint32 eval = (getenvval(&state.vibenv,128)-128) << (state.vibenv.envstruc->type>>6);
		// AMS doc says vibrato with amplify set to 8 equals vibrato 0xF
		// => alright
		envVib = (eval*61408)>>(3+16-8);
	}

	if (state.avibused & 127) 
	{
		// if this is XM style auto vibrato, the running counter is divided by 4
		mp_ubyte vp = state.avibcnt >> ((state.avibused & 128) ? 0 : 2);
		mp_ubyte vd = state.avibdepth;
		
		mp_sint32 vm = 0;
		
		mp_sint32 vl = 0;
		switch (state.avibused & 127) 
		{
			// sine
			case 1 : vl=vibtab[vp&31]; break;
			// square
			case 2 : vl=255; break;
			// ramp down
			case 3 : {
						vl=((vp&31)*539087)>>16;
						if ((vp&63)>31) vl=255-vl;
						vl=-vl;
					 }; break;
			// ramp up
			case 4 : {
						vl=((vp&31)*539087)>>16;
						if ((vp&63)>31) vl=255-vl;
					 }; break;
		}
		
		// IT style vibrato sweep
		if (state.avibused & 128)
		{
			if (state.avibsweep && state.avibswcnt < (vd << 8)) 
				vm = (vl*state.avibswcnt)>>(1+8);
			else
				vm = (vl*vd)>>1;
		}
		// XM style vibrato sweep
		else
		{
			vm = (vl*vd)>>1;
		
			if (state.avibsweep) 
			{
				vm*=(mp_sint32)state.avibswcnt * 256;
				vm/=state.avibsweep;
				vm>>=8;
			}
		}
		
		if ((vp&63)>31) vm=-vm;
		
		// IT style envelope and amiga periods?
		if (!(module->header.freqtab&1) && (state.avibused & 128))
		{
			// vibrato value has 8 bit fractional part
			mp_uint32 vmi = vm >> 8;
			// convert fraction to 16 bit
			mp_uint32 vmf = (vm & 255) << 8;
			
			// table ranges from [-256..256] 
			// elevate index to start by 0
			mp_uint32 fac1 = powtab[vmi+256];
			mp_uint32 fac2 = powtab[vmi+256+1];
			// interpolate between two array values
			mp_uint32 fac = fixedmul(65536-vmf, fac1) + fixedmul(vmf, fac2);
			
			return (fixedmul(p<<8, fac)>>8) + envVib;
			
			// see ITTECH.TXT
			//double fac = pow(2.0, vm/(768.0*256.0));
			//return (mp_sint32)(p*fac) + envVib;
		}
		// linear periods
		return (p+vm+envVib);
	}
	else return (p+envVib);
}

void PlayerIT::playInstrument(TModuleChannel* chnInf, bool bNoRestart/* = false*/)
{
	const mp_sint32 ins = chnInf->getIns();
	const mp_sint32 smp = chnInf->getSmp();
	const mp_sint32 chn = chnInf->getPlaybackChannelIndex();

	if (chn < 0 || !ins || ins > module->header.insnum)
		return;

	if (module->instr[ins-1].samp && smp != -1)
	{
		chnInf->resetFlag(CHANNEL_FLAGS_UPDATE_IGNORE);
		
		const mp_sint32 i = smp;
		
		// start out with the flags for 16bit sample
		mp_sint32 flags = ((module->smp[i].type&16)>>4)<<2;
		// add looping + backward flags
		flags |= module->smp[i].type&(3+128);
		// one shot forward looping?
		flags |= module->smp[i].type & 32;
		
		// force forward playing
		if (chnInf->isFlagSet(CHANNEL_FLAGS_FORCE_FORWARD))
			flags &= ~128;
		
		// force backward playing
		if (chnInf->isFlagSet(CHANNEL_FLAGS_FORCE_BACKWARD))
			flags |= 128;
		
		if (flags&3) 
		{			
			if (chnInf->isFlagSet(CHANNEL_FLAGS_FORCE_BILOOP))
				flags = (flags & ~3) | 2;
			
			// bNoRestart = false means play new sample from beginning or sample offset
			if (!bNoRestart)
			{
				playSample(chn,
						   (mp_sbyte*)module->smp[i].sample,
						   module->smp[i].samplen,											   
						   chnInf->smpoffs + chnInf->smpoffshigh,
						   0, // sample offset fraction
						   !playModeChopSampleOffset,
						   module->smp[i].loopstart,
						   module->smp[i].loopstart+module->smp[i].looplen,
						   flags);
			}
			// bNoRestart = true means play new sample from beginning of the last sample
			else
			{
				mp_sint32 smpoffset = chnInf->smpoffs ? (chnInf->smpoffs+chnInf->smpoffshigh) : getSamplePos(chn);
				mp_sint32 smpoffsetfrac = chnInf->smpoffs ? 0 : getSamplePosFrac(chn);
			
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
						   chnInf->smpoffs + chnInf->smpoffshigh,
						   0, // sample offset fraction
						   !playModeChopSampleOffset,
						   0,
						   module->smp[i].samplen,
						   flags);
			}
			// bNoRestart = true means play new sample from beginning of the last sample AND don't ramp volume up
			else
			{
				mp_sint32 smpoffset = chnInf->smpoffs ? (chnInf->smpoffs+chnInf->smpoffshigh) : getSamplePos(chn);
				mp_sint32 smpoffsetfrac = chnInf->smpoffs ? 0 : getSamplePosFrac(chn);

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

void PlayerIT::updatePlayModeFlags()
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
		case PlayMode_ImpulseTracker:
			newInsPTFlag = false;
			newInsST3Flag = true;
			oldPTInsChangeFlag = false;
			break;
		case PlayMode_FastTracker2:
			newInsPTFlag = false;
			newInsST3Flag = false;
			oldPTInsChangeFlag = false;
			break;
		case PlayMode_Auto:
			break;
	}

	playModeFT2 = (playMode == PlayMode_FastTracker2 ? true : false);
	if (playMode == PlayMode_Auto && (module->header.flags & XModule::MODULE_XMARPEGGIO))
		playModeFT2 = true;

	// Chop off samples which sample offsets greater sample length?
	playModeChopSampleOffset = playModeFT2 || (playMode == PlayMode_ProTracker3);
}

mp_sint32 PlayerIT::calcVibrato(TModuleChannel* chnInf, mp_sint32 effcnt, mp_sint32 depthShift/* = 5*/)
{
	mp_sint32 vp = chnInf->vibpos[effcnt];
	mp_sint32 vd = chnInf->vibdepth[effcnt];
	
	mp_sint32 vm = (vibtab[vp&31]*vd) >> ((module->header.flags & XModule::MODULE_ITNEWEFFECTS) ? (depthShift+1) : depthShift);
	if ((vp&63)>31) vm=-vm;
	return vm;
}

void PlayerIT::doTickVolslidePT(TModuleChannel* chnInf, mp_sint32 effcnt)
{
	mp_ubyte x = chnInf->old[effcnt].volslide>>4;
	mp_ubyte y = chnInf->old[effcnt].volslide&0xf;
	
	// 08/31/04: fixed...
	// don't reject volume slide if both operands are set
	// instead, slide up
	// see other volume slides as well
	if (x&&y) y = 0;
	
	if (ticker) {
		if (x) {
			chnInf->incVol(x*4);
		}
		if (y) {
			chnInf->decVol(y*4);
		}
		chnInf->adjustTremoloTremorVol();
	}
}

void PlayerIT::doTickVolslideST(TModuleChannel* chnInf, mp_sint32 effcnt)
{
	if (!(module->header.flags & XModule::MODULE_OLDS3MVOLSLIDES) && 
		ticker == 0)
		return;
	
	mp_ubyte x = chnInf->old[effcnt].volslide>>4;
	mp_ubyte y = chnInf->old[effcnt].volslide&0xf;
	
	if (x == 0xF && y) return;
	if (y == 0xF && x) return;
	
	if (x && y) y = 0;
	
	if (x) {
		chnInf->incVol(x*4);
	}
	if (y) {
		chnInf->decVol(y*4);
	}
	chnInf->adjustTremoloTremorVol();
}

void PlayerIT::doTickEffect(TModuleChannel* chnInf, mp_sint32 effcnt)
{
	const mp_sint32 chn = chnInf->getPlaybackChannelIndex();
	
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
				doEffect(chnInf, effcnt);
		}
	}

	switch (chnInf->eff[effcnt]) {
		// portamento up
		case 0x01:
			if (ticker) {
				chnInf->decPer(chnInf->old[effcnt].portaup*4);
				handlePeriodUnderflow(chnInf);
				chnInf->adjustVibratoPer();
			}
			break;

		// portamento down
		case 0x02:
			if (ticker) {
				chnInf->incPer(chnInf->old[effcnt].portadown*4);
				handlePeriodOverflow(chnInf);
				chnInf->adjustVibratoPer();
			}
			break;

		// note portamento
		case 0x03:
		{
			if (ticker&&chnInf->destnote) {
				// If this is an XM module we need to store the last portamento operand always in the buffer for the second effect
				mp_sint32 op = ((module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER)  && numEffects == 2) ? chnInf->old[1].portanote : chnInf->old[effcnt].portanote;				
				chnInf->slideToPer(op*4);
				chnInf->adjustVibratoPer();
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
		
			mp_sint32 vmp = chnInf->getPer();
					
			vm = calcVibrato(chnInf, effNum);
			
			if (ticker || (module->header.flags & XModule::MODULE_ITNEWEFFECTS)) 
				chnInf->vibpos[effNum]+=chnInf->vibspeed[effNum];

			vmp+=vm;

			mp_sint32 maxTicks = patDelay ? patDelayCount : tickSpeed;

			// the vibrato in the volumn volumn (index 0) works differently 
			// before applying that, we assure that this is an XM module by checking
			// the module header
			if ((module->header.flags & XModule::MODULE_XMVOLCOLUMNVIBRATO) &&
				ticker == maxTicks - 1)
			{
				if (!effcnt)
					chnInf->setFinalVibratoPer(vmp);
				else
					chnInf->adjustVibratoPer();
			}

			if (chn >= 0)
				setFreq(chn,getFinalFreq(chnInf->chnstat(),getFinalPeriod(chnInf->chnstat(),vmp)));
			break;
		} 
			
		// note porta + volume slide
		case 0x05: 
		{
			if (ticker&&chnInf->destnote) {
				// If this is an XM module we need to store the last portamento operand always in the buffer for the second effect
				mp_sint32 op = (module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER) ? chnInf->old[1].portanote : chnInf->old[effcnt].portanote;
				chnInf->slideToPer(op*4);
				chnInf->adjustVibratoPer();
			}
			
			if (module->header.flags & XModule::MODULE_ST3DUALCOMMANDS)
				doTickVolslideST(chnInf, effcnt);
			else
				doTickVolslidePT(chnInf, effcnt);
			break;
		}
		
		// vibrato + volume slide
		case 0x06:	
		{
			vm = calcVibrato(chnInf, effcnt);
			
			if (ticker) 
				chnInf->vibpos[effcnt]+=chnInf->vibspeed[effcnt];

			if (chn >= 0)
				setFreq(chn,getFinalFreq(chnInf->chnstat(),getFinalPeriod(chnInf->chnstat(),chnInf->getPer()+vm)));
			
			if (module->header.flags & XModule::MODULE_ST3DUALCOMMANDS)
				doTickVolslideST(chnInf, effcnt);
			else
				doTickVolslidePT(chnInf, effcnt);
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
			
			mp_sint32 vmp = playModeFT2 ? (ticker == 0 ? chnInf->getVolume() : chnInf->getTremoloVol()) :
				chnInf->getVolume();
			
			// IT in new effects mode processes at non row tick
			if (ticker || (module->header.flags & XModule::MODULE_ITNEWEFFECTS)) 
			{
				// IT in new effects mode is two times finer
				vm = (vibtab[vp&31]*vd) >> ((module->header.flags & XModule::MODULE_ITNEWEFFECTS) ? (6-1) : (6-2));
				if ((vp&63)>31) vm=-vm;
				vmp+=vm;
				if (vmp<0) vmp=0;
				if (vmp>255) vmp=255;
				chnInf->trmpos[effcnt]+=chnInf->trmspeed[effcnt];
			}
			
			// FT2 hack... final tremolo volume stays on
			if (playModeFT2 && (ticker == tickSpeed - 1))
			{
				chnInf->setFinalTremVol(vmp);
			}

			if (chn >= 0)
				setVol(chn, getFinalVolume(chnInf->chnstat(), vmp, mainVolume));
			break;
		}
		
		// volume slide
		case 0x0A: 
		{
			doTickVolslidePT(chnInf, effcnt);
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

		// deal with eventual tempo slide
		case 0x16: 
		{
			if (!ticker || 
				chnInf->old[effcnt].temposlide >= 0x20 || 
				!(module->header.flags & XModule::MODULE_ITTEMPOSLIDE))
				break;
			
			x = chnInf->old[effcnt].temposlide>>4;
			y = chnInf->old[effcnt].temposlide&0xf;
			
			switch (x >> 4)
			{
				case 0:
					bpm-=y & 0x0F;
					if (bpm < 32)
						bpm = 32;
					break;
				case 1:
					bpm+=y & 0x0F;
					if (bpm > 255)
						bpm = 255;
					break;
			}

			this->adder = getbpmrate(bpm);
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
					chnInf->incPan(x);
				}
				if (y) {
					chnInf->decPan(y);
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
							chnInf->decVol(4);
							break;
						case 0x2 : 
							chnInf->decVol(8);
							break;
						case 0x3 : 
							chnInf->decVol(16);
							break;
						case 0x4 : 
							chnInf->decVol(32);
							break;
						case 0x5 : 
							chnInf->decVol(64);
							break;
						case 0x6 : 
							chnInf->setVol(chnInf->getVol()*2/3); 
							break;
						case 0x7 : 
							chnInf->setVol(chnInf->getVol()>>1); 
							break;
						case 0x9 : 
							chnInf->incVol(4);
							break;
						case 0xA : 
							chnInf->incVol(8);
							break;
						case 0xB : 
							chnInf->incVol(16);
							break;
						case 0xC : 
							chnInf->incVol(32);
							break;
						case 0xD : 
							chnInf->incVol(64);
							break;
						case 0xE : 
						{
							mp_sint32 vol = (chnInf->getVol()*3) >> 1;
							if (vol > 255) vol = 255;
							chnInf->setVol(vol);
							break;
						}
						case 0xF : 
						{
							mp_sint32 vol = chnInf->getVol() << 1;
							if (vol > 255) vol = 255;
							chnInf->setVol(vol);
							break;
						}
					}
					
					chnInf->adjustTremoloTremorVol();

					if (chnInf->validnote)
						playInstrument(chnInf);
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

			mp_sint32 v = (ticker == 0 ? chnInf->getVol() : chnInf->getTremorVol());
			
			if (ticker && chnInf->tremorcnt[effcnt] % (x+y) >= x)
				v = 0;
			
			if (ticker) 
				chnInf->tremorcnt[effcnt]++;
			
			if (ticker == tickSpeed - 1)
			{
				chnInf->setVol(v);
				chnInf->adjustTremoloVol();
			}
			
			if (chn >= 0)
				setVol(chn,getFinalVolume(chnInf->chnstat(), v, mainVolume));
			break;
		}
								
		// MDL/IT Subcommands
		case 0x1E: 
		{
			mp_ubyte eff = chnInf->eop[effcnt]>>4;
			mp_ubyte eop = chnInf->eop[effcnt]&0xf;
			switch (eff) {
				case 0x1 : 
					if (ticker) {
						chnInf->decPan(eop);
					}
					break;
				case 0x2 : 
					if (ticker) {
						chnInf->incPan(eop);
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
			if (chnInf->getNote())
			{
				mp_ubyte arpegLUT[3];
				
				mp_sint32 r = 0;
				mp_sint32 note = 0, onote = chnInf->getNote();
				//mp_sint32 c4spd = chnInf->c4spd;
				mp_sint32 relnote = chnInf->getRelnote();
				mp_sint32 finetune = chnInf->getFinetune();
				mp_sint32 per,nper;
				
				mp_ubyte eop = chnInf->old[effcnt].arpeg;
				
				mp_sint32 x = eop>>4;
				mp_sint32 y = eop&0xf;
				
				if (playModeFT2)
				{
					// dammit, FT2 arpeggios are so screwed:
					// the first 11 tick speeds and their arpeggio patterns (0 is note, 3 is fx digit 3, 2 is fx digit 2): 
					// 0: Totally fucked up. Just test it. 
					// 1: 0 
					// 2: 02 
					// 3: 032 
					// 4: 0032 
					// 5: 02320 
					// 6: 032032 
					// 7: 0032032 
					// 8: 02032032 
					// 9: 032032032 
					// A: 0032032032 				
					if (ticker == 0)
						r = 0;
					else
						r = myMod(ticker-tickSpeed,3);

					arpegLUT[0] = 0; arpegLUT[1] = 2; arpegLUT[2] = 1; 
				}
				else
				{
					r = (ticker)%3;

					arpegLUT[0] = 0; arpegLUT[1] = 1; arpegLUT[2] = 2; 
				}
				
				if (arpegLUT[r] == 0)
				{
					note=chnInf->getNote(); 
				}
				else if (arpegLUT[r] == 1)
				{
					note=chnInf->getNote()+x; 
				}
				else if (arpegLUT[r] == 2)
				{
					note=chnInf->getNote()+y; 
				}
				
				
				// Perform note clipping for XM note range if necessary
				if ((arpegLUT[r] != 0) && // Only done for arpeggio tick 1 & 2
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
					chnInf->setPer(getperiod(note,relnote,finetune));
					if (chn >= 0)
						setFreq(chn,getFinalFreq(chnInf->chnstat(),getFinalPeriod(chnInf->chnstat(),chnInf->getPer())));
				}
				else
				{						
					nper=getperiod(note,relnote,finetune);
					per=getperiod(onote,relnote,finetune);
					
					//nper = (8363*periods[(note-1)%12]*16>>(((note-1)/12)))/c4spd;
					//per = (8363*periods[(onote-1)%12]*16>>(((onote-1)/12)))/c4spd;
					
					nper-=per;
					nper+=chnInf->getPer();
					
					if (chn >= 0)
						setFreq(chn,getFinalFreq(chnInf->chnstat(),getFinalPeriod(chnInf->chnstat(),nper)));
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
					chnInf->setKeyon(true);
					// trigger replay only when last note has been valid
					if (chnInf->validnote)
						playInstrument(chnInf);
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
					chnInf->setVol(0);
					chnInf->adjustTremoloTremorVol();
				}
			//}
			break;
			
		// MDL porta up
		case 0x43: 
			if (ticker) {
				if (chnInf->old[effcnt].portaup<=0xDF) {
					chnInf->decPer(chnInf->old[effcnt].portaup*4);
					handlePeriodUnderflow(chnInf);
					chnInf->adjustVibratoPer();
				}
			}
			break;
			
		// MDL porta down
		case 0x44: 
			if (ticker) {
				if (chnInf->old[effcnt].portaup<=0xDF) {
					chnInf->incPer(chnInf->old[effcnt].portaup*4);
					handlePeriodOverflow(chnInf);
					chnInf->adjustVibratoPer();
				}
			}
			break;
				
		// MDL volslide up
		case 0x45: 
			if (ticker) {
				if (chnInf->old[effcnt].volslide<=0xDF) {
					chnInf->incVol(chnInf->old[effcnt].volslide);
					chnInf->adjustTremoloTremorVol();
				}
			}
			break;
				
		// MDL volslide down
		case 0x46: 
			if (ticker) {				
				if (chnInf->old[effcnt].volslide<=0xDF) {
					chnInf->decVol(chnInf->old[effcnt].volslide);
					chnInf->adjustTremoloTremorVol();
				}
			}
			break;
				
		// S3M porta up
		case 0x47: 
			if (ticker) {
				const mp_sint32 effidx = ((module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER)  && numEffects == 2) ? 1 : effcnt;				
				mp_ubyte* op = (module->header.flags & XModule::MODULE_ITLINKPORTAMEM) ? 
					&chnInf->old[effidx].portanote : &chnInf->old[effidx].portaup;				
				if (*op<=0xDF) {
					chnInf->decPer(*op*4);
					// Special for ST3
					if (chnInf->getPer() <= 0 && chn >= 0) 
						stopSample(chn);
					chnInf->adjustVibratoPer();
				}
			}
			break;
				
		// S3M porta down
		case 0x48: 
			if (ticker) {
				const mp_sint32 effidx = ((module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER)  && numEffects == 2) ? 1 : effcnt;
				mp_ubyte* op = (module->header.flags & XModule::MODULE_ITLINKPORTAMEM) ? 
					&chnInf->old[effidx].portanote : &chnInf->old[effidx].portaup;				
				if (*op<=0xDF) {
					chnInf->incPer(*op*4);
					chnInf->adjustVibratoPer();
				}
			}
			break;
				
		// S3M volslide
		case 0x49: 
		{
			doTickVolslideST(chnInf, effcnt);
			break;
		} 
			
		// fine vibrato
		case 0x4A:
		{
			x = chnInf->eop[effcnt]>>4;
			y = chnInf->eop[effcnt]&0xf;

			if (x) chnInf->vibspeed[effcnt]=x;
			if (y) chnInf->vibdepth[effcnt]=y;
		
			mp_sint32 vmp = chnInf->getPer();
			
			vm = calcVibrato(chnInf, effcnt, 7);
									
			vp = chnInf->vibpos[effcnt];
			
			if (ticker || (module->header.flags & XModule::MODULE_ITNEWEFFECTS)) 
				chnInf->vibpos[effcnt]+=chnInf->vibspeed[effcnt];

			vmp+=vm;

			if (chn >= 0)
				setFreq(chn,getFinalFreq(chnInf->chnstat(),getFinalPeriod(chnInf->chnstat(),vmp)));
			break;
		} 
			
		// high precision portamento up
		case 0x4D:
			if (ticker) {
				chnInf->decPer(chnInf->old[effcnt].portaup);
				handlePeriodUnderflow(chnInf);
				chnInf->adjustVibratoPer();
			}
			break;
			
		// high precision portamento down
		case 0x4E:
			if (ticker) {
				chnInf->incPer(chnInf->old[effcnt].portaup);
				handlePeriodOverflow(chnInf);
				chnInf->adjustVibratoPer();
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
				if (chnInf->getVenv().envstruc!=NULL) {
					if (!chnInf->getVenv().isEnabled()) 
						chnInf->setVol(0);
				}
				else 
					chnInf->setVol(0);

				chnInf->adjustTremoloTremorVol();
				
				chnInf->setKeyon(false);
			}
			break;

		// Oktalyzer arpeggio I, II, III
		case 0x56: 
		case 0x57: 
		case 0x58: 
		{
			if (chnInf->getNote())
			{
				mp_sint32 eff = chnInf->eff[effcnt]-0x56;
				mp_sint32 r;
				
				if (eff == 1)
					r = (ticker)&3;
				else 
					r = (ticker)%3;
					
				mp_sint32 note = 0,onote = chnInf->getNote();
				mp_sint32 relnote = chnInf->getRelnote();
				mp_sint32 finetune = chnInf->getFinetune();
				mp_sint32 per,nper;
				
				mp_ubyte eop = chnInf->eop[effcnt];
				
				mp_sint32 x = eop>>4;
				mp_sint32 y = eop&0xf;
				
				switch (eff)
				{
					case 0x00:
					{
						switch (r) {
							case 0 : note=chnInf->getNote()-x; break;
							case 1 : note=chnInf->getNote(); break;
							case 2 : note=chnInf->getNote()+y; break;
						}
						break;
					}
					
					case 0x01:
					{
						switch (r) {
							case 0 : note=chnInf->getNote(); break;
							case 1 : note=chnInf->getNote()+y; break;
							case 2 : note=chnInf->getNote(); break;
							case 3 : note=chnInf->getNote()-x; break;
						}
						break;
					}

					case 0x02:
					{
						switch (r) {
							case 0 : note=chnInf->getNote()+y; break;
							case 1 : note=chnInf->getNote()+y; break;
							case 2 : note=chnInf->getNote(); break;
						}
						break;
					}
				}
				
				nper=getperiod(note,relnote,finetune);
				per=getperiod(onote,relnote,finetune);
				
				nper-=per;
				nper+=chnInf->getPer();
				
				if (chn >= 0)
					setFreq(chn,getFinalFreq(chnInf->chnstat(),getFinalPeriod(chnInf->chnstat(),nper)));
			}
			break;
		}
		
		// Global volslide
		case 0x59: 
		{
			if (!(module->header.flags & XModule::MODULE_OLDS3MVOLSLIDES) && 
				ticker == 0)
				break;
			
			x = chnInf->old[effcnt].gvolslide>>4;
			y = chnInf->old[effcnt].gvolslide&0xf;
			
			if (x == 0xF && y) break;
			if (y == 0xF && x) break;
			
			if (x && y) y = 0;
			
			if (x) {
				// IT modules increment by 2, all others by 4
				mainVolume+=x*((module->header.flags & XModule::MODULE_ITNOTEOFF) ? 2 : 4);
				
				if (mainVolume>255) mainVolume=255;
			}
			if (y) {
				// IT modules decrement by 2, all others by 4
				mainVolume-=y*((module->header.flags & XModule::MODULE_ITNOTEOFF) ? 2 : 4);
				if (mainVolume<0) mainVolume=0;
			}
			break;
		} 
		// IT/S3M Channel volslide
		case 0x5A: 
		{
			if (!(module->header.flags & XModule::MODULE_OLDS3MVOLSLIDES) && 
				ticker == 0)
				break;
			
			x = chnInf->old[effcnt].chnvolslide>>4;
			y = chnInf->old[effcnt].chnvolslide&0xf;
			
			if (x == 0xF && y) break;
			if (y == 0xF && x) break;
			
			if (x && y) y = 0;
						
			if (x) {
				chnInf->incMasterVol(x*4);
			}
			if (y) {
				chnInf->decMasterVol(y*4);
			}
			break;
		} 
		// IT panning slide
		case 0x5B: 
		{
			if (!(module->header.flags & XModule::MODULE_OLDS3MVOLSLIDES) && 
				ticker == 0)
				break;
			
			x = chnInf->old[effcnt].panslide>>4;
			y = chnInf->old[effcnt].panslide&0xf;
			
			if (x == 0xF && y) break;
			if (y == 0xF && x) break;
			
			if (x && y) y = 0;
						
			if (x) {
				chnInf->decPan(x*4);
			}
			if (y) {
				chnInf->incPan(y*4);
			}
			break;
		} 
		
		// panbrello (Impulse Tracker only)
		case 0x5C: 
		{
			x = chnInf->eop[effcnt]>>4;
			y = chnInf->eop[effcnt]&0xf;
			if (x) chnInf->panbrellospeed[effcnt]=x;
			if (y) chnInf->panbrellodepth[effcnt]=y;
			
			vp = chnInf->panbrellopos[effcnt];
			vd = chnInf->panbrellodepth[effcnt];
			
			mp_sint32 vmp = chnInf->getPan();
			
			// IT in new effects mode processes at non row tick
			if (ticker || (module->header.flags & XModule::MODULE_ITNEWEFFECTS)) 
			{
				// IT in new effects mode is two times finer
				vm = (finesintab[vp&255]*vd) >> ((module->header.flags & XModule::MODULE_ITNEWEFFECTS) ? (3-1) : (3-2));
				vmp+=vm;
				if (vmp<0) vmp=0;
				if (vmp>255) vmp=255;
				chnInf->panbrellopos[effcnt]+=chnInf->panbrellospeed[effcnt];
			}
			
			if (chn >= 0)
				setPan(chn, getFinalPanning(chnInf->chnstat(), vmp));
			break;
		}
			
	}
}

void PlayerIT::doVolslidePT(TModuleChannel* chnInf, mp_sint32 effcnt, mp_ubyte eop)
{
	if (eop) chnInf->old[effcnt].volslide=eop;
}

void PlayerIT::doVolslideST(TModuleChannel* chnInf, mp_sint32 effcnt, mp_ubyte eop)
{
	if (eop) chnInf->old[effcnt].volslide=eop; 
	
	if (chnInf->old[effcnt].volslide) {
		mp_ubyte y=chnInf->old[effcnt].volslide>>4;
		mp_ubyte x=chnInf->old[effcnt].volslide&0xf;
		
		if ((x!=0x0F)&&(y!=0x0F)) return;
		if (x==0x0F && !y) return;
		if (y==0x0F && !x) return;
		
		if (x==0x0F)
		{
			chnInf->incVol(y*4);
			chnInf->adjustTremoloTremorVol();
			return;
		}
		if (y==0x0F)
		{
			chnInf->decVol(x*4);
			chnInf->adjustTremoloTremorVol();
			return;
		}
	}
}

void PlayerIT::doEffect(TModuleChannel* chnInf, mp_sint32 effcnt)
{
	const mp_sint32 chn = chnInf->getPlaybackChannelIndex();
	
	mp_ubyte x,y;
	mp_sint32 eop=chnInf->eop[effcnt];
	switch (chnInf->eff[effcnt]) {
		case 0x01 : if (eop) chnInf->old[effcnt].portaup=eop; break;
		case 0x02 : if (eop) chnInf->old[effcnt].portadown=eop; break;
		case 0x03 : if (module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER) 
					{
						ASSERT(numEffects >= 2);
						if (eop) chnInf->old[1].portanote=eop; 
					}
					else
					{
						if (eop) chnInf->old[effcnt].portanote=eop; 
					}
					break;
		case 0x05 :
		case 0x06 : {
						if (module->header.flags & XModule::MODULE_ST3DUALCOMMANDS) 
							doVolslideST(chnInf, effcnt, eop); 
						else
							doVolslidePT(chnInf, effcnt, eop); 
						break;
					}
		case 0x08 : if (options[PlayModeOptionPanning8xx]) chnInf->setPan(eop); break;
		case 0x09 : {
						if (eop) chnInf->old[effcnt].smpoffset = eop; 
						chnInf->smpoffs = chnInf->old[effcnt].smpoffset<<8;					
					}; 
					break;
		case 0x0A : doVolslidePT(chnInf, effcnt, eop); break;
		case 0x0B : {
						pjump = 1;
						pjumppos = eop;
						pjumprow = 0;
						pjumpPriority = MP_NUMEFFECTS*chnInf->channelIndex + effcnt;
					}; 
					break;
		case 0x0C : chnInf->setVol(eop); 
					chnInf->adjustTremoloTremorVol();
					chnInf->hasSetVolume = true;					
					break;
		case 0x0D : {
						pbreak=1;
						pbreakpos = (eop>>4)*10+(eop&0xf);
						if (pbreakpos > 63)
							pbreakpos = 0;
						pbreakPriority = MP_NUMEFFECTS*chnInf->channelIndex + effcnt;
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
						if (chnInf->getVenv().envstruc == NULL)
							break;
							
						bool bSet = false;
						
						TPrEnv* env = &chnInf->getVenv();
						
						for (mp_sint32 i = 0; i < env->envstruc->num-1; i++)
						{
							if (eop >= env->envstruc->env[i][0] &&
								eop < env->envstruc->env[i+1][0])
							{
								env->a = i;
								env->b = i+1;
								env->step = eop;
								
								bSet = true;
								break;
							}
						}
						
						if (!bSet)
						{
							// if position is beyond the last envelope point
							// we limit it to the last point and exit
							bool beyond = eop > env->envstruc->env[env->envstruc->num-1][0];														
							env->a = env->envstruc->num-1;
							env->b = env->envstruc->num;
							env->step = env->envstruc->env[env->envstruc->num-1][0];
							if (beyond)
								break;
						}
						
						// check if we set envelope position to a loop end point
						// in that case wrap to the loop start, otherwise the loop
						// end is skipped and the envelope will roll out without 
						// looping
						if ((env->envstruc->type & 4) && 
							env->step == env->envstruc->env[env->envstruc->loope][0])
						{
							env->a=env->envstruc->loops;
							env->b=env->envstruc->loops+1;
							env->step=env->envstruc->env[env->a][0];
						}												
						break;
					}
		// set BPM
		case 0x16 : {
						if (eop) {
							chnInf->old[effcnt].temposlide = eop; 
							if ((module->header.flags & XModule::MODULE_ITTEMPOSLIDE) && eop < 0x20)
								break;
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
		// MDL/IT Subcommands
		case 0x1E: 
		{
			mp_ubyte eff = chnInf->eop[effcnt] >> 4;
			mp_ubyte eop = chnInf->eop[effcnt] & 0xf;
			switch (eff) 
			{
				// past note actions/envelope trigger control etc.
				case 0x7:
				{
					switch (eop)
					{
						case 0x0:
						case 0x1:
						case 0x2:
							handlePastNoteAction(chnInf, eop);
							break;
						// set NNA to cut/continue/note off/note fade
						case 0x3:
						case 0x4:
						case 0x5:
						case 0x6:
							// clear out bits 4 and 5
							// shift new NNA value into the position
							chnInf->setInsflags((chnInf->getInsflags() & ~(3 << 4)) | ((eop-0x03) << 4));
							break;
						// turn off volume envelope
						case 0x7:
							chnInf->getVenv().setEnabled(false);
							break;
						// turn on volume envelope
						case 0x8:
							chnInf->getVenv().setEnabled(true);
							break;
						// turn off panning envelope
						case 0x9:
							chnInf->getPenv().setEnabled(false);
							break;
						// turn on panning envelope
						case 0xA:
							chnInf->getPenv().setEnabled(true);
							break;
						// turn off pitch envelope
						case 0xB:
							chnInf->getPitchenv().setEnabled(false);
							break;
						// turn on pitch envelope
						case 0xC:
							chnInf->getPitchenv().setEnabled(true);
							break;
					}
					break;
				}
				// set high sample offset
				case 0xF:
				{
					chnInf->smpoffshigh = (mp_uint32)eop << 16;
					break;
				}
			}
			break;
		}
		// MDL set sample offset
		case 0x1F : {
						chnInf->smpoffs=((mp_sint32)eop<<8)+((mp_sint32)chnInf->eop[(effcnt+1)%numEffects]<<16);
					}; break;
		case 0x20 : if (eop) chnInf->old[effcnt].arpeg=eop; break;
		// ULT set sample offset
		case 0x21 : {
						chnInf->smpoffs=((mp_sint32)eop<<10);
					}; break;
		// ULT Fine set sample offset
		case 0x22 : {
						mp_sint32 op = (((mp_sint32)eop)<<8) + ((mp_sint32)chnInf->eop[(effcnt+1)%numEffects]);
						chnInf->smpoffs=op<<2;
					}; break;					
		// ULT special commands
		case 0x23 : {
						if (((eop >> 4) == 1 || (eop&0xF) == 1) ||
							((eop >> 4) == 12 || (eop&0xF) == 12))
						{
							if (chn < 0)
								break;
							breakLoop(chn);
						}
						if ((eop >> 4) == 2 || (eop&0xF) == 2)
						{
							chnInf->resetFlag(CHANNEL_FLAGS_FORCE_FORWARD);
							chnInf->setFlag(CHANNEL_FLAGS_FORCE_BACKWARD);
							if (chn < 0)
								break;							
							setBackward(chn);
						}
					}; break;					
		// Far position jump (PLM support)
		case 0x2B : {
						pjump = 1;
						pjumppos = eop;
						pjumprow = chnInf->eop[(effcnt+1)%numEffects];
						pjumpPriority = MP_NUMEFFECTS*chnInf->channelIndex + effcnt;
					}; break;
		// Fine porta up
		case 0x31 : {
						if (eop) chnInf->old[effcnt].fineportaup=eop;
						chnInf->decPer(chnInf->old[effcnt].fineportaup*4);
						handlePeriodUnderflow(chnInf);
						chnInf->adjustVibratoPer();
					}; break;
		// Fine porta down
		case 0x32 : {
						if (eop) chnInf->old[effcnt].fineportadown=eop;
						chnInf->incPer(chnInf->old[effcnt].fineportadown*4);
						handlePeriodOverflow(chnInf);
						chnInf->adjustVibratoPer();
					}; break;
		case 0x36 : {
						mp_ubyte op = eop;
						
						// Imitate IT/ST3 behaviour
						// not only S60 can be the loop start point
						// if we jump back to the start row, ignore the argument of this S6x
						if (newInsST3Flag && (chnInf->loopstart==rowcnt) && chnInf->isLooping)
							op = 0;
		
						if (!op) {
							chnInf->execloop=0;
							chnInf->loopstart=rowcnt;
							chnInf->loopingValidPosition = poscnt;
						}
						else {
							if (chnInf->loopcounter==op) 
							{
								// Imitate nasty XM bug here:
								if (playModeFT2)
								{
									startNextRow = chnInf->loopstart;
								}
							
								RESETLOOPING
								
								// Imitate IT/ST3 behaviour
								// not only S60 can be the loop start point
								if (newInsST3Flag)
								{
									chnInf->execloop=0;
									chnInf->loopstart=rowcnt;
									chnInf->loopingValidPosition = poscnt;
								}
							}
							else {
								chnInf->execloop=1;
								chnInf->loopcounter++;
							}
						}
					}; break;
		case 0x38 : if (options[PlayModeOptionPanningE8x]) chnInf->setPan((mp_ubyte)XModule::pan15to255(eop)); break;
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
							chnInf->setKeyon(true);
							// trigger replay only when last note has been valid
							if (chnInf->validnote)
								playInstrument(chnInf);
						}						
					}; break;
		case 0x3A : {
						if (eop) chnInf->old[effcnt].finevolslide=eop;
						chnInf->incVol(chnInf->old[effcnt].finevolslide*4);
						chnInf->adjustTremoloTremorVol();
					}; break;
		case 0x3B : {
						if (eop) chnInf->old[effcnt].finevolslide=eop;
						chnInf->decVol(chnInf->old[effcnt].finevolslide*4);
						chnInf->adjustTremoloTremorVol();
					}; break;
		// Note delay triggers envelopes/autovibrato/fade out again
		case 0x3D : {
						triggerInstrumentFX(chnInf);
						chnInf->setKeyon(true);
					}; break;
		case 0x3E : {
						patDelay = true;
						patDelayCount = (mp_sint32)tickSpeed*((mp_sint32)eop+1);
					}; break;
		// Xtra fine porta up
		case 0x41 : {
						if (eop) chnInf->old[effcnt].xfineportaup=eop;
						chnInf->decPer(chnInf->old[effcnt].xfineportaup);
						handlePeriodUnderflow(chnInf);
						chnInf->adjustVibratoPer();
					}; break;
		case 0x42 : {
						if (eop) chnInf->old[effcnt].xfineportadown=eop;
						chnInf->incPer(chnInf->old[effcnt].xfineportadown);
						handlePeriodOverflow(chnInf);
						chnInf->adjustVibratoPer();
					}; break;
		// MDL fine portas up
		case 0x43 : {
						if (eop) chnInf->old[effcnt].portaup=eop; 
						if (chnInf->old[effcnt].portaup>=0xE0) {
							y=chnInf->old[effcnt].portaup>>4;
							x=chnInf->old[effcnt].portaup&0xf;
							switch (y) {
								case 0xF:
									chnInf->decPer(x*4);
									handlePeriodUnderflow(chnInf);
									chnInf->adjustVibratoPer();
									break;
								case 0xE: 
									chnInf->decPer(x>>1);
									handlePeriodUnderflow(chnInf);
									chnInf->adjustVibratoPer();
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
									chnInf->incPer(x*4);
									handlePeriodOverflow(chnInf);
									chnInf->adjustVibratoPer();
									break;
								case 0xE : 
									chnInf->incPer(x>>1);
									handlePeriodOverflow(chnInf);
									chnInf->adjustVibratoPer();
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
									chnInf->incVol(x*4);
									chnInf->adjustTremoloTremorVol();
									break;
								case 0xE : 
									chnInf->incVol(x);
									chnInf->adjustTremoloTremorVol();
									break;
							}
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
									chnInf->decVol(x*4);
									chnInf->adjustTremoloTremorVol();
									break;
								case 0xE : 
									chnInf->decVol(x);
									chnInf->adjustTremoloTremorVol();
									break;
							}
						}
					}; break;
		// S3M porta up
		case 0x47 : {
						// when MODULE_XMPORTANOTEBUFFER is set
						// we link all effects to the second effect memory
						const mp_sint32 effidx = ((module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER)  && numEffects == 2) ? 1 : effcnt;
						mp_ubyte* op = (module->header.flags & XModule::MODULE_ITLINKPORTAMEM) ? 
							&chnInf->old[effidx].portanote : &chnInf->old[effidx].portaup;				
						if (eop) *op=eop; 
						if (*op>=0xE0) {
							y=*op>>4;
							x=*op&0xf;
							switch (y) {
								case 0xF:
									chnInf->decPer(x*4);
									// Special for ST3
									if (chnInf->getPer() <= 0 && chn >= 0) 
										stopSample(chn);
									chnInf->adjustVibratoPer();
									break;
								case 0xE: 
									chnInf->decPer(x);
									// Special for ST3
									if (chnInf->getPer() <= 0 && chn >= 0) 
										stopSample(chn);
									chnInf->adjustVibratoPer();
									break;
							}
						}
					}; break;
		// S3M porta down
		case 0x48 : {
						// when MODULE_XMPORTANOTEBUFFER is set
						// we link all effects to the second effect memory
						const mp_sint32 effidx = ((module->header.flags & XModule::MODULE_XMPORTANOTEBUFFER)  && numEffects == 2) ? 1 : effcnt;
						mp_ubyte* op = (module->header.flags & XModule::MODULE_ITLINKPORTAMEM) ? 
							&chnInf->old[effidx].portanote : &chnInf->old[effidx].portaup;				
						if (eop) *op=eop; 
						if (*op>=0xE0) {
							y=*op>>4;
							x=*op&0xf;
							switch (y) {
								case 0xF : 
									chnInf->incPer(x*4);
									handlePeriodOverflow(chnInf);
									chnInf->adjustVibratoPer();
									break;
								case 0xE : 
									chnInf->incPer(x);
									handlePeriodOverflow(chnInf);
									chnInf->adjustVibratoPer();
									break;
							}
						}
					}; break;
		// S3M volslide
		case 0x49 : doVolslideST(chnInf, effcnt, eop); break;
		// extra fine volslide up (PSM support)
		case 0x4B : {
						if (eop) chnInf->old[effcnt].finevolslide=eop;
						chnInf->incVol(chnInf->old[effcnt].finevolslide);
						chnInf->adjustTremoloTremorVol();
					}; break;
		// extra fine volslide down (PSM support)
		case 0x4C : {
						if (eop) chnInf->old[effcnt].finevolslide=eop;
						chnInf->decVol(chnInf->old[effcnt].finevolslide);
						chnInf->adjustTremoloTremorVol();
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
						chnInf->resetFlag(CHANNEL_FLAGS_FORCE_BACKWARD);
						chnInf->setFlag(CHANNEL_FLAGS_FORCE_FORWARD);
						if (chn < 0)
							break;
						setForward(chn);
					}
					else if (eop == 1)
					{
						chnInf->resetFlag(CHANNEL_FLAGS_FORCE_FORWARD);
						chnInf->setFlag(CHANNEL_FLAGS_FORCE_BACKWARD);
						if (chn < 0)
							break;
						setBackward(chn);
					}
					else if (eop == 2)
					{
						chnInf->setFlag(CHANNEL_FLAGS_FORCE_BILOOP);
					}
					else if (eop == 3)  // break sampleloop / oktalyzer too
					{
						if (chn < 0)
							break;
						breakLoop(chn);
					}
					break;
		// AMS: Set channel mastervol
		case 0x50 : chnInf->setMasterVol(eop); break;
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
						if (chnInf->getNote() > eop)
						{
							mp_sint32 note = chnInf->getNote();
							note-=eop;
							if (note < 3*12)
								note = 3*12;
							chnInf->setNote(note);	
							chnInf->setPer(getperiod(chnInf->getNote(),chnInf->getRelnote(),chnInf->getFinetune()));
							chnInf->adjustVibratoPer();
						}
					}
					break;
		// Oktalyzer: fine note slide up
		case 0x55:  {
						if (chnInf->getNote() < XModule::NOTE_LAST-eop)
						{
							mp_sint32 note = chnInf->getNote();
							note+=eop;
							if (note > 6*12)
								note = 6*12;
							chnInf->setNote(note);	
							chnInf->setPer(getperiod(chnInf->getNote(),chnInf->getRelnote(),chnInf->getFinetune()));
							chnInf->adjustVibratoPer();
						}
					}		
					break;
		// IT/S3M global volslide (Impulse Tracker)
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
								mainVolume+=y*((module->header.flags & XModule::MODULE_ITNOTEOFF) ? 2 : 4);
								if (mainVolume>255) mainVolume=255;
								break;
							}
							if (y==0x0F)
							{
								mainVolume-=x*((module->header.flags & XModule::MODULE_ITNOTEOFF) ? 2 : 4);
								if (mainVolume<0) mainVolume=0;
								break;
							}
						}
						break;
					}
		// IT Channel volslide
		case 0x5A : {
						if (eop) chnInf->old[effcnt].chnvolslide=eop; 
						
						if (chnInf->old[effcnt].chnvolslide) {
							y=chnInf->old[effcnt].chnvolslide>>4;
							x=chnInf->old[effcnt].chnvolslide&0xf;
							
							if ((x!=0x0F)&&(y!=0x0F)) break;
							if (x==0x0F && !y) break;
							if (y==0x0F && !x) break;
							
							if (x==0x0F)
							{
								chnInf->incMasterVol(y*4);
								break;
							}
							if (y==0x0F)
							{
								chnInf->decMasterVol(x*4);
								break;
							}
						}
					}; break;
		// IT panning slide
		case 0x5B : {
						if (eop) chnInf->old[effcnt].panslide=eop; 
						
						if (chnInf->old[effcnt].panslide) {
							y=chnInf->old[effcnt].panslide>>4;
							x=chnInf->old[effcnt].panslide&0xf;
							
							if ((x!=0x0F)&&(y!=0x0F)) break;
							if (x==0x0F && !y) break;
							if (y==0x0F && !x) break;
							
							if (x==0x0F)
							{
								chnInf->decPan(y*4);
								break;
							}
							if (y==0x0F)
							{
								chnInf->incPan(x*4);
								break;
							}
						}
					}; break;
								
	} // switch
}

void PlayerIT::doTickeffects()
{
	TModuleChannel* chnInf = chninfo;
	for (mp_sint32 chn = 0; chn < numChannels; chn++, chnInf++) 
	{
		for (mp_sint32 effcnt = 0; effcnt < numEffects; effcnt++) 
		{
			doTickEffect(chnInf, effcnt);
		}
	}
}

void PlayerIT::triggerEnvelope(TPrEnv& dstEnv, TEnvelope& srcEnv)
{
	// if the same envelope has been not been assigned already,
	// we take over the "enabled" flag from the envelope
	if (dstEnv.envstruc != &srcEnv)
		dstEnv.enabled = srcEnv.type & 1; 
	dstEnv.envstruc = &srcEnv;
	dstEnv.a = 0;
	dstEnv.b = 1;
	dstEnv.step = 0;
	dstEnv.bpmCounter = 0;
	if (dstEnv.envstruc->speed)
		dstEnv.bpmAdder = getbpmrate(dstEnv.envstruc->speed);
}

void PlayerIT::triggerEnvelopes(TModuleChannel* chnInf)
{
	const mp_sint32 smp = chnInf->getSmp();
	const mp_sint32 ins = chnInf->getIns();

	bool insEnv = (module->instr[ins-1].flags & TXMInstrument::IF_ITENVELOPES);

	mp_uword e = insEnv ? module->instr[ins-1].venvnum : module->smp[smp].venvnum;
	if (e) 
		triggerEnvelope(chnInf->getVenv(), module->venvs[e-1]);
	else 
		chnInf->getVenv().envstruc=NULL;
	
	e = insEnv ? module->instr[ins-1].penvnum : module->smp[smp].penvnum;
	if (e) 
		triggerEnvelope(chnInf->getPenv(), module->penvs[e-1]);
	else 
		chnInf->getPenv().envstruc=NULL;
	
	e = insEnv ? module->instr[ins-1].fenvnum : module->smp[smp].fenvnum;
	if (e) 
		triggerEnvelope(chnInf->getFenv(), module->fenvs[e-1]);
	else 
		chnInf->getFenv().envstruc=NULL;
	
	e = insEnv ? module->instr[ins-1].vibenvnum : module->smp[smp].vibenvnum;
	if (e) 
		triggerEnvelope(chnInf->getVibenv(), module->vibenvs[e-1]);
	else 
		chnInf->getVibenv().envstruc=NULL;				

	e = insEnv ? module->instr[ins-1].pitchenvnum : module->smp[smp].pitchenvnum;
	if (e) 
		triggerEnvelope(chnInf->getPitchenv(), module->pitchenvs[e-1]);
	else 
		chnInf->getPitchenv().envstruc=NULL;				
}

void PlayerIT::triggerAutovibrato(TModuleChannel* chnInf)
{
	const mp_sint32 smp = chnInf->getSmp();

	if (module->smp[smp].vibdepth&&module->smp[smp].vibrate) 
	{
		//chnInf->avibused=1;
		chnInf->setAvibused((module->smp[smp].vibtype+1) | ((module->smp[smp].flags & 16) ? 128 : 0));
		chnInf->setAvibdepth(module->smp[smp].vibdepth);
		chnInf->setAvibspd(module->smp[smp].vibrate);
		chnInf->setAvibswcnt(0);
		chnInf->setAvibsweep(module->smp[smp].vibsweep);
	}
	else chnInf->setAvibused(0);
}

void PlayerIT::triggerInstrumentFX(TModuleChannel* chnInf, bool triggerEnv/* = true*/)
{
	const mp_sint32 smp = chnInf->getSmp();
	const mp_sint32 ins = chnInf->getIns();

	if (smp != -1)
	{
		if (triggerEnv)
			triggerEnvelopes(chnInf);				
		triggerAutovibrato(chnInf);
		
		chnInf->setFadevolstart(65536);
		// Check for IT style fadeout (instrument rather than sample based)
		if (ins && (module->instr[ins-1].flags & TXMInstrument::IF_ITFADEOUT))
			chnInf->setFadevolstep(module->instr[ins-1].volfade);		
		else
			chnInf->setFadevolstep(module->smp[smp].volfade);		
	}
}

#define INVALIDVALUE -12345678

void PlayerIT::progressRow()
{
	mp_sint32 slotsize = (numEffects*2)+2;

	TXMPattern* pattern = &module->phead[patternIndex];

	mp_ubyte *row = pattern->patternData+
						 (pattern->channum*slotsize*rowcnt);

	/*if (rowcnt == 3)
	{
		int i = 0;
		i++;
		i--;
	}*/
	
	//for (mp_sint32 chn=4;chn<5;chn++) {
	for (mp_sint32 chn=0;chn<numChannels;chn++) {
		
		if ((mp_sint32)attick[chn]==ticker && ticker < tickSpeed) {
			TModuleChannel *chnInf = &chninfo[chn];

			mp_sint32 pp = slotsize*chn;
			mp_sint32 note = chnInf->currentnote = row[pp];
			mp_sint32 i    = row[pp+1];

			bool noteporta = false;
			bool notedelay = false;
			bool forcefade = false;

			mp_sint32 oldIns = chnInf->getIns();
			mp_sint32 oldSmp = chnInf->getSmp();
			
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
					// nasty FT2 "feature"
					// if there is a set volume in the first column
					// and note == NOTE_OFF set a flag for later use
					case 0x0C:
					{
						if (playModeFT2 &&
							(note == XModule::NOTE_OFF || (effcnt == 0 && numEffects == 2 && chnInf->eff[effcnt+1] == 0x14)))
						{
							forcefade = true;
						}
						break;
					}
					// XM key off at tick with tick operand == 0 is like normal key off
					case 0x14:
						if (chnInf->eop[effcnt] == 0)
							note = XModule::NOTE_OFF;
						break;
					// set finetune will override the instrument setting
					case 0x35:
						finetune = XModule::modfinetunes[playModeFT2 ? ((chnInf->eop[effcnt] - 8) & 0xF) : (chnInf->eop[effcnt] & 0xF)];
						break;
					// note delay without note retriggers last bnote
					case 0x3d:
						notedelay = true;
						if (!note && chnInf->eop[effcnt] && playModeFT2)
							note = chnInf->lastnoportanote;
						break;
				}				
			}
			
			// Temporary placeholders, those will be applied after
			// having allocated a new virtual channel
			mp_sint32 finalNote			= chnInf->getNote();
			mp_sint32 finalIns			= chnInf->getIns();
			mp_uword  finalInsflags		= chnInf->getInsflags();
			mp_sint32 finalSmp			= chnInf->getSmp();
			mp_sint32 finalPeriod		= chnInf->getPer();
			mp_sint32 finalFreqAdjust	= chnInf->getFreqadjust();
			mp_sint32 finalRelnote		= chnInf->getRelnote();
			mp_sint32 finalFinetune		= chnInf->getFinetune();
			mp_sint32 finalVolume		= chnInf->getVolume();

			bool stopChannel			= false;

			// Check new instrument settings only if valid note or no note at all
			if (i && note <= XModule::NOTE_LAST) {
				// valid sample?
				bool invalidIns = true;
				bool invalidSmp = true;
				
				// invalid instrument
				if (i <= module->header.insnum && module->instr[i-1].samp)
					invalidIns = false;
				// invalid sample
				if (module->instr[i-1].samp && module->instr[i-1].snum[0] != -1)
					invalidSmp = false;
				
				if (!invalidIns) // valid sample
					finalIns = i;
				else if (note) // invalid sample
				{
					// cut means stop sample in FT2
					if (!newInsPTFlag && !newInsST3Flag && !noteporta)
					{
						finalSmp = -1;
						finalIns = 0;
						stopChannel = true;
					}
				}
				
				// protracker sample cut when invalid instrument is triggered
				if (newInsPTFlag)
				{
					if (!note)
					{
						if (invalidSmp)
						{
							finalSmp = -1;
							finalIns = 0;							
							finalVolume = 0; // cut means: volume to zero (no stop sample)
						}
						else
						{
							finalSmp = module->instr[i-1].snum[0];
						}
					}
					else
					{
						if (invalidSmp)
						{
							finalSmp = -1;
							finalIns = 0;							
							finalVolume = 0; // cut means: volume to zero (no stop sample)
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
							finalSmp = module->instr[i-1].snum[0];
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
			if (note && note < XModule::NOTE_OFF) 
			{
				const mp_sint32 ins = finalIns;
				if (ins) 
				{
					finalSmp = module->instr[ins-1].snum[note-1];
					if ((module->instr[ins-1].flags & 1) &&
						module->instr[ins-1].notemap[note-1] != 0xFF)
					{
						chnInf->currentnote = note = module->instr[ins-1].notemap[note-1] + 1;
					}
					
					// if Impulse Tracker instrument, we only apply the sample finetune / relnote when
					// there is a note AND an instrument set, otherwise keep settings
					if (!(module->instr[ins-1].flags & 1) || ((module->instr[ins-1].flags & 1) && i))
					{						
						// invalid sample entry?
						// Only apply new fintune / relative note number when not doing portamento
						mp_sint32 smp = finalSmp;
						if (smp != -1 && !noteporta) {
							mp_sint32 finalNote = note + (mp_sint32)module->smp[smp].relnote;
							// limit to upper boundary is enabled (FT2)
							if (module->header.uppernotebound)
							{
								// Within current note range?
								if (finalNote >= 1 && finalNote <= (mp_sint32)module->header.uppernotebound)
								{
									finalFinetune = (finetune != 0x7FFFFFFF ? finetune : module->smp[smp].finetune);
									finalRelnote = module->smp[smp].relnote;
									finalFreqAdjust = module->smp[smp].freqadjust;
								}
								// This is not a valid note
								else 
								{
									chnInf->validnote = false;
									note = finalNote;
								}
							}
							else
							{
								finalFinetune = (finetune != 0x7FFFFFFF ? finetune : module->smp[smp].finetune);
								finalRelnote = module->smp[smp].relnote;
								finalFreqAdjust = module->smp[smp].freqadjust;
							}
							
						}
					}
				}
				
				mp_sint32 relnote = finalRelnote;
				mp_sint32 finetune = finalFinetune;
				
				// If this is not a note portamento
				// and a valid note => keep that note and calculate new period
				if (!noteporta) {
					finalNote = chnInf->lastnoportanote = note;
					finalPeriod = getperiod(note,relnote,finetune);
					// if there is a valid note => destroy portamento to note memory when playing an S3M(?)
					if (/*newInsPTFlag||*/newInsST3Flag)
					{
						chnInf->destnote=0;
						chnInf->destper=0;
					}
				}
				// If this is a note portamento keep destination's note + period
				else  {
					// if a note delay is happening while the portamento is set, AND we don't have a note (?)
					// we restore the original period, but the destination period keeps set
					if (playModeFT2 && notedelay && !chnInf->currentnote)
						finalPeriod=getperiod(note,relnote,finetune);
					else
					{
						chnInf->lastnoportanote=finalNote;
						chnInf->destnote=finalNote=note;
						chnInf->destper=getperiod(note,relnote,finetune);
					}
				}
				
				// If this has not been a valid note, do not trigger it
				if (!chnInf->validnote)
					note = 0;
			}
			
			// take over instrument flags (NNA/DCT/DCA)
			if ((i == finalIns) && finalIns)
				finalInsflags = module->instr[finalIns-1].flags;
			
			// --- this is the place to allocate new virtual channels ---
			if (finalSmp != -1 && !noteporta &&
				note && note < XModule::NOTE_OFF) 
			{
				TNNATriggerInfo triggerInfo;
				triggerInfo.ins = i;
				triggerInfo.smp = finalSmp;
				triggerInfo.note = note;
				if (!handleNNAs(chnInf, triggerInfo))
					continue;
			}
			
			// apply new state to new channel
			if (finalNote != chnInf->getNote())
			{
				chnInf->setNote(finalNote);
			}
			if (finalIns != chnInf->getIns())
			{
				chnInf->setIns(finalIns);
			}
			if (finalInsflags != chnInf->getInsflags())
			{
				chnInf->setInsflags(finalInsflags);
			}
			if (finalSmp != chnInf->getSmp())
			{
				chnInf->setSmp(finalSmp);
			}
			if (finalPeriod != chnInf->getPer())
			{
				chnInf->setPer(finalPeriod);
				chnInf->adjustVibratoPer();
			}
			if (finalFreqAdjust	!= chnInf->getFreqadjust())
			{
				chnInf->setFreqadjust(finalFreqAdjust);
			}
			if (finalRelnote != chnInf->getRelnote())
			{
				chnInf->setRelnote(finalRelnote);
			}
			if (finalFinetune != chnInf->getFinetune())
			{
				chnInf->setFinetune(finalFinetune);
			}
			if (finalVolume != chnInf->getVol())
			{
				chnInf->setVol(finalVolume);
				chnInf->adjustTremoloTremorVol();
			}
			if (stopChannel && chnInf->hasVchn())
			{
				stopSample(chnInf->getPlaybackChannelIndex());
			}
			
			// man this FT2 bug emulation starts getting on my nerves:
			// only take new instrument of there is no note porta
			if (playModeFT2 && i &&
				(noteporta || !chnInf->validnote))
			{
				i = oldIns;
				chnInf->setIns(i);
				chnInf->setSmp(oldSmp);
			}

			// when we have a new instrument we apply the settings for this instrument
			if (i && chnInf->getSmp() != -1 && note < XModule::NOTE_OFF) 
			{				
				if (module->instr[i-1].flags & TXMInstrument::IF_ITGOBALINSVOL)
					chnInf->setInsMasterVol(module->instr[i-1].res);
				else
					chnInf->setInsMasterVol(255);
				
				mp_sint32 smp = chnInf->getSmp();
				
				if ((module->smp[smp].flags&1)) 
				{
					chnInf->setVol(module->smp[smp].vol);
					chnInf->adjustTremoloTremorVol();
				}
				if (playModeFT2 &&
					(module->smp[smp].flags&2)) 
					chnInf->setPan(module->smp[smp].pan);	
				if ((module->smp[smp].flags&4)) 
					chnInf->setMasterVol(module->smp[smp].vol);
				if ((module->smp[smp].flags&8)) 
					chnInf->setSmpMasterVol(module->smp[smp].res);
				else
					chnInf->setSmpMasterVol(255);
					
				chnInf->setCutoff(module->instr[i-1].ifc);
				chnInf->setResonance(module->instr[i-1].ifr);
				
				if (noteporta && (module->header.flags & XModule::MODULE_ITNEWGXX))
					triggerInstrumentFX(chnInf, false);
				else
					triggerInstrumentFX(chnInf);
					
				// reset vibrato/tremolo/tremor/retrig counters
				for (effcnt=0;effcnt<numEffects;effcnt++) 
					chnInf->vibpos[effcnt] = chnInf->tremorcnt[effcnt] = chnInf->trmpos[effcnt] = chnInf->panbrellopos[effcnt] = chnInf->retrigcounterRxx[effcnt] = 0;
					
				if (playModePT)
					chnInf->smpoffs = 0;
					
				chnInf->setKeyon(true);
			}
			
			// ------ 11/05/05: it seems that note off commands are processed BEFORE effect commands
			// S3M style keyoff:
			// sample is stopped
			if (note == XModule::NOTE_CUT) {
				note=0;
				if (chnInf->getVenv().envstruc!=NULL) {
					if (!chnInf->getVenv().isEnabled())
					{
						chnInf->setVol(0);
						chnInf->adjustTremoloTremorVol();
						if (chnInf->getPlaybackChannelIndex() >= 0)
							stopSample(chnInf->getPlaybackChannelIndex());
					}
				}
				else {
					chnInf->setVol(0);
					chnInf->adjustTremoloTremorVol();
					if (chnInf->getPlaybackChannelIndex() >= 0)
						stopSample(chnInf->getPlaybackChannelIndex());
				}
			}
			// XM/IT style keyoff:
			else if (note == XModule::NOTE_OFF) 
			{
				note = 0;		
				handleNoteOFF(chnInf->chnstat());
			}
			
			chnInf->hasSetVolume = false;
			for (effcnt=0;effcnt<numEffects;effcnt++) {	
				// MTM hack
				// sample offset without note seems to trigger last note
				if (chnInf->eff[effcnt] == 0x09 && !note && module->getType() == XModule::ModuleType_MTM)
				{
					note = chnInf->getNote();
				}			
				doEffect(chnInf, effcnt);
			} // for
	
			if (note) 
			{
				if (note <= XModule::NOTE_OFF)
				{
					if (!noteporta) 
					{
						playInstrument(chnInf);										
					}
					else if (oldPTInsChangeFlag && 
							 newInsPTFlag && 
							 noteporta && 
							 i && 
							 chnInf->getSmp() != -1 && 
							 chnInf->getNote()) 
					{						
						playInstrument(chnInf, true);																	
					}
					
				}
				
			} // note
			else if (oldPTInsChangeFlag && 
					 newInsPTFlag && 
					 i && 
					 chnInf->getNote() && 
					 chnInf->getPer()) 
			{
				playInstrument(chnInf, true);					
			}
			
		}
	
	}
 
}



void PlayerIT::update()
{
	mp_sint32 c;
	
	TVirtualChannel* chn = vchninfo;
	const mp_sint32 curMaxVirChannels = this->curMaxVirChannels;
	for (c = 0; c < curMaxVirChannels; c++, chn++) 
	{
		if (!chn->getActive())
			continue;

		if (chn->isFlagSet(CHANNEL_FLAGS_UPDATE_IGNORE))
			continue;

		const mp_sint32 ins = chn->getIns();
		bool ITEnvelopes = (ins && ins <= module->header.insnum) ? (module->instr[ins-1].flags & TXMInstrument::IF_ITENVELOPES) : false;

		mp_sint32 dfs = chn->getFlags() & CHANNEL_FLAGS_DFS;
		mp_sint32 dvs = chn->getFlags() & CHANNEL_FLAGS_DVS;
		mp_sint32 dps = chn->getFlags() & CHANNEL_FLAGS_DPS;

		if (chn->getPeriod() && !dfs) 
			setFreq(c,getFinalFreq(chn->chnstat(),getFinalPeriod(chn->chnstat(),chn->getPeriod())));
		
		if (!dvs) 
			setVol(c,getFinalVolume(chn->chnstat(), chn->getVolume(), mainVolume));

		if (!dps)
			setPan(c,getFinalPanning(chn->chnstat(),chn->getPan()));

		if (chn->getVenv().envstruc != NULL &&
			!chn->getVenv().envstruc->speed)
		{
			prenvelope(&chn->getVenv(), chn->getKeyon(), ITEnvelopes);
			if (ins && ins <= module->header.insnum && (module->instr[ins-1].flags & TXMInstrument::IF_ITFADEOUT))				
			{
				if (chn->getVenv().finished(chn->getKeyon()) &&
					!chn->getFadeout())
				{
					chn->setFadeout(true);
				}
			}
		}

		// IT filter processing
		mp_sint32 cutoff = MP_INVALID_VALUE;
		if (chn->getCutoff() >= 128)
			cutoff = chn->getCutoff() - 128;

		mp_sint32 resonance = 0;
		if (chn->getResonance() >= 128)
			resonance = chn->getResonance() - 128;
		
		setFilterAttributes(c, getFinalCutoff(chn->chnstat(), cutoff), resonance);
		
		if (chn->getPenv().envstruc != NULL &&
			!chn->getPenv().envstruc->speed)
		{
			prenvelope(&chn->getPenv(), chn->getKeyon(), ITEnvelopes);
		}

		if (chn->getFenv().envstruc != NULL &&
			!chn->getFenv().envstruc->speed)
		{
			prenvelope(&chn->getFenv(), chn->getKeyon(), ITEnvelopes);
		}
		
		if (chn->getVibenv().envstruc != NULL &&
			!chn->getVibenv().envstruc->speed)
		{
			prenvelope(&chn->getVibenv(), chn->getKeyon(), ITEnvelopes);
		}

		if (chn->getPitchenv().envstruc != NULL &&
			!chn->getPitchenv().envstruc->speed)
		{
			prenvelope(&chn->getPitchenv(), chn->getKeyon(), ITEnvelopes);
		}

		if (ins && ins <= module->header.insnum)
		{
			// IT style fadeout also works without active envelope
			if ((module->instr[ins-1].flags & TXMInstrument::IF_ITFADEOUT) &&
				chn->getFadeout())
			{
				chn->decFadevolstart();
			}
			// XM style fadeout works only with key off
			else if (!chn->getKeyon())
			{
				chn->decFadevolstart();
			}
		}
			
		if (chn->getAvibused()) 
		{
			chn->avibAdvance();
		}
	}

	adjustVirtualChannels();

}

void PlayerIT::updateBPMIndependent()
{
	mp_int64 dummy;

	TVirtualChannel* chn = vchninfo;
	const mp_sint32 curMaxVirChannels = this->curMaxVirChannels;
	for (mp_sint32 c = 0; c < curMaxVirChannels; c++,chn++) 
	{
		TVirtualChannel* chn = &vchninfo[c];

		if (!chn->getActive())
			continue;
		
		if (chn->isFlagSet(CHANNEL_FLAGS_UPDATE_IGNORE))
			continue;

		const mp_sint32 ins = chn->getIns();
		bool ITEnvelopes = (ins && ins <= module->header.insnum) ? (module->instr[ins-1].flags & TXMInstrument::IF_ITENVELOPES) : false;

		mp_sint32 dfs = chn->getFlags() & CHANNEL_FLAGS_DFS, dvs = chn->getFlags() & CHANNEL_FLAGS_DVS;

		// Volume envelope
		if (chn->getVenv().envstruc != NULL &&
			chn->getVenv().envstruc->speed)
		{
			dummy = (mp_int64)chn->getVenv().bpmCounter;
			dummy+=(mp_int64)chn->getVenv().bpmAdder;
			chn->getVenv().bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(&chn->getVenv(), chn->getKeyon(), ITEnvelopes);
				if (!dvs) 
					setVol(c,getFinalVolume(chn->chnstat(), chn->getVolume(), mainVolume));
			}
		}
		
		// Panning envelope
		if (chn->getPenv().envstruc != NULL &&
			chn->getPenv().envstruc->speed)
		{
			dummy = (mp_int64)chn->getPenv().bpmCounter;
			dummy+=(mp_int64)chn->getPenv().bpmAdder;
			chn->getPenv().bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(&chn->getPenv(), chn->getKeyon(), ITEnvelopes);
				setPan(c,getFinalPanning(chn->chnstat(),chn->getPan()));
			}
		}

		// Frequency envelope: Digitracker MDL
		if (chn->getFenv().envstruc != NULL &&
			chn->getFenv().envstruc->speed)
		{
			dummy = (mp_int64)chn->getFenv().bpmCounter;
			dummy+=(mp_int64)chn->getFenv().bpmAdder;
			chn->getFenv().bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(&chn->getFenv(), chn->getKeyon(), ITEnvelopes);
				if (chn->getPeriod()&&(!dfs)) 
					setFreq(c,getFinalFreq(chn->chnstat(),getFinalPeriod(chn->chnstat(),chn->getPeriod())));
			}
		}

		// Vibrato envelope: Velvet Studio AMS
		if (chn->getVibenv().envstruc != NULL &&
			chn->getVibenv().envstruc->speed)
		{
			dummy = (mp_int64)chn->getVibenv().bpmCounter;
			dummy+=(mp_int64)chn->getVibenv().bpmAdder;
			chn->getVibenv().bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(&chn->getVibenv(), chn->getKeyon(), ITEnvelopes);
				if (chn->getPeriod()&&(!dfs)) 
					setFreq(c,getFinalFreq(chn->chnstat(),getFinalPeriod(chn->chnstat(),chn->getPeriod())));
			}
		}

		// Pitch envelope: Impulse Tracker
		if (chn->getPitchenv().envstruc != NULL &&
			chn->getPitchenv().envstruc->speed)
		{
			dummy = (mp_int64)chn->getPitchenv().bpmCounter;
			dummy+=(mp_int64)chn->getPitchenv().bpmAdder;
			chn->getPitchenv().bpmCounter = (mp_sint32)dummy;
			// check overflow-carry 
			if (dummy>>32) 
			{
				prenvelope(&chn->getPitchenv(), chn->getKeyon(), ITEnvelopes);
				if (chn->getPeriod()&&(!dfs)) 
					setFreq(c,getFinalFreq(chn->chnstat(),getFinalPeriod(chn->chnstat(),chn->getPeriod())));
			}
		}

	}

	adjustVirtualChannels();

}

void inline PlayerIT::setNewPosition(mp_sint32 poscnt)
{
	if (poscnt == this->poscnt)
		return;

	if (poscnt>=module->header.ordnum) 
		poscnt=module->header.restart;						
	
	// reset looping flags
	RESET_ALL_LOOPING	

	lastUnvisitedPos = this->poscnt;
	
	this->poscnt = poscnt;
}

void PlayerIT::tickhandler()
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
			{
				for (c=0;c<numChannels;c++) 
					chninfo[c].smpoffs = 0;	
			}
			// noteslot will be processed at tick 0   
			memset(attick,0,sizeof(mp_ubyte)*numChannels);
			
			// search for note delays
			mp_sint32 slotsize = (numEffects*2)+2;
			
			mp_ubyte *row = pattern->patternData+(pattern->channum*slotsize*rowcnt);
			
			// process high priority effects in advance to other effects
			mp_ubyte* slot = row;
			
			for (c=0;c<numModuleChannels;c++)
				chninfo[c].channelIndex = c;

			for (c=0;c<numChannels;c++) 
			{				
				chninfo[c].setFlags(0);	
				
				for (mp_sint32 effcnt=0;effcnt<numEffects;effcnt++) 
				{										
					chninfo[c].eff[effcnt] = 0;
					chninfo[c].eop[effcnt] = 0;

					if (slot[2+(effcnt*2)] == 0x04) chninfo[c].setFlag(CHANNEL_FLAGS_DFS);
					else if (slot[2+(effcnt*2)] == 0x4A) chninfo[c].setFlag(CHANNEL_FLAGS_DFS);
					else if (slot[2+(effcnt*2)] == 0x06) chninfo[c].setFlag(CHANNEL_FLAGS_DFS);
					else if (slot[2+(effcnt*2)] == 0x20) chninfo[c].setFlag(CHANNEL_FLAGS_DFS); // normal arpeggio
					else if (slot[2+(effcnt*2)] == 0x56) chninfo[c].setFlag(CHANNEL_FLAGS_DFS); // oktalyzer arpeggio I
					else if (slot[2+(effcnt*2)] == 0x57) chninfo[c].setFlag(CHANNEL_FLAGS_DFS); // oktalyzer arpeggio II
					else if (slot[2+(effcnt*2)] == 0x58) chninfo[c].setFlag(CHANNEL_FLAGS_DFS); // oktalyzer arpeggio III
					else if (slot[2+(effcnt*2)] == 0x07) chninfo[c].setFlag(CHANNEL_FLAGS_DVS); // Tremolo
					else if (slot[2+(effcnt*2)] == 0x1D) chninfo[c].setFlag(CHANNEL_FLAGS_DVS); // Tremor
					else if (slot[2+(effcnt*2)] == 0x5C) chninfo[c].setFlag(CHANNEL_FLAGS_DPS); // Panbrello
					
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

void PlayerIT::halt()
{
	halted = true;
	BPMCounter = adder = 0;
	if (resetOnStopFlag)
		resetChannelsWithoutMuting();
}

bool PlayerIT::grabChannelInfo(mp_sint32 chn, TPlayerChannelInfo& channelInfo) const
{
	channelInfo.note = chninfo[chn].currentnote;
	channelInfo.instrument = chninfo[chn].getIns();
	channelInfo.volume = chninfo[chn].getVol();
	channelInfo.panning = chninfo[chn].getPan();
	channelInfo.numeffects = numEffects;
	memcpy(channelInfo.effects, chninfo[chn].eff, sizeof(chninfo[chn].eff));
	memcpy(channelInfo.operands, chninfo[chn].eop, sizeof(chninfo[chn].eop));
	return true;
}
