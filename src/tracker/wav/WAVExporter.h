#pragma once

#include <string>
#include "WAVExportArgs.h"

class CLIParser;  // Forward declaration

class WAVExporter {
public:
    // Command-line interface
    static int exportFromCommandLine(int argc, char* argv[]);
    WAVExporter(int argc, char* argv[]);

    // Parser interface
    WAVExporter(CLIParser& parser);
    bool initFromParser(CLIParser& parser);

    // Core functionality
    int parseArguments();  // Used with argc/argv constructor
    int performExport();

    // State getters
    bool hasOutputFile() const { return params.outputFile != nullptr; }
    bool hasInputFile() const { return params.inputFile != nullptr; }
    bool hasParseError() const { return parseError; }
    const char* getErrorMessage() const { return errorMessage.c_str(); }

private:
    bool parseError;
    std::string errorMessage;
    WAVExportArgs::Arguments params;

    // Used by command-line interface
    int argc;
    char** argv;
};