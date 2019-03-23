#pragma once

typedef struct 
{
    float state;
    float k;
} Lpf1p;

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
} Lpf2p;


void lpf2p_init(Lpf2p* self, float sample_freq, float cutoff_freq);
float lpf2p_reset(Lpf2p* self, float sample);
float lpf2p_apply(Lpf2p* self, float sample);

void lpf1p_init(Lpf1p* self, float sample_freq, float cutoff_freq);
float lpf1p_apply(Lpf1p* self, float sample);

float lpfrc_apply(float last, float in, float k);

