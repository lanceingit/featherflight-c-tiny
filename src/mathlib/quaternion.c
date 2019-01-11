#include "quaternion.h"
#include "vector.h"
#include <math.h>
#include "mathlib.h"


static Quaternion tmp0;

static Vector vtmp0;

Quaternion quaternion_set(float w, float x, float y, float z)
{
    tmp0.w = w;
    tmp0.x = x;
    tmp0.y = y;
    tmp0.z = z;

    return tmp0;
}

Quaternion quaternion_add(Quaternion q1, Quaternion q2)
{
    tmp0.w = q1.w + q2.w;
    tmp0.x = q1.x + q2.x;
    tmp0.y = q1.y + q2.y;
    tmp0.z = q1.z + q2.z;

    return tmp0;    
}

Quaternion quaternion_mul(Quaternion q1, Quaternion q2)
{
    tmp0.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    tmp0.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    tmp0.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    tmp0.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

    return tmp0;    
}

Quaternion quaternion_scaler(Quaternion q, float s)
{
    tmp0.w = q.w * s;
    tmp0.x = q.x * s;
    tmp0.y = q.y * s;
    tmp0.z = q.z * s;

    return tmp0;    
}

Quaternion quaternion_div(Quaternion q1, Quaternion q2)
{
    // float norm = q.length_squared();
    // return Quaternion(
    //         (  q.w * q.q.w + q.x * q.q.x + q.y * q.q.y + q.z * q.q.z) / norm,
    //         (- q.w * q.q.x + q.x * q.q.w - q.y * q.q.z + q.z * q.q.y) / norm,
    //         (- q.w * q.q.y + q.x * q.q.z + q.y * q.q.w - q.z * q.q.x) / norm,
    //         (- q.w * q.q.z - q.x * q.q.y + q.y * q.q.x + q.z * q.q.w) / norm
    // );
    
    return tmp0;
}

Quaternion quaternion_from_yaw(float yaw) 
{
    tmp0.w = cos_f(yaw / 2.0f);
    tmp0.x = 0.0f;
    tmp0.y = 0.0f;
    tmp0.z = sin_f(yaw / 2.0f);
    
    return tmp0;
}

Quaternion quaternion_from_matrix(Matrix m) 
{
    if(!(m.row == 4 && m.column == 1)) return tmp0;

    tmp0.w = MAT(m, 0, 0);
    tmp0.x = MAT(m, 1, 0);
    tmp0.y = MAT(m, 2, 0);
    tmp0.z = MAT(m, 3, 0);

    return tmp0;
}

Quaternion quaternion_from_dcm(Dcm m) 
{
    float t = dcm_trace(m);
    if (t > 0.0f) {
        t = sqrtf(1.0f + t);
        tmp0.w = 0.5f * t;
        t = 0.5f / t;
        tmp0.x = (m[2][1] - m[1][2]) * t;
        tmp0.y = (m[0][2] - m[2][0]) * t;
        tmp0.z = (m[1][0] - m[0][1]) * t;
    } else if (m[0][0] > m[1][1] && m[0][0] > m[2][2]) {
        t = sqrt(1.0f + m[0][0] - m[1][1]- m[2][2]);
        tmp0.x = 0.5f * t;
        t = 0.5f / t;
        tmp0.w = (m[2][1] - m[1][2]) * t;
        tmp0.y = (m[1][0] + m[0][1]) * t;
        tmp0.z = (m[0][2] + m[2][0]) * t;
    } else if (m[1][1] > m[2][2]) {
        t = sqrt(1.0f - m[0][0] + m[1][1] - m[2][2]);
        tmp0.y = 0.5f * t;
        t = 0.5f / t;
        tmp0.w = (m[0][2] - m[2][0]) * t;
        tmp0.x = (m[1][0] + m[0][1]) * t;
        tmp0.z = (m[2][1] + m[1][2]) * t;
    } else {
        t = sqrt(1.0f - m[0][0] - m[1][1] + m[2][2]);
        tmp0.z = 0.5f * t;
        t = 0.5f / t;
        tmp0.w = (m[1][0] - m[0][1]) * t;
        tmp0.x = (m[0][2] + m[2][0]) * t;
        tmp0.y = (m[2][1] + m[1][2]) * t;
    }

    return tmp0;
}

