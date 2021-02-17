#include "globals.h"
#include "tmm.h"

TMM::Filter::Filter(int p_rate)
{
    m_rate = p_rate;
    m_cutoff = 11025.0;
    m_q = 1.0;
    m_a[0] = 0.0;
    m_a[1] = 0.0;
    m_a[2] = 0.0;
    m_b[0] = 0.0;
    m_b[1] = 0.0;
    m_s[0] = 0.0;
    m_s[1] = 0.0;
    m_f[0] = 0.0;
    m_f[1] = 0.0;
}

void
TMM::Filter::SetCutoff(double p_cutoff)
{
    m_cutoff = p_cutoff;
    Preprocess();
}

void
TMM::Filter::SetQ(double p_q)
{
    m_q = p_q;
    Preprocess();
}

void
TMM::LoPass::Preprocess()
{
    double c = 1.0 / tan(M_PI * m_cutoff / (double)m_rate);

	m_a[0] = 1.0 / (1.0 + (m_q * c) + (c * c));
	m_a[1] = 2.0 * m_a[0];
	m_a[2] = m_a[0];

	m_b[0] = 2.0 * ( 1.0 - (c * c) ) * m_a[0];
	m_b[1] = ( 1.0 - (m_q * c) + (c * c) ) * m_a[0];
}

void
TMM::HiPass::Preprocess()
{
    double c = tan(M_PI * m_cutoff / (double)m_rate);

	m_a[0] = 1.0 / (1.0 + (m_q * c) + (c * c));
	m_a[1] = -2.0 * m_a[0];
	m_a[2] = m_a[0];

	m_b[0] = 2.0 * ( (c * c) - 1.0 ) * m_a[0];
	m_b[1] = ( 1.0 - (m_q * c) + (c * c) ) * m_a[0];
}

double*
TMM::Filter::Process(double* samples, int nsamples)
{
    int i;

    for(i = 0; i < nsamples; i++) {
        double s = samples[i];
        double o = m_a[0] * s
          + m_a[1] * m_s[0]
          + m_a[2] * m_s[1]
          - m_b[0] * m_f[0]
          - m_b[1] * m_f[1];

        m_s[1] = m_s[0];
        m_s[0] = s;

        m_f[1] = m_f[0];
        m_f[0] = o;

        samples[i] = o;
    }

    return samples;
}
