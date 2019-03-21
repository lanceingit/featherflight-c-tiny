#pragma once

typedef struct 
{
    float state;
    float k;
} lpf1p_s;

typedef struct
{
    float cutoff_freq;
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
    float delay_element_1;        // buffered sample -1
    float delay_element_2;        // buffered sample -2
} lpf2p_s;


void lpf2p_init(lpf2p_s* self, float sample_freq, float cutoff_freq);
float lpf2p_reset(lpf2p_s* self, float sample);
float lpf2p_apply(lpf2p_s* self, float sample);

void lpf1p_init(lpf1p_s* self, float sample_freq, float cutoff_freq);
float lpf1p_apply(lpf1p_s* self, float sample);

float lpfrc_apply(float last, float in, float k);

