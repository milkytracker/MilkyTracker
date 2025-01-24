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

    explicit CLIParser(const char* programName);
    
    // Register options before parsing
    void addOption(const char* name, bool requiresValue, const char* description);
    void addPositionalArg(const char* name, const char* description);
    
    // Parse and access results
    bool parse(int argc, char* argv[]);
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

private:
    std::string programName;
    std::string errorMessage;
    std::string additionalHelpText;
    std::vector<Option> options;
    std::vector<std::pair<std::string, std::string>> positionalArgs; // name, description
    bool helpRequested;
    
    // Parsed results
    std::unordered_map<std::string, std::string> parsedOptions;
    std::vector<std::string> parsedPositionalArgs;
    
    bool isOption(const char* arg) const;
    const Option* findOption(const char* name) const;
}; 