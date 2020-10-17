#ifndef TMM_PRESET_H
#define TMM_PRESET_H

#define TMM_EXT_MAGIC   0x7171
#define TMM_EXT_VERSION 3

enum
{
	TMM_TYPE_NONE = 0,
	TMM_TYPE_ADX,
	TMM_TYPE_NOISE,
	TMM_TYPE_SINE,
	TMM_TYPE_PULSE,
	TMM_TYPE_ADDITIVE
};

#define TMM_FLAG_LOOP_FWD   (1 << 0)
#define TMM_FLAG_FIX_ZERO   (1 << 1)

enum
{
	TMM_NOISETYPE_WHITE = 0,
	TMM_NOISETYPE_PINK,
	TMM_NOISETYPE_BROWN
};

//
// Old format for additive patches
// DEPRECATED
//
typedef struct TTMMAdditive_s
{
	unsigned char   	nharmonics;
	// pad 00
	unsigned short  	basefreq;
	unsigned char   	bandwidth;
	unsigned char   	detune;
	unsigned short  	bwscale;
	unsigned short  	rndseed;
	unsigned char   	lpfreq;
	unsigned char   	hpfreq;
	unsigned char   	usescale;
	// pad 00
	unsigned char   	harmonics[64];
	unsigned char   	phasenoisetype;
	unsigned char   	destroyer;
	unsigned short  	envatt;
	unsigned short  	envdec;
	unsigned short  	envsus;
	unsigned short  	envhold;
	unsigned short  	envrel;
} TTMMAdditive;

#pragma pack(push, 1)

//
// Used in tmm_convert ONLY
//
typedef struct TTMMModuleSample_s
{
    char                name[22];
    unsigned short      len;
    unsigned char       tune;
    unsigned char       vol;
    unsigned short      loopstart;
    unsigned short      looplen;
    unsigned char       tmm_type;
} TTMMModuleSample;

typedef struct TTMMNoise_s
{
	unsigned char   	type;
    unsigned char   	reserved[15];
} TTMMNoise;

typedef struct TTMMSine_s
{
	unsigned short  	basefreq;
    unsigned char   	reserved[14];
} TTMMSine;

typedef struct TTMMPulse_s
{
	unsigned short  	basefreq;
	char            	width;
    unsigned char   	reserved[13];
} TTMMPulse;

typedef struct TTMMExtensions_s {
    unsigned short  	magic;
    unsigned char   	ver;
    unsigned short  	flags;
} TTMMExtensions;

typedef struct TTMMAdditive2_s
{
	// 0x00 Resynthesis section
	unsigned char   	nharmonics;
	unsigned short  	basefreq;
	unsigned char   	bandwidth;
	unsigned char   	detune;
	unsigned short  	rndseed;
	unsigned char   	phasenoisetype;
	unsigned char   	destroyer;
	unsigned char   	usescale;
	unsigned short  	bwscale;
	unsigned char   	harmonics[64];
	unsigned char   	reserved1[20];

	// 0x60 POST-Filter
	unsigned char   	usefilters;
	unsigned char   	lpfreq;
	unsigned char   	hpfreq;
	unsigned char   	reserved2[13];

	// 0x70 Envelope
	unsigned char   	useenv;
	unsigned short  	envatt;
	unsigned short  	envdec;
	unsigned short  	envsus;
	unsigned short  	envhold;
	unsigned short  	envrel;
	unsigned char   	reserved3[21];

	// 0x90 Distortion
	//unsigned char   usedist;
} TTMMAdditive2;

//
// Structure is only used intermediate and is not written or read from on disk!!!
//
typedef struct TTMMSettings_s
{
    TTMMExtensions  	extensions;

	unsigned char   	type;

	TTMMNoise       	noise;
	TTMMSine        	sine;
	TTMMPulse       	pulse;
	TTMMAdditive2   	additive;
} TTMMSettings;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

int tmm_generate_samples(int rate, int bits, TTMMSettings * p_settings, void * p_samples, int p_size);
int tmm_convert_to_mod(int rate, int bits, void * in, unsigned int sin, void ** out, unsigned int * sout);

#ifdef __cplusplus
}
#endif

#endif /* TMM_PRESET_H */
