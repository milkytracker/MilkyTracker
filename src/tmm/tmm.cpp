#include "globals.h"
#include "tmm.h"


TMM::TMM(int p_samplerate, int p_bits)
: m_samplerate(p_samplerate)
, m_bits(p_bits)
{
	m_noise = new Noise;
	m_noise->Seed();
	m_additive = new Additive(32768, m_samplerate);
}

TMM::~TMM()
{
	delete m_additive;
	delete m_noise;
}

int
TMM::GenerateSamples(TTMMSettings* p_settings, short* p_samples, int p_size)
{
	int size = p_size;

	switch(p_settings->type) {
	case TMM_TYPE_NOISE:
		{
			switch(p_settings->noise.type) {
			case TMM_NOISETYPE_WHITE:
				{
					for(unsigned int i = 0; i < p_size; i++) {
						p_samples[i] = (short)m_noise->White(32767.0);
					}
				}
				break;
			case TMM_NOISETYPE_PINK:
				{
					for(unsigned int i = 0; i < p_size; i++) {
						p_samples[i] = (short)(m_noise->Pink() * 65535.0);
					}
				}
				break;
			case TMM_NOISETYPE_BROWN:
				{
					for(unsigned int i = 0; i < p_size; i++) {
						p_samples[i] = (short)(m_noise->Brown() * 65535.0);
					}
				}
				break;
			}
		}
		break;
	case TMM_TYPE_SINE:
		{
			for(unsigned int i = 0; i < 32; i++) {
				double s = sin((2.0 * M_PI * 261.63f * (double)i) / 8363.0);
				p_samples[i] = (short)(s * 32767.0);
			}
			size = 32;
		}
		break;
	case TMM_TYPE_PULSE:
		{
			char i;

			for(i = 0; i < p_settings->pulse.width; i++) {
				p_samples[i] = -1 * 32767;
			}
			for(i = p_settings->pulse.width; i < 32; i++) {
				p_samples[i] = 1 * 32767;
			}

			size = 32;
		}
		break;
	case TMM_TYPE_ADDITIVE:
		{
			p_settings->additive.harmonics[0] = 0;
			double* samples = m_additive->Process(&p_settings->additive);
			for(int i = 0; i < 32768; i++) {
				p_samples[i] = (short)(samples[i] * 32767.0);
			}
			size = 32768;
		}
		break;
	}

	return size;
}

int
TMM::ConvertToMOD(void * in, unsigned int sin, void ** out, unsigned int * sout)
{
    return 0;
}

extern "C" int
tmm_generate_samples(int rate, int bits, TTMMSettings * p_settings, short * p_samples, int p_size)
{
    TMM * tmm = new TMM(rate, bits);

    int ret = tmm->GenerateSamples(p_settings, p_samples, p_size);

    delete tmm;

    return ret;
}

extern "C" int
tmm_convert_to_mod(int rate, int bits, void * in, unsigned int sin, void ** out, unsigned int * sout)
{
    TMM * tmm = new TMM(rate, bits);

    int ret = tmm->ConvertToMOD(in, sin, out, sout);

    delete tmm;

    return ret;
}
