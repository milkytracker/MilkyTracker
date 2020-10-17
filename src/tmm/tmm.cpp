#include "globals.h"
#include "tmm.h"

TMM::TMM(int p_samplerate, int p_bits)
: m_samplerate(p_samplerate)
, m_bits(p_bits)
{
	m_noise = new Noise;
	m_noise->Seed();
	m_additive = new Additive(32768, m_samplerate);
	m_lpfilter = new LoPass(m_samplerate);
	m_hpfilter = new HiPass(m_samplerate);
	m_env = new Envelope(m_samplerate);
}

TMM::~TMM()
{
	delete m_env;
	delete m_hpfilter;
	delete m_lpfilter;
	delete m_additive;
	delete m_noise;
}

void
TMM::MoveToZeroCrossing8(TTMMSettings * p_settings, char * p_samples, int p_size)
{
	// Check if sample starts with a zero crossing
	if(p_samples[0] == 0)
		return;

	int i, j, minpos = -1;
	int min;
	char * scp = new char[p_size];

	// If not, find smallest value and its position
	min = 128;
	for(i = 0; i < p_size; i++) {
		int s = abs(p_samples[i]);
		if(s < min) {
			min = s;
			minpos = i;

			if(min == 0)
				break;
		}
	}

	// Now move the whole sample to the "next-to-zero"-crossing
	memcpy(scp, p_samples, p_size * sizeof(char));
	for(i = 0, j = minpos; i < p_size; i++, j++) {
		p_samples[i] = scp[j % p_size];
	}

	delete[] scp;
}

void
TMM::MoveToZeroCrossing16(TTMMSettings * p_settings, short * p_samples, int p_size)
{
	// Check if sample starts with a zero crossing
	if(p_samples[0] == 0)
		return;

	int i, j, minpos = -1;
	int min;
	short * scp = new short[p_size];

	memcpy(scp, p_samples, p_size * sizeof(short));

	// If not, find smallest value and its position
	min = 32768;
	for(i = 0; i < p_size; i++) {
		int s = abs(p_samples[i]);
		if(s < min) {
			min = s;
			minpos = i;

			if(min == 0)
				break;
		}
	}

	// Now move the whole sample to the "next-to-zero"-crossing
	for(i = 0, j = minpos; i < p_size; i++, j++) {
		p_samples[i] = scp[j % p_size];
	}

	delete[] scp;
}

