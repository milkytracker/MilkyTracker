/*
 *  tracker/cocoa/main.mm
 *
 *  Copyright 2014 Dale Whinham
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// -------- Cocoa/OS X --------
#import <Cocoa/Cocoa.h>
#import <mach/mach.h>
#import <mach/mach_time.h>

// ---------- Tracker ---------
#import "BasicTypes.h"
#import "Tracker.h"
#import "ModuleServices.h"
#import <XModule.h>
#import <WAVExporter.h>

// ----------------------------------------------------------
//  Returns number of milliseconds elapsed since last reboot
// ----------------------------------------------------------
pp_uint32 PPGetTickCount()
{
	// Static variable guaranteed to be zero-initialised
	static mach_timebase_info_data_t timebaseInfo;
	
	// Timebase info uninitialised?
	if (timebaseInfo.denom == 0)
		mach_timebase_info(&timebaseInfo);
	
	// Get the system tick count in nanoseconds
	uint64_t absTime = mach_absolute_time();
	uint64_t absTimeNanos = absTime * timebaseInfo.numer / timebaseInfo.denom;
	
	// Convert to milliseconds
	return (pp_uint32) (absTimeNanos / 1e6);
}

// -------------------------------------------------------------------
//  Dummy function; key modifiers are checked/set in MTTrackerView.mm
// -------------------------------------------------------------------
void QueryKeyModifiers() { }

bool exportToWAV(const char* inputFile, const char* outputFile) {
	const char* argv[] = {
		"milkytracker",  // Program name
		inputFile,       // Input file
		"--output",      // Output flag
		outputFile,      // Output file
		nullptr
	};
	int argc = 4;

	return WAVExporter::exportFromCommandLine(argc, (char**)argv) == 0;
}

// --------------------------------------
//  Entry point; start Cocoa application
// --------------------------------------
int main(int argc, const char * argv[])
{
	bool headless = false;
	const char* loadFile = nullptr;
	const char* outputWAVFile = nullptr;

	// Parse command line arguments
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--headless") == 0) {
			headless = true;
		}
		else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
			outputWAVFile = argv[++i];
		}
		else if (argv[i][0] != '-') {
			loadFile = argv[i];
		}
		else {
			fprintf(stderr, "Usage: %s [--headless] [--output file.wav] [inputfile.xm]\n", argv[0]);
			exit(1);
		}
	}

	// Validate arguments
	if (outputWAVFile && !loadFile) {
		fprintf(stderr, "Error: Input XM file must be specified when using --output\n");
		exit(1);
	}

	if (headless && !outputWAVFile) {
		fprintf(stderr, "Error: --output must be specified when using --headless mode\n");
		exit(1);
	}

	// Handle headless mode before starting GUI
	if (headless) {
		if (!exportToWAV(loadFile, outputWAVFile)) {
			return 1;
		}
		return 0;
	}

	// Start GUI application
	return NSApplicationMain(argc, argv);
}
