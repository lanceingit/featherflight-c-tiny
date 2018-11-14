#include "board.h"
#include "mathlib.h"
#include <math.h>


float inv_sqrt(float x)
{
    float x_half = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i>>1);
    y = *(float*)&i;
    y = y * (1.5f - (x_half * y * y));
    
    return y;
}

float constrain(float val, float min_val, float max_val)
{
	return (val < min_val) ? min_val : ((val > max_val) ? max_val : val);
}

float wrap_pi(float bearing)
{
	/* value is inf or NaN */
	if (!isfinite(bearing)) {
		return bearing;
	}

	int c = 0;

	while (bearing >= M_PI_F) {
		bearing -= M_TWOPI_F;

		if (c++ > 3) {
			return NAN;
		}
	}

	c = 0;

	while (bearing < -M_PI_F) {
		bearing += M_TWOPI_F;

		if (c++ > 3) {
			return NAN;
		}
	}

	return bearing;
}

float press2alt(float p)
{
	return 44330 * (1 - powf(((float)p / (float)1013.25),(1/5.255)));  //FIXME:
}

void variance_create(struct variance_s* v, uint8_t size)
{
	v->size = size;
	for(uint8_t i=0; i<v->size; i++) {
		v->data[i] = 0.0f;
		v->data_sq[i] = 0.0f;
	}
	fifo_f_create(&v->fifo, v->data, v->size);
	fifo_f_create(&v->fifo_sq, v->data_sq, v->size);
}

float variance_cal(struct variance_s* v, float val)
{
	float tmp;
	float avg;
	float sq;
	float s2;
	float s3;
	float variance = 0.0f;

	sq = POW2(val);
	if(fifo_f_get_count(&v->fifo) < v->size-1) {
		v->sum += val;
		v->sum_sq += sq;
	} else {
		fifo_f_read(&v->fifo, &tmp);
		v->sum = v->sum - tmp + val;
		avg = v->sum / v->size;

		fifo_f_read(&v->fifo_sq, &tmp);
		v->sum_sq = v->sum_sq - tmp + sq;
		s2 = 2 * avg * v->sum;
		s3 = v->size * POW2(avg);
		variance = (v->sum_sq - s2 + s3)/v->size; 
	}
	fifo_f_write_force(&v->fifo, val);
	fifo_f_write_force(&v->fifo_sq, sq);

	return variance;
}

Vector rotation_ef(Dcm r, Vector* b)
{
	Vector e;

	e.x = r[0][0]*b->x + r[0][1]*b->y + r[0][2]*b->z;
	e.y = r[1][0]*b->x + r[1][1]*b->y + r[1][2]*b->z;
	e.z = r[2][0]*b->x + r[2][1]*b->y + r[2][2]*b->z;

	return e;
}

Vector rotation_bf(Dcm r, Vector* e)
{
	Vector b;

	b.x = r[0][0]*e->x + r[1][0]*e->y + r[2][0]*e->z;
	b.y = r[0][1]*e->x + r[1][1]*e->y + r[2][1]*e->z;
	b.z = r[0][2]*e->x + r[1][2]*e->y + r[2][2]*e->z;

	return b;
}
