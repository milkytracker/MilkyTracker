#include "CLIParser.h"
#include <cstring>
#include <cstdio>

// Constructor is now a template in the header

void CLIParser::addOption(const char* name, bool requiresValue, const char* description, const std::vector<std::string>& allowedValues)
{
	options.emplace_back(name, requiresValue, description, allowedValues);
}

void CLIParser::addPositionalArg(const char* name, const char* description, bool required)
{
	positionalArgs.emplace_back(name, description, required);
}

bool CLIParser::isValidOptionValue(const Option* opt, const char* value) const
{
	if (opt->allowedValues.empty()) {
		return true;  // No restrictions
	}
	
	for (const auto& allowed : opt->allowedValues) {
		if (allowed == value) {
			return true;
		}
	}
	return false;
}

bool CLIParser::parse()
{
	parsedOptions.clear();
	parsedPositionalArgs.clear();
	errorMessage.clear();
	helpRequested = false;

	size_t positionalIndex = 0;
	
	for (int i = 1; i < argc; i++) {
		const char* arg = argv[i];
		
		if (isOption(arg)) {
			// Check for help flag first
			if (strcmp(arg, "--help") == 0) {
				helpRequested = true;
				return true;
			}
			
			const Option* opt = findOption(arg);
			if (!opt) {
				errorMessage = "Unknown option: ";
				errorMessage += arg;
				return false;
			}
			
			if (opt->requiresValue) {
				if (i + 1 >= argc) {
					errorMessage = "Option requires value: ";
					errorMessage += arg;
					return false;
				}
				// Check that next argument isn't an option
				const char* value = argv[i + 1];
				if (isOption(value)) {
					errorMessage = "Option requires value, but got another option: ";
					errorMessage += arg;
					errorMessage += " ";
					errorMessage += value;
					return false;
				}
				
				// Validate value against allowed values if any
				if (!isValidOptionValue(opt, value)) {
					errorMessage = "Invalid value for option ";
					errorMessage += arg;
					errorMessage += ": ";
					errorMessage += value;
					errorMessage += "\nAllowed values are: ";
					for (size_t j = 0; j < opt->allowedValues.size(); j++) {
						if (j > 0) errorMessage += ", ";
						errorMessage += opt->allowedValues[j];
					}
					return false;
				}
				
				parsedOptions[opt->name] = value;
				i++; // Skip the value we just processed
			} else {
				parsedOptions[opt->name] = "true";
			}
		} else {
			if (positionalIndex >= positionalArgs.size()) {
				errorMessage = "Too many positional arguments";
				return false;
			}
			parsedPositionalArgs.push_back(arg);
			positionalIndex++;
		}
	}
	
	// If help was requested, no need to validate other args
	if (helpRequested) {
		return true;
	}
	
	// Check each position to ensure required args are satisfied
	for (size_t i = 0; i < positionalArgs.size(); i++) {
		bool isRequired = std::get<2>(positionalArgs[i]);
		if (isRequired && i >= parsedPositionalArgs.size()) {
			errorMessage = "Missing required argument: ";
			errorMessage += std::get<0>(positionalArgs[i]);
			return false;
		}
	}
	
	return true;
}

bool CLIParser::hasOption(const char* name) const
{
	return parsedOptions.find(name) != parsedOptions.end();
}

const char* CLIParser::getOptionValue(const char* name) const
{
	auto it = parsedOptions.find(name);
	return it != parsedOptions.end() ? it->second.c_str() : nullptr;
}

int CLIParser::getIntOptionValue(const char* name, int defaultValue) const
{
	const char* value = getOptionValue(name);
	return value ? atoi(value) : defaultValue;
}

const char* CLIParser::getPositionalArg(size_t index) const
{
	return index < parsedPositionalArgs.size() ? parsedPositionalArgs[index].c_str() : nullptr;
}

size_t CLIParser::getPositionalArgCount() const
{
	return parsedPositionalArgs.size();
}

void CLIParser::printUsage() const
{
	fprintf(stderr, "Usage: %s", argv[0]);
	
	// Print positional args in order
	for (const auto& arg : positionalArgs) {
		if (std::get<2>(arg)) {
			fprintf(stderr, " <%s>", std::get<0>(arg).c_str());
		} else {
			fprintf(stderr, " [%s]", std::get<0>(arg).c_str());
		}
	}
	
	fprintf(stderr, " [options]\n\nPositional arguments:\n");
	
	// Print positional arg descriptions
	for (const auto& arg : positionalArgs) {
		fprintf(stderr, "  %-20s %s%s\n", 
				std::get<0>(arg).c_str(), 
				std::get<1>(arg).c_str(),
				std::get<2>(arg) ? "" : " (optional)");
	}
	
	fprintf(stderr, "\nOptions:\n");
	
	// Print option descriptions with allowed values
	for (const auto& opt : options) {
		if (opt.requiresValue) {
			fprintf(stderr, "  %s <value>", opt.name.c_str());
			fprintf(stderr, "%*s%s", 
					static_cast<int>(20 - strlen(opt.name.c_str()) - 8), 
					"", opt.description.c_str());
			
			// Print allowed values if any
			if (!opt.allowedValues.empty()) {
				fprintf(stderr, " (allowed values: ");
				for (size_t i = 0; i < opt.allowedValues.size(); i++) {
					if (i > 0) fprintf(stderr, ", ");
					fprintf(stderr, "%s", opt.allowedValues[i].c_str());
				}
				fprintf(stderr, ")");
			}
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "  %s", opt.name.c_str());
			fprintf(stderr, "%*s%s\n",
					static_cast<int>(20 - strlen(opt.name.c_str())),
					"", opt.description.c_str());
		}
	}

	// Print additional help text if present
	if (!additionalHelpText.empty()) {
		fprintf(stderr, "\n%s", additionalHelpText.c_str());
	}
}

const char* CLIParser::getError() const
{
	return errorMessage.c_str();
}

bool CLIParser::isOption(const char* arg) const
{
	return arg[0] == '-' && (arg[1] == '-' || isalpha(arg[1]));  // Accept both -- and -alpha
}

const CLIParser::Option* CLIParser::findOption(const char* name) const
{
	for (const auto& opt : options) {
		if (opt.name == name) {
			return &opt;
		}
	}
	return nullptr;
}

void CLIParser::setPositionalArgValue(size_t index, const char* value) {
	if (index < parsedPositionalArgs.size()) {
		parsedPositionalArgs[index] = value;
	}
}

bool CLIParser::hasPositionalArg(const char* name) const {
	for (const auto& arg : positionalArgs) {
		if (std::get<0>(arg) == name) {
			return true;
		}
	}
	return false;
}