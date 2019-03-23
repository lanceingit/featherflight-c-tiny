#include "board.h"
#include "mathlib.h"
#include <math.h>

float sin_f(float x)
{
#define sinPolyCoef3 -1.666665710e-1f                                          // Double: -1.666665709650470145824129400050267289858e-1
#define sinPolyCoef5  8.333017292e-3f                                          // Double:  8.333017291562218127986291618761571373087e-3
#define sinPolyCoef7 -1.980661520e-4f                                          // Double: -1.980661520135080504411629636078917643846e-4
#define sinPolyCoef9  2.600054768e-6f                                          // Double:  2.600054767890361277123254766503271638682e-6
    
    int32_t xint = x;
    if (xint < -32 || xint > 32) return 0.0f;                               // Stop here on error input (5 * 360 Deg)
    while (x >  M_PI_F) x -= (2.0f * M_PI_F);                                 // always wrap input angle to -PI..PI
    while (x < -M_PI_F) x += (2.0f * M_PI_F);
    if (x >  (0.5f * M_PI_F)) x =  (0.5f * M_PI_F) - (x - (0.5f * M_PI_F));   // We just pick -90..+90 Degree
    else if (x < -(0.5f * M_PI_F)) x = -(0.5f * M_PI_F) - ((0.5f * M_PI_F) + x);
    float x2 = x * x;
    return x + x * x2 * (sinPolyCoef3 + x2 * (sinPolyCoef5 + x2 * (sinPolyCoef7 + x2 * sinPolyCoef9)));
}

float cos_f(float x)
{
    return sin_f(x + (0.5f * M_PI_F));
}

float atan2_f(float y, float x)
{
    #define atanPolyCoef1  3.14551665884836e-07f
    #define atanPolyCoef2  0.99997356613987f
    #define atanPolyCoef3  0.14744007058297684f
    #define atanPolyCoef4  0.3099814292351353f
    #define atanPolyCoef5  0.05030176425872175f
    #define atanPolyCoef6  0.1471039133652469f
    #define atanPolyCoef7  0.6444640676891548f

    float res, absX, absY;
    absX = fabsf(x);
    absY = fabsf(y);
    res  = MAX(absX, absY);
    if (res) res = MIN(absX, absY) / res;
    else res = 0.0f;
    res = -((((atanPolyCoef5 * res - atanPolyCoef4) * res - atanPolyCoef3) * res - atanPolyCoef2) * res - atanPolyCoef1) / ((atanPolyCoef7 * res + atanPolyCoef6) * res + 1.0f);
    if (absY > absX) res = (M_PI_F / 2.0f) - res;
    if (x < 0) res = M_PI_F - res;
    if (y < 0) res = -res;
    return res;
}

// http://http.developer.nvidia.com/Cg/acos.html
// Handbook of Mathematical Functions
// M. Abramowitz and I.A. Stegun, Ed.
// acos_approx maximum absolute error = 6.760856e-05 rads (3.873685e-03 degree)
float acos_f(float x)
{
    float xa = fabsf(x);
    float result = sqrtf(1.0f - xa) * (1.5707288f + xa * (-0.2121144f + xa * (0.0742610f + (-0.0187293f * xa))));
    if (x < 0.0f)
        return M_PI_F - result;
    else
        return result;
}

float powerf(float base, int exp) 
{
    float result = base;
    for (int count = 1; count < exp; count++) result *= base;

    return result;
}

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
	return 44330 * (1 - powerf(((float)p / (float)1013.25),(1/5.255)));  //FIXME:
}

void variance_create(Variance* self, uint8_t size)
{
	self->size = size;
	for(uint8_t i=0; i<self->size; i++) {
		self->data[i] = 0.0f;
		self->data_sq[i] = 0.0f;
	}
	fifo_f_create(&self->fifo, self->data, self->size);
	fifo_f_create(&self->fifo_sq, self->data_sq, self->size);
}

float variance_cal(Variance* self, float val)
{
	float tmp;
	float avg;
	float sq;
	float s2;
	float s3;
	float variance = 0.0f;

	sq = POW2(val);
	if(fifo_f_get_count(&self->fifo) < self->size-1) {
		self->sum += val;
		self->sum_sq += sq;
	} else {
		fifo_f_read(&self->fifo, &tmp);
		self->sum = self->sum - tmp + val;
		avg = self->sum / self->size;

		fifo_f_read(&self->fifo_sq, &tmp);
		self->sum_sq = self->sum_sq - tmp + sq;
		s2 = 2 * avg * self->sum;
		s3 = self->size * POW2(avg);
		variance = (self->sum_sq - s2 + s3)/self->size; 
	}
	fifo_f_write_force(&self->fifo, val);
	fifo_f_write_force(&self->fifo_sq, sq);

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

