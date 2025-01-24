#include <WAVExporter.h>
#include <CLIParser.h>

int main(int argc, const char* argv[])
{
	static CLIParser parser(argc, argv);
	auto exporter = WAVExporter::createFromParser(parser);

    if (exporter->hasParseError() || exporter->performExport() != 0) {
        fprintf(stderr, "Error: %s\n", exporter->getErrorMessage());
        return 1;
    }

    return 0;
} 