int
TMM::GenerateSamples8(TTMMSettings * p_settings, char * p_samples, int p_size)
{
	int size = p_size;

	switch(p_settings->type) {
	case TMM_TYPE_NOISE:
		{
			switch(p_settings->noise.type) {
			case TMM_NOISETYPE_WHITE:
				{
					for(unsigned int i = 0; i < size; i++) {
						p_samples[i] = (char) m_noise->White(127.0);
					}
				}
				break;
			case TMM_NOISETYPE_PINK:
				{
					for(unsigned int i = 0; i < size; i++) {
						p_samples[i] = (char) (m_noise->Pink() * 255.0);
					}
				}
				break;
			case TMM_NOISETYPE_BROWN:
				{
					for(unsigned int i = 0; i < size; i++) {
						p_samples[i] = (char) (m_noise->Brown() * 255.0);
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
				p_samples[i] = (char) (s * 127.0);
			}
			size = 32;
		}
		break;
	case TMM_TYPE_PULSE:
		{
			char i;

			for(i = 0; i < p_settings->pulse.width; i++) {
				p_samples[i] = -127;
			}
			for(i = p_settings->pulse.width; i < 32; i++) {
				p_samples[i] = 127;
			}

			size = 32;
		}
		break;
	case TMM_TYPE_ADDITIVE:
		{
			double * samples = m_additive->Process(&p_settings->additive);

			if(p_settings->additive.usefilters) {
				m_lpfilter->SetCutoff((double)p_settings->additive.lpfreq * 100.0);
				samples = m_lpfilter->Process(samples, 32768);
				m_hpfilter->SetCutoff((double)p_settings->additive.hpfreq * 100.0);
				samples = m_hpfilter->Process(samples, 32768);
			}

			if(p_settings->additive.useenv) {
				m_env->SetAttack((double)p_settings->additive.envatt / 32768.0);
				m_env->SetDecay((double)p_settings->additive.envdec / 32768.0);
				m_env->SetSustain((double)p_settings->additive.envsus / 32768.0);
				m_env->SetHold((double)p_settings->additive.envhold / 32768.0);
				m_env->SetRelease((double)p_settings->additive.envrel / 32768.0);
				samples = m_env->Process(samples, 32768);
			}

			if(p_settings->additive.usedist) {
				double drive = (double)p_settings->additive.distdrive / 16;
				double gain = (double)p_settings->additive.distgain / 80;

				for(int i = 0; i < 32768; i++) {
					double s = samples[i];

					if(p_settings->additive.disttype == 1) {
						s = pow(tanh(pow(fabs(s * drive), 5.0)), 0.2);
						if(samples[i] < 0.0)
							s *= -1;
					} else {
						s = tanh(s * drive);
					}

					s *= gain;
					s = s < -1.0 ? -1.0 : (s > 1.0 ? 1.0 : s);

					samples[i] = s;
				}
			}

			for(int i = 0; i < 32768; i++) {
				p_samples[i] = (char) (samples[i] * 127.0);
			}
			size = 32768;
		}
		break;
	}

	return size;
}

int
TMM::GenerateSamples16(TTMMSettings * p_settings, short * p_samples, int p_size)
{
	int size = p_size;

	switch(p_settings->type) {
	case TMM_TYPE_NOISE:
		{
			switch(p_settings->noise.type) {
			case TMM_NOISETYPE_WHITE:
				{
					for(unsigned int i = 0; i < size; i++) {
						p_samples[i] = (short)m_noise->White(32767.0);
					}
				}
				break;
			case TMM_NOISETYPE_PINK:
				{
					for(unsigned int i = 0; i < size; i++) {
						p_samples[i] = (short)(m_noise->Pink() * 65535.0);
					}
				}
				break;
			case TMM_NOISETYPE_BROWN:
				{
					for(unsigned int i = 0; i < size; i++) {
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
			double * samples = m_additive->Process(&p_settings->additive);

			if(p_settings->additive.usefilters) {
				m_lpfilter->SetCutoff((double)p_settings->additive.lpfreq * 100.0);
				samples = m_lpfilter->Process(samples, 32768);
				m_hpfilter->SetCutoff((double)p_settings->additive.hpfreq * 100.0);
				samples = m_hpfilter->Process(samples, 32768);
			}

			if(p_settings->additive.useenv) {
				m_env->SetAttack((double)p_settings->additive.envatt / 32768.0);
				m_env->SetDecay((double)p_settings->additive.envdec / 32768.0);
				m_env->SetSustain((double)p_settings->additive.envsus / 32768.0);
				m_env->SetHold((double)p_settings->additive.envhold / 32768.0);
				m_env->SetRelease((double)p_settings->additive.envrel / 32768.0);
				samples = m_env->Process(samples, 32768);
			}

			if(p_settings->additive.usedist) {
				double drive = (double)p_settings->additive.distdrive / 16;
				double gain = (double)p_settings->additive.distgain / 80;

				for(int i = 0; i < 32768; i++) {
					double s = samples[i];

					if(p_settings->additive.disttype == 1) {
						s = pow(tanh(pow(fabs(s * drive), 5.0)), 0.2);
						if(samples[i] < 0.0)
							s *= -1;
					} else {
						s = tanh(s * drive);
					}

					s *= gain;
					s = s < -1.0 ? -1.0 : (s > 1.0 ? 1.0 : s);

					samples[i] = s;
				}
			}

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
TMM::GenerateSamples(TTMMSettings * p_settings, void * p_samples, int p_size)
{
	int ret;
	unsigned short flags = p_settings->extensions.flags;

	if(m_bits == 16) {
		ret = GenerateSamples16(p_settings, (short *) p_samples, p_size);

		if(flags & TMM_FLAG_LOOP_FWD && flags & TMM_FLAG_FIX_ZERO) {
			MoveToZeroCrossing16(p_settings, (short *) p_samples, ret);
		}
	} else {
		ret = GenerateSamples8(p_settings, (char *) p_samples, p_size);

		if(flags & TMM_FLAG_LOOP_FWD && flags & TMM_FLAG_FIX_ZERO) {
			MoveToZeroCrossing8(p_settings, (char *) p_samples, ret);
		}
	}

	return ret;
}

int
TMM::ConvertToMOD(void * in, unsigned int sin, void ** out, unsigned int * sout)
{
    Converter * cvt = new Converter(this);

    cvt->SetTMMSource(in, sin);
    cvt->SetMODDest(out, sout);

    if(cvt->IsSourceValid()) {
        return cvt->Convert();
    }

    return 1;
}

extern "C" int
tmm_generate_samples(int rate, int bits, TTMMSettings * p_settings, void * p_samples, int p_size)
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
