#pragma once

#include <stdint.h>
#include "vector.h"

#define MAT(m, r, c) (m.data[r*m.column + c])
#define M_DIAG(m, i) (m.data[i*m.column + i])

typedef struct 
{
    uint8_t row;
    uint8_t column;
    float* data;
} Matrix;

void matrix_init(Matrix* m, uint8_t row, uint8_t column);
float matrix_item(Matrix m, uint8_t row, uint8_t column);
void matrix_separate(Matrix* m);
void matrix_destroy(Matrix* m);
float matrix_trace(Matrix m);
Matrix matrix_mul(Matrix m1, Matrix m2);
Matrix matrix_scalar(Matrix m, float s);
Matrix matrix_add(Matrix m1, Matrix m2);
Matrix matrix_sub(Matrix m1, Matrix m2);
Matrix matrix_transpose(Matrix m);
void matrix_set(Matrix m, float val);
void matrix_set_row(Matrix m, uint8_t r, Vector v);
int8_t matrix_qr(Matrix m, Matrix* q, Matrix* r);
void matrix_div_qr(Matrix *X, Matrix *A, Matrix *B, Matrix *Q, Matrix *R, Matrix *AQ);
