#pragma once

#include "matrix.h"
#include "dcm.h"

typedef struct
{
    float w;
    float x;
    float y;
    float z;
} Quaternion;

Quaternion quaternion_set(float w, float x, float y, float z);
Quaternion quaternion_add(Quaternion q1, Quaternion q2);
Quaternion quaternion_mul(Quaternion q1, Quaternion q2);
Quaternion quaternion_scaler(Quaternion q, float s);
Quaternion quaternion_from_yaw(float yaw);
Quaternion quaternion_from_matrix(Matrix m);
Quaternion quaternion_from_dcm(Dcm m);
Vector quaternion_to_euler(Quaternion q);
void quaternion_to_dcm(Quaternion q, Dcm m);
float quaternion_length(Quaternion q);
Quaternion quaternion_normalize(Quaternion q);
Vector quaternion_conjugate(Quaternion q, Vector v);
Vector quaternion_conjugate_inversed(Quaternion q, Vector v);
Quaternion quaternion_derivative(Quaternion q, Vector v);
