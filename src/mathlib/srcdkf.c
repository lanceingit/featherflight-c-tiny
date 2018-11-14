#include "srcdkf.h"
#include <math.h>
#include <stdlib.h>
#include "mathlib.h"
#include "matrix.h"

float *srcdkfGetState(struct srcdkf_s* f) 
{
    return f->x.data;
}

void srcdkfSetVariance(struct srcdkf_s *f, float *q, float *v, float *n, uint8_t nn) 
{
    uint8_t i;

	// state variance
	if (q)
		for (i = 0; i < f->S; i++)                      //TODO:use matrix lib to set diag
            M_DIAG(f->Sx, i) = sqrtf(fabsf(q[i]));    

	// process noise
	if (v)
		for (i = 0; i < f->V; i++)
            M_DIAG(f->Sv, i) = sqrtf(fabsf(q[i]));

	// observation noise
	if (n && nn) {
		// resize Sn
		f->Sn.row = nn;
		f->Sn.column = nn;

		for (i = 0; i < nn; i++)
            M_DIAG(f->Sn, i) = sqrtf(fabsf(q[i]));
	}
}

void srcdkfGetVariance(struct srcdkf_s *f, float *q) 
{
	// state variance
	if (q)
		for (uint8_t i = 0; i < f->S; i++) {
			q[i] = M_DIAG(f->Sx, i);
			q[i] = q[i]*q[i];
		}
}

// states, max observations, process noise, max observation noise
struct srcdkf_s *srcdkfInit(uint8_t s, uint8_t m, uint8_t v, uint8_t n, srcdkf_time_update_func *timeUpdate) 
{
	struct srcdkf_s *f;
	uint8_t maxN = MAX(v, n);

	f = (struct srcdkf_s *)malloc(sizeof(struct srcdkf_s));   //TODO:

	f->S = s;
	f->V = v;

	matrix_init(&f->Sx, s, s);
	matrix_init(&f->SxT, s, s);
	matrix_init(&f->Sv, v, v);
	matrix_init(&f->Sn, n, n);
	matrix_init(&f->x, s, 1);
	matrix_init(&f->Xa, s+maxN, 1+(s+maxN)*2);

	matrix_init(&f->qrTempS, s, (s+v)*2);
	matrix_init(&f->y, m, 1);
	matrix_init(&f->Y, m, 1+(s+n)*2);
	matrix_init(&f->qrTempM, m, (s+n)*2);
	matrix_init(&f->Sy, m, m);
	matrix_init(&f->SyT, m, m);
	matrix_init(&f->SyC, m, m);
	matrix_init(&f->Pxy, s, m);
	matrix_init(&f->C1, m, s);
	matrix_init(&f->C1T, s, m);
	matrix_init(&f->C2, m, n);
	matrix_init(&f->D, m, s+n);
	matrix_init(&f->K, s, m);
	matrix_init(&f->inov, m, 1);
	matrix_init(&f->xUpdate, s, 1);
	matrix_init(&f->qrFinal, s, 2*s + 2*n);
	matrix_init(&f->Q, s, s+n);	// scratch
	matrix_init(&f->R, n, n);	// scratch
	matrix_init(&f->AQ, s, n);	// scratch

	f->xOut = (float *)malloc(s * sizeof(float));
	f->xNoise = (float *)malloc(maxN * sizeof(float));
	f->xIn = (float *)malloc(s * sizeof(float));

	f->h = SRCDKF_H;
	f->hh = f->h*f->h;
//	f->w0m = (f->hh - (float)s) / f->hh;	// calculated in process
	f->wim = 1.0f / (2.0f * f->hh);
	f->wic1 = sqrtf(1.0f / (4.0f * f->hh));
	f->wic2 = sqrtf((f->hh - 1.0f) / (4.0f * f->hh*f->hh));

    f->timeUpdate = timeUpdate;

	return f;
}

