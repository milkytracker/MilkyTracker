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
        smp->len = SWAPW(smp->len) << 1;
        smp->loopstart = SWAPW(smp->loopstart) << 1;
        smp->looplen = SWAPW(smp->looplen) << 1;
#endif

        switch(ts.type) {
        case TMM_TYPE_NOISE:
            memcpy(&ts.noise, p, sizeof(TTMMNoise));
            p += sizeof(TTMMNoise);
            break;
        case TMM_TYPE_SINE:
            memcpy(&ts.sine, p, sizeof(TTMMSine));
            p += sizeof(TTMMSine);
#if defined(P_AMIGA)
            ts.sine.basefreq = BO_SWAPW(ts.sine.basefreq);
#endif
            break;
        case TMM_TYPE_PULSE:
            memcpy(&ts.pulse, p, sizeof(TTMMPulse));
            p += sizeof(TTMMPulse);
#if defined(P_AMIGA)
            ts.pulse.basefreq = BO_SWAPW(ts.pulse.basefreq);
#endif
            break;
        case TMM_TYPE_ADDITIVE:
            memcpy(&ts.additive, p, sizeof(TTMMAdditive));
            p += sizeof(TTMMAdditive);
#if defined(P_AMIGA)
            ts.additive.basefreq = BO_SWAPW(ts.additive.basefreq);
            ts.additive.bwscale = BO_SWAPW(ts.additive.bwscale);
#endif
            break;
        case TMM_TYPE_NONE: default:
            // Skip normal samples
            data[i] = NULL;
            slen += smp->len;
            continue;
        }

        // Generate samples
        data[i] = (char *) malloc(32768);
        smp->len = m_tmm->GenerateSamples(&ts, data[i], 32768);
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
    d += MOD_LEN_NAME;

    // Copy over sample descriptions
    for(i = 0; i < MOD_COUNT_SAMPLES; i++) {
        smps[i].len = SWAPW(smps[i].len);
        smps[i].loopstart = SWAPW(smps[i].loopstart);
        smps[i].looplen = SWAPW(smps[i].looplen);
        memcpy(d, &smps[i], MOD_LEN_SAMPLE_DESC);
        d += MOD_LEN_SAMPLE_DESC;
    }

    // Song length and restart byte
    *(d++) = songlen;
    *(d++) = 0x1a;

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
    for(i = 0; i < 31; i++) {
        if(data[i]) {
            memcpy(d, data[i], smps[i].len);
            d += smps[i].len;
            free(data[i]);
        } else {
            memcpy(d, p, smps[i].len);
            d += smps[i].len;
            p += smps[i].len;
        }
    }

    return 0;
}
