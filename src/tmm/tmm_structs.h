#ifndef TMM_PRESET_H
#define TMM_PRESET_H

enum
{
	TMM_TYPE_NONE = 0,
	TMM_TYPE_ADX,
	TMM_TYPE_NOISE,
	TMM_TYPE_SINE,
	TMM_TYPE_PULSE,
	TMM_TYPE_ADDITIVE
};

enum
{
	TMM_NOISETYPE_WHITE = 0,
	TMM_NOISETYPE_PINK,
	TMM_NOISETYPE_BROWN
};

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
	unsigned short  bwscale;
	unsigned char   usescale;
	unsigned char   harmonics[64];
	unsigned char   phasenoisetype;
	unsigned char   destroyer;
} TTMMAdditive;

typedef struct TTMMSettings_s
{
	unsigned char   type;
	TTMMNoise       noise;
	TTMMSine        sine;
	TTMMPulse       pulse;
	TTMMAdditive    additive;
} TTMMSettings;

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

#ifdef __cplusplus
extern "C" {
#endif
int tmm_generate_samples(int rate, int bits, TTMMSettings * p_settings, void * p_samples, int p_size);
int tmm_convert_to_mod(int rate, int bits, void * in, unsigned int sin, void ** out, unsigned int * sout);
#ifdef __cplusplus
}
#endif

#endif /* TMM_PRESET_H */
