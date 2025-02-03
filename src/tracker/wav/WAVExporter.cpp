#include "WAVExporter.h"
#include <BasicTypes.h>
#include <ModuleServices.h>
#include <TrackerSettingsDatabase.h>
#include <XMFile.h>
#include <PPSystem.h>
#include <XModule.h>
#include "WAVExportArgs.h"
#include "WAVUtils.h"
#include <CLIParser.h>
#include <cstring>
#include <stdexcept>

// Static factory method for parser usage
std::unique_ptr<WAVExporter> WAVExporter::createFromParser(CLIParser& parser, WAVExportArgs::DashFormat dashFormat) {
	auto exporter = std::unique_ptr<WAVExporter>(new WAVExporter());
	
	WAVExportArgs::registerOptions(parser, dashFormat);

	if (!parser.parse()) {
		exporter->errorMessage = parser.getError();
		exporter->parseError = true;
		return exporter;
	}

	if (parser.isHelpRequested()) {
		parser.printUsage();
		exit(0);
	}

	// Load settings from config file
	TrackerSettingsDatabase settingsDB;
	const char* configFile = System::getConfigFileName();
	if (XMFile::exists(configFile)) {
		XMFile f(configFile);
		settingsDB.serialize(f);
	}

	try {
		exporter->params = WAVExportArgs::initFromParser(parser, settingsDB, dashFormat);
	}
	catch (const std::runtime_error& e) {
		exporter->errorMessage = e.what();
		exporter->argumentError = true;
	}

	return exporter;
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

	// Update channel count and initialize muting array
	params.channelCount = module.header.channum;
	mp_ubyte* newMuting = new mp_ubyte[params.channelCount];
	for (pp_uint32 i = 0; i < params.channelCount; i++) {
		newMuting[i] = 0;  // Initialize all channels to unmuted
	}
	delete[] params.muting;	 // Safe to delete nullptr
	params.muting = newMuting;

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
		
		for (pp_uint32 i = 0; i < params.channelCount; i++) {
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