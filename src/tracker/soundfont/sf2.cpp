#include <cstdio>
#include <cstdint>
#include <cstring>

#include "sf2.h"

SF2::~SF2() { 
	delete[] headers; 
	delete[] sampleData; 
	if (f) fclose(f); 
}

bool SF2::parse(FILE *file) {
	f = file;
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);
	char *buf = new char[size];
	fread(buf, 1, size, f);

	char *riff = buf;
	if (memcmp(riff, "RIFF", 4) || memcmp(riff + 8, "sfbk", 4)) return delete[] buf, false;

	char *pdta = strstr(buf, "pdta");
	char *shdr = pdta ? strstr(pdta, "shdr") : nullptr;
	char *sdta = strstr(buf, "sdta");
	char *smpl = sdta ? strstr(sdta, "smpl") : nullptr;

	if (!shdr || !smpl) return delete[] buf, false;

	int shdrSize = *(int *)(shdr + 4);
	headerCount = shdrSize / sizeof(SampleHeader);
	headers = new SampleHeader[headerCount];
	memcpy(headers, shdr + 8, headerCount * sizeof(SampleHeader));

	sampleDataSize = *(int *)(smpl + 4);
	sampleData = new int16_t[sampleDataSize / 2];
	memcpy(sampleData, smpl + 8, sampleDataSize);

	delete[] buf;
	return true;
}


int SF2::getMaxSampleHeaders() {
	int count = headerCount;
	if (count > 0 && strcmp(headers[count - 1].name, "EOS") == 0) {
		count--; // Exclude the EOS entry
	}
	return count;
}

SampleHeader * SF2::getSampleHeader(int n) { 
	return (n < headerCount) ? &headers[n] : nullptr; 
}

int16_t * SF2::getSampleData(int sh) { 
	return (sh < headerCount) ? &sampleData[headers[sh].start] : nullptr; 
}