// given noise matrix
static void srcdkfCalcSigmaPoints(struct srcdkf_s *f, Matrix *Sn) {
	uint8_t S = f->S;			// number of states
	uint8_t N = Sn->row;		// number of noise variables
	uint8_t A = S+N;			// number of agumented states
	uint8_t L = 1+A*2;			// number of sigma points
	int i, j;

	// set the number of sigma points
	f->L = L;

	// resize output matrix
	f->Xa.row = A;
	f->Xa.column = L;

	//	-	   -
	// Sa =	| Sx	0  |
	//	| 0	Sn |
	//	-	   -
	// xa = [ x 	0  ]
	// Xa = [ xa  (xa + h*Sa)  (xa - h*Sa) ]
	//
	for (i = 0; i < A; i++) {
		int rOffset = i*L;
		float base = (i < S) ? f->x.data[i] : 0.0f;

		f->Xa.data[rOffset + 0] = base;

		for (j = 1; j <= A; j++) {
			float t = 0.0f;

			if (i < S && j < S+1)
				t = f->Sx.data[i*S + (j-1)]*f->h;

			if (i >= S && j >= S+1)
				t = Sn->data[(i-S)*N + (j-S-1)]*f->h;

			f->Xa.data[rOffset + j]     = base + t;
			f->Xa.data[rOffset + j + A] = base - t;
		}
	}
}

void srcdkfTimeUpdate(struct srcdkf_s *f, float *u, float dt) {
	int S = f->S;			// number of states
	int V = f->V;			// number of noise variables
	int L;				// number of sigma points

//	float *xIn = f->xIn;	// callback buffer
//	float *xOut = f->xOut;	// callback buffer
//	float *xNoise = f->xNoise;	// callback buffer
	int i, j;

	srcdkfCalcSigmaPoints(f, &f->Sv);
	L = f->L;

	// Xa = f(Xx, Xv, u, dt)
//	for (i = 0; i < L; i++) {
//		for (j = 0; j < S; j++)
//			xIn[j] = Xa[j*L + i];
//
//		for (j = 0; j < V; j++)
//			xNoise[j] = Xa[(S+j)*L + i];
//
//		f->timeUpdate(xIn, xNoise, xOut, u, dt);
//
//		for (j = 0; j < S; j++)
//			Xa[j*L + i] = xOut[j];
//	}
	f->timeUpdate(f->Xa.data, &f->Xa.data[S*L], f->Xa.data, u, dt, L);

	// sum weighted resultant sigma points to create estimated state
	f->w0m = (f->hh - (float)(S+V)) / f->hh;
	for (i = 0; i < S; i++) {
		int rOffset = i*L;

		f->x.data[i] = f->Xa.data[rOffset + 0] * f->w0m;

		for (j = 1; j < L; j++)
			f->x.data[i] += f->Xa.data[rOffset + j] * f->wim;
	}

	// update state covariance
	for (i = 0; i < S; i++) {
		int rOffset = i*(S+V)*2;

		for (j = 0; j < S+V; j++) {
			f->qrTempS.data[rOffset + j] = (f->Xa.data[i*L + j + 1] - f->Xa.data[i*L + S+V + j + 1]) * f->wic1;
			f->qrTempS.data[rOffset + S+V + j] = (f->Xa.data[i*L + j + 1] + f->Xa.data[i*L + S+V + j + 1] - 2.0f*f->Xa.data[i*L + 0]) * f->wic2;
		}
	}

	matrix_qr(f->qrTempS, NULL, &f->SxT);   // with transposition 
	f->SxT = matrix_transpose(f->Sx);
}

