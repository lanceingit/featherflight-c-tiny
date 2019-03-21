#include "matrix.h"
#include <stdlib.h>
#include <string.h>
#include "mathlib.h"
#include <math.h>
#include "mm.h"

static float tmp_buf[30*30];
static Matrix tmp0 = {30,30,tmp_buf};

void matrix_init(Matrix* m, uint8_t row, uint8_t column)
{
    m->row = row;
    m->column = column;
}

float matrix_item(Matrix m, uint8_t row, uint8_t column)
{
    return m.data[row*m.row + column];
}

void matrix_separate(Matrix* m)
{
    uint16_t len = m->row*m->column*sizeof(float);
    float* buf = mm_malloc(len);       
    if(buf != NULL) {
        memcpy(buf, m->data, len);
        m->data = buf;
    }
}

void matrix_destroy(Matrix* m)
{
    mm_free(m->data);
    m->data = NULL;
}

float matrix_trace(Matrix m)
{
    if(!(m.row==m.column)) return 0;

    float res=0;
    for(uint8_t i = 0; i < m.row; i++) {
        res += MAT(m, i, i);
    }
    return res;    
}

Matrix matrix_mul(Matrix m1, Matrix m2)
{
    if(!(m1.column==m2.row)) return tmp0;

    tmp0.row = m1.row;
    tmp0.column = m2.column;

    for (uint8_t i = 0; i < m1.row; i++) {
        for (uint8_t j = 0; j < m2.column; j++) {
            for (uint8_t k = 0; k < m1.column; k++) {
                MAT(tmp0, i, k) += MAT(m1, i, j) * MAT(m2, j, k);
            }
        }
    }        
    return tmp0;    
}

Matrix matrix_scalar(Matrix m, float s)
{
    tmp0.row = m.row;
    tmp0.column = m.column;

    for (uint8_t i = 0; i < m.row; i++) {
        for (uint8_t j = 0; j < m.column; j++) {
            MAT(tmp0, i, j) = MAT(m, i, j) * s;
        }
    }
    return tmp0;    
}

Matrix matrix_add(Matrix m1, Matrix m2)
{
    if(!(m1.row==m2.row && m1.column==m2.column)) return tmp0;

    tmp0.row = m1.row;
    tmp0.column = m1.column;

    for (uint8_t i = 0; i < m1.row; i++) {
        for (uint8_t j = 0; j < m1.column; j++) {
            MAT(tmp0, i, j) = MAT(m1, i, j) + MAT(m2, i, j);
        }
    }
    return tmp0;    
}

Matrix matrix_sub(Matrix m1, Matrix m2)
{
    if(!(m1.row==m2.row && m1.column==m2.column)) return tmp0;

    tmp0.row = m1.row;
    tmp0.column = m1.column;

    for (uint8_t i = 0; i < m1.row; i++) {
        for (uint8_t j = 0; j < m1.column; j++) {
            MAT(tmp0, i, j) = MAT(m1, i, j) - MAT(m2, i, j);
        }
    }
    return tmp0; 
}

Matrix matrix_transpose(Matrix m)
{
    for (uint8_t i = 0; i < m.row; i++) {
        for (uint8_t j = 0; j < m.column; j++) {
            MAT(tmp0, j, i) =  MAT(m, i, j);
        }
    }    
    return tmp0; 
}

void matrix_set(Matrix m, float val)
{
    for (uint8_t i = 0; i < m.row; i++) {
        for (uint8_t j = 0; j < m.column; j++) {
            MAT(m, i, j) = val;
        }
    }    
}

void matrix_set_row(Matrix m, uint8_t r, Vector v)
{
//    for (uint8_t i=0; i<m.column; i++) {
//        MAT(m, r, i) = row(i, 0);
//    }    
    MAT(m, r, 0) = v.x;
    MAT(m, r, 0) = v.y;
    MAT(m, r, 0) = v.z;    
}

