#include "board.h"

#include "alt_est_ukf.h"
#include "timer.h"
#include <math.h>

#define ALT_S           3   // states
#define ALT_M           2   // measurements
#define ALT_V           2   // process noise
#define ALT_N           2   // measurement noise

#define ALT_STATE_POS   0
#define ALT_STATE_VEL   1
#define ALT_STATE_BIAS  2

#define ALT_NOISE_BIAS  0
#define ALT_NOISE_VEL   1

#define ALT_POS_Q_NOISE  5.0f
#define ALT_VEL_Q_NOISE  1e-6f
#define ALT_ACC_Q_NOISE  0.05f

#define ALT_POS_M_NOISE  0.02f
#define ALT_VEL_M_NOISE  0.001f

#define ALT_BIAS_P_NOISE  5e-4f//5e-5f
#define ALT_VEL_P_NOISE   5e-4f

struct alt_est_ukf_s alt_est_ukf = {
	.heir = {
        .heir = {
            .init = &alt_est_ukf_init,
            .run = &alt_est_ukf_run,
        },
	},
};

static struct alt_est_ukf_s* this=&alt_est_ukf;

void altUkfTimeUpdate(float *in, float *noise, float *out, float *u, float dt, int n);

bool alt_est_ukf_init(void)
{
    float Q[ALT_S];		// state variance
    float V[ALT_V];		// process variance

    this->kf = srcdkfInit(ALT_S, ALT_M, ALT_V, ALT_N, altUkfTimeUpdate);

    this->x = srcdkfGetState(this->kf);

    Q[ALT_STATE_POS] = ALT_POS_Q_NOISE;
    Q[ALT_STATE_VEL] = ALT_VEL_Q_NOISE;
    Q[ALT_STATE_BIAS] = ALT_ACC_Q_NOISE;

    V[ALT_NOISE_BIAS] = ALT_BIAS_P_NOISE;
    V[ALT_NOISE_VEL] = ALT_VEL_P_NOISE;

    srcdkfSetVariance(this->kf, Q, V, 0, 0);

    this->x[ALT_STATE_POS] = baro_get_alt();
    this->x[ALT_STATE_VEL] = 0.0f;
    this->x[ALT_STATE_BIAS] = 0.0f;    


    this->heir.inited = true;

    return true;
}

bool alt_est_ukf_run(float dt)
{
	if(inertial_sensor_is_update()) {
		if(!att_valid())  return;

        inertial_sensor_get_acc(0, &this->acc_body);
        Dcm r;
        att_get_dcm(r);
        this->acc_body = vector_sub(this->acc_body, this->acc_bias);
        Vector acc_ned = vector_rotate(this->acc_body, r);
		this->heir.acc_ned_z = acc_ned.z + CONSTANTS_ONE_G;

		/******* px4 inav *********/
		this->heir.alt += this->heir.vel * dt + this->heir.acc_ned_z * dt * dt / 2.0f;
		this->heir.vel += this->heir.acc_ned_z * dt;
		baro_vel +=  acc_ned[2] * dt;
//		baro_corr = -inav_baro_get_baro_alt_f() - est_acc_z[0];
//		PRINT("get:t_est:%f\n", inav_baro_get_baro_alt_f(), time_buf_get(&est_time_buf[0], baro_delay));
//		baro_corr = -inav_baro_get_baro_alt_f() - time_buf_get(&est_time_buf[0], baro_delay);
		baro_corr = -baro_get_alt() - time_buf_get(&est_time_buf[0], baro_delay);
//		PRINT("corr:%f\n", baro_corr);
		acc_bias_corr[0] = 0.0f;
		acc_bias_corr[1] = 0.0f;
		acc_bias_corr[2] -= baro_corr * w_pos * w_pos;
		ef_to_bf(att.r, acc_bias, acc_bias_corr);
		acc_bias[0] += acc_bias[0] * w_bias * est_dt;
		acc_bias[1] += acc_bias[1] * w_bias * est_dt;
		acc_bias[2] += acc_bias[2] * w_bias * est_dt;
		est_acc_z[0] += baro_corr * w_pos;
		est_acc_z[1] += baro_corr * w_vel;
//		PRINT("put:t_est:%f\n", est_acc_z[0]);
		time_buf_put(&est_time_buf[0], est_acc_z[0]);
//		time_buf_put(&est_time_buf[1], est_acc_z[1]);
		/******* px4 inav *********/



		ukf_y[0] = baro_get_alt();
		ukf_y[1] = est_acc_z[1];
		altUkfProcess(est_dt, acc_ned_z_raw, ukf_y);
//		PRINT("ukf run time:%f\n", get_diff_time(&ukf_time, false));

		est_z[0] = -alt_ukf_get_pos();
		est_z[1] = alt_ukf_get_vel();
		acc_z_bias = alt_ukf_get_bias();
		acc_ned_z_raw += acc_z_bias;

		alt_ukf_get_variance(&baro_alt, &acc_vel, &baro_corr);


	}
}

void altUkfTimeUpdate(float *in, float *noise, float *out, float *u, float dt, int n) 
{
    float acc;
    int i;

    // assume out == in
    out = in;

    for (i = 0; i < n; i++) {
        acc = u[0] + in[ALT_STATE_BIAS*n + i];

        out[ALT_STATE_BIAS*n + i] = in[ALT_STATE_BIAS*n + i] + (noise[ALT_NOISE_BIAS*n + i] * dt);
        out[ALT_STATE_VEL*n + i] = in[ALT_STATE_VEL*n + i] + (acc * dt) + (noise[ALT_NOISE_VEL*n + i] * dt);
        out[ALT_STATE_POS*n + i] = in[ALT_STATE_POS*n + i] - (in[ALT_STATE_VEL*n + i] * dt) - (acc * dt * dt * 0.5f);
    }
}