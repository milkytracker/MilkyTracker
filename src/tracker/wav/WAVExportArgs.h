#pragma once

#include <ModuleServices.h>
#include <TrackerSettingsDatabase.h>
#include <vector>
#include <cstring>
#include <string>

class CLIParser;  // Forward declaration

class WAVExportArgs {
public:
	enum class DashFormat {
		SINGLE,  // -option
		DOUBLE   // --option
	};

	class Arguments : public ModuleServices::WAVWriterParameters {
	public:
		Arguments();
		~Arguments();

		// Copy constructor
		Arguments(const Arguments& other);
		// Copy assignment
		Arguments& operator=(const Arguments& other);

		// Only declare fields we add beyond WAVWriterParameters
		const char* inputFile;
		const char* outputFile;
		bool verbose;  // Flag for verbose output
		pp_uint32 channelCount;	 // Number of channels in the module

	private:
		// Helper to handle deep copying of strings
		void copyStrings(const Arguments& other);
	};

	// Initialize Arguments from an already-parsed parser
	static Arguments initFromParser(CLIParser& parser, TrackerSettingsDatabase& settingsDB, DashFormat format = DashFormat::DOUBLE);
	
	// Helper to register WAV export options with a parser
	static void registerOptions(CLIParser& parser, DashFormat format = DashFormat::DOUBLE);

protected:
	// Helper to get option name with appropriate dash format
	static std::string getOptionName(const char* name, DashFormat format) {
		return (format == DashFormat::SINGLE ? "-" : "--") + std::string(name);
	}
};