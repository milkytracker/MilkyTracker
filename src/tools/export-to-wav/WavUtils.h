#pragma once

#include <milkyplay/XModule.h>

namespace WavUtils {
    /**
     * Check if a WAV file contains only silence (all samples are zero)
     * 
     * @param filename Path to the WAV file to check
     * @return true if the file is silent or invalid, false if it contains any non-zero samples
     */
    bool isWavSilent(const char* filename);
} 