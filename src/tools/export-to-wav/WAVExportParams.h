#pragma once

#include <tracker/ModuleServices.h>
#include <tracker/TrackerSettingsDatabase.h>

class WAVExportParams {
public:
    class Parameters : public ModuleServices::WAVWriterParameters {
    public:
        Parameters();
        ~Parameters();
    };

    static Parameters parseFromCommandLine(int argc, char* argv[], TrackerSettingsDatabase& settingsDB);
    static void printUsage(const char* programName);

private:
    static int getIntOption(int argc, char* argv[], const char* option, int defaultValue);
    static bool hasOption(int argc, char* argv[], const char* option);
}; 