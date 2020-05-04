#ifndef TMM_H
#define TMM_H

#include "kiss_fft.h"
#include "tmm_preset.h"

class TMM
{
private:
	class Noise
	{
	private:
		unsigned long m_seed;
		unsigned long m_count;
		unsigned long m_white;
		double        m_pink;
		double        m_brown;
		double        m_pinkStore[16];
	public:
		inline int   CTZ(int p_num);
		void         Seed(unsigned long p_seed = 0);
		double       White(double p_scale = 0.5f);
		double       Pink(void);
		double       Brown(void);

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
    int       m_bits;
public:
	class Converter
	{

	public:
		Converter();
		~Converter();
	};

	void ADXInflate();
	void ADXDeflate();
	int  GenerateSamples(TTMMSettings*, short*, int);
    int  ConvertToMOD(void * in, unsigned int sin, void ** out, unsigned int * sout);

	TMM(int samplerate, int bits);
	~TMM();
};

#endif /* TMM_H */
