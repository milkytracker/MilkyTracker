#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

class CLIParser {
public:
    struct Option {
        std::string name;
        bool requiresValue;
        std::string description;
        
        Option(const char* n, bool rv, const char* d)
            : name(n), requiresValue(rv), description(d) {}
    };

    explicit CLIParser(int argc, const char* argv[]);
    
    // Register options before parsing
    void addOption(const char* name, bool requiresValue, const char* description);
    void addPositionalArg(const char* name, const char* description, bool required = true);
    
    // Parse and access results
    bool parse();
    bool hasOption(const char* name) const;
    const char* getOptionValue(const char* name) const;
    int getIntOptionValue(const char* name, int defaultValue) const;
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
    const char** argv;
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
}; 