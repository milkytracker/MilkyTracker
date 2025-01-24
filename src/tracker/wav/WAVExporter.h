#ifndef __WAVEXPORTER_H__
#define __WAVEXPORTER_H__

class WAVExporter {
public:
    static int exportFromCommandLine(int argc, char* argv[]);

private:
    // Private constructor since this is a utility class with only static methods
    WAVExporter() = delete;
};

#endif