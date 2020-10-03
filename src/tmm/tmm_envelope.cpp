#include "globals.h"
#include "tmm.h"

TMM::Envelope::Envelope(int p_rate)
: m_rate(p_rate)
, m_att(0)
, m_dec(0)
, m_sus(1.0)
, m_hold(32768)
, m_rel(0)
{

}

void
TMM::Envelope::SetAttack(double p_att)
{
    m_att = (int)(p_att * (double)m_rate);
}

void
TMM::Envelope::SetDecay(double p_dec)
{
    m_dec = (int)(p_dec * (double)m_rate);
}

void
TMM::Envelope::SetSustain(double p_sus)
{
    m_sus = p_sus;
}

void
TMM::Envelope::SetHold(double p_hold)
{
    m_hold = (int)(p_hold * (double)m_rate);
}

void
TMM::Envelope::SetRelease(double p_rel)
{
    m_rel = (int)(p_rel * (double)m_rate);
}

double*
TMM::Envelope::Process(double* samples, int nsamples)
{
    int i;

    for(i = 0; i < nsamples; i++) {
        double amp = 0.0;

        if(i < m_att) {
            // Attack in m_att samples
            amp = i / (double)m_att;
        } else if(i < (m_att + m_dec)) {
            // Decay in m_dec samples (1.0 -> Sustain level)
            amp = 1.0 - ((i - m_att) / (double)m_dec) * (1.0 - m_sus);
        } else if(i < (m_att + m_dec + m_hold)) {
            // Hold for m_hold samples
            amp = m_sus;
        } else if(i < (m_att + m_dec + m_hold + m_rel)) {
            // Release in m_rel samples
            amp = m_sus - ((i - (m_att + m_dec + m_hold)) / (double)m_rel) * m_sus;
        }

        samples[i] *= amp;
    }
    return samples;
}
