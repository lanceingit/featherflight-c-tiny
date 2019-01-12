#include "board.h"

#include "att_est_cf.h"
#include "mathlib.h"
#include "timer.h"
#include <math.h>

#if 0
#define LINK_DEBUG(a) _link->send_text(a)
#else
#define LINK_DEBUG(a)
#endif

struct att_est_cf_s att_est_cf = {
	.heir = {
        .init = &att_est_cf_init,
        .run = &att_est_cf_run,
	},
};

static struct att_est_cf_s* this=&att_est_cf;

bool att_est_cf_init(void)
{
    this->kp = 0.4f;
    this->ki = 0.001f;
    this->heir.inited = true;
    this->heir.use_compass = false;

    return true;
}

bool att_est_cf_run(float dt)
{
    this->heir.spin_rate = vector_length(this->heir.gyro);

    if (this->heir.use_compass) {
        this->heir.mag = vector_normalized_fast(this->heir.mag);

        // For magnetometer correction we make an assumption that magnetic field is perpendicular to gravity (ignore Z-component in EF).
        // This way magnetic field will only affect heading and wont mess roll/pitch angles

        // (hx; hy; 0) - measured mag field vector in EF (assuming Z-component is zero)
        // (bx; 0; 0) - reference mag field vector heading due North in EF (assuming Z-component is zero)
        float hx = this->dcm[0][0] * this->heir.mag.x + this->dcm[0][1] * this->heir.mag.y + this->dcm[0][2] * this->heir.mag.z;
        float hy = this->dcm[1][0] * this->heir.mag.x + this->dcm[1][1] * this->heir.mag.y + this->dcm[1][2] * this->heir.mag.z;
        float bx = sqrtf(hx * hx + hy * hy);

        // magnetometer error is cross product between estimated magnetic north and measured magnetic north (calculated in EF)
        float ez_ef = -(hy * bx);

        // Rotate mag error vector back to BF and accumulate
        this->heir.corr.x += this->dcm[2][0] * ez_ef;
        this->heir.corr.y += this->dcm[2][1] * ez_ef;
        this->heir.corr.z += this->dcm[2][2] * ez_ef;        
    }

    // Normalise accelerometer measurement
    this->heir.acc = vector_normalized_fast(this->heir.acc);

    // Error is sum of cross product between estimated direction and measured direction of gravity
    this->heir.corr.x += (this->heir.acc.y * this->dcm[2][2] - this->heir.acc.z * this->dcm[2][1]);
    this->heir.corr.y += (this->heir.acc.z * this->dcm[2][0] - this->heir.acc.x * this->dcm[2][2]);
    this->heir.corr.z += (this->heir.acc.x * this->dcm[2][1] - this->heir.acc.y * this->dcm[2][0]);
        
    // Compute and apply integral feedback if enabled
    if (this->ki > 0.0f) {
        // Stop integrating if spinning beyond the certain limit
        if (this->heir.spin_rate < SPIN_RATE_LIMIT) {
            // integral error scaled by Ki
            this->heir.gyro_bias = vector_add(this->heir.gyro_bias, vector_mul(this->heir.corr, this->ki*dt));
        }
    } else {
        this->heir.gyro_bias = vector_zero();
    }        

    // Apply proportional and integral feedback
    this->heir.gyro = vector_add(this->heir.gyro, vector_add(vector_mul(this->heir.corr, this->kp), this->heir.gyro_bias)); 

    // Integrate rate of change of quaternion
    this->heir.q = quaternion_normalize(quaternion_add(this->heir.q, quaternion_scaler(quaternion_derivative(this->heir.q, this->heir.corr), dt)));
    
    return true;
}
