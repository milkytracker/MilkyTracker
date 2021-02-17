#include "globals.h"
#include "tmm.h"

#define MOD_LEN_NAME        20
#define MOD_COUNT_SAMPLES   31
#define MOD_LEN_SAMPLE_DESC (22+2+1+1+2+2)
#define MOD_LEN_ORDER       128

#define SWAPW(x) (((((unsigned short)x) & 0xff00) >> 8)  | ((((unsigned short)x) & 0xff) << 8))

TMM::Converter::Converter(::TMM * tmm)
: m_tmm_data(NULL)
, m_tmm_size(0)
, m_pmod_data(NULL)
, m_pmod_size(NULL)
, m_tmm(tmm)
{

}

TMM::Converter::~Converter()
{

}

void
TMM::Converter::SetTMMSource(void * data, unsigned int size)
{
    m_tmm_data = data;
    m_tmm_size = size;
}

void
TMM::Converter::SetMODDest(void ** pdata, unsigned int * psize)
{
    m_pmod_data = pdata;
    m_pmod_size = psize;
}

bool
TMM::Converter::IsSourceValid() const
{
    return m_tmm_data != NULL && m_tmm_size > 0 && m_pmod_data != NULL && m_pmod_size != NULL;
}

int
TMM::Converter::Convert()
{
    int i;
    unsigned char * p = (unsigned char *)m_tmm_data;
    unsigned char * d;
    TTMMModuleSample smps[MOD_COUNT_SAMPLES];
    char * data[MOD_COUNT_SAMPLES];
    unsigned int len[MOD_COUNT_SAMPLES];
    unsigned int slen = 0, plen;
    unsigned char songlen;
    unsigned char seq[MOD_LEN_ORDER];
    unsigned char pmax = 0;

    // Magic ok?
    if(*(p++) != 232)
        return 1;

    // Read samples
    for(i = 0; i < MOD_COUNT_SAMPLES; i++) {
        TTMMSettings ts;
        TTMMModuleSample * smp = &smps[i];

        memcpy(smp, p, sizeof(TTMMModuleSample));
        p += sizeof(TTMMModuleSample);

        ts.type = smp->tmm_type;

#if !defined(P_AMIGA)
        smp->len = SWAPW(smp->len);
        smp->loopstart = SWAPW(smp->loopstart);
        smp->looplen = SWAPW(smp->looplen);
#endif

        smp->len <<= 1;
        smp->loopstart <<= 1;
        smp->looplen <<= 1;

        switch(ts.type) {
        case TMM_TYPE_NOISE:
            memcpy(&ts.noise, p, sizeof(TTMMNoise));
            p += sizeof(TTMMNoise);
            break;
        case TMM_TYPE_SINE:
            memcpy(&ts.sine, p, sizeof(TTMMSine));
            p += sizeof(TTMMSine);
#if defined(P_AMIGA)
            ts.sine.basefreq = SWAPW(ts.sine.basefreq);
#endif
            break;
        case TMM_TYPE_PULSE:
            memcpy(&ts.pulse, p, sizeof(TTMMPulse));
            p += sizeof(TTMMPulse);
#if defined(P_AMIGA)
            ts.pulse.basefreq = SWAPW(ts.pulse.basefreq);
#endif
            break;
        case TMM_TYPE_ADDITIVE:
            memcpy(&ts.additive, p, sizeof(TTMMAdditive));
            p += sizeof(TTMMAdditive);
#if defined(P_AMIGA)
            ts.additive.basefreq = SWAPW(ts.additive.basefreq);
            ts.additive.bwscale = SWAPW(ts.additive.bwscale);
            ts.additive.rndseed = SWAPW(ts.additive.rndseed);
#endif
            break;
        case TMM_TYPE_NONE: default:
            // Skip normal samples
            data[i] = NULL;
            len[i] = smp->len;
            slen += smp->len;
            continue;
        }

        // Generate samples
        data[i] = (char *) malloc(32768);
        len[i] = smp->len = m_tmm->GenerateSamples(&ts, data[i], 32768);
        slen += smp->len;
    }

    // Song length and restart byte
    songlen = *(p++);
    p++;

    // Read sequence
    for(i = 0; i < MOD_LEN_ORDER; i++) {
        if(*p > pmax) {
            pmax = *p;
        }
        seq[i] = *(p++);
    }
    pmax++;

    // Skip ID (M.K.)

    // Create MOD
    plen = pmax * 4 * 64 * (2 + 1 + 1);
    *m_pmod_size = MOD_LEN_NAME+(MOD_COUNT_SAMPLES*MOD_LEN_SAMPLE_DESC)+1+1+MOD_LEN_ORDER+4+plen+slen;
    *m_pmod_data = d = (unsigned char *)malloc(*m_pmod_size);
    memset(d, 0, *m_pmod_size);

    // Skip name
    sprintf((char *)d, "TMM%08d", 6052020);
    d += MOD_LEN_NAME;

    // Copy over sample descriptions
    for(i = 0; i < MOD_COUNT_SAMPLES; i++) {
        smps[i].len >>= 1;
        smps[i].loopstart >>= 1;
        smps[i].looplen >>= 1;
#if !defined(P_AMIGA)
        smps[i].len = SWAPW(smps[i].len);
        smps[i].loopstart = SWAPW(smps[i].loopstart);
        smps[i].looplen = SWAPW(smps[i].looplen);
#endif
        memcpy(d, &smps[i], MOD_LEN_SAMPLE_DESC);
        d += MOD_LEN_SAMPLE_DESC;
    }

    // Song length and restart byte
    *(d++) = songlen;
    *(d++) = 0x7f;

    // Write sequence
    for(i = 0; i < MOD_LEN_ORDER; i++) {
        *(d++) = seq[i];
    }

    // ID
    *(d++) = 'M'; *(d++) = '.'; *(d++) = 'K'; *(d++) = '.';

    // Copy patterns
    memcpy(d, p, plen);
    d += plen;
    p += plen;

    // Copy samples
    for(i = 0; i < MOD_COUNT_SAMPLES; i++) {
        if(data[i]) {
            memcpy(d, data[i], len[i]);
            d += len[i];
            free(data[i]);
        } else {
            memcpy(d, p, len[i]);
            d += len[i];
            p += len[i];
        }
    }

    return 0;
}
