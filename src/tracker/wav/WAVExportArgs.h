#pragma once

#include <ModuleServices.h>
#include <TrackerSettingsDatabase.h>
#include <vector>

class WAVExportArgs {
public:
    class Arguments : public ModuleServices::WAVWriterParameters {
    public:
        Arguments();
        ~Arguments();

        // Only declare fields we add beyond WAVWriterParameters
        const char* inputFile;
        const char* outputFile;
        bool verbose;  // Flag for verbose output
    };

    static Arguments parseFromCommandLine(int argc, char* argv[], TrackerSettingsDatabase& settingsDB);
    static void printUsage(const char* programName);

private:
    static int getIntOption(int argc, char* argv[], const char* option, int defaultValue);
    static const char* getStringOption(int argc, char* argv[], const char* option, const char* defaultValue);
    static bool hasOption(int argc, char* argv[], const char* option);
    static bool isParameterThatTakesValue(const char* arg);
    static bool isValueForParameter(const char* arg, int argIndex, int argc, char* argv[]);
}; 