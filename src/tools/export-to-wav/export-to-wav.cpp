#include <BasicTypes.h>
#include <ModuleServices.h>
#include <TrackerSettingsDatabase.h>
#include <XMFile.h>
#include <PPSystem.h>
#include <XModule.h>
#include <WAVExportArgs.h>
#include <WAVUtils.h>
#include <cstring>
#include <stdexcept>

int main(int argc, char* argv[])
{

    // Load settings from config file
    TrackerSettingsDatabase settingsDB;
    const char* configFile = System::getConfigFileName();
    if (XMFile::exists(configFile)) {
        XMFile f(configFile);
        settingsDB.serialize(f);
    }

    WAVExportArgs::Arguments params;

    try {
        // Get filenames and WAV writer parameters from command line arguments
        params = WAVExportArgs::parseFromCommandLine(argc, argv, settingsDB);
    }
    catch (const std::runtime_error& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
    // Load the module
    XModule module;
    if (module.loadModule(params.inputFile) != MP_OK) {
        fprintf(stderr, "Failed to load module: %s\n", params.inputFile);
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
        fprintf(stderr, "Failed to export WAV file: %s\n", params.outputFile);
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