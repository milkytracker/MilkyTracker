#include "Synth.h" 
#include "DialogSliders.h"
#include "FilterParameters.h"

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
  MilkySine(true); 
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
    case SYNTH_MILKY_SINE:    MilkySine();   break;
    case SYNTH_NEBULA_DRUM:   NebulaDrum();  break;
  }
}

void Synth::MilkySine( bool init ){
  pp_int32 ID = SYNTH_MILKY_SINE;

  if( init ){
    synths[ID].nparams = 4;
    synths[ID].ID      = ID;
    synths[ID].param[0].name  = "milky sine";
    synths[ID].param[0].value = 0.0f;
    synths[ID].param[0].min   = 0;
    synths[ID].param[0].max   = SYNTH_TOTAL;

    synths[ID].param[1].name  = "volume";
    synths[ID].param[1].value = 50.0f;
    synths[ID].param[1].min   = 0;
    synths[ID].param[1].max   = 100.0f;

    synths[ID].param[2].name  = "samples";
    synths[ID].param[2].value = 128.0f;
    synths[ID].param[2].min   = 0;
    synths[ID].param[2].max   = 5000.0f;

    synths[ID].param[3].name  = "periods";
    synths[ID].param[3].value = 3.0f;
    synths[ID].param[3].min   = 1.0f;
    synths[ID].param[3].max   = 100.0f;

    synths[ID].param[4].name  = "wavetype";
    synths[ID].param[4].value = 1.0f;
    synths[ID].param[4].min   = 1.0f;
    synths[ID].param[4].max   = 6.0f;
    return;
  }

  // processing
  if( sampleEditor->isEmptySample() ){
    FilterParameters par(2);
    par.setParameter(0, FilterParameters::Parameter( (pp_int32)synth->param[2].value ) );
    par.setParameter(1, FilterParameters::Parameter( 16 ) );
    sampleEditor->tool_newSample(&par);
  }

  FilterParameters parWave(2);
  parWave.setParameter(0, FilterParameters::Parameter( synth->param[1].value / 100.0f ));
  parWave.setParameter(1, FilterParameters::Parameter( synth->param[3].value ) );
  switch( (int)synth->param[4].value ){
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
    synths[ID].nparams = 3;
    synths[ID].ID      = ID;
    synths[ID].param[0].name  = "nebula kick";
    synths[ID].param[0].value = 0.0f;
    synths[ID].param[0].min   = 0;
    synths[ID].param[0].max   = SYNTH_TOTAL;

    synths[ID].param[1].name  = "attack";
    synths[ID].param[1].value = 0.0f;
    synths[ID].param[1].min   = 0;
    synths[ID].param[1].max   = 100.0f;
    return;
  }

  // processing
  FilterParameters par(2);
  par.setParameter(0, FilterParameters::Parameter( (pp_int32)synth->param[1].value ) );
  par.setParameter(1, FilterParameters::Parameter( 16 ) );
  sampleEditor->tool_newSample(&par);
}
