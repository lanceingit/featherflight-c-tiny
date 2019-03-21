#include <math.h>
#include "mathlib.h"
#include "lpf.h"

void lpf2p_set_cutoff_frequency(lpf2p_s* self, float sample_freq, float cutoff_freq)
{
	self->cutoff_freq = cutoff_freq;
    if (self->cutoff_freq <= 0.0f) {
        // no filtering
        return;
    }
    float fr = sample_freq/self->cutoff_freq;
    float ohm = tanf(M_PI_F/fr);
    float c = 1.0f+2.0f*cos_f(M_PI_F/4.0f)*ohm + ohm*ohm;
    self->b0 = ohm*ohm/c;
    self->b1 = 2.0f*self->b0;
    self->b2 = self->b0;
    self->a1 = 2.0f*(ohm*ohm-1.0f)/c;
    self->a2 = (1.0f-2.0f*cos_f(M_PI_F/4.0f)*ohm+ohm*ohm)/c;
}

void lpf2p_init(lpf2p_s* self, float sample_freq, float cutoff_freq)
{
	self->cutoff_freq=cutoff_freq;
	self->a1=0.0f;
	self->a2=0.0f;
	self->b0=0.0f;
	self->b1=0.0f;
	self->b2=0.0f;
	self->delay_element_1=0.0f;
	self->delay_element_2=0.0f;
        // set initial parameters
	lpf2p_set_cutoff_frequency(self, sample_freq, cutoff_freq);
}

float lpf2p_apply(lpf2p_s* self, float sample)
{
    if (self->cutoff_freq <= 0.0f) {
        // no filtering
        return sample;
    }

    // do the filtering
    float delay_element_0 = sample - self->delay_element_1 * self->a1 - self->delay_element_2 * self->a2;
    if (!isfinite(delay_element_0)) {
        // don't allow bad values to propagate via the filter
        delay_element_0 = sample;
    }
    float output = delay_element_0 * self->b0 + self->delay_element_1 * self->b1 + self->delay_element_2 * self->b2;
    
    self->delay_element_2 = self->delay_element_1;
    self->delay_element_1 = delay_element_0;

    // return the value.  Should be no need to check limits
    return output;
}

float lpf2p_reset(lpf2p_s* self, float sample) {
	float dval = sample / (self->b0 + self->b1 + self->b2);
	self->delay_element_1 = dval;
	self->delay_element_2 = dval;
    return lpf2p_apply(self, sample);
}


void lpf1p_init(lpf1p_s* self, float sample_freq, float cutoff_freq)
{
    self->k = sample_freq / (1.0f/(2.0f*M_PI_F*cutoff_freq) + sample_freq);   
}

float lpf1p_apply(lpf1p_s* self, float sample)
{
    self->state = self->state + self->k * (sample - self->state);
    return self->state;    
}

float lpfrc_apply(float last, float in, float k)
{
	return ((last * k) + (in * (1.0f - k)));
}

