#include "board.h"
#include <math.h>

#include "att_est_q.h"
#include "timer.h"
#include "lpf.h"
#include "param.h"

#include "att_param.h"
#include "debug.h"


struct att_est_q_s att_est_q = {
	.heir = {
		.init = &att_est_q_init,
		.run = &att_est_q_run,
		.roll = 0.0f,
		.pitch = 0.0f,
		.yaw = 0.0f,
		.use_compass = false,
	},
};

static struct att_est_q_s* this=&att_est_q;


bool att_est_q_init(void)
{ 
	PRINT("att q init \n");
    PARAM_REGISTER(att)
	lpf2p_init(&this->acc_filter_x, 625.0f, 30.0f);
	lpf2p_init(&this->acc_filter_y, 625.0f, 30.0f);
	lpf2p_init(&this->acc_filter_z, 625.0f, 30.0f);
	lpf2p_init(&this->gyro_filter_x, 625.0f, 30.0f);
	lpf2p_init(&this->gyro_filter_y, 625.0f, 30.0f);
	lpf2p_init(&this->gyro_filter_z, 625.0f, 30.0f);
    this->mag_decl_auto = true;
    this->mag_decl = 0.0f;
	this->bias_max = PARAM_GET(ATT_BIAS_MAX);//10.05f;
	this->w_accel = PARAM_GET(ATT_W_ACCEL);//0.2f;
	this->w_mag = PARAM_GET(ATT_W_MAG);//0.1f;
	this->w_gyro_bias = PARAM_GET(ATT_W_GYRO_BIAS;);//0.1f;
	PRINT("w_gyro_bias=%f\n", (double)this->w_gyro_bias);
    
    
    this->heir.acc = imu->acc;

	vector_print("acc", this->heir.acc);	

	Vector k = vector_normalized(vector_reverse(this->heir.acc));

	if (vector_length(this->heir.acc) < 9 || vector_length(this->heir.acc) > 11) {
		PRINT("init: degenerate accel!\n");
		return false;
	}

	// 'i' is Earth X axis (North) unit vector in body frame, orthogonal with 'k'
    Vector i = {1, 0, 0};

	if(this->heir.use_compass)
    {
        this->heir.mag = compass->mag;
        //esprintf(buf, "mag0:%.3f 1:%.3f 2:%.3f", (double)_mag(0),(double)_mag(1),(double)_mag(2));
        //PRINT(buf);
        if (vector_length(this->heir.mag) < 0.01f) {
            PRINT("init: degenerate mag!\n");
        }
        
        i = vector_normalized(vector_sub(this->heir.mag, vector_mul(k, vector_scalar(this->heir.mag, k))));
    }

	// 'j' is Earth Y axis (East) unit vector in body frame, orthogonal with 'k' and 'i'
	Vector j = vector_cross(k, i);
    
	// Fill rotation matrix
	Dcm r;
	r[0][0] = i.x;
	r[0][1] = i.y;
	r[0][2] = i.z;
	r[1][0] = j.x;
	r[1][1] = j.y;
	r[1][2] = j.z;
	r[2][0] = k.x;
	r[2][1] = k.y;
	r[2][2] = k.z;

	// Convert to quaternion
	this->heir.q = quaternion_from_dcm(r);

	// Compensate for magnetic declination
	Quaternion decl_rotation = quaternion_from_yaw(this->mag_decl);
    this->heir.q = quaternion_normalize(quaternion_mul(decl_rotation, this->heir.q));

	if (isfinite(this->heir.q.w) && isfinite(this->heir.q.x) &&
			isfinite(this->heir.q.y) && isfinite(this->heir.q.z) &&
	    quaternion_length(this->heir.q) > 0.95f && quaternion_length(this->heir.q) < 1.05f) {

		this->heir.inited = true;
	} else {
		this->heir.inited = false;
		PRINT("q init definite\n");
	}

	return this->heir.inited;
}

