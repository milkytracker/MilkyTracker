#include "WAVExportArgs.h"
#include "CLIParser.h"
#include <cstring>
#include <cstdio>
#include <stdexcept>

WAVExportArgs::Arguments::Arguments()
	: inputFile(nullptr)
	, outputFile(nullptr)
	, verbose(false)
	, channelCount(0)
{
	// Initialize base class fields to safe defaults
	sampleRate = 44100;
	mixerVolume = 256;
	mixerShift = 1;
	resamplerType = 4;
	fromOrder = 0;
	toOrder = -1;
	muting = nullptr;  // Will be allocated when we know the channel count
	panning = nullptr;
	rampin = false;
	playMode = 0;
	multiTrack = false;
	limiterDrive = 0;
}

WAVExportArgs::Arguments::~Arguments()
{
	delete[] inputFile;
	delete[] outputFile;
	delete[] muting;
}

WAVExportArgs::Arguments::Arguments(const Arguments& other)
	: ModuleServices::WAVWriterParameters(other)  // Copy base class
	, inputFile(nullptr)
	, outputFile(nullptr)
	, verbose(other.verbose)
	, channelCount(other.channelCount)
{
	copyStrings(other);
	// Handle muting array
	if (other.muting && other.channelCount > 0) {
		mp_ubyte* newMuting = new mp_ubyte[other.channelCount];
		memcpy(newMuting, other.muting, other.channelCount);
		muting = newMuting;
	}
}

WAVExportArgs::Arguments& WAVExportArgs::Arguments::operator=(const Arguments& other)
{
	if (this != &other) {
		ModuleServices::WAVWriterParameters::operator=(other);	// Copy base class
		delete[] inputFile;
		delete[] outputFile;
		delete[] muting;
		inputFile = nullptr;
		outputFile = nullptr;
		muting = nullptr;
		verbose = other.verbose;
		channelCount = other.channelCount;
		copyStrings(other);
		// Handle muting array
		if (other.muting && other.channelCount > 0) {
			mp_ubyte* newMuting = new mp_ubyte[other.channelCount];
			memcpy(newMuting, other.muting, other.channelCount);
			muting = newMuting;
		}
	}
	return *this;
}

void WAVExportArgs::Arguments::copyStrings(const Arguments& other)
{
	if (other.inputFile) {
		size_t len = strlen(other.inputFile);
		char* newInput = new char[len + 1];
		strcpy(newInput, other.inputFile);
		inputFile = newInput;
	}
	
	if (other.outputFile) {
		size_t len = strlen(other.outputFile);
		char* newOutput = new char[len + 1];
		strcpy(newOutput, other.outputFile);
		outputFile = newOutput;
	}
}

void WAVExportArgs::registerOptions(CLIParser& parser, DashFormat format) {
	parser.addOption(getOptionName("output", format).c_str(), true, "Output file name");
	parser.addOption(getOptionName("sample-rate", format).c_str(), true, "Sample rate in Hz (default: from settings or 44100)");
	parser.addOption(getOptionName("volume", format).c_str(), true, "Mixer volume (default: from settings or 256)");
	parser.addOption(getOptionName("shift", format).c_str(), true, "Mixer shift (default: from settings or 1)");
	parser.addOption(getOptionName("resampler", format).c_str(), true, "Resampler type (default: from settings or 4)");
	parser.addOption(getOptionName("multi-track", format).c_str(), false, "Export each track to a separate WAV file");
	parser.addOption(getOptionName("verbose", format).c_str(), false, "Enable verbose output");

	if (!parser.hasPositionalArg("input")) {
		parser.addPositionalArg("input", "Input module file (.xm)", true);
	}
	
	parser.setAdditionalHelpText(
		"When using --multi-track, output files will be named:\n"
		"  output_01.wav, output_02.wav, etc.\n"
		"  (Silent tracks will be automatically removed)\n"
	);
}

WAVExportArgs::Arguments WAVExportArgs::initFromParser(CLIParser& parser, TrackerSettingsDatabase& settingsDB, DashFormat format) {
	Arguments params;

	// Set defaults from settings database
	params.sampleRate = settingsDB.hasKey("HDRECORDER_MIXFREQ") ? 
					   settingsDB.restore("HDRECORDER_MIXFREQ")->getIntValue() : 44100;
	params.mixerVolume = settingsDB.hasKey("HDRECORDER_MIXERVOLUME") ? 
						settingsDB.restore("HDRECORDER_MIXERVOLUME")->getIntValue() : 256;
	params.mixerShift = settingsDB.hasKey("HDRECORDER_MIXERSHIFT") ? 
					   settingsDB.restore("HDRECORDER_MIXERSHIFT")->getIntValue() : 1;
	params.resamplerType = settingsDB.hasKey("HDRECORDER_INTERPOLATION") ? 
						  settingsDB.restore("HDRECORDER_INTERPOLATION")->getIntValue() : 4;

	// Get required output file
	const char* outputArg = parser.getOptionValue(getOptionName("output", format).c_str());
	if (!outputArg) {
		throw std::runtime_error("Output file (" + getOptionName("output", format) + ") is required");
	}
	
	// Copy output file (safe now since we checked for nullptr)
	char* outputCopy = new char[strlen(outputArg) + 1];
	strcpy(outputCopy, outputArg);
	params.outputFile = outputCopy;

	// Get input file (first positional arg)
	const char* inputArg = parser.getPositionalArg(0);
	if (!inputArg) {
		throw std::runtime_error("Input file is required");
	}
	
	char* inputCopy = new char[strlen(inputArg) + 1];
	strcpy(inputCopy, inputArg);
	params.inputFile = inputCopy;

	// Override defaults with command line arguments if provided
	if (parser.hasOption(getOptionName("sample-rate", format).c_str())) {
		params.sampleRate = parser.getIntOptionValue(getOptionName("sample-rate", format).c_str(), params.sampleRate);
	}
	if (parser.hasOption(getOptionName("volume", format).c_str())) {
		params.mixerVolume = parser.getIntOptionValue(getOptionName("volume", format).c_str(), params.mixerVolume);
	}
	if (parser.hasOption(getOptionName("shift", format).c_str())) {
		params.mixerShift = parser.getIntOptionValue(getOptionName("shift", format).c_str(), params.mixerShift);
	}
	if (parser.hasOption(getOptionName("resampler", format).c_str())) {
		params.resamplerType = parser.getIntOptionValue(getOptionName("resampler", format).c_str(), params.resamplerType);
	}
	
	params.multiTrack = parser.hasOption(getOptionName("multi-track", format).c_str());
	params.verbose = parser.hasOption(getOptionName("verbose", format).c_str());

	return params;
}