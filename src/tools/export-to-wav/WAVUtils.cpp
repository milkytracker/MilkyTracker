#include "WAVUtils.h"
#include <milkyplay/SampleLoaderWAV.h>

namespace WAVUtils {

bool isWAVSilent(const char* filename) {
    // Create a temporary module to use with SampleLoaderWAV
    XModule module;
    module.createEmptySong(true, true, 1);  // Initialize with 1 channel
    
    // Ensure we have at least one sample slot
    module.header.smpnum = 1;
    module.header.insnum = 1;
    
    // Create WAV loader
    SampleLoaderWAV loader(filename, module);
    
    // First verify this is a valid WAV file
    if (!loader.identifySample()) {
        return true;  // Invalid WAV file, consider it silent
    }
    
    // Load the sample into index 0
    if (loader.loadSample(0, -1) != MP_OK) {
        return true;  // Failed to load, consider it silent
    }
    
    // Get the loaded sample
    TXMSample* smp = &module.smp[0];
    if (!smp->sample || smp->samplen == 0) {
        return true;  // No sample data, consider it silent
    }
    
    // Check for silence based on sample type (8-bit or 16-bit)
    if (smp->type & 16) {
        // 16-bit samples
        mp_sword* samples = (mp_sword*)smp->sample;
        for (mp_uint32 i = 0; i < smp->samplen; i++) {
            if (samples[i] != 0) {
                return false;  // Found non-zero sample
            }
        }
    } else {
        // 8-bit samples
        mp_sbyte* samples = (mp_sbyte*)smp->sample;
        for (mp_uint32 i = 0; i < smp->samplen; i++) {
            if (samples[i] != 0) {
                return false;  // Found non-zero sample
            }
        }
    }
    
    return true;  // All samples were zero
}

} 