#include "board.h"
#include <math.h>
#include <float.h>
#include "mathlib.h"

static float mixer_out[4];
static float roll_control;
static float pitch_control;
static float yaw_control;
static float thrust_control;

void mixer_set_roll(float m)
{
    roll_control = m;
}

void mixer_set_pitch(float m)    
{
    pitch_control = m;
}

void mixer_set_yaw(float m)  
{
    yaw_control = m;
}

void mixer_set_thrust(float m) 
{
    thrust_control = m;
}

struct Rotor {
	float	roll_scale;		/**< scales roll for this rotor */
	float	pitch_scale;	/**< scales pitch for this rotor */
	float	yaw_scale;		/**< scales yaw for this rotor */
	float	thrust_scale;	/**< scales thrust for this rotor */
};

void mixer_mix(float roll, float pitch, float yaw, float thrust, float mixer_out[4])
{
	float		min_out = 1.0f;
	float		max_out = 0.0f;
	uint8_t     rotor_count = 4;
	struct Rotor rotors[4] = {
		{ 0.5f, -0.5f,  1.0f, 1.0f},
		{-0.5f, -0.5f, -1.0f, 1.0f},
		{-0.5f,  0.5f,  1.0f, 1.0f},
		{ 0.5f,  0.5f, -1.0f, 1.0f},
	};
	float outputs[4];
	float thrust_factor = 0.0f;

	/* perform initial mix pass yielding unbounded outputs, ignore yaw */
	for (uint8_t i = 0; i < rotor_count; i++) {
		float out = roll * rotors[i].roll_scale +
			    pitch * rotors[i].pitch_scale +
			    thrust * rotors[i].thrust_scale;

		/* calculate min and max output values */
		if (out < min_out) {
			min_out = out;
		}

		if (out > max_out) {
			max_out = out;
		}

		outputs[i] = out;
	}

	float boost = 0.0f;		// value added to demanded thrust (can also be negative)
	float roll_pitch_scale = 1.0f;	// scale for demanded roll and pitch
	float delta_out_max = max_out - min_out; // distance between the two extrema

	// If the difference between the to extrema is smaller than 1.0, the boost can safely unsaturate a motor if needed
	// without saturating another one.
	// Otherwise, a scaler is computed to make the distance between the two extrema exacly 1.0 and the boost
	// value is computed to maximize the roll-pitch control.
	//
	// Note: thrust boost is computed assuming thrust_scale==1 for all motors.
	// On asymmetric platforms, some motors have thrust_scale<1,
	// which may result in motor saturation after thrust boost is applied
	// TODO: revise the saturation/boosting strategy
	if (delta_out_max <= 1.0f) {
		if (min_out < 0.0f) {
			boost = -min_out;

		} else if (max_out > 1.0f) {
			boost = -(max_out - 1.0f);
		}

	} else {
		roll_pitch_scale = 1.0f / (delta_out_max);
		boost = 1.0f - ((max_out - thrust) * roll_pitch_scale + thrust);
	}

	// Thrust reduction is used to reduce the collective thrust if we hit
	// the upper throttle limit
	float thrust_reduction = 0.0f;

	// mix again but now with thrust boost, scale roll/pitch and also add yaw
	for (unsigned i = 0; i < rotor_count; i++) {
		float out = (roll * rotors[i].roll_scale +
			     pitch * rotors[i].pitch_scale) * roll_pitch_scale +
			    yaw * rotors[i].yaw_scale +
			    (thrust + boost) * rotors[i].thrust_scale;

		// scale yaw if it violates limits. inform about yaw limit reached
		if (out < 0.0f) {
			if (fabsf(rotors[i].yaw_scale) <= FLT_EPSILON) {
				yaw = 0.0f;

			} else {
				yaw = -((roll * rotors[i].roll_scale + pitch * rotors[i].pitch_scale) *
					roll_pitch_scale + thrust + boost) / rotors[i].yaw_scale;
			}

		} else if (out > 1.0f) {
			// allow to reduce thrust to get some yaw response
			float prop_reduction = MIN(0.15f, out - 1.0f);
			// keep the maximum requested reduction
			thrust_reduction = MAX(thrust_reduction, prop_reduction);

			if (fabsf(rotors[i].yaw_scale) <= FLT_EPSILON) {
				yaw = 0.0f;

			} else {
				yaw = (1.0f - ((roll * rotors[i].roll_scale + pitch * rotors[i].pitch_scale) *
					       roll_pitch_scale + (thrust - thrust_reduction) + boost)) / rotors[i].yaw_scale;
			}
		}
	}

	// Apply collective thrust reduction, the maximum for one prop
	thrust -= thrust_reduction;

	// add yaw and scale outputs to range idle_speed...1
	for (unsigned i = 0; i < rotor_count; i++) {
		outputs[i] = (roll * rotors[i].roll_scale +
			      pitch * rotors[i].pitch_scale) * roll_pitch_scale +
			     yaw * rotors[i].yaw_scale +
			     (thrust + boost) * rotors[i].thrust_scale;

		/*
			implement simple model for static relationship between applied motor pwm and motor thrust
			model: thrust = (1 - thrust_factor) * PWM + thrust_factor * PWM^2
			this model assumes normalized input / output in the range [0,1] so this is the right place
			to do it as at this stage the outputs are in that range.
		 */
		if (thrust_factor > 0.0f) {
			outputs[i] = -(1.0f - thrust_factor) / (2.0f * thrust_factor) + sqrtf((1.0f - thrust_factor) *
					(1.0f - thrust_factor) / (4.0f * thrust_factor * thrust_factor) + (outputs[i] < 0.0f ? 0.0f : outputs[i] /
							thrust_factor));
		}

		mixer_out[i] = outputs[i];
	}

    //for log
	// mixer_control_mix[0] = roll*roll_pitch_scale;
	// mixer_control_mix[1] = pitch*roll_pitch_scale;
	// mixer_control_mix[2] = yaw;
	// mixer_control_mix[3] = thrust + boost;
}

void mixer_update(float dt)
{
	
	mixer_mix(roll_control, pitch_control, yaw_control, thrust_control, mixer_out);
	
    //hal_set_motor(mixer_out,mixer_num);  //TODO:
}
