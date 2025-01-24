#pragma once

#include <string>
#include "WAVExportArgs.h"

class WAVExporter {
public:
    // Static factory method that maintains backward compatibility
    static int exportFromCommandLine(int argc, char* argv[]);

    // Static method to export using an existing parser
    static bool exportFromParser(CLIParser& parser);

    // Constructor takes command line arguments
    WAVExporter(int argc, char* argv[]);

    // Phase 1: Parse arguments
    int parseArguments();

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