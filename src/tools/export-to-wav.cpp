#include <ppui/BasicTypes.h>
#include <tracker/ModuleServices.h>
#include <tracker/TrackerSettingsDatabase.h>
#include <milkyplay/XMFile.h>
#include <ppui/osinterface/posix/PPSystem_POSIX.h>
#include <milkyplay/XModule.h>
#include "WAVExportParams.h"

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

    // Load settings from MilkyTracker's config file
    TrackerSettingsDatabase settingsDB;
    const char* configFile = System::getConfigFileName();
    if (XMFile::exists(configFile)) {
        XMFile f(configFile);
        settingsDB.serialize(f);
    }

    // Get WAV writer parameters from command line arguments
    ModuleServices::WAVWriterParameters params = WAVExportParams::parseFromCommandLine(argc, argv, settingsDB);
    params.toOrder = module.header.ordnum - 1;  // Set the end order

    // Convert paths to PPSystemString
    PPSystemString outputFilePath(outputFile);

    // Export to WAV using ModuleServices
    int numWrittenSamples = services.exportToWAV(outputFilePath, params);
    if (numWrittenSamples == MP_DEVICE_ERROR) {
        fprintf(stderr, "Failed to export WAV file: %s\n", outputFile);
        return 1;
    }

    return 0;
} 