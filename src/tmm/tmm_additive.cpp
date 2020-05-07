#include "globals.h"
#include "tmm.h"

TMM::Additive::Additive(int p_bins, int p_samplerate)
: m_bins(p_bins), m_samplerate(p_samplerate)
{
	m_ifft       = kiss_fft_alloc(m_bins, 1, NULL, NULL);
	m_ifft_in    = new kiss_fft_cpx[m_bins];
	m_ifft_out   = new kiss_fft_cpx[m_bins];
	m_freq_amp   = new double[m_bins >> 1];
	m_freq_phase = new double[m_bins >> 1];
	m_samples    = new double[m_bins];
	m_noise      = new TMM::Noise;
}

TMM::Additive::~Additive()
{
	delete   m_noise;
	delete[] m_samples;
	delete[] m_freq_phase;
	delete[] m_freq_amp;
	delete[] m_ifft_out;
	delete[] m_ifft_in;
	free(m_ifft);
}

double
TMM::Additive::RelativeFreq(double p_freq)
{
	return p_freq * (1.0 + p_freq * 0.1);
}

double
TMM::Additive::Profile(double p_fi, double p_bwi)
{
	double x = p_fi / p_bwi;
	x *= x;
	if(x > 14.71280603) return 0.0;

	return exp(-x) / p_bwi;
}

void
TMM::Additive::InverseFFT(TTMMAdditive* p_settings)
{
	int i;

	memset(m_ifft_in, 0, m_bins * sizeof(kiss_fft_cpx));
	memset(m_ifft_out, 0, m_bins * sizeof(kiss_fft_cpx));

	// Feed IFFT with harmonics
	for(i = 0; i < m_bins >> 1; i++) {
		m_ifft_in[i].r = m_freq_amp[i] * cos(m_freq_phase[i]);
		m_ifft_in[i].i = m_freq_amp[i] * sin(m_freq_phase[i]);
	}

	// Process IFFT
	kiss_fft(m_ifft, m_ifft_in, m_ifft_out);

	if(p_settings->destroyer) {
		// Convert absolute of complex result to get samples (yes, we use the numerical noise here ... sounds noisy, but is cool =p)
		for(i = 0; i < m_bins; i++) {
			m_samples[i] = sqrt(
				(m_ifft_out[i].r * m_ifft_out[i].r) +
				(m_ifft_out[i].i * m_ifft_out[i].i)
			);
		}
	} else {
		// Use just real values, due to hermetian symmetry imaginary part only contains numerical noise which is not desired
		for(i = 0; i < m_bins; i++) {
			m_samples[i] = m_ifft_out[i].r;
		}
	}
}

void
TMM::Additive::Normalize(TTMMAdditive* p_settings)
{
	int i;
	double max = 0.0;

	// Eval maximum value
	for(i = 0; i < m_bins; i++) {
		if(fabs(m_samples[i]) > max)
			max = fabs(m_samples[i]);
	}

	// Clamp value
	if(max < 1e-5f) max = 1e-5f;

	// Normalize
	if(p_settings->destroyer) {
		for(i = 0; i < m_bins; i++) {
			double s = m_samples[i] / (max * M_SQRT2);
			s -= (M_SQRT1_2 * 0.5);
			s *= 2.0;
			m_samples[i] = s;
		}
	} else {
		for(i = 0; i < m_bins; i++) {
			double s = m_samples[i] / (max * M_SQRT2);
			m_samples[i] = s;
		}
	}
}

double*
TMM::Additive::Process(TTMMAdditive* p_settings)
{
	int i, nh;

	// Zero all freq amps
	for(i = 0; i < m_bins >> 1; i++) {
		m_freq_amp[i] = 0.0;
	}

	// Generate harmonic profile
	for(nh = 1; nh < p_settings->nharmonics; nh++) {
		double
			hs    = p_settings->usescale ? pow(RelativeFreq(nh), (double)p_settings->bwscale / 1000.0) : nh,
			bw_hz = (pow(2.0, (double)p_settings->bandwidth / 1200.0) - 1.0) * (double)p_settings->basefreq * hs,
			bwi   = bw_hz / (2.0 * (double)m_samplerate),
			fi    = ((double)p_settings->basefreq * hs) / (double)m_samplerate,
			h     = (double)p_settings->harmonics[nh] / 255.0;

		for(i = 0; i < m_bins >> 1; i++) {
			m_freq_amp[i] += (Profile(((double)i / (double)m_bins) - fi, bwi) * h);
		}
	}

	// Add some random phases
	switch(p_settings->phasenoisetype) {
	default:
	case TMM_NOISETYPE_WHITE:
		for(i = 0; i < m_bins >> 1; i++) {
			m_freq_phase[i] = ((double)rand() / ((double)RAND_MAX + 1)) * 2.0 * M_PI;
		}
		break;
	case TMM_NOISETYPE_BROWN:
		m_noise->Reset();
		m_noise->Seed(1588799834);
		for(i = 0; i < m_bins >> 1; i++) {
			m_freq_phase[i] = (0.5 + m_noise->Brown()) * 2.0 * M_PI;
		}
		break;
	case TMM_NOISETYPE_PINK:
		m_noise->Reset();
		m_noise->Seed(1588799834);
		for(i = 0; i < m_bins >> 1; i++) {
			m_freq_phase[i] = (0.5 + m_noise->Pink()) * 2.0 * M_PI;
		}
		break;
	}

	// IFFT, normalize and rescale resulting reals
	InverseFFT(p_settings);
	Normalize(p_settings);

	return m_samples;
}