bool att_est_q_run(float dt)
{
	if (!this->heir.inited) {
		return att_est_q_init();
	}

	// this->heir.acc.x = lpf2p_apply(&this->acc_filter_x, this->heir.acc.x);
	// this->heir.acc.y = lpf2p_apply(&this->acc_filter_y, this->heir.acc.y);
	// this->heir.acc.z = lpf2p_apply(&this->acc_filter_z, this->heir.acc.z);
	// this->heir.gyro.x = lpf2p_apply(&this->gyro_filter_x, this->heir.gyro.x);
	// this->heir.gyro.y = lpf2p_apply(&this->gyro_filter_y, this->heir.gyro.y);
	// this->heir.gyro.z = lpf2p_apply(&this->gyro_filter_z, this->heir.gyro.z);


	Quaternion q_last = this->heir.q;

	// Angular rate of correction
    this->heir.corr = vector_set(0,0,0);
	float spin_rate = vector_length(this->heir.gyro);

	if (this->heir.use_compass) {
		// Magnetometer correction
		// Project mag field vector to global frame and extract XY component
        this->mag_earth = quaternion_conjugate(this->heir.q, this->heir.mag);
		float mag_err = wrap_pi(atan2f(this->mag_earth.y, this->mag_earth.x) - this->mag_decl);

		// Project magnetometer correction to body frame
        this->heir.corr = vector_add(this->heir.corr, vector_mul(quaternion_conjugate_inversed(this->heir.q, vector_set(0.0f, 0.0f, -mag_err)), this->w_mag));
	}

	this->heir.q = quaternion_normalize(this->heir.q);

	// Accelerometer correction
	// Project 'k' unit vector of earth frame to body frame
	// Vector<3> k = _q.conjugate_inversed(Vector<3>(0.0f, 0.0f, 1.0f));
	// Optimized version with dropped zeros
	Vector k;
	k = vector_set(2.0f * (this->heir.q.x * this->heir.q.z - this->heir.q.w * this->heir.q.y),
		           2.0f * (this->heir.q.y * this->heir.q.z + this->heir.q.w * this->heir.q.x),
		           (this->heir.q.w * this->heir.q.w - this->heir.q.x * this->heir.q.x - this->heir.q.y * this->heir.q.y + this->heir.q.z * this->heir.q.z)
			   );

    this->heir.corr = vector_add(this->heir.corr, vector_mul(vector_cross(k, vector_normalized(this->heir.acc)), this->w_accel));
	//_corr_acc = corr;

	// Gyro bias estimation
	if (spin_rate < SPIN_RATE_LIMIT) {
        this->heir.gyro_bias = vector_add(this->heir.gyro_bias, vector_mul(this->heir.corr, (this->w_gyro_bias * dt)));

        this->heir.gyro_bias.x = constrain(this->heir.gyro_bias.x, -this->bias_max, this->bias_max);
        this->heir.gyro_bias.y = constrain(this->heir.gyro_bias.y, -this->bias_max, this->bias_max);
        this->heir.gyro_bias.z = constrain(this->heir.gyro_bias.z, -this->bias_max, this->bias_max);
	}

    this->rate = vector_add(this->heir.gyro, this->heir.gyro_bias);

	// Feed forward gyro
    this->heir.corr = vector_add(this->heir.corr, this->rate);

	// Apply correction to state
	this->heir.q = quaternion_normalize(quaternion_add(this->heir.q, quaternion_scaler(quaternion_derivative(this->heir.q, this->heir.corr), dt)));

	if (!(isfinite(this->heir.q.w) && isfinite(this->heir.q.x) &&
			isfinite(this->heir.q.y) && isfinite(this->heir.q.z))) {
		// Reset quaternion to last good state
		this->heir.q = q_last;
		this->rate = vector_set(0,0,0);
        this->heir.gyro_bias = vector_set(0,0,0);
		PRINT("q definite\n");
		return false;
	}

	return true;
}