void srcdkfMeasurementUpdate(struct srcdkf_s *f, float *u, float *ym, int M, int N, float *noise, srcdkf_measure_update_func *measurementUpdate) 
{
	int L;					// number of sigma points
	int i, j;

	// make measurement noise matrix if provided
	if (noise) {
		f->Sn.row = N;
		f->Sn.column = N;
		matrix_set(f->Sn, 0);
		for (i = 0; i < N; i++)
            M_DIAG(f->Sn, i) = sqrtf(fabsf(noise[i]));
	}

	// generate sigma points
	srcdkfCalcSigmaPoints(f, &f->Sn);
	L = f->L;

	// resize all N and M based storage as they can change each iteration
	f->y.row = M;
	f->Y.row = M;
	f->Y.column = L;
	f->qrTempM.row = M;
	f->qrTempM.column = (f->S+N)*2;
	f->Sy.row = M;
	f->Sy.column = M;
	f->SyT.row = M;
	f->SyT.column = M;
	f->SyC.row = M;
	f->SyC.column = M;
	f->Pxy.column = M;
	f->C1.row = M;
	f->C1T.column = M;
	f->C2.row = M;
	f->C2.column = N;
	f->D.row = M;
	f->D.column = f->S+N;
	f->K.column = M;
	f->inov.row = M;
	f->qrFinal.column = 2*f->S + 2*N;

	// Y = h(Xa, Xn)
	for (i = 0; i < L; i++) {
		for (j = 0; j < f->S; j++)
			f->xIn[j] = f->Xa.data[j*L + i];

		for (j = 0; j < N; j++)
			f->xNoise[j] = f->Xa.data[(f->S+j)*L + i];

		measurementUpdate(u, f->xIn, f->xNoise, f->xOut);

		for (j = 0; j < M; j++)
			f->Y.data[j*L + i] = f->xOut[j];
	}

	// sum weighted resultant sigma points to create estimated measurement
	f->w0m = (f->hh - (float)(f->S+N)) / f->hh;
	for (i = 0; i < M; i++) {
		int rOffset = i*L;

		f->y.data[i] = f->Y.data[rOffset + 0] * f->w0m;

		for (j = 1; j < L; j++)
			f->y.data[i] += f->Y.data[rOffset + j] * f->wim;
	}

	// calculate measurement covariance components
	for (i = 0; i < M; i++) {
		int rOffset = i*(f->S+N)*2;

		for (j = 0; j < f->S+N; j++) {
			float c, d;

			c = (f->Y.data[i*L + j + 1] - f->Y.data[i*L + f->S+N + j + 1]) * f->wic1;
			d = (f->Y.data[i*L + j + 1] + f->Y.data[i*L + f->S+N + j + 1] - 2.0f*f->Y.data[i*L]) * f->wic2;

			f->qrTempM.data[rOffset + j] = c;
			f->qrTempM.data[rOffset + f->S+N + j] = d;

			// save fragments for future operations
			if (j < f->S) {
				f->C1.data[i*f->S + j] = c;
				f->C1T.data[j*M + i] = c;
			}
			else {
				f->C2.data[i*N + (j-f->S)] = c;
			}
			f->D.data[i*(f->S+N) + j] = d;
		}
	}

    matrix_qr(f->qrTempM, NULL, &f->SyT);       // with transposition

    f->Sy = matrix_transpose(f->SyT);
    f->SyC = matrix_transpose(f->SyT);  // make copy as later Div is destructive

	// create Pxy
    f->Pxy = matrix_mul(f->Sx, f->C1T);

	// K = (Pxy / SyT) / Sy
	matrix_div_qr(&f->K, &f->Pxy, &f->SyT, &f->Q, &f->R, &f->AQ);

	matrix_div_qr(&f->K, &f->K, &f->Sy, &f->Q, &f->R, &f->AQ);

	// x = x + k(ym - y)
	for (i = 0; i < M; i++)
		f->inov.data[i] = ym[i] - f->y.data[i];
    f->xUpdate = matrix_mul(f->K, f->inov);

	for (i = 0; i < f->S; i++)
		f->x.data[i] += f->xUpdate.data[i];

	// build final QR matrix
	//	rows = s
	//	cols = s + n + s + n
	//	use Q as temporary result storage

	f->Q.row = f->S;
	f->Q.column = f->S;
    f->Q = matrix_mul(f->K, f->C1);
	for (i = 0; i < f->S; i++) {
		int rOffset = i*(2*f->S + 2*N);

		for (j = 0; j < f->S; j++)
			f->qrFinal.data[rOffset + j] = f->Sx.data[i*f->S + j] - f->Q.data[i*f->S + j];
	}

	f->Q.row = f->S;
	f->Q.column = N;
    f->Q = matrix_mul(f->K, f->C2);
	for (i = 0; i < f->S; i++) {
		int rOffset = i*(2*f->S + 2*N);

		for (j = 0; j < N; j++)
			f->qrFinal.data[rOffset + f->S+j] = f->Q.data[i*N + j];
	}

	f->Q.row = f->S;
	f->Q.column = f->S+N;
    f->Q = matrix_mul(f->K, f->D);
	for (i = 0; i < f->S; i++) {
		int rOffset = i*(2*f->S + 2*N);

		for (j = 0; j < f->S+N; j++)
			f->qrFinal.data[rOffset + f->S+N+j] = f->Q.data[i*(f->S+N) + j];
	}

	// Sx = qr([Sx-K*C1 K*C2 K*D]')
	// this method is not susceptable to numeric instability like the Cholesky is
    matrix_qr(f->qrFinal, NULL, &f->SxT);	// with transposition
    f->SxT = matrix_transpose(f->Sx);
}

