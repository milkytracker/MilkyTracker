#ifndef __WAVEXPORTER_H__
#define __WAVEXPORTER_H__

#include <string>
#include "WAVExportArgs.h"

class WAVExporter {
public:
    // Static factory method that maintains backward compatibility
    static int exportFromCommandLine(int argc, char* argv[]);

    // Constructor takes command line arguments
    WAVExporter(int argc, char* argv[]);

    // Phase 1: Parse arguments
    bool parseArguments();

    // Phase 2: Perform the export
    int performExport();

    // State getters
    bool hasOutputFile() const { return params.outputFile != nullptr; }
    bool hasInputFile() const { return params.inputFile != nullptr; }
    bool hasParseError() const { return parseError; }
    const char* getErrorMessage() const { return errorMessage.c_str(); }

private:
    int argc;
    char** argv;
    bool parseError;
    std::string errorMessage;
    WAVExportArgs::Arguments params;
};

#endif