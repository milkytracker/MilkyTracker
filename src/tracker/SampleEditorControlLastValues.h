/*
 *  tracker/SampleEditorControlLastValues.h
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

#ifndef __SAMPLEEDITORCONTROLASTVALUES_H__
#define __SAMPLEEDITORCONTROLASTVALUES_H__

#include "BasicTypes.h"
#include "Dictionary.h"

// Last values
struct SampleEditorControlLastValues
{
	pp_int32 newSampleSize;
	pp_int32 changeSignIgnoreBits;
	float saturator[5];
	float milkyexcite[4];
	float boostSampleVolume;
	float fadeSampleVolumeStart;
	float fadeSampleVolumeEnd;
	float DCOffset;
	pp_int32 silenceSize;
	float waveFormVolume;
	float waveFormNumPeriods;
	
	bool hasEQ3BandValues;
	float EQ3BandValues[3];
	
	bool hasEQ10BandValues;
	float EQ10BandValues[10];
	
	pp_int32 resampleInterpolationType;
	bool adjustFtAndRelnote;
	bool adjustSampleOffsetCommand;

	float reverbSize; 
	float reverbDryWet;
	pp_int32 IRSample; 
	float filterCutoffL;
	float filterCutoffH;
	float filterRes;
	float filterSweep;
	
	static float invalidFloatValue() 
	{
		return -12345678.0f;
	}
	
	static int invalidIntValue() 
	{
		return -12345678;
	}
	
	void reset()
	{
		newSampleSize = invalidIntValue();
		changeSignIgnoreBits = invalidIntValue();
		boostSampleVolume = invalidFloatValue();
		fadeSampleVolumeStart = invalidFloatValue();
		fadeSampleVolumeEnd = invalidFloatValue();
		DCOffset = invalidFloatValue();
		silenceSize = invalidIntValue();
		waveFormVolume = invalidFloatValue();
		waveFormNumPeriods = invalidFloatValue();
		hasEQ3BandValues = hasEQ10BandValues = false;
		resampleInterpolationType = invalidIntValue();
		adjustFtAndRelnote = true;
		adjustSampleOffsetCommand = false;
		reverbSize = invalidFloatValue();
		reverbDryWet = invalidFloatValue();
		IRSample = invalidIntValue();
		filterCutoffL = invalidFloatValue();
		filterCutoffH = invalidFloatValue();
		filterRes = invalidFloatValue();
		filterSweep = invalidFloatValue();
		for( pp_uint8 i = 0; i < 5; i++) saturator[i] = invalidFloatValue();
		for( pp_uint8 i = 0; i < 4; i++) milkyexcite[i] = invalidFloatValue();
	}
		
	PPDictionary convertToDictionary()
	{
		PPDictionary result;
		
		result.store("newSampleSize", newSampleSize);

		result.store("changeSignIgnoreBits", changeSignIgnoreBits);

		result.store("boostSampleVolume", PPDictionary::convertFloatToIntNonLossy(boostSampleVolume));

		result.store("fadeSampleVolumeStart", PPDictionary::convertFloatToIntNonLossy(fadeSampleVolumeStart));
		result.store("fadeSampleVolumeEnd", PPDictionary::convertFloatToIntNonLossy(fadeSampleVolumeEnd));

		result.store("DCOffset", PPDictionary::convertFloatToIntNonLossy(DCOffset));

		result.store("silenceSize", silenceSize);

		result.store("waveFormVolume", PPDictionary::convertFloatToIntNonLossy(waveFormVolume));
		result.store("waveFormNumPeriods", PPDictionary::convertFloatToIntNonLossy(waveFormNumPeriods));

		result.store("resampleInterpolationType", resampleInterpolationType);
		
		result.store("adjustFtAndRelnote", adjustFtAndRelnote);

		result.store("adjustSampleOffsetCommand", adjustSampleOffsetCommand);
		result.store("reverbSize", PPDictionary::convertFloatToIntNonLossy(reverbSize));
		result.store("reverbDryWet", PPDictionary::convertFloatToIntNonLossy(reverbDryWet));
		result.store("IRSample", IRSample );
		result.store("filterCutoffL", PPDictionary::convertFloatToIntNonLossy(filterCutoffL));
		result.store("filterCutoffH", PPDictionary::convertFloatToIntNonLossy(filterCutoffH));
		result.store("filterRes", PPDictionary::convertFloatToIntNonLossy(filterRes));
		result.store("filterSweep", PPDictionary::convertFloatToIntNonLossy(filterSweep));
		
		result.store("saturator_0", PPDictionary::convertFloatToIntNonLossy(saturator[0]));
		result.store("saturator_1", PPDictionary::convertFloatToIntNonLossy(saturator[1]));
		result.store("saturator_2", PPDictionary::convertFloatToIntNonLossy(saturator[2]));
		result.store("saturator_3", PPDictionary::convertFloatToIntNonLossy(saturator[3]));
		result.store("saturator_4", PPDictionary::convertFloatToIntNonLossy(saturator[4]));
		result.store("milkyexcite_0", PPDictionary::convertFloatToIntNonLossy(milkyexcite[0]));
		result.store("milkyexcite_1", PPDictionary::convertFloatToIntNonLossy(milkyexcite[1]));
		result.store("milkyexcite_2", PPDictionary::convertFloatToIntNonLossy(milkyexcite[2]));
		result.store("milkyexcite_3", PPDictionary::convertFloatToIntNonLossy(milkyexcite[3]));
		return result;
	}
	
	void restoreFromDictionary(PPDictionary& dictionary)
	{
		PPDictionaryKey* key = dictionary.getFirstKey();
		while (key)
		{
			if (key->getKey().compareToNoCase("newSampleSize") == 0)
			{
				newSampleSize = key->getIntValue();
			}
			else if (key->getKey().compareToNoCase("changeSignIgnoreBits") == 0)
			{
				changeSignIgnoreBits = key->getIntValue();
			}
			else if (key->getKey().compareToNoCase("boostSampleVolume") == 0)
			{
				boostSampleVolume = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("fadeSampleVolumeStart") == 0)
			{
				fadeSampleVolumeStart = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("fadeSampleVolumeEnd") == 0)
			{
				fadeSampleVolumeEnd = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("DCOffset") == 0)
			{
				DCOffset = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("silenceSize") == 0)
			{
				silenceSize = key->getIntValue();
			}
			else if (key->getKey().compareToNoCase("waveFormVolume") == 0)
			{
				waveFormVolume = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("waveFormNumPeriods") == 0)
			{
				waveFormNumPeriods = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("resampleInterpolationType") == 0)
			{
				resampleInterpolationType = key->getIntValue();
			}
			else if (key->getKey().compareToNoCase("adjustFtAndRelnote") == 0)
			{
				adjustFtAndRelnote = key->getBoolValue();
			}
			else if (key->getKey().compareToNoCase("adjustSampleOffsetCommand") == 0)
			{
				adjustSampleOffsetCommand = key->getBoolValue();
			}
			else if (key->getKey().compareToNoCase("reverbSize") == 0)
			{
				reverbSize = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("reverbDryWet") == 0)
			{
				reverbDryWet = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("IRSample") == 0)
			{
				IRSample = key->getIntValue();
			}
			else if (key->getKey().compareToNoCase("filterCutoffL") == 0)
			{
				filterCutoffL = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("filterCutoffH") == 0)
			{
				filterCutoffH = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("filterRes") == 0)
			{
				filterRes = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("filterSweep") == 0)
			{
				filterSweep = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("saturator_0") == 0)
			{
				saturator[0] = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("saturator_1") == 1)
			{
				saturator[1] = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("saturator_2") == 2)
			{
				saturator[2] = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("saturator_3") == 3)
			{
				saturator[3] = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}
			else if (key->getKey().compareToNoCase("saturator_4") == 4)
			{
				saturator[4] = PPDictionary::convertIntToFloatNonLossy(key->getIntValue());
			}

			key = dictionary.getNextKey();
		}
	}
};

#endif
