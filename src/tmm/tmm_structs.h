#ifndef TMM_PRESET_H
#define TMM_PRESET_H

#define TMM_EXT_MAGIC   0x7171
#define TMM_EXT_VERSION 2

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

#pragma pack(push, 1)

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

#pragma pack(pop)

typedef struct TTMMNoise_s
{
	unsigned char   type;
} TTMMNoise;

typedef struct TTMMSine_s
{
	unsigned short  basefreq;
} TTMMSine;

typedef struct TTMMPulse_s
{
	unsigned short  basefreq;
	char width;
} TTMMPulse;

typedef struct TTMMAdditive_s
{
	unsigned char   nharmonics;
	unsigned short  basefreq;
	unsigned char   bandwidth;
	unsigned char   detune;
	unsigned short  bwscale;
	unsigned short  rndseed;
	unsigned char   lpfreq;
	unsigned char   hpfreq;
	unsigned char   usescale;
	unsigned char   harmonics[64];
	unsigned char   phasenoisetype;
	unsigned char   destroyer;
	unsigned short  envatt;
	unsigned short  envdec;
	unsigned short  envsus;
	unsigned short  envhold;
	unsigned short  envrel;
} TTMMAdditive;

#pragma pack(push, 1)

typedef struct TTMMExtensions_s {
    unsigned short  magic;
    unsigned char   ver;
    unsigned short  flags;
} TTMMExtensions;

#pragma pack(pop)

typedef struct TTMMSettings_s
{
	unsigned char   type;
    unsigned short  flags;

	TTMMNoise       noise;
	TTMMSine        sine;
	TTMMPulse       pulse;
	TTMMAdditive    additive;

    TTMMExtensions  extensions;
} TTMMSettings;

#ifdef __cplusplus
extern "C" {
#endif

int tmm_generate_samples(int rate, int bits, TTMMSettings * p_settings, void * p_samples, int p_size);
int tmm_convert_to_mod(int rate, int bits, void * in, unsigned int sin, void ** out, unsigned int * sout);

#ifdef __cplusplus
}
#endif

#endif /* TMM_PRESET_H */
