/* src/cpu/vector.c */

#include <string.h>

#include "vector.h"

void vec_zero(Complex *v, int n)
{
    memset(v, 0, (size_t)n * sizeof(Complex));
}

void vec_copy(Complex *dst, const Complex *src, int n)
{
    memcpy(dst, src, (size_t)n * sizeof(Complex));
}

void vec_axpy_complex(Complex *y, Complex alpha, const Complex *x, int n)
{
    for (int i = 0; i < n; i++) {
        const float xr = x[i].real;
        const float xi = x[i].imaginary;
        y[i].real += alpha.real * xr - alpha.imaginary * xi;
        y[i].imaginary += alpha.real * xi + alpha.imaginary * xr;
    }
}

void vec_scale_add(Complex *out, const Complex *a, Complex beta, const Complex *b, int n)
{
    for (int i = 0; i < n; i++) {
        const float br = b[i].real;
        const float bi = b[i].imaginary;
        out[i].real = a[i].real + beta.real * br - beta.imaginary * bi;
        out[i].imaginary = a[i].imaginary + beta.real * bi + beta.imaginary * br;
    }
}

/* Returns sum(conj(a_i) * b_i). */
Complex vec_dot_hermitian(const Complex *a, const Complex *b, int n)
{
    Complex dot = {0.0f, 0.0f};
    for (int i = 0; i < n; i++) {
        dot.real += a[i].real * b[i].real + a[i].imaginary * b[i].imaginary;
        dot.imaginary += a[i].real * b[i].imaginary - a[i].imaginary * b[i].real;
    }
    return dot;
}

float vec_norm2(const Complex *v, int n)
{
    float norm2 = 0.0f;
    for (int i = 0; i < n; i++) {
        norm2 += v[i].real * v[i].real + v[i].imaginary * v[i].imaginary;
    }
    return norm2;
}

