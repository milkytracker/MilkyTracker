/*
 *  tracker/FileExtProvider.cpp
 *
 *  Copyright 2009 Peter Barth
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
 *  FileExtProvider.cpp
 *  milkytracker_universal
 *
 *  Created by Peter Barth on 12.07.08.
 *
 */

#include "FileExtProvider.h"
#include "Decompressor.h"

const char* FileExtProvider::moduleExtensions[] = 
{
	"669","Composer669",
	"amf","Asylum Music Format",
	"ams","Velvet Studio/Extreme Tracker",
	"cba","Chuck Biscuits & Black Artist",
	"dbm","Digibooster Pro",
	"digi","Digibooster",
	"dsm","Digisound Interface Kit/Dynamic Studio",
	"dtm","Digital Tracker/Digitrekker",
	"far","Farandole Composer",
	"gdm","General Digimusic",
	"gmc","Game Music Creator",
	"imf","Imago Orpheus",
	"it","Impulse Tracker",
	"mdl","Digitrakker",
	"mod","Protracker",
	"mtm","Multitracker",
	"mxm","Cubic Tiny XM",
	"okt","Oktalyzer",
	"plm","DisorderTracker",
	"psm","Epic Megagames MASI",
	"ptm","Polytracker",
	"s3m","Screamtracker 3",
	"stm","Screamtracker 2",
	"ult","Ultratracker",
	"uni","MikMod",
	"xm","Fasttracker 2",
	NULL, NULL
};

const char* FileExtProvider::instrumentExtensions[] = 
{
	"xi","FT2 Instruments",
	"pat","GUS Patches",
	NULL, NULL
};

const char* FileExtProvider::sampleExtensions[] = 
{
	"wav","WAV uncompressed",
	"iff","IFF (un)compressed",
	"8svx","IFF (un)compressed",
	"aif","Apple AIFF uncompressed",
	"aiff","Apple AIFF uncompressed",
	NULL, NULL
};

const char* FileExtProvider::patternExtensions[] = 
{
	"xp","FT2 Pattern",
	NULL, NULL
};

const char* FileExtProvider::trackExtensions[] = 
{
	"xt","FT2 Track",
	NULL, NULL
};

const char* FileExtProvider::colorExtensions[] = 
{
	"mct","MilkyTracker colors",
	NULL, NULL
};

const char* const* FileExtProvider::fillList(const char* const* baseList, ExtensionTypes type)
{
	tempExtensions.clear();

	pp_int32 i = 0;
	while (true)
	{
		if (baseList[i] == NULL)
			break;
	
		tempExtensions.add(new PPString(baseList[i]));
		
		i++;
	}
	
	// misuse a decompressor to retrieve the file types it can decompress
	// they're not ordered though
	Decompressor decompressor("");

	if (decompressor.doesServeHint((DecompressorBase::Hints)type))
	{
		const PPSimpleVector<Descriptor>& src = decompressor.getDescriptors((DecompressorBase::Hints)type);
	
		for (pp_int32 j = 0; j < src.size(); j++)
		{
			tempExtensions.add(new PPString(src.get(j)->extension));
			tempExtensions.add(new PPString(src.get(j)->description));
			i+=2;
		}
	}

	delete[] tempList;
	tempList = new char*[i+2];
	
	pp_int32 j = 0;
	for (j = 0; j < i; j++)
	{
		tempList[j] = (char*)((const char*)(*tempExtensions.get(j)));
	}
	tempList[j++] = NULL;
	tempList[j++] = NULL;

	return (const char**)tempList;
}

const char* const* FileExtProvider::getModuleExtensions()
{
	return fillList(moduleExtensions, ExtensionTypeModules);
}

const char* FileExtProvider::getModuleExtension(ModuleExtensions extension) { return moduleExtensions[extension*2]; }
const char* FileExtProvider::getModuleDescription(ModuleExtensions extension) { return moduleExtensions[extension*2+1]; }

const char* const* FileExtProvider::getInstrumentExtensions()
{
	return fillList(instrumentExtensions, ExtensionTypeInstruments);
}

const char* FileExtProvider::getInstrumentExtension(InstrumentExtensions extension) { return instrumentExtensions[extension*2]; }
const char* FileExtProvider::getInstrumentDescription(InstrumentExtensions extension) { return instrumentExtensions[extension*2+1]; }

const char* const* FileExtProvider::getSampleExtensions()
{
	return fillList(sampleExtensions, ExtensionTypeSamples);
}

const char* FileExtProvider::getSampleExtension(SampleExtensions extension) { return sampleExtensions[extension*2]; }
const char* FileExtProvider::getSampleDescription(SampleExtensions extension) { return sampleExtensions[extension*2+1]; }

const char* const* FileExtProvider::getPatternExtensions()
{
	return fillList(patternExtensions, ExtensionTypePatterns);
}

const char* FileExtProvider::getPatternExtension(PatternExtensions extension) { return patternExtensions[extension*2]; }
const char* FileExtProvider::getPatternDescription(PatternExtensions extension) { return patternExtensions[extension*2+1]; }

const char* const* FileExtProvider::getTrackExtensions()
{
	return fillList(trackExtensions, ExtensionTypeTracks);
}

const char* FileExtProvider::getTrackExtension(TrackExtensions extension) { return trackExtensions[extension*2]; }
const char* FileExtProvider::getTrackDescription(TrackExtensions extension) { return trackExtensions[extension*2+1]; }

const char* const* FileExtProvider::getColorExtensions()
{
	return colorExtensions;
}

const char* FileExtProvider::getColorExtension(ColorExtensions extension) { return colorExtensions[extension*2]; }
const char* FileExtProvider::getColorDescription(ColorExtensions extension) { return colorExtensions[extension*2+1]; }	

