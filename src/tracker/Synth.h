/*
 *  tracker/Synth.h 
 *
 *  Copyright 2023 Leon van Kammen (coderofsalvation)
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
 * Lightweight synths from the milkyverse.
 * Rules for new synths:
 *   1. don't introduce new classes/FFT libraries e.g., KISS.
 *   2. name your synth as a planet (Jupiter e.g.) in our milkyway
 */

#ifndef __SYNTH_H
#define __SYNTH_H

#include "BasicTypes.h"
#include "XModule.h"
#include "SampleEditor.h"
#include "Screen.h"
#include "DialogSliders.h"

#define PREFIX_CHARS 5                        // "milk:"
#define PARAMS_MAX 22-PREFIX_CHARS            // max samplechars (22) minus "milk:" (5)
#define OFFSET_CHAR 32                        // printable chars only 32..127 = 0..92
#define PARAM_TO_FLOAT(x) (1.0f/92)*(float)x  // 0..92      -> 0.0f..1.0f
#define FLOAT_TO_PARAM(x) (int)(x/(0.99f/92)) // 0.0f..1.0f -> 0..92

#ifndef M_PI
#define M_PI   3.14159265358979323846264338327950288
#endif

struct MSynthParam{
  PPString name;
  float value;
};

struct MSynth{
  PPString name;
  MSynthParam param[PARAMS_MAX];
};
	

class Synth
{

  private:
    MSynth synth;

  public:
    Synth();
    ~Synth();
    DialogSliders * create( SampleEditor *s, PPScreen *screen, DialogResponder *dr);
    PPString exportPreset( MSynth &m );
    bool importPreset( MSynth &m, PPString preset );
};

#endif
