#include <math.h>
#include <stdint.h>
#include "vector.h"
#include "mathlib.h"
#include "debug.h"

static Vector tmp0;


Vector vector_set(float x, float y, float z)
{
    tmp0.x = x;
    tmp0.y = y;
    tmp0.z = z; 

    return tmp0;
}

Vector vector_cross(Vector v1,  Vector v2)
{
    tmp0.x =  v1.y*v2.z - v1.z*v2.y;
    tmp0.y = -v1.x*v2.z + v1.z*v2.x;
    tmp0.z =  v1.x*v2.y - v1.y*v2.x;    

    return tmp0;
}

float vector_scalar(Vector v1,  Vector v2)
{
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

Vector vector_add(Vector v1,  Vector v2)
{
    tmp0.x = v1.x + v2.x;
    tmp0.y = v1.y + v2.y;
    tmp0.z = v1.z + v2.z;

    return tmp0;
}

Vector vector_sub(Vector v1,  Vector v2)
{
    tmp0.x = v1.x - v2.x;
    tmp0.y = v1.y - v2.y;
    tmp0.z = v1.z - v2.z;    

    return tmp0;
}

Vector vector_mul(Vector v, float s)
{
    tmp0.x = s * v.x;
    tmp0.y = s * v.y;
    tmp0.z = s * v.z;

    return tmp0;
}

float vector_length(Vector v)
{
    return (sqrtf(v.x*v.x + v.y*v.y + v.z*v.z));
}

Vector vector_normalized(Vector v)
{
    float length = vector_length(v);

    tmp0.x = v.x / length;
    tmp0.y = v.y / length;
    tmp0.z = v.z / length;  

    return tmp0;
} 

Vector vector_normalized_fast(Vector v)
{
    float norm = inv_sqrt(vector_length(v));

    tmp0.x = v.x * norm;
    tmp0.y = v.y * norm;
    tmp0.z = v.z * norm;  

    return tmp0;
} 

Vector vector_reverse(Vector v)
{
    tmp0.x = -v.x;
    tmp0.y = -v.y;
    tmp0.z = -v.z;

    return tmp0;
}

Vector vector_rotate(Vector v, Dcm d)
{
    uint8_t i;

    for(i = 0; i < 3; i++) {
        tmp0.x += d[0][i] * v.x;
    }
    for(i = 0; i < 3; i++) {
        tmp0.y += d[0][i] * v.y;
    }
    for(i = 0; i < 3; i++) {
        tmp0.z += d[0][i] * v.z;
    }
    return tmp0;
}

void vector_print(char* name, Vector v)
{
    PRINT("%s:x:%f y:%f z:%f\n", name, v.x, v.y, v.z);
}
