#include <milkyplay/PlayerGeneric.h>
#include <milkyplay/XModule.h>
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
    const char* outputFile = argv[2];
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

    // Create player with default 44.1kHz sample rate
    PlayerGeneric player(44100);

    // Default parameters
    int startOrder = 0;
    int endOrder = -1;

    // Parse command line options
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug_enabled = true;
        }
        else if (strcmp(argv[i], "--samplerate") == 0 && i + 1 < argc) {
            int freq = atoi(argv[++i]);

            DEBUG_LOG("Setting sample rate: %d Hz\n", freq);

            player.adjustFrequency(freq);
        }
        else if (strcmp(argv[i], "--ramping") == 0 && i + 1 < argc) {
            int ramp = atoi(argv[++i]);

            DEBUG_LOG("Setting ramping: %d\n", ramp);

            player.setResamplerType(ramp > 0, ramp == 2);
        }
        else if (strcmp(argv[i], "--resampler") == 0 && i + 1 < argc) {
            int type = atoi(argv[++i]);

            DEBUG_LOG("Setting resampler type: %d\n", type);

            player.setResamplerType((ChannelMixer::ResamplerTypes)(type));
        }
        else if (strcmp(argv[i], "--mixershift") == 0 && i + 1 < argc) {
            int shift = atoi(argv[++i]);

            DEBUG_LOG("Setting mixer shift: %d\n", shift);

            player.setSampleShift(shift);
        }
        else if (strcmp(argv[i], "--mixervolume") == 0 && i + 1 < argc) {
            int vol = atoi(argv[++i]);

            DEBUG_LOG("Setting mixer volume: %d\n", vol);

            player.setMasterVolume(vol);
        }
        else if (strcmp(argv[i], "--from") == 0 && i + 1 < argc) {
            startOrder = atoi(argv[++i]);

            DEBUG_LOG("Setting start order: %d\n", startOrder);
        }
        else if (strcmp(argv[i], "--to") == 0 && i + 1 < argc) {
            endOrder = atoi(argv[++i]);
            
            DEBUG_LOG("Setting end order: %d\n", endOrder);
        }
    }

    // Validate order range
    if (endOrder == -1) {
        endOrder = module->header.ordnum - 1;
        DEBUG_LOG("Using default end order: %d\n", endOrder);
    }
    if (startOrder < 0 || startOrder >= module->header.ordnum) {
        fprintf(stderr, "Start order %d out of range (0-%d)\n", startOrder, module->header.ordnum - 1);
        delete module;
        return 1;
    }
    if (endOrder < startOrder || endOrder >= module->header.ordnum) {
        fprintf(stderr, "End order %d out of range (%d-%d)\n", endOrder, startOrder, module->header.ordnum - 1);
        delete module;
        return 1;
    }

    DEBUG_LOG("Exporting to WAV file: %s\n", outputFile);
    DEBUG_LOG("Orders to export: %d through %d\n", startOrder, endOrder);

    // Set buffer size for better performance
    DEBUG_LOG("Setting buffer size to 1024 samples\n");
    player.setBufferSize(1024);

    // Export to WAV with proper parameters
    DEBUG_LOG("Starting export...\n");
    
    int numWrittenSamples = player.exportToWAV(outputFile, module, startOrder, endOrder);
    if (numWrittenSamples == MP_DEVICE_ERROR) {
        fprintf(stderr, "Failed to export WAV file: %s\n", outputFile);
        delete module;
        return 1;
    }

    delete module;
    printf("Successfully exported to WAV: %s. %d samples written\n", outputFile, numWrittenSamples);
    return 0;
} 