#include <cstdio>
#include <cstdint>
#include <cstring>

#ifndef _SF2_H
#define _SF2_H

#pragma pack(push,1)
struct SampleHeader {
    char name[20];
    uint32_t start, end, loopStart, loopEnd;
    uint32_t sampleRate;
    uint8_t originalPitch, pitchCorrection;
    uint16_t sampleLink, sampleType;
};
#pragma pack(pop)

class SF2 {

	private:
		FILE *f = nullptr;
		SampleHeader *headers = nullptr;
		int headerCount = 0;
		int16_t *sampleData = nullptr;
		int sampleDataSize = 0;
	public:
		~SF2();
		bool parse(FILE *file);
		int getMaxSampleHeaders();
		SampleHeader *getSampleHeader(int n);
		int16_t *getSampleData(int sh);
};

#endif
