#pragma once

struct lpf1p_s 
{
    float state;
    float k;
};

struct lpf2p_s
{
    float cutoff_freq;
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
    float delay_element_1;        // buffered sample -1
    float delay_element_2;        // buffered sample -2
};


void lpf2p_init(struct lpf2p_s* filter, float sample_freq, float cutoff_freq);
float lpf2p_reset(struct lpf2p_s* filter, float sample);
float lpf2p_apply(struct lpf2p_s* filter, float sample);

void lpf1p_init(struct lpf1p_s* filter, float sample_freq, float cutoff_freq);
float lpf1p_apply(struct lpf1p_s* filter, float sample);

float lpfrc_apply(float last, float in, float k);

