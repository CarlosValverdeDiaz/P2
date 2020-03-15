#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

float pow_ref=-100;
float zrc_ref=0;
int count=0;
int sov=0; //si es maybe voice estará a 1 y si es maybe silence a 0
int strikes=0; //numero de tramas por los que no pasara de UNDEF a S o V (en nuestro caso seran 10)

Features compute_features(const float *x, int N, float fm) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  Features feat;
  feat.am = compute_am(x,N);
  feat.p = compute_power(x,N,fm);
  feat.zcr = compute_zcr(x,N,fm);

  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */
  
  Features f = compute_features(x, vad_data->frame_length, vad_data->sampling_rate);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */
  
  switch (vad_data->state) {
  case ST_INIT:
    if(count==0){
      pow_ref = f.p;
      zrc_ref = f.zcr;
    }
    count++;
    if(pow_ref < f.p){
      pow_ref = f.p;
    }
    if (zrc_ref < f.zcr){
      zrc_ref = f.zcr;
    }
    if (count > 3){
      pow_ref = fabs(0.095*pow_ref)+pow_ref;
      zrc_ref = zrc_ref*1.45;
      count = 0;
      vad_data->state = ST_SILENCE;
    }


    break;

  case ST_SILENCE:
    if (f.p > pow_ref || f.zcr >= zrc_ref){
      vad_data->state = ST_UNDEF;
      sov = 1;
    }
    break;

  case ST_VOICE:
    if (f.p < pow_ref || f.zcr >= zrc_ref)
      vad_data->state = ST_UNDEF;
      sov = 0;
    break;

  case ST_UNDEF:
    count++;
    switch (sov)
    {
    case 1: // maybe-voice
      if (f.p < pow_ref || f.zcr >= zrc_ref){
        strikes++;
      }
      if (strikes >= 2){
        vad_data->state = ST_SILENCE;
        count=0;
        strikes=0;
      }else if (count >= 5){
        vad_data->state = ST_VOICE;
        count=0;
        strikes=0;
      }
      break;
    
    case 0: // maybe-silence
      if (f.p > pow_ref || f.zcr >= zrc_ref){
        strikes++;
      }
      if (strikes >= 2){
        vad_data->state = ST_VOICE;
        count=0;
        strikes=0;
      }else if (count >= 9){
        vad_data->state = ST_SILENCE;
        count=0;
        strikes=0;
      }
      
      
      break;
    }
    break;
  }

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE){
    return vad_data->state;
  }else if(vad_data->state == ST_INIT){
    return ST_SILENCE;
  }else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
