/*
 *  tracker/Synth.h 
 *
 *  Copyright 2023 Leon van Kammen (coderofsalvation)
 * 
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

#ifndef __SYN_H
#define __SYN_H

#include "BasicTypes.h"
#include "XModule.h"
#include "Screen.h"
#include "Event.h"

#define SYN_PREFIX "milk:" 
#define SYN_PREFIX_CHARS 5                           // "milk:"
#define SAMPLE_CHARS 22                              // max samplechars
#define SYN_PARAMS_MAX SAMPLE_CHARS-SYN_PREFIX_CHARS // max samplechars minus "milk:" (5)     
#define SYN_OFFSET_CHAR 32                           // printable chars only 32..127 = 0..92
#define SYN_PARAM_MAX_VALUE 92                       // 92 printable chars
#define SYN_PARAM_NORMALIZED(x) (1.0f/(float)SYN_PARAM_MAX_VALUE)*x
                                                    
#define SYNTH_TOTAL       2  // update when adding synths 
// synth ID's
#define SYNTH_MILKY_WAVE  0
#define SYNTH_NEBULA_DRUM 1

#ifndef M_PI
#define M_PI   3.14159265358979323846264338327950288
#endif

struct MSynthParam{
  PPString name;
  float value;
  int min;
  int max;
};

struct MSynth{
  PPString name;
  MSynthParam param[SYN_PARAMS_MAX];
  int nparams;
  pp_uint32 ID;
  bool inited;
};
	
class SampleEditor; // forward
class DialogSliders;                    
class DialogResponder;

class Synth
{

  private:
    MSynth *synth;
    MSynth synths[SYNTH_TOTAL];
    DialogSliders *sliders;

    SampleEditor *sampleEditor;
    PPScreen *screen;
    DialogResponder *dr;

  public:
    Synth();
    ~Synth();
    DialogSliders * dialog( SampleEditor *s, PPScreen *screen, DialogResponder *dr);

    void setParam( int i, float v);
    MSynthParam& getParam( int i ){ return synth->param[i]; }
    int getMaxParam(){ return synth->nparams; }
    
    PPString toString();
    bool load( PPString preset );

    void reset();
    void init();
    void process( MSynth *s, PPString *preset );

    // synths
    void MilkyWave( bool init = false );
    void NebulaDrum( bool init = false );

};

#endif
