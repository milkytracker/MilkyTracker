#include "WAVExportParams.h"
#include <cstring>
#include <cstdio>

WAVExportParams::Parameters::Parameters() {
    // Initialize all fields to safe defaults
    sampleRate = 44100;
    mixerVolume = 256;
    mixerShift = 1;
    resamplerType = 4;
    fromOrder = 0;
    toOrder = -1;
    muting = nullptr;
    panning = nullptr;
    multiTrack = false;
    limiterDrive = 0;
    verbose = false;
}

WAVExportParams::Parameters::~Parameters() {
    delete[] muting;
}

WAVExportParams::Parameters WAVExportParams::parseFromCommandLine(int argc, char* argv[], TrackerSettingsDatabase& settingsDB) {
    Parameters params;

    // Set defaults from settings database
    params.sampleRate = settingsDB.hasKey("HDRECORDER_MIXFREQ") ? 
                       settingsDB.restore("HDRECORDER_MIXFREQ")->getIntValue() : 44100;
    params.mixerVolume = settingsDB.hasKey("HDRECORDER_MIXERVOLUME") ? 
                        settingsDB.restore("HDRECORDER_MIXERVOLUME")->getIntValue() : 256;
    params.mixerShift = settingsDB.hasKey("HDRECORDER_MIXERSHIFT") ? 
                       settingsDB.restore("HDRECORDER_MIXERSHIFT")->getIntValue() : 1;
    params.resamplerType = settingsDB.hasKey("HDRECORDER_INTERPOLATION") ? 
                          settingsDB.restore("HDRECORDER_INTERPOLATION")->getIntValue() : 4;

    // Override with command line arguments if provided
    params.sampleRate = getIntOption(argc, argv, "--sample-rate", params.sampleRate);
    params.mixerVolume = getIntOption(argc, argv, "--volume", params.mixerVolume);
    params.mixerShift = getIntOption(argc, argv, "--shift", params.mixerShift);
    params.resamplerType = getIntOption(argc, argv, "--resampler", params.resamplerType);
    params.verbose = hasOption(argc, argv, "--verbose");
    
    // Set additional required parameters
    params.fromOrder = 0;
    params.toOrder = -1; // Will be set by the caller
    mp_ubyte* mutingArray = new mp_ubyte[256]; // Allocate with safe size
    for (int i = 0; i < 256; i++) {
        mutingArray[i] = 0;  // Initialize all channels to unmuted
    }
    params.muting = mutingArray;
    params.panning = nullptr;
    params.multiTrack = hasOption(argc, argv, "--multi-track");
    params.limiterDrive = 0;

    return params;
}

void WAVExportParams::printUsage(const char* programName) {
    fprintf(stderr, "Usage: %s <input.xm> <output.wav> [options]\n", programName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --sample-rate <rate>  Sample rate in Hz (default: from settings or 44100)\n");
    fprintf(stderr, "  --volume <volume>     Mixer volume (default: from settings or 256)\n");
    fprintf(stderr, "  --shift <shift>       Mixer shift (default: from settings or 1)\n");
    fprintf(stderr, "  --resampler <type>    Resampler type (default: from settings or 4)\n");
    fprintf(stderr, "  --multi-track         Export each track to a separate WAV file\n");
    fprintf(stderr, "  --verbose             Enable verbose output\n");
    fprintf(stderr, "\nWhen using --multi-track, output files will be named:\n");
    fprintf(stderr, "  output_01.wav, output_02.wav, etc.\n");
    fprintf(stderr, "  (Silent tracks will be automatically removed)\n");
}

int WAVExportParams::getIntOption(int argc, char* argv[], const char* option, int defaultValue) {
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], option) == 0 && i + 1 < argc) {
            return atoi(argv[i + 1]);
        }
    }
    return defaultValue;
}

bool WAVExportParams::hasOption(int argc, char* argv[], const char* option) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], option) == 0) {
            return true;
        }
    }
    return false;
} 