#ifndef TMM_H
#define TMM_H

#include "kiss_fft.h"

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

struct TTMMNoise
{
	unsigned char type;
};

struct TTMMSine
{
	unsigned short basefreq;
};

struct TTMMPulse
{
	unsigned short basefreq;
	char width;
};

struct TTMMAdditive
{
	unsigned char  nharmonics;
	unsigned short basefreq;
	unsigned char  bandwidth;
	unsigned short bwscale;
	unsigned char  usescale;
	unsigned char  harmonics[64];
	unsigned char  phasenoisetype;
	unsigned char  destroyer;
};

struct TTMMSettings
{
	unsigned char type;
	TTMMNoise     noise;
	TTMMSine      sine;
	TTMMPulse     pulse;
	TTMMAdditive  additive;
};

class TMM
{
private:
	class Noise
	{
	private:
		unsigned long m_seed;
		unsigned long m_count;
		unsigned long m_white;
		float         m_pink;
		float         m_brown;
		float         m_pinkStore[16];
	public:
		inline int   CTZ(int p_num);
		void         Seed(unsigned long p_seed = 0);
		float        White(float p_scale = 0.5f);
		float        Pink(void);
		float        Brown(void);

		Noise();
		~Noise();
	};

	class Additive
	{
	private:
		kiss_fft_cfg  m_ifft;
		kiss_fft_cpx* m_ifft_in;
		kiss_fft_cpx* m_ifft_out;
		double*       m_freq_amp;
		double*       m_freq_phase;
		double*       m_samples;
		int           m_samplerate;
		int           m_bins;
		TMM::Noise*   m_noise;

		inline double Profile(double p_fi, double p_bwi);
		inline double RelativeFreq(double p_freq);
		inline void   Normalize(TTMMAdditive*);
		inline void   InverseFFT(TTMMAdditive*);
	public:
		double*       Process(TTMMAdditive*);

		Additive(int p_bins, int p_samplerate);
		~Additive();
	};

	Noise*    m_noise;
	Additive* m_additive;
	int       m_samplerate;
public:
	void ADXInflate();
	void ADXDeflate();
	int  GenerateSamples(TTMMSettings*, short*, int);

	TMM(int p_samplerate);
	~TMM();
};

#endif /* TMM_H */
