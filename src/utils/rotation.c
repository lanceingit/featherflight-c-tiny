#include "rotation.h"

#define HALF_SQRT_2 0.70710678118654757f

#define USE_COMPLEX_ROT  0

void rotate_3f(rotation_e rot, float* x_in, float* y_in, float* z_in)
{
	float tmp;
	float x = *x_in;
	float y = *y_in;
	float z = *z_in;

	switch (rot) {
	case ROTATION_NONE:
	case ROTATION_MAX:
		break;

	case ROTATION_YAW_45: {
			tmp = HALF_SQRT_2 * (x - y);
			y   = HALF_SQRT_2 * (x + y);
			x = tmp;
			break;
		}

	case ROTATION_YAW_90: {
			tmp = x; x = -y; y = tmp;
			break;
		}

	case ROTATION_YAW_135: {
			tmp = -HALF_SQRT_2 * (x + y);
			y   =  HALF_SQRT_2 * (x - y);
			x = tmp;
			break;
		}

	case ROTATION_YAW_180:
		x = -x; y = -y;
		break;

	case ROTATION_YAW_225: {
			tmp = HALF_SQRT_2 * (y - x);
			y   = -HALF_SQRT_2 * (x + y);
			x = tmp;
			break;
		}

	case ROTATION_YAW_270: {
			tmp = x; x = y; y = -tmp;
			break;
		}

	case ROTATION_YAW_315: {
			tmp = HALF_SQRT_2 * (x + y);
			y   = HALF_SQRT_2 * (y - x);
			x = tmp;
			break;
		}

	case ROTATION_ROLL_180: {
			y = -y; z = -z;
			break;
		}

	case ROTATION_ROLL_180_YAW_90: {
			tmp = x; x = y; y = tmp; z = -z;
			break;
		}

	case ROTATION_PITCH_180: {
			x = -x; z = -z;
			break;
		}

	case ROTATION_ROLL_180_YAW_270: {
			tmp = x; x = -y; y = -tmp; z = -z;
			break;
		}

	case ROTATION_ROLL_90: {
			tmp = z; z = y; y = -tmp;
			break;
		}

	case ROTATION_ROLL_90_YAW_90: {
			tmp = z; z = y; y = -tmp;
			tmp = x; x = -y; y = tmp;
			break;
		}

	case ROTATION_ROLL_270: {
			tmp = z; z = -y; y = tmp;
			break;
		}

	case ROTATION_ROLL_270_YAW_90: {
			tmp = z; z = -y; y = tmp;
			tmp = x; x = -y; y = tmp;
			break;
		}

	case ROTATION_ROLL_270_YAW_270: {
			tmp = z; z = -y; y = tmp;
			tmp = x; x = y; y = -tmp;
			break;
		}

	case ROTATION_PITCH_90: {
			tmp = z; z = -x; x = tmp;
			break;
		}

	case ROTATION_PITCH_270: {
			tmp = z; z = x; x = -tmp;
			break;
		}

	case ROTATION_ROLL_180_PITCH_270: {
			tmp = z; z = x; x = tmp;
			y = -y;
			break;
		}

	case ROTATION_PITCH_90_YAW_180: {
			tmp = x; x = z; z = tmp;
			y = -y;
			break;
		}

	case ROTATION_PITCH_90_ROLL_90: {
			tmp = x; x = y;
			y = -z; z = -tmp;
			break;
		}

	case ROTATION_PITCH_90_ROLL_270: {
			tmp = x; x = -y;
			y = z; z = -tmp;
			break;
		}

	case ROTATION_PITCH_9_YAW_180: {
			float tmpx = x;
			float tmpy = y;
			float tmpz = z;
			x = -0.987688f * tmpx +  0.000000f * tmpy + -0.156434f * tmpz;
			y =  0.000000f * tmpx + -1.000000f * tmpy +  0.000000f * tmpz;
			z = -0.156434f * tmpx +  0.000000f * tmpy +  0.987688f * tmpz;
			break;
		}
    
#if USE_COMPLEX_ROT
	case ROTATION_ROLL_180_YAW_45: {
			tmp = HALF_SQRT_2 * (x + y);
			y   = HALF_SQRT_2 * (x - y);
			x = tmp; z = -z;
			break;
		}

	case ROTATION_ROLL_180_YAW_135: {
			tmp = HALF_SQRT_2 * (y - x);
			y   = HALF_SQRT_2 * (y + x);
			x = tmp; z = -z;
			break;
		}

	case ROTATION_ROLL_180_YAW_225: {
			tmp = -HALF_SQRT_2 * (x + y);
			y   =  HALF_SQRT_2 * (y - x);
			x = tmp; z = -z;
			break;
		}

	case ROTATION_ROLL_180_YAW_315: {
			tmp =  HALF_SQRT_2 * (x - y);
			y   = -HALF_SQRT_2 * (x + y);
			x = tmp; z = -z;
			break;
		}		
		
	case ROTATION_ROLL_90_YAW_45: {
			tmp = z; z = y; y = -tmp;
			tmp = HALF_SQRT_2 * (x - y);
			y   = HALF_SQRT_2 * (x + y);
			x = tmp;
			break;
		}

	case ROTATION_ROLL_90_YAW_135: {
			tmp = z; z = y; y = -tmp;
			tmp = -HALF_SQRT_2 * (x + y);
			y   =  HALF_SQRT_2 * (x - y);
			x = tmp;
			break;
		}		
		
	case ROTATION_ROLL_270_YAW_45: {
			tmp = z; z = -y; y = tmp;
			tmp = HALF_SQRT_2 * (x - y);
			y   = HALF_SQRT_2 * (x + y);
			x = tmp;
			break;
		}

	case ROTATION_ROLL_270_YAW_135: {
			tmp = z; z = -y; y = tmp;
			tmp = -HALF_SQRT_2 * (x + y);
			y   =  HALF_SQRT_2 * (x - y);
			x = tmp;
			break;
		}

	case ROTATION_YAW_293_PITCH_68_ROLL_90: {
			float tmpx = x;
			float tmpy = y;
			float tmpz = z;
			x =  0.143039f * tmpx +  0.368776f * tmpy + -0.918446f * tmpz;
			y = -0.332133f * tmpx + -0.856289f * tmpy + -0.395546f * tmpz;
			z = -0.932324f * tmpx +  0.361625f * tmpy +  0.000000f * tmpz;
			break;
		}		        
#endif //USE_COMPLEX_ROT        
        
	default:break;
	}
	*x_in = x;
	*y_in = y;
	*z_in = z;
}
