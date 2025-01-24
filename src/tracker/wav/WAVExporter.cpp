#include "WAVExporter.h"
#include <BasicTypes.h>
#include <ModuleServices.h>
#include <TrackerSettingsDatabase.h>
#include <XMFile.h>
#include <PPSystem.h>
#include <XModule.h>
#include "WAVExportArgs.h"
#include "WAVUtils.h"
#include <cstring>
#include <stdexcept>

// Static factory method for backward compatibility
int WAVExporter::exportFromCommandLine(int argc, char* argv[]) {
    WAVExporter exporter(argc, argv);
    if (!exporter.parseArguments()) {
        fprintf(stderr, "Error: %s\n", exporter.getErrorMessage());
        return 1;
    }
    return exporter.performExport();
}

WAVExporter::WAVExporter(int argc, char* argv[])
    : argc(argc)
    , argv(argv)
    , parseError(false)
{
}

bool WAVExporter::parseArguments() {
    // Load settings from config file
    TrackerSettingsDatabase settingsDB;
    const char* configFile = System::getConfigFileName();
    if (XMFile::exists(configFile)) {
        XMFile f(configFile);
        settingsDB.serialize(f);
    }

    // Get filenames and WAV writer parameters from command line arguments
    try {
        params = WAVExportArgs::parseFromCommandLine(argc, argv, settingsDB);
        return true;
    }
    catch (const std::runtime_error& e) {
        errorMessage = e.what();
        parseError = true;
        return false;
    }
}

int WAVExporter::performExport() {
    // Check if the input file exists
    if (!XMFile::exists(params.inputFile)) {
        errorMessage = "Input file does not exist: ";
        errorMessage += params.inputFile;
        return 1;
    }

    // Load the module
    XModule module;
    if (module.loadModule(params.inputFile) != MP_OK) {
        errorMessage = "Failed to load module: ";
        errorMessage += params.inputFile;
        return 1;
    }

    // Create ModuleServices instance
    ModuleServices services(module);

    // Set the end order
    params.toOrder = module.header.ordnum - 1;  

    // Convert paths to PPSystemString
    PPSystemString outputFilePath(params.outputFile);

    // Export to WAV using ModuleServices
    int numWrittenSamples = services.exportToWAV(outputFilePath, (ModuleServices::WAVWriterParameters&) params);

    if (numWrittenSamples == MP_DEVICE_ERROR) {
        errorMessage = "Failed to export WAV file: ";
        errorMessage += params.outputFile;
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