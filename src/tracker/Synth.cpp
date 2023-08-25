#include "Synth.h"
#include <math.h>
#include "SampleEditor.h"
#include "DialogSliders.h"

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
    str[ i + SYN_PREFIX_CHARS] = SYN_FLOAT_TO_PARAM( synth->param[i].value ) + SYN_OFFSET_CHAR;  
  }
  printf( "preset=%s\n", PPString(str).getStrBuffer() );
  return PPString(str);
}

bool Synth::load( PPString preset ) {
  if( preset.startsWith(SYN_PREFIX_CHARS) ){
    const char *str = preset.getStrBuffer();
    printf("load %s\n", str);
    // switch to synth
    int ID = str[ SYN_PREFIX_CHARS ] - SYN_OFFSET_CHAR; 
    synth = &(synths[ID]);
    // set params
    for( int i = 0; i < SYN_PARAMS_MAX; i++ ){ 
        synth->param[i].value = str[ i + SYN_PREFIX_CHARS ] - SYN_OFFSET_CHAR; 
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
    sliders->initSlider(i, (int)synth->param[i].min, (int)synth->param[i].max, synth->param[i].value, synth->param[i].name );
  }
  return sliders;
}

void Synth::setParam( int i, float v ){
  synth->param[i].value = v;
  printf("%i = %f\n",i,v);
} 
