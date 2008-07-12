/*
 *  tracker/FileExtProvider.h
 *
 *  Copyright 2008 Peter Barth
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

/*
 *  FileExtProvider.h
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 12.07.08.
 *
 */

#ifndef __FILEEXTPROVIDER_H__
#define __FILEEXTPROVIDER_H__

#include "BasicTypes.h"
#include "SimpleVector.h"

class FileExtProvider
{
private:
	enum ExtensionTypes
	{
		ExtensionTypeAll,
		ExtensionTypeModules,
		ExtensionTypeInstruments,
		ExtensionTypeSamples,
		ExtensionTypePatterns,
		ExtensionTypeTracks
	};

	static const char* moduleExtensions[];
	static const char* instrumentExtensions[];
	static const char* sampleExtensions[];
	static const char* patternExtensions[];
	static const char* trackExtensions[];
	static const char* colorExtensions[];

	char** tempList;

	PPSimpleVector<PPString> tempExtensions;
	
	const char** fillList(const char** baseList, ExtensionTypes type);

public:
	FileExtProvider() :
		tempList(NULL)
	{
	}
	
	~FileExtProvider()
	{
		delete[] tempList;
	}

	enum ModuleExtensions
	{
		ModuleExtension669,
		ModuleExtensionAMF,
		ModuleExtensionAMS,
		ModuleExtensionCBA,
		ModuleExtensionDBM,
		ModuleExtensionDIGI,
		ModuleExtensionDSM,
		ModuleExtensionDTM,
		ModuleExtensionFAR,
		ModuleExtensionGDM,
		ModuleExtensionGMC,
		ModuleExtensionIMF,
		ModuleExtensionIT,
		ModuleExtensionMDL,
		ModuleExtensionMOD,
		ModuleExtensionMTM,
		ModuleExtensionMXM,
		ModuleExtensionOKT,
		ModuleExtensionPLM,
		ModuleExtensionPSM,
		ModuleExtensionPTM,
		ModuleExtensionS3M,
		ModuleExtensionSTM,
		ModuleExtensionULT,
		ModuleExtensionUNI,
		ModuleExtensionXM,
		
		ModuleExtensionLAST,
	};	
	
	const char** getModuleExtensions();
	const char* getModuleExtension(ModuleExtensions extension);
	const char* getModuleDescription(ModuleExtensions extension);
	
	enum InstrumentExtensions
	{
		InstrumentExtensionXI,
		InstrumentExtensionPAT,

		InstrumentExtensionLAST,
	};

	const char** getInstrumentExtensions();
	const char* getInstrumentExtension(InstrumentExtensions extension);
	const char* getInstrumentDescription(InstrumentExtensions extension);
	
	enum SampleExtensions
	{
		SampleExtensionWAV,
		SampleExtensionIFF,
		SampleExtension8SVX,
		SampleExtensionAIF,
		SampleExtensionAIFF,

		SampleExtensionLAST
	};

	const char** getSampleExtensions();
	const char* getSampleExtension(SampleExtensions extension);
	const char* getSampleDescription(SampleExtensions extension);
	
	enum PatternExtensions
	{
		PatternExtensionXP,
		
		PatternExtensionLAST
	};
	
	const char** getPatternExtensions();	
	const char* getPatternExtension(PatternExtensions extension);
	const char* getPatternDescription(PatternExtensions extension);
	
	enum TrackExtensions
	{
		TrackExtensionXT,

		TrackExtensionLAST
	};

	const char** getTrackExtensions();
	const char* getTrackExtension(TrackExtensions extension);
	const char* getTrackDescription(TrackExtensions extension);

	enum ColorExtensions
	{
		ColorExtensionMCT,

		ColorExtensionLAST
	};

	const char** getColorExtensions();
	const char* getColorExtension(ColorExtensions extension);
	const char* getColorDescription(ColorExtensions extension);
};

#endif
