#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <map>
#include <tuple>

class CLIParser {
public:
	struct Option {
		std::string name;
		bool requiresValue;
		std::string description;
		std::vector<std::string> allowedValues;
		
		Option(const char* n, bool rv, const char* d, const std::vector<std::string>& av = std::vector<std::string>())
			: name(n), requiresValue(rv), description(d), allowedValues(av) {}
	};

	// Template constructor to handle both const and non-const argv
	template<typename T>
	explicit CLIParser(int argc, T argv[]) 
		: argc(argc)
		, argv(const_cast<const char**>(argv))  // Store as const internally
		, helpRequested(false)
	{
		// Add built-in help option
		addOption("--help", false, "Show this help message and exit");
	}
	
	// Register options before parsing
	void addOption(const char* name, bool requiresValue, const char* description, const std::vector<std::string>& allowedValues = std::vector<std::string>());
	void addPositionalArg(const char* name, const char* description, bool required = true);
	
	// Parse and access results
	bool parse();
	bool hasOption(const char* name) const;
	const char* getOptionValue(const char* name) const;
	int getIntOptionValue(const char* name, int defaultValue = 0) const;
	const char* getPositionalArg(size_t index) const;
	size_t getPositionalArgCount() const;
	
	// Help text generation
	void printUsage() const;
	void setAdditionalHelpText(const char* text) { additionalHelpText = text ? text : ""; }
	
	// Error handling
	const char* getError() const;
	
	// Check if help was requested
	bool isHelpRequested() const { return helpRequested; }

	// New method to set positional argument value
	void setPositionalArgValue(size_t index, const char* value);

	// Check if a positional argument exists
	bool hasPositionalArg(const char* name) const;

private:
	int argc;
	const char** argv;  // Store as const internally
	std::string errorMessage;
	std::string additionalHelpText;
	std::vector<Option> options;
	std::vector<std::tuple<std::string, std::string, bool>> positionalArgs; // name, description, required
	bool helpRequested;
	
	// Parsed results
	std::unordered_map<std::string, std::string> parsedOptions;
	std::vector<std::string> parsedPositionalArgs;
	
	bool isOption(const char* arg) const;
	const Option* findOption(const char* name) const;
	bool isValidOptionValue(const Option* opt, const char* value) const;
};