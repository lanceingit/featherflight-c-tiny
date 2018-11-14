#include "matrix.h"

#if 1

// dynamics:
//
//	x(+) = A * x(-) + B * u(+)
//	y_i = C_i*x
//
// kalman filter
//
//	E[xx'] = P
//	E[uu'] = W
//	E[y_iy_i'] = R_i
//
//	prediction
//		x(+|-) = A*x(-|-) + B*u(+)
//		P(+|-) = A*P(-|-)*A' + B*W*B'
//
//	correction
//		x(+|+) =  x(+|-) + K_i * (y_i - H_i * x(+|-) )
//
//
// input:
//      ax, ay, az (acceleration NED)
//
// states:
//      px, py, pz , ( position NED, m)
//      vx, vy, vz ( vel NED, m/s),
//      bx, by, bz ( accel bias, m/s^2)
//
// measurements:
//
//      baro: pz
//
//      flow: vx, vy (flow is in body x, y frame)
//

#define BIAS_MAX	1e-1f

enum {X_x = 0, X_y, X_z, X_vx, X_vy, X_vz, X_bx, X_by, X_bz, n_x};
enum {U_ax = 0, U_ay, U_az, n_u};

static float acc[3];
static att_s att;

static float x[n_x];	// state vector
static float u[n_u];	// input vector
static float P[n_x][n_x];	// state covariance matrix

static float A[n_x][n_x];	// dynamics matrix
static float B[n_x][n_u];	// input matrix
static float R[n_u][n_u];	// input covariance
static float Q[n_x][n_x];	// process noise covariance


static void update_state_space(float r[3][3])
{
	A[X_vx, X_bx] = -r[0, 0];
	A[X_vx, X_by] = -r[0, 1];
	A[X_vx, X_bz] = -r[0, 2];

	A[X_vy, X_bx] = -r[1, 0];
	A[X_vy, X_by] = -r[1, 1];
	A[X_vy, X_bz] = -r[1, 2];

	A[X_vz, X_bx] = -r[2, 0];
	A[X_vz, X_by] = -r[2, 1];
	A[X_vz, X_bz] = -r[2, 2];
}

static void dynamics(float* out, float t, float* x, float* u)
{
	//_A * x + _B * u;
	float temp1[n_x][0];
	float temp2[n_x][0];
	MatMul(A, x, n_x, n_x, 0, temp1);
	MatMul(B, x, n_x, n_u, 0, temp2);

	MatAdd(temp1, temp2, n_x, 0, out);
}

static void predict(float dt)
{
	imu_get_acc_filted(acc);
	attitude_get(&att);
	bf_to_ef(att.r, acc, u);
	u[2] += CONSTANTS_ONE_G;

	update_state_space(att.r);

	// continuous time kalman filter prediction
	// integrate runge kutta 4th order
	// https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
	Matrix k1 = [n_x]
    k2[n_x], k3[n_x], k4[n_x];
	float temp1[n_x],temp2[n_x],temp3[n_x];
	k1 = dynamics(0, x, u);
    k2 = dynamics(dt/2, matrix_add(x, matrix_scalar(k1, dt/2)), _u);
    k3 = dynamics(dt/2, matrix_add(x, matrix_scalar(k1, dt/2)), _u);
	k4 = dynamics(dt, matrix_add(x, matrix_scalar(k3, dt)), _u);

	MatScalar(k2, dt/2, n_x, 0, temp1);
	MatAdd(x, temp1, n_x, 0, temp2)
	dynamics(k3, dt/2, temp2, u);

	MatScalar(k2, dt/2, n_x, 0, temp1);
	MatAdd(x, temp1, n_x, 0, temp2)
	dynamics(k4, dt, temp2, u);

	//(k1 + k2 * 2 + k3 * 2 + k4) * (dt / 6)
	MatScalar(k2, 2, n_x, 0, temp1);   //k2*2
	MatAdd(k1, temp1, n_x, 0, temp2);  //k1+k2*2
	MatScalar(k3, 2, n_x, 0, temp1);   //k3*2
	MatAdd(temp2, temp1, n_x, 0, temp3); //k1+k2*2+k3*2
	MatAdd(temp3, k4, n_x, 0, temp1);  //k1+k2*2+k3*2+k4
	MatScalar(temp1, dt/6, n_x, 0, temp2); //(k1+k2*2+k3*2+k4)*(dt/6)
	float dx[n_x];
	memcpy(dx, temp2, n_x);

	// saturate bias
	float bx = dx[X_bx] + x[X_bx];
	float by = dx[X_by] + x[X_by];
	float bz = dx[X_bz] + x[X_bz];

	if (fabs(bx) > BIAS_MAX) {
		bx = BIAS_MAX * bx / fabs(bx);
		dx[X_bx] = bx - x[X_bx];
	}

	if (fabs(by) > BIAS_MAX) {
		by = BIAS_MAX * by / fabs(by);
		dx[X_by] = by - x[X_by];
	}

	if (fabs(bz) > BIAS_MAX) {
		bz = BIAS_MAX * bz / fabs(bz);
		dx[X_bz] = bz - x[X_bz];
	}

	// propagate
	MatAdd(x, dx, n_x, 0, temp1);
	memcpy(x, temp1, n_x);
	Matrix<float, n_x, n_x> dP = (A * P + P * A.transpose() + B * R * B.transpose() + Q) * dt;
}


void lpe_update(float dt)
{
	predict(dt);

	baroCorrect();

	flowCorrect();
}



#endif
