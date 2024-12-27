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
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage();
        return 1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];

    // Create module and load file
    XModule* module = new XModule();
    if (module->loadModule(inputFile) != MP_OK) {
        fprintf(stderr, "Failed to load module: %s\n", inputFile);
        delete module;
        return 1;
    }

    // Create player with default 44.1kHz sample rate
    PlayerGeneric player(44100);

    // Default parameters
    int startOrder = 0;
    int endOrder = -1;

    // Parse command line options
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--samplerate") == 0 && i + 1 < argc) {
            player.adjustFrequency(atoi(argv[++i]));
        }
        else if (strcmp(argv[i], "--ramping") == 0 && i + 1 < argc) {
            int ramp = atoi(argv[++i]);
            player.setResamplerType(ramp > 0, ramp == 2);
        }
        else if (strcmp(argv[i], "--resampler") == 0 && i + 1 < argc) {
            player.setResamplerType((ChannelMixer::ResamplerTypes)(atoi(argv[++i])));
        }
        else if (strcmp(argv[i], "--mixershift") == 0 && i + 1 < argc) {
            player.setSampleShift(atoi(argv[++i]));
        }
        else if (strcmp(argv[i], "--mixervolume") == 0 && i + 1 < argc) {
            player.setMasterVolume(atoi(argv[++i]));
        }
        else if (strcmp(argv[i], "--from") == 0 && i + 1 < argc) {
            startOrder = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--to") == 0 && i + 1 < argc) {
            endOrder = atoi(argv[++i]);
        }
    }

    fprintf(stderr, "Exporting to WAV file: %s\n", outputFile);
    fprintf(stderr, "From order: %d, To order: %d\n", startOrder, endOrder);

    // Set buffer size for better performance
    player.setBufferSize(1024);

    // Export to WAV with proper parameters
    if (player.exportToWAV(outputFile, module, startOrder, endOrder) != MP_OK) {
        fprintf(stderr, "Failed to export WAV file: %s\n", outputFile);
        delete module;
        return 1;
    }

    delete module;
    printf("Successfully exported to WAV: %s\n", outputFile);
    return 0;
} 