int8_t matrix_qr(Matrix mt, Matrix* qm, Matrix* rm)
{
    uint8_t minor;
    uint8_t row, col;
    uint8_t m = mt.row;
    uint8_t n = mt.column;
    uint8_t min;

    // clear R
    matrix_set(*rm, 0);

    min = MIN(m, n);

    /*
    * The QR decomposition of a matrix A is calculated using Householder
    * reflectors by repeating the following operations to each minor
    * A(minor,minor) of A:
    */
    for (minor = 0; minor < min; minor++) {
	    float xNormSqr = 0.0f;
	    float a;

	    /*
	    * Let x be the first column of the minor, and a^2 = |x|^2.
	    * x will be in the positions A[minor][minor] through A[m][minor].
	    * The first column of the transformed minor will be (a,0,0,..)'
	    * The sign of a is chosen to be opposite to the sign of the first
	    * component of x. Let's find a:
	    */
	    for (row = minor; row < m; row++)
		    xNormSqr += mt.data[minor*m + row]*mt.data[minor*m + row];

	    a = sqrtf(xNormSqr);
	    if (mt.data[minor*m + minor] > 0.0f)
		    a = -a;

	    if (a != 0.0f) {
		    rm->data[minor*rm->column + minor] = a;

		    /*
		    * Calculate the normalized reflection vector v and transform
		    * the first column. We know the norm of v beforehand: v = x-ae
		    * so |v|^2 = <x-ae,x-ae> = <x,x>-2a<x,e>+a^2<e,e> =
		    * a^2+a^2-2a<x,e> = 2a*(a - <x,e>).
		    * Here <x, e> is now A[minor][minor].
		    * v = x-ae is stored in the column at A:
		    */
		    mt.data[minor*m + minor] -= a; // now |v|^2 = -2a*(A[minor][minor])

		    /*
		    * Transform the rest of the columns of the minor:
		    * They will be transformed by the matrix H = I-2vv'/|v|^2.
		    * If x is a column vector of the minor, then
		    * Hx = (I-2vv'/|v|^2)x = x-2vv'x/|v|^2 = x - 2<x,v>/|v|^2 v.
		    * Therefore the transformation is easily calculated by
		    * subtracting the column vector (2<x,v>/|v|^2)v from x.
		    *
		    * Let 2<x,v>/|v|^2 = alpha. From above we have
		    * |v|^2 = -2a*(A[minor][minor]), so
		    * alpha = -<x,v>/(a*A[minor][minor])
		    */
		    for (col = minor+1; col < n; col++) {
			    float alpha = 0.0f;

			    for (row = minor; row < m; row++)
				    alpha -= mt.data[col*m + row]*mt.data[minor*m + row];

			    alpha /= a*mt.data[minor*m + minor];

			    // Subtract the column vector alpha*v from x.
			    for (row = minor; row < m; row++)
				    mt.data[col*m + row] -= alpha*mt.data[minor*m + row];
		    }
	    } else {  // rank deficient
    		return -1;
        }
    }

    // Form the matrix R of the QR-decomposition.
    //      R is supposed to be m x n, but only calculate n x n
    // copy the upper triangle of A
    for (int8_t row = min-1; row >= 0; row--)
	    for (col = row+1; col < n; col++)
		    rm->data[row*rm->column + col] = mt.data[col*m + row];

    // Form the matrix Q of the QR-decomposition.
    //      Q is supposed to be m x m

    // only compute Q if requested
    if (qm) {
	    matrix_set(*qm, 0);

	    /*
	    * Q = Q1 Q2 ... Q_m, so Q is formed by first constructing Q_m and then
	    * applying the Householder transformations Q_(m-1),Q_(m-2),...,Q1 in
	    * succession to the result
	    */
	    for (minor = m-1; minor >= min; minor--)
		    qm->data[minor*m + minor] = 1.0f;

	    for (int8_t minor = min-1; minor >= 0; minor--) {
		    qm->data[minor * m + minor] = 1.0f;

		    if (mt.data[minor*m + minor] != 0.0f) {
			    for (col = minor; col < m; col++) {
				    float alpha = 0.0f;

				    for (row = minor; row < m; row++)
					    alpha -= qm->data[row*m + col]*mt.data[minor*m + row];

				    alpha /= rm->data[minor*rm->column + minor]*mt.data[minor*m + minor];

				    for (row = minor; row < m; row++)
					    qm->data[row*m + col] -= alpha*mt.data[minor*m + row];
			    }
		    }
	    }
    }

    return 0;    
}

void matrix_div_qr(Matrix *X, Matrix *A, Matrix *B, Matrix *Q, Matrix *R, Matrix *AQ)
{
    uint8_t m, n;

    // this is messy (going into a class's private data structure),
    // but it is better than malloc/free
    Q->row = B->row;
    Q->column = B->row;
    R->row = B->row;
    R->column = B->column;
    AQ->row = A->row;
    AQ->column = B->row;

    m = A->row;
    n = B->column;

    matrix_qr(*B, Q, R);
    *AQ = matrix_mul(*A, *Q);

    // solve for X by backsubstitution
    for (uint8_t i = 0; i < m; i++) {
        for (int8_t j = n-1; j >= 0; j--) {
            for (uint8_t k = j+1; k < n; k++)
                AQ->data[i*n + j] -= R->data[j*n + k] * X->data[i*n + k];
            X->data[i*n + j] = AQ->data[i*n + j] / R->data[j*n + j];
        }
    }
}
