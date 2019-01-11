#pragma once

#include "fifo.h"
#include "dcm.h"
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"

#ifndef M_PI
#define M_PI			3.14159265
#endif

#define M_PI_F			3.14159265f
#define M_DEG_TO_RAD 	0.017453292519943295f
#define M_RAD_TO_DEG 	57.295779513082323f
#define M_PI_2_F		1.57079632f
#define M_TWOPI_F		6.28318531f

#define POW2(_x)		((_x) * (_x))
#define MAX(a,b)        (a>b? a:b)
#define MIN(a,b)        (a<b? a:b)

#define CONSTANTS_ONE_G     9.80665f;						// m/s^2



struct variance_s
{
	float sum;
	float sum_sq;
	uint8_t size;
	struct fifo_f_s fifo;
	float data[100];
	struct fifo_f_s fifo_sq;
	float data_sq[100];
};

float sin_f(float x);
float cos_f(float x);
float atan2_f(float y, float x);
float acos_f(float x);
float powerf(float base, int exp);
float inv_sqrt(float x);

float constrain(float val, float min_val, float max_val);
float wrap_pi(float bearing);
float press2alt(float p);

void variance_create(struct variance_s* v, uint8_t size);
float variance_cal(struct variance_s* v, float val);

Vector rotation_ef(Dcm r, Vector* b);
Vector rotation_bf(Dcm r, Vector* e);
