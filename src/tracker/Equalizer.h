/*
 *  tracker/Equalizer.h
 *
 *  Copyright 2009 David Ross (david_ross@hotmail.co.uk)
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
#ifndef __EQUALIZER_H__
#define __EQUALIZER_H__

#include "math.h"

class Equalizer
{
public:
	Equalizer(void);
	~Equalizer(void);

	void CalcCoeffs(float centre, float width, float rate, float gain);
	void Filter(double xL, double xR, double &yL, double &yR);

	// Calculate frequency from 20Hz to 20,000 Hz, a value of 0 to 1 should be passed (as is normally used in linear controls)
	static float CalcFreq(float f) { return (float)(pow(1000.0f,f)*20); }

	// Calculate Gain value, which is passed when you CalcCoeffs(), a value from 0 to 1 should be passed, this will result in a -12 to 12 dB cut \ boost. i.e. 0 = -12dB, 0.5 = 0dB, 1 = 12dB
	static float CalcGain(float val)
	{
		float dB = 12.0f * ((2.0f * val) - 1.0f);
		float gain = (float)pow(10.0f, (dB / 20.0f));
		return (gain);
	}

private:
	double b0, b1, b2;
	double a0, a1, a2;

	double xL1, xL2;
	double xR1, xR2;
	double yL1, yL2;
	double yR1, yR2;
};

#endif
