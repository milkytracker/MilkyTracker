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
		
			key = dictionary.getNextKey();
		}
	}
};

#endif
