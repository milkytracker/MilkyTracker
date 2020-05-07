#ifndef TMM_H
#define TMM_H

#include "kiss_fft.h"
#include "tmm_structs.h"

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
		void         Reset(void);

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

	Noise*      m_noise;
	Additive*   m_additive;
	int         m_samplerate;
    int         m_bits;

    int         GenerateSamples8(TTMMSettings*, char *, int);
	int         GenerateSamples16(TTMMSettings*, short *, int);
    void        MoveToZeroCrossing8(TTMMSettings*, char *, int);
    void        MoveToZeroCrossing16(TTMMSettings*, short *, int);
public:
	class Converter
	{
    private:
        ::TMM         * m_tmm;
        void          * m_tmm_data;
        unsigned int    m_tmm_size;
        void         ** m_pmod_data;
        unsigned int  * m_pmod_size;
	public:
        void            SetTMMSource(void *, unsigned int);
        void            SetMODDest(void **, unsigned int *);

        bool            IsSourceValid() const;
        int             Convert();

		Converter(::TMM * tmm);
		~Converter();
	};

	void ADXInflate();
	void ADXDeflate();
	int  GenerateSamples(TTMMSettings*, void *, int);
    int  ConvertToMOD(void * in, unsigned int sin, void ** out, unsigned int * sout);

	TMM(int samplerate, int bits);
	~TMM();
};

#endif /* TMM_H */