Vector quaternion_to_euler(Quaternion q)
{
    vtmp0.x = atan2_f(2.0f * (q.w * q.x + q.y * q.z), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));
    vtmp0.y = 0.5f * M_PI_F - acos_f(2.0f * (q.w * q.y - q.z * q.x));
    vtmp0.z = atan2_f(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));
    
    return vtmp0;
}

void quaternion_to_dcm(Quaternion q, Dcm m)
{
    m[0][0] = q.w*q.w + q.x*q.x - q.y*q.y - q.z*q.z;
    m[0][1] = 2 * (q.x*q.y - q.w*q.z);
    m[0][2] = 2 * (q.w*q.y + q.x*q.z);
    m[1][0] = 2 * (q.x*q.y + q.w*q.z);
    m[1][1] = q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z;
    m[1][2] = 2 * (q.y*q.z - q.w*q.x);
    m[2][0] = 2 * (q.x*q.z - q.w*q.y);
    m[2][1] = 2 * (q.w*q.x + q.y*q.z);
    m[2][2] = q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z;
}

float quaternion_length(Quaternion q)
{
    return (sqrtf(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z));
}

Quaternion quaternion_normalize(Quaternion q)
{
    float length = quaternion_length(q);

    tmp0.w = q.w / length;
    tmp0.x = q.x / length;
    tmp0.y = q.y / length;  
    tmp0.z = q.z / length;  

    return tmp0;
}

Vector quaternion_conjugate(Quaternion q, Vector v)
{
    float q0q0 = q.w * q.w;
    float q1q1 = q.x * q.x;
    float q2q2 = q.y * q.y;
    float q3q3 = q.z * q.z;

    vtmp0.x = v.x * (q0q0 + q1q1 - q2q2 - q3q3) +
              v.y * 2.0f * (q.x * q.y - q.w * q.z) +
              v.z * 2.0f * (q.w * q.y + q.x * q.z);

    vtmp0.y = v.x * 2.0f * (q.x * q.y + q.w * q.z) +
              v.y * (q0q0 - q1q1 + q2q2 - q3q3) +
              v.z * 2.0f * (q.y * q.z - q.w * q.x);

    vtmp0.z = v.x * 2.0f * (q.x * q.z - q.w * q.y) +
              v.y * 2.0f * (q.w * q.x + q.y * q.z) +
              v.z * (q0q0 - q1q1 - q2q2 + q3q3);

    return vtmp0;
}

Vector quaternion_conjugate_inversed(Quaternion q, Vector v)
{
    float q0q0 = q.w * q.w;
    float q1q1 = q.x * q.x;
    float q2q2 = q.y * q.y;
    float q3q3 = q.z * q.z;

    vtmp0.x =  v.x * (q0q0 + q1q1 - q2q2 - q3q3) +
               v.y * 2.0f * (q.x * q.y + q.w * q.z) +
               v.z * 2.0f * (q.x * q.z - q.w * q.y);
   
    vtmp0.y =  v.x * 2.0f * (q.x * q.y - q.w * q.z) +
               v.y * (q0q0 - q1q1 + q2q2 - q3q3) +
               v.z * 2.0f * (q.y * q.z + q.w * q.x);
   
    vtmp0.z =  v.x * 2.0f * (q.x * q.z + q.w * q.y) +
               v.y * 2.0f * (q.y * q.z - q.w * q.x) +
               v.z * (q0q0 - q1q1 - q2q2 + q3q3);

    return vtmp0;
}

Quaternion quaternion_derivative(Quaternion q, Vector v) 
{
    Quaternion V = {0.0f, v.x, v.y, v.z};

    tmp0 = quaternion_scaler(quaternion_mul(q, V), 0.5f);

    return tmp0;
}
