#pragma once

#include "matrix.h"

#define SRCDKF_H	(__sqrtf(3.0f) * 3.0f)
#define SRCDKF_RM	0.0001f		// Robbins-Monro stochastic term

typedef void srcdkf_time_update_func(float *x_I, float *noise_I, float *x_O, float *u, float dt, int n);
typedef void srcdkf_measure_update_func(float *u, float *x, float *noise_I, float *y);

// define all temporary storage here so that it does not need to be allocated each iteration
struct srcdkf_s
{
	uint8_t S;
	uint8_t V;
	uint8_t M;		// only used for parameter estimation
	uint8_t N;		// only used for parameter estimation
	uint8_t L;

	float h;
	float hh;
	float w0m, wim, wic1, wic2;
	float rm;

	Matrix Sx;	// state covariance
	Matrix SxT;	// Sx transposed
	Matrix Sv;	// process noise
	Matrix Sn;	// observation noise
	Matrix x;	// state estimate vector
	Matrix Xa;	// augmented sigma points
	float *xIn;
	float *xNoise;
	float *xOut;
	Matrix qrTempS;
	Matrix Y;	// resultant measurements from sigma points
	Matrix y;	// measurement estimate vector
	Matrix qrTempM;
	Matrix Sy;	// measurement covariance
	Matrix SyT;	// Sy transposed
	Matrix SyC;	// copy of Sy
	Matrix Pxy;
	Matrix C1;
	Matrix C1T;
	Matrix C2;
	Matrix D;
	Matrix K;
	Matrix KT;	// only used for param est
	Matrix inov;	// inovation
	Matrix inovT;// only used for param est
	Matrix xUpdate;
	Matrix qrFinal;
	Matrix rDiag;
	Matrix Q, R, AQ;	// scratch

	srcdkf_time_update_func *timeUpdate;
	srcdkf_measure_update_func *map;	// only used for param est
};

struct srcdkf_s *srcdkfInit(uint8_t s, uint8_t m, uint8_t v, uint8_t n, srcdkf_time_update_func *timeUpdate);
float *srcdkfGetState(struct srcdkf_s* f);
void srcdkfSetVariance(struct srcdkf_s *f, float *q, float *v, float *n, uint8_t nn);
void srcdkfGetVariance(struct srcdkf_s* f, float *q);
void srcdkfTimeUpdate(struct srcdkf_s* f, float *u, float dt);
void srcdkfMeasurementUpdate(struct srcdkf_s* f, float *u, float *y, int M, int N, float *noise, srcdkf_measure_update_func *measurementUpdate);
void srcdkfFree(struct srcdkf_s* f);
struct srcdkf_s *paramsrcdkfInit(int w, int d, int n, srcdkf_measure_update_func *map);
void paramsrcdkfUpdate(struct srcdkf_s* f, float *u, float *d);
void paramsrcdkfSetVariance(struct srcdkf_s* f, float *v, float *n);
void paramsrcdkfGetVariance(struct srcdkf_s* f, float *v, float *n);
void paramsrcdkfSetRM(struct srcdkf_s* f, float rm);

