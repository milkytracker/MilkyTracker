#include "globals.h"
#include "tmm.h"

TMM::Noise::Noise()
: m_white(0), m_count(1), m_brown(0.0), m_pink(0)
{
	memset(m_pinkStore, 0, 16 * sizeof(double));
}

TMM::Noise::~Noise()
{
}

void
TMM::Noise::Seed(unsigned long p_seed)
{
	if(p_seed) {
		m_seed = p_seed;
	} else {
		m_seed = time(NULL);
	}
}

double
TMM::Noise::White(double p_scale)
{
	m_seed   = (m_seed * 196314165) + 907633515;
	m_white  = m_seed >> 9;
	m_white |= 0x40000000;

	return (double)(((*(float*)&m_white) - 3.0) * p_scale);
}

int
TMM::Noise::CTZ(int p_num)
{
	int i = 0;

	while (((p_num >> i) & 1) == 0 && i < sizeof(int)) i++;

	return i;
}

double
TMM::Noise::Brown()
{
	while (true) {
		double  r = White();

		m_brown += r;

		if (m_brown < -8.0 || m_brown > 8.0) {
			m_brown -= r;
		} else {
			break;
		}
	}

	return m_brown * 0.0625;
}

double
TMM::Noise::Pink(void)
{
	double prevr;
	double r;
	unsigned long k = CTZ(m_count) & 15;

	// get previous value of this octave
	prevr = m_pinkStore[k];

	while (true) {
		r = White();

		// store new value
		m_pinkStore[k] = r;

		r -= prevr;

		// update total
		m_pink += r;

		if (m_pink <-4.0 || m_pink > 4.0) {
			m_pink -= r;
		} else {
			break;
		}
	}

	// update counter
	m_count++;

	return (White() + m_pink) * 0.125;
}
