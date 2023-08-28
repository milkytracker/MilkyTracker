#include "Synth.h" 
#include "DialogSliders.h"
#include "FilterParameters.h"
#include "XModule.h"
#include "SampleEditor.h"
#include <math.h>

/*
 * Lightweight synths from the milkyverse.
 *
 * Rules for new synths:
 *   1. don't introduce new classes/FFT libraries e.g., KISS.
 *   2. re-use SampleEditor operations (recursive sample-editing)  
 *   3. synth naming convention: <astrologyterm> <topic> ('Jupiter kick' e.g.)
 *   4. please use volume as first slider
 */


void Synth::init(){
  MilkyWave(true); 
  NebulaDrum(true);
}

void Synth::process( MSynth *s, PPString *preset ){

  if( s == NULL ){  //fetch current synth from param 0
    pp_uint32 ID = (int)getParam(0).value;
    s = &(synths[ID]);
  }

  // if slider 0 changed, change synth + relaunch dialog
  if( s->ID != synth->ID ){     
    synth  = s;                 
    synth->param[0].value = (float)synth->ID;
    dialog(NULL,NULL,NULL);
  }

  switch( s->ID ){
    case SYNTH_MILKY_WAVE:    MilkyWave();   break;
    case SYNTH_NEBULA_DRUM:   NebulaDrum();  break;
  }
}

void Synth::MilkyWave( bool init ){
  pp_int32 ID = SYNTH_MILKY_WAVE;

  if( init ){
    synths[ID].nparams = 4;
    synths[ID].ID      = ID;
    synths[ID].param[0].name  = "milky wave";
    synths[ID].param[0].value = 0.0f;
    synths[ID].param[0].min   = 0;
    synths[ID].param[0].max   = SYNTH_TOTAL;

    synths[ID].param[1].name  = "volume";
    synths[ID].param[1].value = 75.0f;
    synths[ID].param[1].min   = 0;
    synths[ID].param[1].max   = 90.0f;

    synths[ID].param[2].name  = "generator";
    synths[ID].param[2].value = 1.0f;
    synths[ID].param[2].min   = 1.0f;
    synths[ID].param[2].max   = 6.0f;

    synths[ID].param[3].name  = "periods";
    synths[ID].param[3].value = 3.0f;
    synths[ID].param[3].min   = 1.0f;
    synths[ID].param[3].max   = (float)SYN_PARAM_MAX_VALUE;

    return;
  }

  // processing
  if( sampleEditor->isEmptySample() ){
    FilterParameters par(2);
    par.setParameter(0, FilterParameters::Parameter( 128 ) );
    par.setParameter(1, FilterParameters::Parameter( 16 ) );
    sampleEditor->tool_newSample(&par);
    //const PPString TrackerConfig::defaultPredefinedVolumeEnvelope("060203050700000000C000040100000800B0000E00200018005800200020");
    //ins->volfade = 0x0500;
  }

  FilterParameters parWave(2);
  parWave.setParameter(0, FilterParameters::Parameter( SYN_PARAM_NORMALIZED(synth->param[1].value) ));
  parWave.setParameter(1, FilterParameters::Parameter( synth->param[3].value ) );
  switch( (int)synth->param[2].value ){
    case 1: sampleEditor->tool_generateSine(&parWave);         break;
    case 2: sampleEditor->tool_generateSquare(&parWave);       break;
    case 3: sampleEditor->tool_generateTriangle(&parWave);     break;
    case 4: sampleEditor->tool_generateSawtooth(&parWave);     break;
    case 5: sampleEditor->tool_generateHalfSine(&parWave);     break;
    case 6: sampleEditor->tool_generateAbsoluteSine(&parWave); break;
  }

  sampleEditor->setLoopType(1);
}

void Synth::NebulaDrum( bool init ){
  pp_int32 ID = SYNTH_NEBULA_DRUM;

  if( init ){
    synths[ID].nparams = 4;
    synths[ID].ID      = ID;
    synths[ID].param[0].name  = "nebula drum";
    synths[ID].param[0].value = 0.0f;
    synths[ID].param[0].min   = 0;
    synths[ID].param[0].max   = SYNTH_TOTAL;

    synths[ID].param[1].name  = "attack";
    synths[ID].param[1].value = 2.0f;
    synths[ID].param[1].min   = 0;
    synths[ID].param[1].max   = (float)SYN_PARAM_MAX_VALUE;
    
    synths[ID].param[2].name  = "decay";
    synths[ID].param[2].value = 5.0f;
    synths[ID].param[2].min   = 0;
    synths[ID].param[2].max   = (float)SYN_PARAM_MAX_VALUE;

    synths[ID].param[3].name  = "release";
    synths[ID].param[3].value = 10.0f;
    synths[ID].param[3].min   = 0;
    synths[ID].param[3].max   = (float)SYN_PARAM_MAX_VALUE;

    synths[ID].param[4].name  = "note";
    synths[ID].param[4].value = 40.0f;
    synths[ID].param[4].min   = 0;
    synths[ID].param[4].max   = (float)SYN_PARAM_MAX_VALUE;
    return;
  }

  // processing
  FilterParameters par(2);
  pp_int32 segment  = 100;
  pp_int32 attack   = segment * (int)synth->param[1].value;
  pp_int32 decay    = segment * (int)synth->param[2].value;
  pp_int32 release  = segment * (int)synth->param[3].value;
  pp_int32 note     = segment * (int)synth->param[4].value;
  pp_int32 duration = attack + decay + release;
  par.setParameter(0, FilterParameters::Parameter( duration ) );
  par.setParameter(1, FilterParameters::Parameter( 16 ) );
  sampleEditor->tool_newSample(&par);

  //if srate == nil then srate=44100 end
  //if note  == nil then note=-24+(rand%6)*-12 end
  //x = math.sin( ((2*math.pi)*(i*hz(note))/srate) )
  //if( i < 50 ) then return 0 end
  //return mirror( fadeout(x,i,synths.perc.samples,srate) * 1.1)
  for( pp_int32 i = 0; i < duration; i++ ){
    if( i < 50 ) sampleEditor->setFloatSampleInWaveform( i, 0.0f);
    float x = sin( (2.0f*M_PI)*(float(i) * NOTE2HZ(note)) / 44100.0f );
    if( i < attack ) x = x * ((1.0f/attack)*float(i));
    if( i > attack+decay && i < duration ) x = x * (1.0f-(1.0f/attack)*float(i));
    sampleEditor->setFloatSampleInWaveform( i, x);
  }

	sampleEditor->notifyListener(SampleEditor::NotificationChanges); // update UI
}

//function hz(note)
//    local a = 440  -- frequency of A (note 49=440Hz   note 40=C4)
//    local n = note + 52
//    return 440 * (2 / ((n  - 49) / 12));
//end
