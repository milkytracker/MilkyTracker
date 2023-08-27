#include "Synth.h"
#include <math.h>
#include "SampleEditor.h"
#include "DialogSliders.h"
#include "TrackerConfig.h"

Synth::Synth(){
  init();
  // assign default synth 
  synth = &synths[0];
}

Synth::~Synth(){
}

PPString Synth::toString(  ) {
  PPString preset = "milk:                 "; // 22 chars (=max samplename)
  char * str = (char *)preset.getStrBuffer();
  for( int i = 0; i < SYN_PARAMS_MAX; i++ ){ 
    float positiveRange = synth->param[i].max - synth->param[i].min;
    str[ i + SYN_PREFIX_CHARS] = (int)synth->param[i].value + SYN_OFFSET_CHAR;  
  }
  return PPString(str);
}

bool Synth::load( PPString preset ) {
  if( preset.startsWith(SYN_PREFIX) ){ // set synth + params
    const char *str = preset.getStrBuffer();
    int ID = str[ SYN_PREFIX_CHARS ] - SYN_OFFSET_CHAR; 
    synth = &(synths[ID]);
    for( int i = 0; i < preset.length() && i < SYN_PARAMS_MAX; i++ ){ 
       setParam(i, str[ i + SYN_PREFIX_CHARS ] - SYN_OFFSET_CHAR ); 
    }
    return true;
  }else return false;
}

void Synth::reset(){
  for( int i = 0; i < SYN_PARAMS_MAX; i++){ 
    synth->param[i].value = 0.0f;
    synth->param[i].name  = PPString("");
  }
}

DialogSliders * Synth::dialog( SampleEditor *s, PPScreen *screen, DialogResponder *dr ){
  if( s != NULL && screen != NULL && dr != NULL ){
    this->sampleEditor = s;
    this->screen = screen;
    this->dr = dr;
  }else{
    sliders->show(false);
    delete sliders;
  }
  sliders = new DialogSliders( this->screen, this->dr, PP_DEFAULT_ID, "milky synths", synth->nparams, this->sampleEditor, &SampleEditor::tool_synth );
  sliders->show();
  for( int i = 0; i < synth->nparams && i < SYN_PARAMS_MAX; i++){
    sliders->initSlider(i, (int)synth->param[i].min, (int)synth->param[i].max, synth->param[i].value, synth->param[i].name, i == 0 ? (PPColor *)&TrackerConfig::colorPatternEditorNote: NULL );
  }
  return sliders;
}

void Synth::setParam( int i, float v ){
  synth->param[i].value = v;
} 
