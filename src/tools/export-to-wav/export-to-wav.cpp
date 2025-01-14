#include <BasicTypes.h>
#include <ModuleServices.h>
#include <TrackerSettingsDatabase.h>
#include <XMFile.h>
#include <PPSystem.h>
#include <XModule.h>
#include <wav/WAVExportParams.h>
#include <wav/WAVUtils.h>
#include <cstring>

int main(int argc, char* argv[])
{
    if (argc < 3) {
        WAVExportParams::printUsage(argv[0]);
        return 1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];

    // Load the module
    XModule module;
    if (module.loadModule(inputFile) != MP_OK) {
        fprintf(stderr, "Failed to load module: %s\n", inputFile);
        return 1;
    }

    // Create ModuleServices instance
    ModuleServices services(module);

    // Load settings from config file
    TrackerSettingsDatabase settingsDB;
    const char* configFile = System::getConfigFileName();
    if (XMFile::exists(configFile)) {
        XMFile f(configFile);
        settingsDB.serialize(f);
    }

    // Get WAV writer parameters from command line arguments
    auto params = WAVExportParams::parseFromCommandLine(argc, argv, settingsDB);
    params.toOrder = module.header.ordnum - 1;  // Set the end order

    // Convert paths to PPSystemString
    PPSystemString outputFilePath(outputFile);

    // Export to WAV using ModuleServices
    int numWrittenSamples = services.exportToWAV(outputFilePath, params);

    if (numWrittenSamples == MP_DEVICE_ERROR) {
        fprintf(stderr, "Failed to export WAV file: %s\n", outputFile);
        return 1;
    }
    
    // Clean up silent files in multi-track mode
    if (params.multiTrack) {
        PPSystemString baseName = outputFilePath.stripExtension();
        PPSystemString extension = outputFilePath.getExtension();
        
        for (pp_uint32 i = 0; i < module.header.channum; i++) {
            char filename[1024];
            snprintf(filename, sizeof(filename), "%s_%02d%s", baseName.getStrBuffer(), i+1, extension.getStrBuffer());
            
            if (WAVUtils::isWAVSilent(filename)) {
                remove(filename);
                if (params.verbose) {
                    fprintf(stderr, "Removed silent track: %s\n", filename);
                }
            } else if (params.verbose) {
                fprintf(stderr, "Kept non-silent track: %s\n", filename);
            }
        }
    }

    return 0;
} 