/* include/vector.h */

#ifndef VECTOR_H
#define VECTOR_H

#include "complex.h"

void vec_zero(Complex *v, int n);
void vec_copy(Complex *dst, const Complex *src, int n);
void vec_axpy_complex(Complex *y, Complex alpha, const Complex *x, int n);
void vec_scale_add(Complex *out, const Complex *a, Complex beta, const Complex *b, int n);
Complex vec_dot_hermitian(const Complex *a, const Complex *b, int n);
float vec_norm2(const Complex *v, int n);

#endif /* VECTOR_H */

