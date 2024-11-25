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
#include "XModule.h"

#define SYN_PREFIX_V1 "M1"                           // samplename 'M<version><params>' hints XM editors that sample was created with milkysynth <version> using <params> 
#define SYN_PREFIX_CHARS 2                           // "M*"
#define SYN_PARAMS_MAX MP_MAXTEXT-SYN_PREFIX_CHARS   // max samplechars minus "M*" (32-2=30)     
#define SYN_OFFSET_CHAR 40                           // printable chars only ascii (dec) 40..126 = 0..86 
#define SYN_PARAM_MAX_VALUE 86                       // 86 printable chars (which allows textual ascii copy/paste of synths in the future)
#define SYN_PARAM_NORMALIZE(x) (1.0f/(float)SYN_PARAM_MAX_VALUE)*x
#define NOTE2HZ(m) (440.0 * pow(2, (m - 69) / 12.0)) 
#define NOTE_START 60                                // C3
                                                    
// synth ID's
#define SYNTH_FM    0                  //
#define SYNTH_CYCLE 1                  // incremental numbers
#define SYNTH_LAST        SYNTH_CYCLE  // update this when adding a synth
											 //
#define SYNTH_PRESETS 24

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
	int samplerate;
	bool additive;
    MSynth *synth;
    MSynth synths[SYNTH_LAST+1];
    DialogSliders *sliders;

    SampleEditor *sampleEditor;
    PPScreen *screen;
    DialogResponder *dr;

	// SYNTH PRESETS
	PPString preset[SYNTH_PRESETS] = { // update PRESETS_TOTAL when adding synths
		"M1(N*(51)D)vA)/)M),(Xt@(*(((((((", // FM                                 
		"M1(J+@?9G])~+*~)()<,*{VM)(((((((", // FM
		"M1(5+85,GF)~i+U*Ds+<2}~c)(((((((", // FM
		"M1(V)()(**,1,-(.65A(*{((((((((((", // FM
		"M1(V)(*)(()1,-(.\\\\.5~@b((((((((((",// FM
		"M1(V)(+)(()1,-})n/0(@b7(((((((((", // FM
		"M1(++85,GJ-~J*b.(<A~3}v_)(((((((", // FM
		"M1(L)(>((F+p@+W-()/n/Z\(*(((((((", // FM
		"M1(7)(>((F-p@+W-()/~/xFa*(((((((", // FM
		"M1(J)(IG,F,pj*~*o))X/x((*(((((((", // FM
		"M1(9)(BG*-+~v*{*()(;/x~()(((((((", // FM
		"M1(Q*(E1)u+vP)/)@),(,M@(*(((((((", // FM
		"M1(H+ioL)M)~X)[)()889{6(*(((((((", // FM
		"M1(A+ioL,0)~X)[)()A89{6(*(((((((", // FM
		"M1(I)(B.)~-~f-U.b[(~2}:Z*(((((((", // FM
											//
		"M1)Sn)(/()((((((((((((((((((((((", // CYCLE
		"M1)Sn,(0()((((((((((((((((((((((", // CYCLE
		"M1)Sn,,C<)((((((((((((((((((((((", // CYCLE
		"M1)\\~)((()((((((((((((((((((((((",	// CYCLE
		"M1)\\~+)*J*((((((((((((((((((((((", // CYCLE
		"M1)R~,h/()((((((((((((((((((((((", // CYCLE
		"M1)G~,(/()((((((((((((((((((((((", // CYCLE
		"M1)T~,((.)((((((((((((((((((((((", // CYCLE
		"M1)M~-(9()((((((((((((((((((((((", // CYCLE
	};


  public:
    Synth(int samplerate);
    ~Synth();
    DialogSliders * dialog( SampleEditor *s, PPScreen *screen, DialogResponder *dr);

    void setParam( int i, float v);
    MSynthParam& getParam( int i ){ return synth->param[i]; }
    int getMaxParam(){ return synth->nparams; }
    
	// spec here: https://github.com/coderofsalvation/ASCIISYNTH
	PPString ASCIISynthExport( );
    bool ASCIISynthImport( PPString preset );

    void reset();
    void init();
	void random();
    void process( MSynth *s, PPString *preset );
	TXMSample * prepareSample(pp_uint32 duration);
	void setSampleEditor( SampleEditor *s ){ this->sampleEditor = s; }

    // synths
    void Cycle( bool init = false );
    void FM( bool init = false );

};

#endif
