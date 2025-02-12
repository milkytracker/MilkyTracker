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
#include "Tracker.h"

#define SYN_PREFIX_V1 "M1"                           // samplename 'M<version><params>' hints XM editors that sample was created with milkysynth <version> using <params> 
#define SYN_PREFIX_CHARS 2                           // "M*"
#define SYN_PARAMS_MAX MP_MAXTEXT-SYN_PREFIX_CHARS   // max samplechars minus "M*" (32-2=30)     
#define SYN_OFFSET_CHAR 40                           // printable chars only ascii (dec) 40..126 = 0..86 
#define SYN_PARAM_MAX_VALUE 86                       // 86 printable chars (which allows textual ascii copy/paste of synths in the future)
#define SYN_PARAM_NORMALIZE(x) (1.0f/(float)SYN_PARAM_MAX_VALUE)*x
#define NOTE2HZ(m) (440.0 * pow(2, (m - 69) / 12.0)) 
#define NOTE_START 60                                // C3
                                                    
// synth ID's
#define SYNTH_FM      0                  //
#define SYNTH_CYCLE   1                  // incremental numbers
#define SYNTH_PL      2                  //
#define SYNTH_UNZ     3                  //
#define SYNTH_LAST  SYNTH_UNZ             // update this when adding a synth
#define SYNTH_PRESETS 47

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
	int random_index;
	bool additive;
    MSynth *synth;
    MSynth synths[SYNTH_LAST+1];
    DialogSliders *sliders;

    SampleEditor *sampleEditor;
    PPScreen *screen;
    DialogResponder *dr;
    Tracker *tracker;

	// ASCIISYNTH PRESETS: https://github.com/coderofsalvation/ASCIISYNTH
	PPString preset[SYNTH_PRESETS] = {      // NOTE: update PRESETS_TOTAL when adding synths
		"M1(C*(51)D)vA)/)M),(Xt@(*(((((((", // FM piano          
		"M1)8n)(/()((((((((((((((((((((((", // CYCLE organ        
		"M1*l(?(0+(((((*~S)IM((.d)(((((((", // SONANT organ
		"M1(J+@?9G])~+*~)()<,*{VM)(((((((", // FM bell
		"M1)9n,(0()((((((((((((((((((((((", // CYCLE brass
		"M1*l(/646((M*(*~S(=~((-x+(((((((", // SONANT airchamber 
		"M1(5+85,GF)~i+U*Ds+<2}~c)(((((((", // FM bell
		"M1*S2c(2-(((((*SS)5F((*H+(((((((", // sonant piano
		"M1+a[|w\\l4j(eAF(2)Z2-)((((((((((", // UNZ kick
		"M1+a[xw1~4U(Hj7(28V2/)((((((((((", // UNZ acoust kick
	    "M1(Y)(+),()b9+()U.)),;((((((((((", // FM 909 kick
	    "M1(Y)(*),3)p9+()U,)),;((((((((((", // FM 909 kick2
        "M1+l[[w|[Zl9bj7[8_V2/0((((((((((", // UNZ snare
        "M1+d[Qw|y2p9bj7(<__2/D((((((((((", // UNZ snare 2
		"M1(~*()+*4)~U-~)s,)0,;8m((((((((", // FM snare
		"M1+/y~(~~)(X))-26~q-9)F(((((((((", // UNZ hihat
		"M1(q*(06*u-8~*~+()9(*~9}((((((((", // FM hihat
		"M1+Jy|W~]cU,h>(@[)(*))((((((((((", // UNZ tom
        "M1(~)(+9+()~5)~,BB(o=xFo)(((((((", // FM flute
		"M1*S(k(2-(((((*SS)DO((0H*(((((((", // PL junglepad
		"M1(V)(+)(()1,-})n/0(@b7(((((((((", // FM
		"M1(++85,GJ-~J*b.(<A~3}v_)(((((((", // FM
		"M1(L)(>((F+p@+W-()/n/Z\(*(((((((", // FM
		"M1(7)(>((F-p@+W-()/~/xFa*(((((((", // FM
		"M1(J)(IG,F,pj*~*o))X/x((*(((((((", // FM
		"M1(A)(BG*/+~t*{*()(;/x~()(((((((", // FM
		"M1(Q*(E1)u+vP)/)@),(,M@(*(((((((", // FM
		"M1(H+ioL)M)~X)[)()889{6(*(((((((", // FM
		"M1(A+ioL,0)~X)[)()A89{6(*(((((((", // FM
		"M1(I)(B.)~-~f-U.b[(~2}:Z*(((((((", // FM
		"M1(9*(51,X)O/*X+B~7l~{Za)(((((((", // FM 
		"M1(?*A:@,(+vn,1*,Q1/A{I()(((((((", // FM
		"M1(M+(*-*;+fZ)`,1Q9(A{X(*(((((((", // FM
		"M1(T+()0+d)d~)(,--W(+{\(*(((((((", // FM
		"M1(X*().+O,[~)(,~)[(+{((*(((((((", // FM
		"M1(B,(*.+i+b~-1*g)<~.{_(*(((((((", // FM
		"M1(@*M;F+i+U~-(.()2(.t(()(((((((", // FM 
		"M1(3*:0:+~,AZ)r.`44~;~ea)(((((((", // FM
		"M1(E-?5F,R)vK)A*()5(XtwJ)(((((((", // FM bg
		"M1(?*85,,G)~()6*~4,(2}~()(((((((", // FM tone
										
		"M1)\\~)((()((((((((((((((((((((((",	// CYCLE
		"M1)\\~+)*J*((((((((((((((((((((((", // CYCLE
		"M1)R~,h/()((((((((((((((((((((((", // CYCLE
		"M1)G~,(/()((((((((((((((((((((((", // CYCLE
		"M1)T~,((.)((((((((((((((((((((((", // CYCLE
		"M1)M~-(9()(((((((((((((((((((((("  // CYCLE
	};



  public:
    Synth(int samplerate);
    ~Synth();
    DialogSliders * dialog();
	void update();

    void setParam( int i, float v);
    MSynthParam& getParam( int i ){ return synth->param[i]; }
    int getMaxParam(){ return synth->nparams; }
    
	// spec here: https://github.com/coderofsalvation/ASCIISYNTH
	PPString ASCIISynthExport( );
    bool ASCIISynthImport( PPString preset );

    void reset();
    void init();
    void attach( SampleEditor *s, PPScreen *screen, DialogResponder *dr, Tracker *tracker);
	void random();
    void process( MSynth *s, PPString *preset );
	TXMSample * prepareSample(pp_uint32 duration);
	void setSampleEditor( SampleEditor *s ){ this->sampleEditor = s; }

    // synths
    void Cycle( bool init = false );
    void FM( bool init = false );
    void PL( bool init = false );
    void UNZ( bool init = false );

};

#endif
