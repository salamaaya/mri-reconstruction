/* include/matrix.h */
#ifndef MATRIX_H
#define MATRIX_H

#include "complex.h"

typedef struct {
    int rows;
    int cols;
    Complex *data;
} Matrix;

Matrix* matrix_construct(int, int);
void    matrix_destroy(Matrix *);

Complex matrix_getat(Matrix *, int, int);
int     matrix_setat(Matrix *, Complex, int, int);

Matrix* matrix_add(Matrix *, Matrix *);
Matrix* matrix_mul(Matrix *, Matrix *);
Matrix* matrix_transpose(Matrix *);
Matrix* matrix_hermitian(Matrix *);

void    matrix_print(Matrix *);

#endif /* MATRIX_H */
