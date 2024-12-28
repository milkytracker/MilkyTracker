#include <ppui/BasicTypes.h>
#include <tracker/ModuleServices.h>
#include <tracker/TrackerSettingsDatabase.h>
#include <milkyplay/XMFile.h>
#include <ppui/osinterface/posix/PPSystem_POSIX.h>
#include <milkyplay/XModule.h>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.xm> <output.wav>\n", argv[0]);
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

    // Create WAV writer parameters with defaults from settings
    ModuleServices::WAVWriterParameters params;
    params.sampleRate = settingsDB.hasKey("HDRECORDER_MIXFREQ") ? 
                       settingsDB.restore("HDRECORDER_MIXFREQ")->getIntValue() : 44100;
    params.mixerVolume = settingsDB.hasKey("HDRECORDER_MIXERVOLUME") ? 
                        settingsDB.restore("HDRECORDER_MIXERVOLUME")->getIntValue() : 256;
    params.mixerShift = settingsDB.hasKey("HDRECORDER_MIXERSHIFT") ? 
                       settingsDB.restore("HDRECORDER_MIXERSHIFT")->getIntValue() : 1;
    params.resamplerType = settingsDB.hasKey("HDRECORDER_INTERPOLATION") ? 
                          settingsDB.restore("HDRECORDER_INTERPOLATION")->getIntValue() : 4;

    // Set additional required parameters
    params.fromOrder = 0;
    params.toOrder = module.header.ordnum - 1;
    params.muting = nullptr;
    params.panning = nullptr;
    params.multiTrack = false;
    params.limiterDrive = 0;

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