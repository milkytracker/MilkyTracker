#include "Synth.h"
#include <math.h>

Synth::Synth(){
}

Synth::~Synth(){
}

PPString Synth::exportPreset( MSynth &m ) {
  return PPString("milk:ASDFASDFAS");
}

bool Synth::importPreset( MSynth &m, PPString preset) {
  return false; 
}

DialogSliders * Synth::create( SampleEditor *s, PPScreen *screen, DialogResponder *dr ){
  DialogSliders *sliders = new DialogSliders(screen, dr, PP_DEFAULT_ID, "Synth", PARAMS_MAX, s, &SampleEditor::tool_scaleSample );
  for( int i = 0; i < PARAMS_MAX; i++){
    sliders->initSlider(i,0.0f, 300.0f, 100.f,"?");
  }
  return sliders;
}
