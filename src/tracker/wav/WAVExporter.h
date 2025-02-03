#pragma once

#include <string>
#include <memory>
#include "WAVExportArgs.h"

class CLIParser;  // Forward declaration

class WAVExporter {
public:
	// Create from parser
	static std::unique_ptr<WAVExporter> createFromParser(CLIParser& parser, WAVExportArgs::DashFormat dashFormat = WAVExportArgs::DashFormat::DOUBLE);

	// Core functionality
	int performExport();

	// State getters
	bool hasOutputFile() const { return params.outputFile != nullptr; }
	bool hasInputFile() const { return params.inputFile != nullptr; }
	bool hasParseError() const { return parseError; }
	bool hasArgumentError() const { return argumentError; }
	const char* getErrorMessage() const { return errorMessage.c_str(); }

	virtual ~WAVExporter() = default;

protected:
	bool parseError;
    bool argumentError;
	std::string errorMessage;
	WAVExportArgs::Arguments params;
};