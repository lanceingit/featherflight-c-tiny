#include <math.h>
#include "mathlib.h"
#include "lpf.h"

void lpf2p_set_cutoff_frequency(struct lpf2p_s* filter, float sample_freq, float cutoff_freq)
{
	filter->cutoff_freq = cutoff_freq;
    if (filter->cutoff_freq <= 0.0f) {
        // no filtering
        return;
    }
    float fr = sample_freq/filter->cutoff_freq;
    float ohm = tanf(M_PI_F/fr);
    float c = 1.0f+2.0f*cos_f(M_PI_F/4.0f)*ohm + ohm*ohm;
    filter->b0 = ohm*ohm/c;
    filter->b1 = 2.0f*filter->b0;
    filter->b2 = filter->b0;
    filter->a1 = 2.0f*(ohm*ohm-1.0f)/c;
    filter->a2 = (1.0f-2.0f*cos_f(M_PI_F/4.0f)*ohm+ohm*ohm)/c;
}

void lpf2p_init(struct lpf2p_s* filter, float sample_freq, float cutoff_freq)
{
	filter->cutoff_freq=cutoff_freq;
	filter->a1=0.0f;
	filter->a2=0.0f;
	filter->b0=0.0f;
	filter->b1=0.0f;
	filter->b2=0.0f;
	filter->delay_element_1=0.0f;
	filter->delay_element_2=0.0f;
        // set initial parameters
	lpf2p_set_cutoff_frequency(filter, sample_freq, cutoff_freq);
}

float lpf2p_apply(struct lpf2p_s* filter, float sample)
{
    if (filter->cutoff_freq <= 0.0f) {
        // no filtering
        return sample;
    }

    // do the filtering
    float delay_element_0 = sample - filter->delay_element_1 * filter->a1 - filter->delay_element_2 * filter->a2;
    if (!isfinite(delay_element_0)) {
        // don't allow bad values to propagate via the filter
        delay_element_0 = sample;
    }
    float output = delay_element_0 * filter->b0 + filter->delay_element_1 * filter->b1 + filter->delay_element_2 * filter->b2;
    
    filter->delay_element_2 = filter->delay_element_1;
    filter->delay_element_1 = delay_element_0;

    // return the value.  Should be no need to check limits
    return output;
}

float lpf2p_reset(struct lpf2p_s* filter, float sample) {
	float dval = sample / (filter->b0 + filter->b1 + filter->b2);
	filter->delay_element_1 = dval;
	filter->delay_element_2 = dval;
    return lpf2p_apply(filter, sample);
}


void lpf1p_init(struct lpf1p_s* filter, float sample_freq, float cutoff_freq)
{
    filter->k = sample_freq / (1.0f/(2.0f*M_PI_F*cutoff_freq) + sample_freq);   
}

float lpf1p_apply(struct lpf1p_s* filter, float sample)
{
    filter->state = filter->state + filter->k * (sample - filter->state);
    return filter->state;    
}

float lpfrc_apply(float last, float in, float k)
{
	return ((last * k) + (in * (1.0f - k)));
}