void paramsrcdkfSetVariance(struct srcdkf_s *f, float *v, float *n) 
{
	srcdkfSetVariance(f, v, v, n, f->N);

	for (uint8_t i = 0; i < f->S; i++)
		f->rDiag.data[i] = 0.0;
}

void paramsrcdkfGetVariance(struct srcdkf_s *f, float *v, float *n) 
{
	int i;

	// artificial parameter variance
	if (v) {
		for (i = 0; i < f->S; i++) {
			v[i] = f->Sx.data[i*f->S + i];
			v[i] = v[i]*v[i];
		}
    }

	if (n) {
		for (i = 0; i < f->N; i++) {
			n[i] = f->Sn.data[i*f->N + i];
			n[i] = n[i]*n[i];
		}
    }
}

void paramsrcdkfSetRM(struct srcdkf_s *f, float rm) 
{
	f->rm = rm;
}

// parameters, outputs, output noise, map func
struct srcdkf_s *paramsrcdkfInit(int w, int d, int n, srcdkf_measure_update_func *map) 
{
	struct srcdkf_s *f;

	f = srcdkfInit(w, d, w, n, NULL);

	f->M = d;
	f->N = n;
	f->map = map;
	f->rm = SRCDKF_RM;

	matrix_init(&f->KT, d, w);
	matrix_init(&f->inovT, 1, d);
	matrix_init(&f->rDiag, w, 1);

	return f;
}

void paramsrcdkfUpdate(struct srcdkf_s *f, float *u, float *d) 
{
	uint8_t S = f->S;
	uint8_t i;

	srcdkfMeasurementUpdate(f, u, d, f->M, f->N, 0, f->map);

	if (f->rm) {
		// Robbins-Monro innovation estimation for Rr
		// Rr = (1 - SRCDKF_RM)*Rr + SRCDKF_RM * K * [dk - g(xk, wk)] * [dk - g(xk, wk)]^T * K^T

        f->KT = matrix_transpose(f->K);
        f->inovT = matrix_transpose(f->inov);

		// xUpdate == K*inov
        f->K = matrix_mul(f->xUpdate, f->inovT);
        f->Sv = matrix_mul(f->K, f->KT);

		for (i = 0; i < S; i++) {
			f->rDiag.data[i] = (1.0f - f->rm)*f->rDiag.data[i] + f->rm * f->Sv.data[i*S + i];
			f->Sx.data[i*S + i] = sqrt(f->Sx.data[i*S + i] * f->Sx.data[i*S + i] + f->rDiag.data[i]);
		}
	}
}

