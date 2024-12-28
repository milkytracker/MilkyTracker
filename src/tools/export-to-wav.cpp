#include <milkyplay/PlayerGeneric.h>
#include <milkyplay/XModule.h>
#include <tracker/ModuleServices.h>
#include <ppui/osinterface/PPSystem.h>
#include <stdio.h>
#include <string.h>

void printUsage() {
    printf("Usage: export-to-wav <input.xm> <output.wav> [options]\n");
    printf("Options:\n");
    printf("  --samplerate <hz>    Sample rate in Hz (default: 44100)\n");
    printf("  --ramping <0|1|2>    Volume ramping (0=off, 1=on, 2=FT2) (default: 0)\n");
    printf("  --resampler <0|1>    Resampler type (0=normal, 1=linear) (default: 0)\n");
    printf("  --mixershift <n>     Mixer shift value (default: 0)\n");
    printf("  --mixervolume <n>    Mixer volume (default: 256)\n");
    printf("  --from <n>           Start from order number (default: 0)\n");
    printf("  --to <n>             End at order number (default: last)\n");
    printf("  --debug              Enable debug logging\n");
}

#define DEBUG_LOG(fmt, ...) \
    do { if (debug_enabled) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage();
        return 1;
    }

    const char* inputFile = argv[1];
    PPSystemString outputFile = argv[2];
    bool debug_enabled = false;

    // Create module and load file
    XModule* module = new XModule();
    
    DEBUG_LOG("Loading module: %s\n", inputFile);

    if (module->loadModule(inputFile) != MP_OK) {
        fprintf(stderr, "Failed to load module: %s\n", inputFile);
        delete module;
        return 1;
    }

    DEBUG_LOG("Module loaded successfully. Orders: %d, Channels: %d\n", 
            module->header.ordnum, module->header.channum);

    // Create ModuleServices instance
    ModuleServices services(*module);

    // Setup export parameters with defaults
    ModuleServices::WAVWriterParameters params;
    params.sampleRate = 44100;
    params.resamplerType = 0;
    params.rampin = false;
    params.playMode = 0;
    params.mixerShift = 0;
    params.mixerVolume = 256;
    params.fromOrder = 0;
    params.toOrder = module->header.ordnum - 1;
    params.multiTrack = false;

    // Parse command line options
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug_enabled = true;
        }
        else if (strcmp(argv[i], "--samplerate") == 0 && i + 1 < argc) {
            params.sampleRate = atoi(argv[++i]);
            DEBUG_LOG("Setting sample rate: %d Hz\n", params.sampleRate);
        }
        else if (strcmp(argv[i], "--ramping") == 0 && i + 1 < argc) {
            int ramp = atoi(argv[++i]);
            params.rampin = (ramp == 2);
            params.resamplerType = (ramp > 0) ? 1 : 0;
            DEBUG_LOG("Setting ramping: %d\n", ramp);
        }
        else if (strcmp(argv[i], "--resampler") == 0 && i + 1 < argc) {
            params.resamplerType = atoi(argv[++i]);
            DEBUG_LOG("Setting resampler type: %d\n", params.resamplerType);
        }
        else if (strcmp(argv[i], "--mixershift") == 0 && i + 1 < argc) {
            params.mixerShift = atoi(argv[++i]);
            DEBUG_LOG("Setting mixer shift: %d\n", params.mixerShift);
        }
        else if (strcmp(argv[i], "--mixervolume") == 0 && i + 1 < argc) {
            params.mixerVolume = atoi(argv[++i]);
            DEBUG_LOG("Setting mixer volume: %d\n", params.mixerVolume);
        }
        else if (strcmp(argv[i], "--from") == 0 && i + 1 < argc) {
            params.fromOrder = atoi(argv[++i]);
            DEBUG_LOG("Setting start order: %d\n", params.fromOrder);
        }
        else if (strcmp(argv[i], "--to") == 0 && i + 1 < argc) {
            params.toOrder = atoi(argv[++i]);
            DEBUG_LOG("Setting end order: %d\n", params.toOrder);
        }
    }

    // Validate order range
    if (params.fromOrder < 0 || params.fromOrder >= module->header.ordnum) {
        fprintf(stderr, "Start order %d out of range (0-%d)\n", params.fromOrder, module->header.ordnum - 1);
        delete module;
        return 1;
    }
    if (params.toOrder < params.fromOrder || params.toOrder >= module->header.ordnum) {
        fprintf(stderr, "End order %d out of range (%d-%d)\n", params.toOrder, params.fromOrder, module->header.ordnum - 1);
        delete module;
        return 1;
    }

    DEBUG_LOG("Exporting to WAV file: %s\n", (const char*)outputFile);
    DEBUG_LOG("Orders to export: %d through %d\n", params.fromOrder, params.toOrder);

    // Export to WAV with proper parameters
    DEBUG_LOG("Starting export...\n");
    
    int numWrittenSamples = services.exportToWAV(outputFile, params);
    if (numWrittenSamples == MP_DEVICE_ERROR) {
        fprintf(stderr, "Failed to export WAV file: %s\n", (const char*)outputFile);
        delete module;
        return 1;
    }

    delete module;
    printf("Successfully exported to WAV: %s. %d samples written\n", (const char*)outputFile, numWrittenSamples);
    return 0;
} 